#include "../Common.hlsli"
#include "../cbPerView.hlsli"
#include "../cbPerFrame.hlsli"
#include "../BRDFs/Lambert_CookTorrance.hlsli"
#include "../cbLights.hlsli"
#include "../ShadowMapping/cbShadowMap.hlsli"

//GBUFFERS
Texture2D<float3>          g_albedo              : register(srv_GB_ALBEDO);
Texture2D<float2>          g_roughMetal          : register(srv_GB_ROUGH_METAL);
Texture2D<float4>          g_normals             : register(srv_GB_NORMAL);
Texture2D<uint>            g_objectID            : register(srv_GB_OBJECTID);
Texture2D<float1>          g_depth               : register(srv_GB_DEPTH);
Texture2D<float3>          g_emission            : register(srv_GB_EMISSION);
//LIGHT MASKING 	       			             
Texture2D<float1>          g_spotMask            : register(srv_LIGHTMASK);
//SHADOW MAPPING	       				         
Texture2D<float1>          g_directionalSM       : register(srv_SM_DIRECT);
Texture2D<float1>          g_spotSM              : register(srv_SM_SPOT);
TextureCubeArray<float1>   g_pointSM             : register(srv_SM_POINT);
//ENVIRONMENTAL IBL
TextureCube<float3>        g_diffuseIrradiance   : register(srv_IBL_D_IRR);
TextureCube<float3>        g_specularIrradiance  : register(srv_IBL_S_IRR);
Texture2D<float2>          g_specularReflectance : register(srv_IBL_S_REF);


//MRP computing flags
static const uint NO_MRP = 0;
static const uint KARIS = 1;
static const uint CARPENTIER_ITERATION = 2;
static const uint CARPENTIER_NO_ITERATION = 4;

struct EnvironmentalIBL
{
	float3 diffuseIrr;
	float3 specularIrr;
	float2 reflLUT;
};

void addEnvironmentReflection(in View v, in Surface s, in EnvironmentalIBL ibl, inout float3 diffuse, inout float3 specular)
{
	diffuse += s.diffuseColor * ibl.diffuseIrr;

	float2 reflectanceLUT = ibl.reflLUT;

	float3 reflectance = (reflectanceLUT.r * s.F0 + reflectanceLUT.g);

	specular += reflectance * ibl.specularIrr;
}

void computeMRP(in Light light, in float3 normal, in View v, inout float NoL, out float NoH, out float VoH, out float HoL)
{
	float3 _L, _H;
	float _NoL = NoL, _NoH, _VoH, _HoL;

	if (g_MRP_Flags == KARIS)
	{
		bool intersects;
		_L = approximateClosestSphereDir(intersects, v.reflectionDir, light.cos, light.dir * light.distance, light.dir, light.distance, light.radius);
		_NoL = dot(normal, _L);

	}

	if (g_MRP_Flags == CARPENTIER_ITERATION || g_MRP_Flags == CARPENTIER_NO_ITERATION)//???? //don't know how to fix this implementation
	{
		//1- HoL = VoH, don't need to calculate that
		//2 - Why in carpentier method early return doesn't change NoL? Karis method returns reflectionDir, yields in different NoL
		//3 - If NoL < 0.0f , do I recalculate NoH and VoH?
		//4 - Carpentier method returns different NoL when RoS < 0.0f, which "leaks" light behind the light source. Why?

		//Attempt just using NoH and calculating left dot products with original L
		float _VoL = dot(v.viewDir, light.dir);
		float dummyNoL = _NoL;
		bool intersects;
		SphereMaxNoH(intersects, v.NoV, dummyNoL, _VoL, sqrt(1.0f - light.cos * light.cos), light.cos, g_MRP_Flags == CARPENTIER_ITERATION, _NoH, _VoH);

		_L = light.dir;


	}

	clampDirToHorizon(_L, _NoL, normal, MIN_NoV_CLAMP);
	_H = normalize(v.viewDir + _L);

	_HoL = _VoH = dot(_H, _L);

	if (g_MRP_Flags == KARIS)
	{
		_NoH = max(dot(normal, _H), MIN_NoV_CLAMP);
	}

	NoL = _NoL;
	NoH = _NoH;
	VoH = _VoH;
	HoL = _HoL;


}

//Computes light contribution for a specific surface and light
void resolveBRDFSphereLight(in Surface s, in View v, in Light light, in float NoL, in float visFactor, inout float3 diffuse, inout float3 specular)
{
	float3 lightRadiance = light.radiance * visFactor;

	float3 _diffuse, _specular;
	if (g_MRP_Flags == NO_MRP)
	{
		float3 L = light.dir;

		L_CT_BRDF(s, v, L, NoL, light.solidAngle, _diffuse, _specular);
	}
	else
	{
		float NoH, VoH, HoL;

		computeMRP(light, s.normal, v, NoL, NoH, VoH, HoL);
		L_CT_BRDF(NoL, v.NoV, NoH, HoL, s, light.solidAngle, _diffuse, _specular);
	}


	diffuse  = _diffuse  * lightRadiance;
	specular = _specular * lightRadiance;
}


