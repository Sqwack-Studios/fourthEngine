#define PLASMA_PARTICLES
//#define COMBUSTIBLE_VORONOI

cbuffer PS_PER_FRAME_CONSTANT_BUFFER : register(b0)
{
	float4   g_resolution;
	float4   g_mousePos;
	float    g_time;
}

struct PSInput
{
	float4 posPix : SV_POSITION;
	float4 color : COLOR;
};

float noise(float2 co)
{
	return frac(sin(dot(co.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float3 firePalette(float i)
{

	float T = 1400.f + 1300.f * i; // Temperature range (in Kelvin).
	float3 L = float3(7.4f, 5.6f, 4.4f); // Red, green, blue wavelengths (in hundreds of nanometers).
	L = pow(L, float3(5.0f, 5.0f, 5.0f)) * (exp(1.43876719683e5 / (T * L)) - 1.f);
	return 1.f - exp(-5e8 / L); // Exposure level. Set to "50." For "70," change the "5" to a "7," etc.
}

// Hash function. This particular one probably doesn't disperse things quite as nicely as some 
// of the others around, but it's compact, and seems to work.
//
float3 hash33(float3 p) {

	float n = sin(dot(p, float3(7.f, 157.f, 113.f)));
	return frac(float3(2097152.f, 262144.f, 32768.f) * n);
}

float voronoi(float3 p) {

	float3 b, r, g = floor(p);
	p = frac(p); // "p -= g;" works on some GPUs, but not all, for some annoying reason.

	// Maximum value: I think outliers could get as high as "3," the squared diagonal length 
	// of the unit cube, with the mid point being "0.75." Is that right? Either way, for this 
	// example, the maximum is set to one, which would cover a good part of the range, whilst 
	// dispensing with the need to clamp the final result.
	float d = 1.;

	// I've unrolled one of the loops. GPU architecture is a mystery to me, but I'm aware 
	// they're not fond of nesting, branching, etc. My laptop GPU seems to hate everything, 
	// including multiple loops. If it were a person, we wouldn't hang out. 
	for (int j = -1; j <= 1; j++) {
		for (int i = -1; i <= 1; i++) {

			b = float3(i, j, -1);
			r = b - p + hash33(g + b);
			d = min(d, dot(r, r));

			b.z = 0.0;
			r = b - p + hash33(g + b);
			d = min(d, dot(r, r));

			b.z = 1.;
			r = b - p + hash33(g + b);
			d = min(d, dot(r, r));

		}
	}

	return d; // Range: [0, 1]
}

float noiseLayers(in float3 p) {

	// Normally, you'd just add a time vector to "p," and be done with 
	// it. However, in this instance, time is added seperately so that 
	// its frequency can be changed at a different rate. "p.z" is thrown 
	// in there just to distort things a little more.
	float3 t = float3(0.f, 0.f, p.z + g_time * 1.5f);

	const int iter = 5; // Just five layers is enough.
	float tot = 0.f, sum = 0.f, amp = 1.f; // Total, sum, amplitude.

	for (int i = 0; i < iter; i++) {
		tot += voronoi(p + t) * amp; // Add the layer to the total.
		p *= 2.f; // Position multiplied by two.
		t *= 1.5f; // Time multiplied by less than two.
		sum += amp; // Sum of amplitudes.
		amp *= .5f; // Decrease successive layer amplitude, as normal.
	}

	return tot / sum; // Range: [0, 1].
}

//source https://www.shadertoy.com/view/4tlSzl
float4 combustibleVoronoi(PSInput input)
{
	// Screen coordinates.
	float2 uv = (input.posPix.xy - g_resolution.xy * .5f) / g_resolution.y;

	// Shifting the central position around, just a little, to simulate a 
	// moving camera, albeit a pretty lame one.
	uv += float2(sin(g_time * .5f) * .25f, cos(g_time * .5f) * .125f);

	// Constructing the unit ray. 
	float3 rd = normalize(float3(uv.x, uv.y, 3.1415926535898f / 8.f));

	// Rotating the ray about the XY plane, to simulate a rolling camera.
	float cs = cos(g_time * .25f), si = sin(g_time * .25f);
	// Apparently "r *= rM" can break in some older browsers.
	rd.xy = mul(rd.xy, float2x2(cs, -si, si, cs));

	// Passing a unit ray multiple into the Voronoi layer function, which 
	// is nothing more than an fBm setup with some time dialation.
	float c = noiseLayers(rd * 2.);

	// Optional: Adding a bit of random noise for a subtle dust effect. 
	c = max(c + dot(hash33(rd) * 2.f - 1.f, float3(.015f, .015f, .015f)), 0.f);

	// Coloring:

	// Nebula.
	c *= sqrt(c) * 1.5; // Contrast.
	float3 col = firePalette(c); // Palettization.
	//col = mix(col, col.zyx*.1+ c*.9, clamp((1.+rd.x+rd.y)*0.45, 0., 1.)); // Color dispersion.
	col = lerp(col, col.zyx * .15f + c * .85f, min(pow(dot(rd.xy, rd.xy) * 1.2f, 1.5f), 1.f)); // Color dispersion.
	col = pow(col, float3(1.25f, 1.25f, 1.25f)); // Tweaking the contrast a little.

	// The fire palette on its own. Perhaps a little too much fire color.
	//c = pow(c*1.33, 1.25);
	//vec3 col =  firePalette(c);

	// Black and white, just to keep the art students happy. :)
	//c *= c*1.5;
	//vec3 col = vec3(c);

	// Rough gamma correction, and done.
	return float4(sqrt(clamp(col, 0.f, 1.f)), 1.f);
}



//source https://www.shadertoy.com/view/MlfcDN
float4 plasmaParticles(PSInput input)
{
	float2 uv = input.posPix.xy / g_resolution.xy;

	float u_brightness = 1.2f;
	float u_blobiness = 0.9f;
	float u_particles = 140.0f;
	float u_limit = 70.0f;
	float u_energy = 1.0 * 0.75f;


	float2 position = (input.posPix.xy / g_resolution.x);
	float t = g_time * u_energy;
	float a = 0.0;
	float b = 0.0;
	float c = 0.0;

	float2 pos;
	float2 center = float2(0.5f, 0.5f * (g_resolution.y / g_resolution.x));

	float na, nb, nc, nd, d;
	float limit = u_particles / u_limit;
	float step = 1.0f / u_particles;
	float n = 0.0f;

	for (float i = 0.0f; i <= 1.0f; i += 0.025f) {

		if (i <= limit) {

			float2 np = float2(n, 1.0f - 1.0f);

			na = noise(np * 1.1f);
			nb = noise(np * 2.8f);
			nc = noise(np * 0.7f);
			nd = noise(np * 3.2f);

			pos = center;
			pos.x += sin(t * na) * cos(t * nb) * tan(t * na * 0.15f) * 0.3f;
			pos.y += tan(t * nc) * sin(t * nd) * 0.1f;

			d = pow(1.6f * na / length(pos - position), u_blobiness);

			if (i < limit * 0.3333f) a += d;
			else if (i < limit * 0.5f) b += d;
			else c += d;


			n += step;
		}
	}

	float3 col = float3(a * 25.5f, 0.0f, a * b) * 0.0001f * u_brightness;

	return float4(col, 1.0f);
}


float4 main(PSInput input) : SV_Target0
{
#ifdef PLASMA_PARTICLES
	return plasmaParticles(input);
#endif

#ifdef  COMBUSTIBLE_VORONOI
	return combustibleVoronoi(input);
#else
	return input.color * 0.01f *  g_time;
#endif
}

