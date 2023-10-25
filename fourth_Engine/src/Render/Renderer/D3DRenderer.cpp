#include "pch.h"

#pragma once
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/D3DAssert.h"
#include "include/Render/Primitive Shapes/Vertex.h"
#include "include/Render/Renderer/ConstantBuffers.h"
#include "include/Systems/MeshSystem.h"
#include "include/Render/Renderer/Grid.h"
#include "include/Render/Skybox.h"
#include "include/Render/Renderer/PostProcess.h"
#include "Shaders/RegisterSlots.hlsli"
#include "Shaders/RenderTargetSlots.hlsli"
#include "include/Systems/LightSystem.h"
#include "include/Systems/ParticleSystem.h"
#include "include/Specifications.h"
#include "include/Systems/DecalSystem.h"


namespace fth
{
	ID3D11Device5* s_device = nullptr;
	ID3D11DeviceContext4* s_devcon = nullptr;
	IDXGIFactory5* s_factory = nullptr;
	ID3D11Debug* s_debug = nullptr;

	void ibl::EnvironmentalIBL::Bind() const
	{
		diffuseIrradiance->BindPS(TX_IBL_D_IRR_SLOT);
		specularIrradiance->BindPS(TX_IBL_S_IRR_SLOT);
		specularReflectance->BindPS(TX_IBL_S_REF_SLOT);
	}

	namespace renderer
	{

		inline uint32_t nextPoT(uint32_t x)
		{
			--x;
			x |= x >> 1u;
			x |= x >> 2u;
			x |= x >> 4u;
			x |= x >> 8u;
			x |= x >> 16u;

			return ++x;
		}

		inline uint32_t fft2d_clz(uint32_t x)
		{
			static constexpr uint32_t lut[32] = {
				0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
				1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
			};

			++x;
			x = nextPoT(x);

			return lut[x * 0x076be629U >> 27U];
		}


		ID3D11Device5* device = nullptr;
		ID3D11DeviceContext4* devcon = nullptr;
		IDXGIFactory5* factory = nullptr;
		ID3D11Debug* debug = nullptr;

		const std::wstring D3DRenderer::shadersPath = L"../fourth_Engine/Shaders/";

		D3DRenderer::D3DRenderer(){}
		D3DRenderer::~D3DRenderer(){}

		void D3DRenderer::InitContext()
		{
#ifndef NDEBUG
#endif

			FTH_VERIFY_HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)m_factory.reset()));
			FTH_VERIFY_HR(m_factory->QueryInterface(__uuidof(IDXGIFactory5), (void**)m_factory5.reset()));

			{
				uint32_t index = 0;
				IDXGIAdapter1* adapter;
				while (m_factory5->EnumAdapters1(index++, &adapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);

					std::cout << "GPU #" << index << desc.Description << std::endl;
				}
			}

			// Init D3D Device & Context

			const D3D_FEATURE_LEVEL featureLevelRequested = D3D_FEATURE_LEVEL_11_1;
			D3D_FEATURE_LEVEL featureLevelInitialized = D3D_FEATURE_LEVEL_11_1;
			UINT creationFlag = 0;

#ifndef NDEBUG
			creationFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif


			FTH_VERIFY_HR(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlag,
				&featureLevelRequested, 1, D3D11_SDK_VERSION, m_device.reset(), &featureLevelInitialized, m_devcon.reset()));

			FTH_ASSERT_ENGINE(featureLevelRequested == featureLevelInitialized, "D3D_FEATURE_LEVEL_11_0");

			FTH_VERIFY_HR(m_device->QueryInterface(__uuidof(ID3D11Device5), (void**)m_device5.reset()));
			FTH_VERIFY_HR(m_devcon->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)m_devcon4.reset()));

#ifndef NDEBUG
			FTH_ASSERT_HR(m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)m_devdebug.reset()));
			DxDebugParser::Init();
			s_debug = debug = m_devdebug.ptr();