//Resolves PBR opaque geometry for Lambert_CookTorrance_BRDF
float4 resolveLightning(in float2 pixel)
{
	Surface s;

	//---------------Load GBUFFER-----------------//
	uint objectID        = g_objectID.Load(uint3(pixel, 0));
	float  depth         = g_depth.Load(uint3(pixel, 0));
	float3 emission      = g_emission.Load(uint3(pixel, 0));

	//We have to unpack normals and reconstruct positions to compute View structure
	float3 macroNormal;
	{
		float4 normals = g_normals.Load(uint3(pixel, 0));
		s.normal    = unpackOctahedron(normals.xy);
		macroNormal = unpackOctahedron(normals.zw);
	}
	float metalness;
	{
		float2 rough_metal = g_roughMetal.Load(uint3(pixel, 0));
		s.roughnessLinear = rough_metal.x;
		metalness = rough_metal.y;

		float rough2 = s.roughnessLinear * s.roughnessLinear;
		s.roughness4 = max(rough2 * rough2, 0.001f);
	}

	{
		float3 albedo = g_albedo.Load(uint3(pixel, 0));

		s.diffuseColor = albedo * (1.0f - metalness);
		s.F0 = fresnel0(albedo, metalness);
	}
//---------------------------------------------------//

	float3 worldPos;
	float3 viewVector;

	float4 clip;
	float2 invBufferRes = g_resolution.zw;
	clip.x =   2.0f * (pixel.x * invBufferRes.x) - 1.0f;
	clip.y = - 2.0f * (pixel.y * invBufferRes.y) + 1.0f;
	clip.z = depth;
	clip.w = 1.0f;


//Reconstruct position and view vector using invVP matrix or invP for view space case
#ifdef ILLUMINATION_VIEW_SPACE

	clip = mul(clip, g_invP);
	clip /= clip.w;
	worldPos = clip.xyz;
	viewVector = normalize(- worldPos);

#elif CAMERA_CENTER_WS

	clip = mul(clip, g_invVP);
	clip /= clip.w;

	worldPos = clip.xyz - g_invV[3].xyz;
	viewVector = normalize(-worldPos);

#else

	clip = mul(clip, g_invVP);
	clip /= clip.w;

	worldPos = clip.xyz;
	viewVector = normalize(g_invV[3].xyz - worldPos);

#endif

	float NoV = dot(s.normal, viewVector);
	float3 reflectionVector = reflect(-viewVector, s.normal);
	View v = { viewVector, reflectionVector, NoV };


	//Start light accumulation
	float3 accumulateDiffuse = float3(0.0f, 0.0f, 0.0f);
	float3 accumulateSpecular = float3(0.0f, 0.0f, 0.0f);


	{
		float3 envDiffuse = 0.0f;
		float3 envSpec = 0.0f;

		float2 lutUV = float2(v.NoV, 1.0f - s.roughnessLinear);
		EnvironmentalIBL ibl;
		ibl.diffuseIrr  = g_diffuseIrradiance.Sample(g_trilinearClamp, s.normal).rgb;
		ibl.specularIrr = g_specularIrradiance.SampleLevel(g_trilinearClamp, v.reflectionDir, s.roughnessLinear * (g_reflectionMips - 1.0f));
		ibl.reflLUT     = g_specularReflectance.Sample(g_trilinearClamp, lutUV).rg;

		//Add environment contribution
		addEnvironmentReflection(v, s, ibl, envDiffuse, envSpec);

		accumulateDiffuse += g_enableIBL * envDiffuse;
		accumulateSpecular += g_enableIBL * envSpec;
	}

	uint lookupFrustumIndex = 0;

	{
		DirectionalSphereLight light = g_directionalLight;
		float3 L = -light.direction;
		float NoL = dot(s.normal, L);


		float visible = computeVisibilityDirectional(worldPos, g_directionalVP, macroNormal, L, lookupFrustumIndex, g_directionalSM);
		if (visible)
		{
			//Compute directional

			NoL = max(NoL, MIN_NoV_CLAMP);
			float solidAngle = light.solidAngle;
			float3 lightColor = light.luminance * visible;

			float3 diffuse, spec  = float3(0.0f, 0.0f, 0.0f);

			L_CT_BRDF(s, v, L, NoL, solidAngle, diffuse, spec);

			accumulateDiffuse  += diffuse * lightColor;
			accumulateSpecular += spec    * lightColor;

		}
	}

	++lookupFrustumIndex;

	{
		SpotSphereLight spotLight = g_spotLight;

		float NoL;
		Light light = sampleLight(spotLight, worldPos, s.normal, g_spotMask, NoL);


		float visibility = computeVisibilityPerspective(worldPos, g_spotVP, macroNormal, light.dir, lookupFrustumIndex, g_spotSM);
		float horizonFalloff = horizonFalloffFactor(macroNormal, light.radius, s.normal, light.dir * light.distance);

		float visFactor = visibility * horizonFalloff;
		bool lit = visFactor > 0.0f;

		if (lit)
		{
			float3 diffuse, spec = float3(0.0f, 0.0f, 0.0f);
			resolveBRDFSphereLight(s, v, light, NoL, visFactor, diffuse, spec);

			accumulateDiffuse += diffuse;
			accumulateSpecular += spec;
		}
	}

	++lookupFrustumIndex;

	for (uint pointIdx = 0; pointIdx < g_numPointLights; ++pointIdx)
	{
		PointSphereLight pointLight = g_pointLights[pointIdx];

		float NoL;
		Light light = sampleLight(pointLight, worldPos, s.normal, NoL);


		float visibility = computeVisibilityPerspective(worldPos, macroNormal, light.dir, lookupFrustumIndex, pointIdx, g_pointSM);
		float horizonFalloff = horizonFalloffFactor(macroNormal, light.radius, s.normal, light.dir * light.distance);

		float visFactor = visibility * horizonFalloff;
		bool lit = visFactor > 0.0f;

		if (lit)
		{
			float3 diffuse, spec = float3(0.0f, 0.0f, 0.0f);
			resolveBRDFSphereLight(s, v, light, NoL, visFactor, diffuse, spec);

			accumulateDiffuse  += diffuse;
			accumulateSpecular += spec;
		}
	}

	float3 finalColor = accumulateDiffuse * g_enableDiffuse + accumulateSpecular * g_enableSpecular + emission;

	return float4(finalColor, 1.0f);
}