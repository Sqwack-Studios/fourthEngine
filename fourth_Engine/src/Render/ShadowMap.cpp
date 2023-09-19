#include "pch.h"

#pragma once
#include "include/Render/ShadowMap.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Managers/CameraManager.h"

namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;
	void ShadowMap::Init2D(float inDistance, float inMargin, float inResolution, float inNearPlane)
	{
		fth::TextureDsc txdsc;
		txdsc.height = txdsc.width = static_cast<uint32_t>(inResolution);
		txdsc.depth = 0;
		txdsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txdsc.format = DXGI_FORMAT_R24G8_TYPELESS;
		txdsc.numMips = 1;
		txdsc.numTextures = 1;
		txdsc.arraySize = 1;
		txdsc.isCubemap = false;
		txdsc.multisamples = 1;
		texture.Init(txdsc, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0);
		texture.MakeShaderResource(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

		shadow_distance = inDistance;
		margin = inMargin;
		resolution = inResolution;
		nearPlane = inNearPlane;

		depthTarget.CreateDepthStencilTarget(texture.GetResource(), DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);

	}

	void ShadowMap::InitCubemap(float inDistance, float inMargin, float inResolution, uint16_t numCubes, float inNearPlane)
	{
		fth::TextureDsc txdsc;
		txdsc.height = txdsc.width = static_cast<uint32_t>(inResolution);
		txdsc.depth = 0;
		txdsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txdsc.format = DXGI_FORMAT_R24G8_TYPELESS;
		txdsc.numMips = 1;
		txdsc.numTextures = numCubes;
		txdsc.arraySize = numCubes * 6;
		txdsc.isCubemap = true;
		txdsc.multisamples = 1;

		texture.Init(txdsc, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0);
		texture.MakeShaderResource(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

		shadow_distance = inDistance;
		margin = inMargin;
		resolution = inResolution;
		nearPlane = inNearPlane;

		depthTarget.CreateDepthStencilTarget(texture.GetResource(), DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0, 0, numCubes * 6);

	}
	math::Matrix ShadowMap::ComputeTightFrustumOrthogonal(const renderer::Camera& pov, const math::Vector3& direction, float& outNearTexelSize) const
	{
		//1-First, shrink far camera plane to fit shadow distance
		//2-Pick a light position in the center of shrinked camera frustum, oriented correctly
		//3-Build lightView using light position and direction
		//4-Z range is [- (ShadowDistance * 0.5 + ShadowMargin), -ShadowDistance * 0.5] because light is in the center
		//5-Calculate which corner of the camera frustum is further away. That distance is radius of a sphere that bounds whole frustum.
		//6-Create Orthogonal( radius, radius, Znear, Zfar)
		//7- multiply lightView * projection
		
		//Upgrades TODO: make every calculation in WS-centered, and dont rely in absolute WS positions
		//Camera FOV angle (degrees) is VERTICAL. Remember convert to radians.

		math::Frustum pov_frustum = pov.getFrustum();

		const renderer::ViewportSettings& vp = pov.GetViewportSettings();

		const float AR = vp.aspectRatio;
		const float fov_rad = DirectX::XMConvertToRadians(vp.fov);
		//Calculate far lenght, and push frustum corners along directions
		float sinHalf, cosHalf;
		DirectX::XMScalarSinCos(&sinHalf, &cosHalf, fov_rad * 0.5f);
		const float halfTan{ sinHalf / cosHalf };

		const float H = shadow_distance * std::sqrtf(1.0f + std::powf(halfTan, 2) * (1.0f  + AR * AR));
		//Remove camera position in camera_center_ws

		const math::Vector3& pov_Pos = pov.GetPosition();
		math::Vector3 newCenter = pov_Pos + shadow_distance * 0.5f * pov.GetForward();

		const float boxSize = H * 0.5f;


		//We will use this matrix to texel-snap center position XY coords in light view space
		math::Matrix lightView = math::Matrix::CreateLookAtLH(math::Vector3::Zero, direction, math::Vector3::Up);
		math::Matrix lightInvView;
		lightView.Invert(lightInvView);

		const float pixelsSize = boxSize / resolution;
		const float invPixelSize = 1.0f / pixelsSize;

		math::Vector3 offsetLS = math::Vector3::Transform(newCenter, lightView);

		offsetLS.x = std::floorf(offsetLS.x * invPixelSize) * pixelsSize;
		offsetLS.y = std::floorf(offsetLS.y * invPixelSize) * pixelsSize;

		//this is center position after snapping in WS
		const math::Vector3 offsetSnapped = math::Vector3::Transform(offsetLS, lightInvView);
		newCenter = offsetSnapped;

		//compute light position based on shadow, margin, light direction and center
		math::Vector3 lightPos = -(shadow_distance * 0.5f + margin) * direction + newCenter;

		//set new translation into invView matrix and compute VP
#ifdef CAMERA_CENTER_WS
		lightInvView.Translation(lightPos - pov_Pos);
#else
		lightInvView.Translation(lightPos);
#endif
		math::InvertOrthonormalMatrix(lightInvView, lightView);

		outNearTexelSize = pixelsSize;

		return lightView * math::Matrix::CreateOrthographicReversedLH(boxSize, boxSize, 0.01f, shadow_distance + margin);
	}

	math::Matrix ShadowMap::ComputeTightFrustumPerspective(const math::Vector3& pos, const math::Vector3& direction, float fov, float& outNearTexelSize) const
	{
		//Not necessary to fit tight light frustum
		//Let's stick for now to AspectRatio = 1

		constexpr float AR = 1.0f;

		outNearTexelSize = ComputeNearPlaneTexelSizeFor_FoVy(fov);
		 
		math::Vector3 target;
#ifdef CAMERA_CENTER_WS
		target = pos - CameraManager::GetActiveCamera().GetPosition();
#else
		target = pos;
#endif
		return math::Matrix::CreateLookAtLH(target, target + direction, math::Vector3::Up) * math::Matrix::CreatePerspectiveFieldOfViewReversedLH(fov, AR, nearPlane, shadow_distance);
	}

	const std::vector<math::Matrix>& ShadowMap::ComputeFrustumPerspectiveStream(const math::Vector3& pos) const
	{
		constexpr math::Matrix invViewBase[6] =
		{
			//X FACE
		    math::Matrix( 0.0f,  0.0f,  -1.0f,  0.0f,   0.0f,  1.0f,  0.0f,  0.0f,    1.0f,  0.0f,  0.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f),
			math::Matrix( 0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  1.0f,  0.0f,  0.0f,   -1.0f,  0.0f,  0.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f),
			//Y FACE
			math::Matrix( 1.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f, -1.0f,  0.0f,    0.0f,  1.0f,  0.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f),
			math::Matrix( 1.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f,  1.0f,  0.0f,    0.0f, -1.0f,  0.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f),
			//Z FACE				     
			math::Matrix( 1.0f,  0.0f,   0.0f,  0.0f,   0.0f,  1.0f,  0.0f,  0.0f,    0.0f,  0.0f,  1.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f),
			math::Matrix(-1.0f,  0.0f,   0.0f,  0.0f,   0.0f,  1.0f,  0.0f,  0.0f,    0.0f,  0.0f, -1.0f,  0.0f,     0.0f,  0.0f,  0.0f,  1.0f)
		};

		static std::vector<math::Matrix> viewProjectionStream;
		viewProjectionStream.resize(6);

		//For input position, make copy of invViewBase, translate it to target pos, invert it and multiply by perspective matrix, whose FOV is always 90 or PI/2
		//We avoid allocations this way (?)
		memcpy(viewProjectionStream.data(), invViewBase, sizeof(math::Matrix) * 6);

		math::Matrix perspective = math::Matrix::CreatePerspectiveFieldOfViewReversedLH(ShadowMap::vertical_FoV_Cube, 1.0f, nearPlane, shadow_distance);

		math::Vector3 target;

#ifdef CAMERA_CENTER_WS
		target = pos - CameraManager::GetActiveCamera().GetPosition();
#else
		target = pos;
#endif
		for (math::Matrix& matrix : viewProjectionStream)
		{
			matrix.Translation(target);
			math::InvertOrthonormalMatrix(matrix, matrix);
			matrix = matrix * perspective;
		}


		return viewProjectionStream;
	}
	void ShadowMap::BindDepthPass(uint16_t slot) const
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.MinDepth = 0.0f;
		vp.Height = vp.Width = resolution;

		DX_CALL(s_devcon->RSSetViewports(1, &vp));

		Texture::ClearPS(slot);
		depthTarget.Clear(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
		depthTarget.BindDepthOnly();
	}

	void ShadowMap::Bind(uint16_t slot) const
	{
		texture.BindPS(slot);
	}


	float ShadowMap::ComputeNearPlaneTexelSizeFor_FoVy(float verticalFoV_radians) const
	{
		float sinHalf, cosHalf;
		DirectX::XMScalarSinCos(&sinHalf, &cosHalf, verticalFoV_radians * 0.5f);
		const float halfTan{ sinHalf / cosHalf };

		return nearPlane * 2.0f * halfTan / resolution;
	}
}