#endif


			//Allocate GBuffers pointers and render targets
			m_gBuffer = std::make_unique<GBuffer>();

			// Write global pointers
			s_factory = factory = m_factory5.ptr();
			s_device  = device  = m_device5.ptr();
			s_devcon  = devcon  = m_devcon4.ptr();

			
		}

		void D3DRenderer::SetupMainCamera()
		{
			ViewportSettings vpSettings;
			vpSettings.fov = 90.0f;
			vpSettings.nearPlane = 0.1f;
			vpSettings.farPlane = 100.f;
			vpSettings.aspectRatio = 16.0f / 9.0f;

			m_mainCamera.SetWorldPosition(math::Vector3(0.0f, 0.0f, 0.0f));
			m_mainCamera.SetWorldLookAtDirection(math::Vector3::Forward, math::Vector3::Up);
			m_mainCamera.UpdateViewPortSettings(vpSettings);
			m_mainCamera.UpdateMatrices();
		}
	
		void D3DRenderer::Init(uint16_t width, uint16_t height, uint16_t multisamples)
		{

			InitContext();
			SetupMainCamera();
			InitRenderPipeline(width, height, multisamples);
			SetClearColor(DirectX::Colors::Black.v);
			CreateDepthStencilStates();
			CreateRasterizerStates();
			CreateBlendStates();
			CreateSamplers();

			InitConstantBuffers();
			LoadShaders();
			SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			m_enableWireframe = false;
			m_debugLuminance = false;

			m_Grid = std::make_unique<Grid>();
			m_Grid->Init();

			m_Skybox = std::make_unique<Skybox>();
			m_Skybox->Init();
			setBloomTechnique(DOWNSAMPLE_16BILINEAR_UPSAMPLE_GAUSSIANBILINEAR);
		}


		void D3DRenderer::CreateDepthStencilStates()
		{
			m_currentDepthStencilState = DSSTYPES::DISABLE;
			m_currentStencilRef = StencilValues::SKY;
			m_depthStencilStates[(uint8_t)m_currentDepthStencilState].BindState(m_currentStencilRef);

			

			m_depthStencilStates[(uint8_t)DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED);
			m_depthStencilStates[(uint8_t)DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS);
			m_depthStencilStates[(uint8_t)DSSTYPES::DSS_DEPTH_RO_GREATEREQ_STENCIL_RO_OPEQUAL] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DSS_DEPTH_RO_GREATEREQ_STENCIL_RO_OPEQUAL);
			m_depthStencilStates[(uint8_t)DSSTYPES::DSS_DEPTH_RO_GREATER_STENCIL_DISABLED] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DSS_DEPTH_RO_GREATER_STENCIL_DISABLED);
			m_depthStencilStates[(uint8_t)DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL);
			m_depthStencilStates[(uint8_t)DSSTYPES::DEFAULT] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DEFAULT);
			m_depthStencilStates[(uint8_t)DSSTYPES::DISABLE] = DepthStencilState::CreateDepthStencilState(DSSTYPES::DISABLE);
		}

		void D3DRenderer::CreateRasterizerStates()
		{
			m_currentRasterizerState = RSTYPES::DEFAULT;
			m_rasterizerStates[(uint8_t)m_currentRasterizerState].BindState();

			m_rasterizerStates[(uint8_t)RSTYPES::RSTYPE_NOCULL_SOLID] = RasterizerState::CreateRasterizerState(RSTYPES::RSTYPE_NOCULL_SOLID);
			m_rasterizerStates[(uint8_t)RSTYPES::RSTYPE_NOCULL_WIREFRAME] = RasterizerState::CreateRasterizerState(RSTYPES::RSTYPE_NOCULL_WIREFRAME);
			m_rasterizerStates[(uint8_t)RSTYPES::RSTYPE_DEFAULT_DEPTH_BIAS] = RasterizerState::CreateRasterizerState(RSTYPES::RSTYPE_DEFAULT_DEPTH_BIAS);
			m_rasterizerStates[(uint8_t)RSTYPES::DEFAULT] = RasterizerState::CreateRasterizerState(RSTYPES::DEFAULT);
		}

		void D3DRenderer::CreateBlendStates()
		{
			m_currentBlendState = BSTYPES::DEFAULT;
			m_blendStates[(uint8_t)m_currentBlendState].BindState(nullptr, 0xFFFFFFFF);


			m_blendStates[(uint8_t)BSTYPES::GENERAL_TRANSLUCENCY] = BlendState::CreateBlendState(BSTYPES::GENERAL_TRANSLUCENCY);
			m_blendStates[(uint8_t)BSTYPES::ADDITIVE_TRANSLUCENCY] = BlendState::CreateBlendState(BSTYPES::ADDITIVE_TRANSLUCENCY);
			m_blendStates[(uint8_t)BSTYPES::GENERAL_TRANSLUCENCY_ALBEDO_ROUGHMETAL_EMISSION] = BlendState::CreateBlendState(BSTYPES::GENERAL_TRANSLUCENCY_ALBEDO_ROUGHMETAL_EMISSION);
			m_blendStates[(uint8_t)BSTYPES::DEFAULT_A2C] = BlendState::CreateBlendState(BSTYPES::DEFAULT_A2C);
			m_blendStates[(uint8_t)BSTYPES::DEFAULT] = BlendState::CreateBlendState(BSTYPES::DEFAULT);

		}

		void D3DRenderer::CreateSamplers()
		{
			D3D11_SAMPLER_DESC sDsc;
			sDsc.MipLODBias = 0.0f;
			sDsc.BorderColor[0] = 0.0f;
			sDsc.BorderColor[1] = 0.0f;
			sDsc.BorderColor[2] = 0.0f;
			sDsc.BorderColor[3] = 0.0f;
			sDsc.MinLOD = -FLT_MAX;
			sDsc.MaxLOD = FLT_MAX;

			sDsc.ComparisonFunc = D3D11_COMPARISON_NEVER;


			sDsc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			sDsc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			sDsc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;

			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
			sDsc.MaxAnisotropy = 1;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerPointWrap.reset()));
			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerTrilinearWrap.reset()));
			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
			sDsc.MaxAnisotropy = 16;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerAnisotropicWrap.reset()));


			sDsc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			sDsc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
			sDsc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;

			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
			sDsc.MaxAnisotropy = 1;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerPointClamp.reset()));
			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerBilinearClamp.reset()));
			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerTrilinearClamp.reset()));
			sDsc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
			sDsc.MaxAnisotropy = 16;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerAnisotropicClamp.reset()));


			sDsc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			sDsc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
			sDsc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;

			sDsc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			sDsc.MaxAnisotropy = 1;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerPointBorder.reset()));

			//CMP samplers
			sDsc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

			sDsc.ComparisonFunc = D3D11_COMPARISON_GREATER;
			DX_HRCALL(device->CreateSamplerState(&sDsc, m_samplerCmpBilinearBorder.reset()));

			ID3D11SamplerState* samplers[] = {
	            m_samplerPointWrap.ptr(), m_samplerTrilinearWrap.ptr(), m_samplerAnisotropicWrap.ptr(),
	            m_samplerPointClamp.ptr(), m_samplerBilinearClamp.ptr(), m_samplerTrilinearClamp.ptr(), m_samplerAnisotropicClamp.ptr(),
				m_samplerPointBorder.ptr(), m_samplerCmpBilinearBorder.ptr()};

			DX_CALL(devcon->PSSetSamplers(0, 9, samplers));
			DX_CALL(devcon->VSSetSamplers(0, 9, samplers));
		}

		void D3DRenderer::createBloomResources(uint16_t frameWidth, uint16_t frameHeight)
		{
			TextureDsc dsc;
			dsc.width  =  frameWidth / 2;
			dsc.height = frameHeight / 2;
			dsc.depth = 1;
			dsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
			dsc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			//dsc.numMips = 8;
			dsc.numMips = std::min(
				static_cast<uint8_t>(1.0f + std::floorf(std::log2f(std::max(static_cast<float>(dsc.width), static_cast<float>(dsc.height))))),
				MAX_MIPS_BLOOM);
			dsc.isCubemap = false;
			dsc.arraySize = 1;
			m_optional1_BloomTexture.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
			m_optional2_BloomTexture.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);

			m_bloomTexture.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);

			DxResPtr<ID3D11Resource>& res = m_bloomTexture.GetResource();
			DxResPtr<ID3D11Resource>& optional1Res = m_optional1_BloomTexture.GetResource();
			DxResPtr<ID3D11Resource>& optional2Res = m_optional2_BloomTexture.GetResource();


			for (uint8_t n = 0; n < dsc.numMips; ++n)
			{
				m_bloomTextureSRV[n].CreateShaderResourceView(res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n, 1);
				m_bloomTextureUAV[n].createTextureUAV(res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n);
				m_optional1_BloomTextureSRV[n].CreateShaderResourceView(optional1Res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n, 1);
				m_optional1_BloomTextureUAV[n].createTextureUAV(optional1Res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n);
				m_optional2_BloomTextureSRV[n].CreateShaderResourceView(optional2Res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n, 1);
				m_optional2_BloomTextureUAV[n].createTextureUAV(optional2Res, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, n);
			}

			dsc.width = frameWidth;
			dsc.height = frameHeight;
			dsc.numMips = 1;

			m_output.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

			dsc.numMips = 1;

			dsc.width = static_cast<uint16_t>(nextPoT(static_cast<uint32_t>(frameWidth)));
			dsc.height = static_cast<uint16_t>(nextPoT(static_cast<uint32_t>(frameHeight)));
			m_realXYZimX.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS);

			dsc.format = DXGI_FORMAT_R16G16_FLOAT;
			m_imYZ.Init(dsc, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS);

			m_fftUAVs[0].createTextureUAV(m_output.GetResource(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
			m_fftUAVs[1].createTextureUAV(m_realXYZimX.GetResource(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
			m_fftUAVs[2].createTextureUAV(m_imYZ.GetResource(), DXGI_FORMAT_R16G16_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);


		}

		void D3DRenderer::LoadShaders()
		{
			m_normalDebugPS.LoadShader(L"NormalDebug_PS.cso");
			m_resolveUnlitPS.LoadShader(L"Resolve_Unlit_PS.cso");
			m_resolveLightningPS.LoadShader(L"Resolve_Lightning_CookTorrance_PS.cso");

			const D3D11_INPUT_ELEMENT_DESC ied[] =
			{
				{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 0                        , D3D11_INPUT_PER_VERTEX_DATA, 0},

				{"MW",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0                                       ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",       1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",       2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 2 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",       3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},

			};

			m_depthPass2D_VS.LoadShader(L"ShadowPass_2D_VS.cso", ied, static_cast<uint32_t>(std::size(ied)));
			m_fullPassPassthroughVS.LoadShader(L"Passthrough_VS.cso", nullptr, 0);

			m_depthPassCube_VS.LoadShader(L"ShadowPass_Cube_VS.cso", ied, static_cast<uint32_t>(std::size(ied)));
			m_depthPassCube_GS.LoadShader(L"ShadowPass_Cube_GS.cso");

			//m_fft0_CS.loadShader(L"FFT_TRIPLE_R2C_CS.cso");
			//m_fft1_CS.loadShader(L"FFT_TRIPLE_C2C_CS.cso");
			m_fftdouble_CS.loadShader(L"FFT_C2C_CS.cso");
			m_fftAperture_CS.loadShader(L"ApertureDiffraction_CS.cso");


		}

		void D3DRenderer::AttachLDR_Target(const fth::Texture& texture)
		{
			if(texture.GetResource().valid())
			{
				m_backBufferLDR_RTV.CreateRenderTarget(texture.GetResource(), texture.Format(), D3D11_RESOURCE_DIMENSION::D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
			}

		}

		void D3DRenderer::BindLDR_Target()
		{
			m_backBufferLDR_RTV.BindRenderTargetView();
		}

		void D3DRenderer::BindHDR_Target()
		{
			m_hdrRTV.BindRenderTargetView(&m_DSV);
		}

		void D3DRenderer::SetMainViewport(D3D11_VIEWPORT viewport)
		{
			m_mainViewport = viewport;
		}

		void D3DRenderer::BindMainViewport()
		{
			DX_CALL(s_devcon->RSSetViewports(1, &m_mainViewport));
		}



		void D3DRenderer::BindRasterizerState(RSTYPES state)
		{
			if (m_currentRasterizerState == state)
				return;

			m_rasterizerStates[(uint8_t)state].BindState();
			m_currentRasterizerState = state;
		}

		void D3DRenderer::BindDepthStencilState(DSSTYPES state, StencilValues refStencil)
		{
			if (m_currentDepthStencilState == state && m_currentStencilRef == refStencil)
				return;

			m_depthStencilStates[(uint8_t)state].BindState(refStencil);
			m_currentDepthStencilState = state;
			m_currentStencilRef = refStencil;
		}


		void D3DRenderer::BindBlendState(BSTYPES state, bool forceBind, const FLOAT* blendFactor, uint32_t mask)
		{
			if (forceBind)
			{
				m_blendStates[(uint8_t)state].BindState(blendFactor, mask);
				m_currentBlendState = state;
				return;
			}

			if (m_currentBlendState == state)
				return;


			m_blendStates[(uint8_t)state].BindState(blendFactor, mask);
			m_currentBlendState = state;
		}

		void D3DRenderer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
		{
			DX_CALL(devcon->IASetPrimitiveTopology(topology));
		}

		void D3DRenderer::SetSkybox(std::string_view cubemapName)
		{
			m_Skybox->SetCubemap(cubemapName);
		}

		void D3DRenderer::MakeFullpass()
		{
			DX_CALL(devcon->IASetInputLayout(nullptr));
			DX_CALL(devcon->Draw(3, 0));
		}

		void D3DRenderer::resolveMSDepthCopy(const ShaderResourceView& src, const RenderTargetView& dst)
		{
			m_fullPassPassthroughVS.Bind();
			PostProcess::Get().ResolveMSDepthTexture(src, dst);
		}

		void D3DRenderer::BindRenderTargets(RenderTargetView* const rtv, uint16_t numRTV, DepthStencilView* dsv)
		{
			if (dsv && dsv->getView().valid())
			{
				DX_CALL(s_devcon->OMSetRenderTargets(numRTV, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv->getView()), dsv->getView()));
			}
			else
			{
				DX_CALL(s_devcon->OMSetRenderTargets(numRTV, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv->getView()), nullptr));
			}
		}

		void D3DRenderer::BindRenderTargetsAndUAV(RenderTargetView* const rtv, uint32_t numRTV, DepthStencilView const* dsv, UnorderedAccessView const* uav, uint32_t numUav, uint16_t startSlot)
		{
			//TODO: ADD SOME VALIDATION. ASK EMIL ABOUT HANDLING VIEWS AND TRACK RESOURCES
			if (dsv && dsv->getView().valid())
			{
				DX_CALL(s_devcon->OMSetRenderTargetsAndUnorderedAccessViews(
					numRTV, 
					reinterpret_cast<ID3D11RenderTargetView* const*>(rtv),
					dsv->getView(),
					startSlot,
					numUav,
					reinterpret_cast<ID3D11UnorderedAccessView* const*>(uav),
					nullptr));
			}
			else
			{
				DX_CALL(s_devcon->OMSetRenderTargetsAndUnorderedAccessViews(
					numRTV, 
					reinterpret_cast<ID3D11RenderTargetView* const*>(rtv), 
					nullptr, 
					startSlot, 
					numUav, 
					reinterpret_cast<ID3D11UnorderedAccessView* const*>(uav),
					nullptr));
			}
		}

		void D3DRenderer::BindPSNormalDebug()
		{
			m_normalDebugPS.Bind();
		}


		void D3DRenderer::UnbindViews()
		{
			DX_CALL(s_devcon->OMSetRenderTargets(0, NULL, NULL));
		}

		void D3DRenderer::bindGBuffer()
		{
			RenderTargetView::BindRenderTargetViews(m_gBuffer->gRenderTargets, NUM_G_TEXTURES, &m_DSV);
		}

		void D3DRenderer::StartFrame()
		{
			ClearViews();

			ID3D11SamplerState* samplers[] = { 
				m_samplerPointWrap.ptr(), m_samplerTrilinearWrap.ptr(), m_samplerAnisotropicWrap.ptr(),
				m_samplerPointClamp.ptr(), m_samplerTrilinearClamp.ptr(), m_samplerAnisotropicClamp.ptr(),
			    m_samplerPointBorder.ptr(), m_samplerCmpBilinearBorder.ptr()};
			DX_CALL(devcon->PSSetSamplers(0, 8, samplers));
			DX_CALL(devcon->VSSetSamplers(0, 8, samplers));
			DX_CALL(devcon->CSSetSamplers(0, 8, samplers));
			if(m_environment.valid())
			{
				m_environment.Bind();
			}
		}

		void D3DRenderer::EndFrame()
		{
			ShaderResourceView::ClearPS(TX_GB_ALBEDO_SLOT);
			ShaderResourceView::ClearPS(TX_GB_ROUGH_METAL_SLOT);
			ShaderResourceView::ClearPS(TX_GB_NORMAL_SLOT);
			ShaderResourceView::ClearPS(TX_GB_OBJECTID_SLOT);
			ShaderResourceView::ClearPS(TX_GB_EMISSION_SLOT);
			ShaderResourceView::ClearPS(TX_GB_DEPTH_SLOT);

			UnbindViews();

		}

		const Texture& D3DRenderer::computeDepthBufferCopy() 
		{
			m_bufferDepthStencil.MakeCopyDepthNonMS(m_depthCopy, m_depthCopyRTV);
			
			return m_depthCopy;
		}

		const Texture& D3DRenderer::getDepthBufferCopy() const
		{
			return m_depthCopy;
		}
		
		const Texture& D3DRenderer::getGBufferNormalsCopy() const
		{
			return m_gbufferNormalsCopy;
		}

		const Texture& D3DRenderer::getGBufferObjectIDCopy()const
		{
			return m_gbufferObjectIDCopy;
		}

		void D3DRenderer::ClearViews()
		{
			//in fact its not needed to clear everything
			m_hdrRTV.Clear(m_clearColor);
			m_backBufferLDR_RTV.Clear(m_clearColor);
			m_resolvePassRTV.Clear(m_clearColor);

			m_gBuffer->gRenderTargets[TARGET_ALBEDO_SLOT].Clear(m_clearColor);
			m_gBuffer->gRenderTargets[TARGET_ROUGH_METAL_SLOT].Clear(math::Vector4::Zero);
			m_gBuffer->gRenderTargets[TARGET_TX_GM_NORMAL_SLOT].Clear(math::Vector4::Zero);
			m_gBuffer->gRenderTargets[TARGET_EMISSION_SLOT].Clear(math::Vector4::Zero);
			m_DSV.Clear(D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, StencilValues::SKY);

			m_clearObjectID_RTV.BindRenderTargetView();
			m_clearObjectID_RTV.Clear(math::Vector4(-1.0f));
		}

		void D3DRenderer::InitRenderPipeline(uint16_t width, uint16_t height, uint16_t multisamples)
		{
			m_width = width;
			m_height = height;

			constexpr DXGI_FORMAT albedoFormat      = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
			constexpr DXGI_FORMAT rough_metalFormat = DXGI_FORMAT::DXGI_FORMAT_R16G16_UNORM;
			constexpr DXGI_FORMAT normalsFormat     = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM;
			constexpr DXGI_FORMAT objectIDFormat    = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
			constexpr DXGI_FORMAT emissionFormat = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
			constexpr DXGI_FORMAT gBufferFormats[NUM_G_TEXTURES] =
			{
                  albedoFormat,    
                  rough_metalFormat,
                  normalsFormat,    
                  objectIDFormat,
				  emissionFormat
			};

			constexpr DXGI_FORMAT hdrFormat         = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
			constexpr DXGI_FORMAT depthFormat       = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
			constexpr DXGI_FORMAT resolvePassFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

			TextureDsc txDsc;
			txDsc.width = width;
			txDsc.height = height;
			txDsc.depth = 0;
			txDsc.numMips = 1;
			txDsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
			txDsc.arraySize = 1;
			txDsc.isCubemap = false;

			if (multisamples != 1)
			{
				//MSAA is not implemented in the pipeline right now.

				/*DX_CALL(s_device->CheckMultisampleQualityLevels(hdrFormat, multisamples, reinterpret_cast<UINT*>(&m_maxMSAA_qualitySupported)));
				txDsc.multisamples = multisamples;
				txDsc.msQuality = m_maxMSAA_qualitySupported - 1;*/

			}
	
			//Init GBuffer
			for (uint8_t i = 0; i < NUM_G_TEXTURES; ++i)
			{
				bool valid{ false };
				Texture& tx = m_gBuffer->gTextures[i];
				txDsc.format = gBufferFormats[i];

				DXGI_FORMAT rtFmt;

				tx.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, nullptr);

				if (i == TARGET_OBJECTID_SLOT)
				{
					rtFmt = DXGI_FORMAT_R32_UINT;
					//Create another render target just for clearing objectID target to UINT32_T_MAX
					m_clearObjectID_RTV.CreateRenderTarget(tx.GetResource(), DXGI_FORMAT_R32_SINT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
				}
				else
				{
					rtFmt = txDsc.format;
				}
				tx.MakeShaderResource(rtFmt);
	
				m_gBuffer->gRenderTargets[i].CreateRenderTarget(tx.GetResource(), rtFmt, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
				valid = tx.GetResource().valid() && tx.GetSRV().getView().valid() && m_gBuffer->gRenderTargets[i].getView().valid();
				FTH_VERIFY_ENGINE(valid == true, "GBuffer validation during creation failed");
			}


			//HDR buffer that where illumination gets resolved
			txDsc.format = hdrFormat;
			m_hdrBufferColor.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_hdrBufferColor.MakeShaderResource(hdrFormat);

			//LDR buffer that gets resolved from HDR buffer
			txDsc.format = resolvePassFormat;
			m_resolvePassBuffer.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_resolvePassBuffer.MakeShaderResource(resolvePassFormat);
			m_resolvePassRTV.CreateRenderTarget(m_resolvePassBuffer.GetResource(), resolvePassFormat, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);


			txDsc.format = DXGI_FORMAT_R24G8_TYPELESS;
			m_bufferDepthStencil.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_bufferDepthStencil.MakeShaderResource(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
			if (m_hdrBufferColor.Multisamples() > 1)
			{
				m_hdrRTV.CreateRenderTarget2D_MS(m_hdrBufferColor.GetResource(), hdrFormat);
				m_DSV.CreateDepthStencilTarget2D_MS(m_bufferDepthStencil.GetResource(), depthFormat);

			}
			else
			{
				m_hdrRTV.CreateRenderTarget(m_hdrBufferColor.GetResource(), hdrFormat, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);
				m_DSV.CreateDepthStencilTarget(m_bufferDepthStencil.GetResource(), depthFormat, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);

			}

			//create depth copy resources
			txDsc.multisamples = 1;
			txDsc.msQuality = 0;
			txDsc.format = DXGI_FORMAT_R24G8_TYPELESS;
			m_depthCopy.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_depthCopy.MakeShaderResource(DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

			txDsc.format = normalsFormat;
			m_gbufferNormalsCopy.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_gbufferNormalsCopy.MakeShaderResource(txDsc.format);

			txDsc.format = DXGI_FORMAT_R32_UINT;
			m_gbufferObjectIDCopy.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, nullptr);
			m_gbufferObjectIDCopy.MakeShaderResource(txDsc.format);
			//m_depthCopyRTV.CreateRenderTarget(m_depthCopy.GetResource(), DXGI_FORMAT_R32_FLOAT, D3D11_RESOURCE_DIMENSION_TEXTURE2D, 0);

			createBloomResources(width, height);
		}


		void D3DRenderer::Render()
		{
			RenderShadows();

			bindGBuffer();
			BindMainViewport();

			//Draw opaques
			const renderer::Camera& camera{ CameraManager::GetActiveCamera() };

			renderer::cb_PER_VIEW cbPV{
				camera.GetViewProjection(),
				camera.GetInverseViewProjection(),
				camera.GetProjection(),
				camera.GetInverseView(),
				camera.GetInverseProjection(),
				camera.GetView(),
				camera.getFrustum().get_BL_Dir(),
				camera.getFrustum().get_BR_Dir(),
				camera.getFrustum().get_TL_Dir(),
				camera.GetNearPlane(),
				camera.GetFarPlane(),
				camera.m_ViewPortWidth,
				camera.m_ViewPortHeight,
				camera.GetAspectRatio()
			};
			UpdatePerViewUniform(cbPV);

			BindBlendState(BSTYPES::DEFAULT);
			MeshSystem::Get().Draw();



			//Render the grid
		    //m_Grid->Draw();

			m_bufferDepthStencil.MakeCopy(m_depthCopy);
			m_gBuffer->gTextures[TARGET_TX_GM_NORMAL_SLOT].MakeCopy(m_gbufferNormalsCopy);
			m_gBuffer->gTextures[TARGET_OBJECTID_SLOT].MakeCopy(m_gbufferObjectIDCopy);

			BindDepthStencilState(DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL, StencilValues::PBR_SHADOWS);
			BindBlendState(BSTYPES::GENERAL_TRANSLUCENCY_ALBEDO_ROUGHMETAL_EMISSION);
			DecalSystem::Get().draw();

			
			//----------RESOLVE INTO HDR TARGET---------//
			//1-Bind HDR target as RTV
			//2-BindGBuffer as SRV
			BindHDR_Target();
			
			ParticleSystem::Get().updateGPUSimulation(m_depthCopy.GetSRV(), m_gBuffer->gTextures[TARGET_TX_GM_NORMAL_SLOT].GetSRV());

			//If would be much nicer if we manage to bunch SRVs in a single array to make a single API call
			Texture* gbTx{ m_gBuffer->gTextures };

			{
				ShaderResourceView resolveGB[6]{
					gbTx[TARGET_ALBEDO_SLOT].GetSRV(),
					gbTx[TARGET_ROUGH_METAL_SLOT].GetSRV(),
					gbTx[TARGET_TX_GM_NORMAL_SLOT].GetSRV(),
					gbTx[TARGET_OBJECTID_SLOT].GetSRV(),
					gbTx[TARGET_EMISSION_SLOT].GetSRV(),
					m_depthCopy.GetSRV() };

				ShaderResourceView::BindPS(resolveGB, 6, TX_GB_ALBEDO_SLOT);
			}


			m_fullPassPassthroughVS.Bind();
			//Resolve light materials
			LightSystem& ls{ LightSystem::Get() };

			ls.GetDirectionalShadowmap().Bind(TX_SM_DIRECT_SLOT);
		    ls.GetSpotlightShadowmap().Bind(TX_SM_SPOT_SLOT);
		    ls.GetPointShadowmap().Bind(TX_SM_POINT_SLOT);
			MeshSystem::Get().BindShadowmapUniform(SHADOW_MAP_SLOT);

			BindBlendState(BSTYPES::DEFAULT);
			BindRasterizerState(RSTYPES::DEFAULT);
			BindDepthStencilState(DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL, StencilValues::PBR_SHADOWS);
			m_resolveLightningPS.Bind();
			MakeFullpass();

			//Resolve no-lightning materials
			BindDepthStencilState(DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL, StencilValues::NO_LIGHTNING);
			m_resolveUnlitPS.Bind();
			MakeFullpass();

			//Draw skybox
			m_Skybox->Draw();

			//Resolve transparency materials
			BindBlendState(BSTYPES::GENERAL_TRANSLUCENCY);
			ParticleSystem::Get().Draw();



			//ResolveHDR into resolve buffer(tonemap, gamma correct...)
			m_fullPassPassthroughVS.Bind();
			BindBlendState(BSTYPES::DEFAULT);

			UnbindViews();

		
		


			switch (m_bloomTechnique)
			{
				case fth::DOWNSAMPLE_16BILINEAR_UPSAMPLE_3x3TENT:
				{
					break;
				}
				case fth::DOWNSAMPLE_GAUSSIAN_UPSAMPLE_BILINEARMEAN:
				{
					break;
				}
				case fth::DOWNSAMPLE_16BILINEAR_UPSAMPLE_GAUSSIANBILINEAR:
				{
					PostProcess::Get().resolveBloom_downsampleCoD_upsampleBilinearGaussian(
						m_hdrBufferColor.GetSRV(),
						m_bloomTexture.Width(),
						m_bloomTexture.Height(),
						m_bloomTexture.NumMips(),
						m_bloomTextureUAV,
						m_bloomTextureSRV,
						m_optional1_BloomTextureUAV,
						m_optional1_BloomTextureSRV,
						m_optional2_BloomTextureUAV,
						m_optional2_BloomTextureSRV);

					PostProcess::Get().Resolve(m_hdrBufferColor.GetSRV().getView(), m_resolvePassRTV.getView().ptr(), m_bloomTextureSRV);

					break;
				}
				case fth::DOWNSAMPLE_GAUSSIAN:
				{
					break;
				}
				case fth::DISABLED:
				{
					PostProcess::Get().Resolve(m_hdrBufferColor.GetSRV().getView(), m_resolvePassRTV.getView().ptr(), nullptr);
					break;
				}
			}


			//Resolve FXAA into final LDR target(screen)
			PostProcess::Get().applyFXAA(
				static_cast<float>(m_width),
				static_cast<float>(m_height), 
				m_resolvePassBuffer.GetSRV(), 
				m_backBufferLDR_RTV);

		}


		void D3DRenderer::RenderShadows()
		{
			//Steps:
            //1-Generate per-light VP matrix
            //2-Update View uniform
            //3-Render all geometry

			static std::vector<ShadowMap::cbSMFrustumPlanesGPU> frustumPlanesStream;
			//We only need one space por point lights, because they share same frustum characteristics
			frustumPlanesStream.reserve(Config::MAX_DIRECITONAL_LIGHTS + Config::MAX_SPOT_LIGHTS + 1);
			frustumPlanesStream.clear();//clear just sets size to zero, but capacity remains the same, so no allocations occur
			ShadowMap::cbSMFrustumPlanesGPU frustumPlane;

			//DIRECTIONAL
			math::Matrix directionalVP;
			{
				ShadowMap& directionalSM = LightSystem::Get().GetDirectionalShadowmap();
				directionalVP = directionalSM.ComputeTightFrustumOrthogonal(CameraManager::GetActiveCamera(), LightSystem::Get().QueryDirectionalLightByIndex(0).direction, frustumPlane.nearTexelSizeWS);
				frustumPlane.nearPlane = directionalSM.nearPlane;
				frustumPlane.farPlane = directionalSM.ComputeFarPlaneOrthogonalShadowmap();

				frustumPlanesStream.emplace_back(frustumPlane);

				UpdatePerViewUniform({ directionalVP });


				directionalSM.BindDepthPass(TX_SM_DIRECT_SLOT);
				MeshSystem::Get().RenderDepth2D();
			}

			//SPOT
			math::Matrix spotVP;
			{
				ShadowMap& spotSM = LightSystem::Get().GetSpotlightShadowmap();
				//TODO: Add light IDs into ShadowMaps, and we know by static typing where to find those IDs.
				
				//uint32_t numSpot{ LightSystem::Get().GetNumSpotLights() };
				lights::SpotSphere& spot = LightSystem::Get().QuerySpotLightByIndex(0);

				const math::Matrix& spotParentTransform = spot.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(spot.parentID) : math::Matrix::Identity;

				spotVP = spotSM.ComputeTightFrustumPerspective(
					math::Vector3::Transform(spot.position, spotParentTransform),
					math::Vector3::TransformRotate(spot.direction, spotParentTransform),
					ShadowMap::vertical_FoV_Cube, frustumPlane.nearTexelSizeWS);

				frustumPlane.nearPlane = spotSM.nearPlane;
				frustumPlane.farPlane = spotSM.ComputeFarPlanePerspectiveShadowmap();
				frustumPlanesStream.emplace_back(frustumPlane);

			    spotSM.BindDepthPass(TX_SM_SPOT_SLOT);
			    UpdatePerViewUniform({ spotVP });
			    MeshSystem::Get().RenderDepth2D();
			}



			//OMNIDIRECTIONAL
			static std::vector<math::Vector3> pointPositions;


			const uint32_t numPointLights{ LightSystem::Get().GetNumPointLights() };
			{
				if (numPointLights > 0)
				{
					ShadowMap& pointShadowmap = LightSystem::Get().GetPointShadowmap();

					pointPositions.resize(Config::MAX_POINT_LIGHTS);
					pointPositions.clear();

					//In this case, shadowmaps share frustum characteristics
					frustumPlane.nearPlane = pointShadowmap.nearPlane;
					frustumPlane.farPlane = pointShadowmap.ComputeFarPlanePerspectiveShadowmap();
					frustumPlane.nearTexelSizeWS = pointShadowmap.ComputeNearPlaneTexelSizeFor_FoVy(ShadowMap::vertical_FoV_Cube);
					frustumPlanesStream.emplace_back(frustumPlane);

					for (uint32_t i = 0; i < numPointLights; ++i)
					{
						const lights::PointSphere& light = LightSystem::Get().QueryPointLightByIndex(i);

						pointPositions.emplace_back(math::Vector3::Transform(light.position, light.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(light.parentID) : math::Matrix::Identity));


					}

					pointShadowmap.BindDepthPass(TX_SM_POINT_SLOT);
					MeshSystem::Get().RenderDepthCubemaps(pointPositions);
					
				}

			}


			MeshSystem::Get().SubmitShadowMapUniformData(directionalVP, spotVP, frustumPlanesStream);

		}

		void D3DRenderer::ToggleWireframe()
		{
			m_enableWireframe = !m_enableWireframe;
			
			if (WireframeEnabled())
				BindRasterizerState(RSTYPES::RSTYPE_NOCULL_WIREFRAME);
			else
				BindRasterizerState(RSTYPES::DEFAULT);
		}


		void D3DRenderer::InitConstantBuffers()
		{
			m_uniformPerFrame.CreateGPUBuffer(sizeof(PER_FRAME_CONSTANT_BUFFER), 1, nullptr);
			m_uniformPerView.CreateGPUBuffer(sizeof(cb_PER_VIEW), 1, nullptr);
			m_fftUniform.CreateGPUBuffer(sizeof(fftdata), 1, nullptr);
		}

		void D3DRenderer::Update(const PER_FRAME_CONSTANT_BUFFER& cb, uint16_t width, uint16_t height, uint16_t multisamples)
		{
			UpdatePerFrameConstantBuffer(cb);


			//Verify if resolution is equal

			if (m_hdrBufferColor.Width() != width || m_hdrBufferColor.Height() != height || m_hdrBufferColor.Multisamples() != multisamples)
				InitRenderPipeline(width, height, multisamples);
		}

		void D3DRenderer::UpdatePerViewUniform(const renderer::cb_PER_VIEW& cb)
		{
			void* mapping = m_uniformPerView.Map();

			memcpy(mapping, &cb, sizeof(cb_PER_VIEW));
			m_uniformPerView.Unmap();
			m_uniformPerView.BindVS(PER_VIEW_SLOT);
			m_uniformPerView.BindPS(PER_VIEW_SLOT);
			m_uniformPerView.BindGS(PER_VIEW_SLOT);
			m_uniformPerView.BindHS(PER_VIEW_SLOT);
			m_uniformPerView.BindDS(PER_VIEW_SLOT);
			m_uniformPerView.BindCS(PER_VIEW_SLOT);


		}

		void D3DRenderer::UpdatePerFrameConstantBuffer(const PER_FRAME_CONSTANT_BUFFER& cbIn)
		{
			PER_FRAME_CONSTANT_BUFFER cb {cbIn}; 
			cb.EV100 = PostProcess::Get().GetEV100();
			cb.validateLuminance = m_debugLuminance;
			cb.overrideRoughness = m_overrideRoughness;
			cb.overrideRoughValue = m_overrideRoughValue;
			cb.enableDiffuse = m_enableDiffuse;
			cb.enableSpecular = m_enableSpecular;
			cb.enableIBL = m_enableIBL;
			cb.numMipsSpecularIrradianceIBL = m_environment.specularIrradiance->NumMips();
			cb.specularMRP_Flags = static_cast<uint32_t>(m_MRPFlags);
			cb.mainRTVSubsamples = m_hdrBufferColor.Multisamples();

			void* mapping = m_uniformPerFrame.Map();
			memcpy(mapping, &cb, sizeof(PER_FRAME_CONSTANT_BUFFER));
			m_uniformPerFrame.Unmap();
			m_uniformPerFrame.BindVS(PER_FRAME_SLOT);
			m_uniformPerFrame.BindPS(PER_FRAME_SLOT);
			m_uniformPerFrame.BindGS(PER_FRAME_SLOT);
			m_uniformPerFrame.BindCS(PER_FRAME_SLOT);
		}

		void D3DRenderer::ShutdownViews()
		{
			UnbindViews();
			m_backBufferLDR_RTV.getView().release();
		}

		void D3DRenderer::ClearGeometryStage()
		{
			DX_CALL(m_devcon->GSSetShader(nullptr, nullptr, 0));
		}

		void D3DRenderer::ClearTesselationStage()
		{
			DX_CALL(m_devcon->HSSetShader(nullptr, nullptr, 0));
			DX_CALL(m_devcon->DSSetShader(nullptr, nullptr, 0));

			//For now, default topology is triangle list
			SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		void D3DRenderer::SetRenderTargets(const DxResPtr<ID3D11RenderTargetView>& rtv, uint16_t numTargets, DxResPtr<ID3D11DepthStencilView>* dsv)
		{
			if(dsv && dsv->valid())
			{
				DX_CALL(m_devcon->OMSetRenderTargets(numTargets, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv), *dsv));
			}
			else
			{
				DX_CALL(m_devcon->OMSetRenderTargets(numTargets, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv), nullptr));
			}
		}

		void D3DRenderer::Shutdown()
		{
			ShutdownViews();
			m_Grid->Shutdown();
			m_devcon4.release();
			m_device5.release();
			m_devcon.release();
			m_device.release();
			m_factory5.release();
			m_factory.release();
			DxDebugParser::Shutdown();
			m_devdebug.release();
		}


	}
}