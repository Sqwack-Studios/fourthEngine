#include "pch.h"
#pragma once
#include "include/Render/Renderer/PostProcess.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "Shaders/RegisterSlots.hlsli"

extern ID3D11DeviceContext4* s_devcon;
namespace fth::renderer
{
	const PostProcess::FXAAConfig  PostProcess::FXAAConfig::DEFAULT = { fxaa_qualitySubpixel_default, fxaa_qualityEdgeThreshold_default, fxaa_qualityEdgeThresholdMin_default };
	const PostProcess::BloomConfig PostProcess::BloomConfig::DEFAULT = { bloom_lThreshold, bloom_uThreshold };

	void PostProcess::Init()
	{
		m_resolvePS.LoadShader(L"Resolve_PS.cso");
		m_resolveDepthTextureMS_PS.LoadShader(L"ResolveDepthTextureMS_PS.cso");
		m_resolveFXAA_PS.LoadShader(L"FXAA_PS.cso");
		m_bloomDownsample_CS.loadShader(L"BloomThreshold_CS.cso");
		m_bloomGaussian_CS.loadShader(L"GaussianBlur_CS.cso");

		EV100 = defaultEV100;

		m_fxaaConfig = FXAAConfig::DEFAULT;
		m_bloomConfig = BloomConfig::DEFAULT;
		m_postprocessUniform.CreateGPUBuffer(POSTPROCESS_CBUFFER_SIZE, 1, nullptr);

		FTH_VERIFY_ENGINE(sizeof(FXAAConfig) <= POSTPROCESS_CBUFFER_SIZE, "FXAAConfig struct is bigger than PPUniform size");
	}
	void PostProcess::Resolve(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst, const ShaderResourceView* bloom)
	{
		m_resolvePS.Bind();

		DX_CALL(s_devcon->OMSetRenderTargets(1, &dst, nullptr));
		DX_CALL(s_devcon->PSSetShaderResources(TX_HDR_RESOLVE_SLOT, 1, &src));

		uint32_t flags{};
		if (bloom)
		{
			bloom->BindPS(TX_RESOLVE_BLOOM_SLOT);
			flags |= 1;
		}
		D3DRenderer::Get().BindDepthStencilState(DSSTYPES::DISABLE);
		D3DRenderer::Get().BindRasterizerState(RSTYPES::DEFAULT);

		uint32_t* map = static_cast<uint32_t*>(m_postprocessUniform.Map());
		memcpy(map, &flags, sizeof(uint32_t));
		m_postprocessUniform.Unmap();
		m_postprocessUniform.BindPS(POSTPROCESS_UNIFORM_SLOT);

		D3DRenderer::Get().MakeFullpass();

		//Cleanup SRV
		ID3D11ShaderResourceView* const pSRV[1] = { NULL };
		DX_CALL(s_devcon->PSSetShaderResources(TX_HDR_RESOLVE_SLOT, 1, pSRV));

		
	}

	void PostProcess::ResolveMSDepthTexture(const ShaderResourceView& src, const RenderTargetView& dst)
	{
		m_resolveDepthTextureMS_PS.Bind();

		dst.BindRenderTargetView();
		src.BindPS(TX_RESOLVE_DEPTH);

		D3DRenderer::Get().MakeFullpass();

		ShaderResourceView::ClearPS(TX_RESOLVE_DEPTH);
	}
	
	void PostProcess::applyFXAA(float width, float height, const ShaderResourceView& src, RenderTargetView& dst)
	{
		m_resolveFXAA_PS.Bind();

		cbFXAA* dstGPU = static_cast<cbFXAA*>(m_postprocessUniform.Map());
		cbFXAA upload;
		upload.imageSize = math::Vector4(width, height, 1.0f / width, 1.0f / height);
		upload.config = m_fxaaConfig;

		memcpy(static_cast<void*>(dstGPU), &upload, sizeof(cbFXAA));
		m_postprocessUniform.Unmap();

		m_postprocessUniform.BindPS(CB_FXAA_RESOLVE_SLOT);


		dst.BindRenderTargetView();
		src.BindPS(TX_LDR_FXAA_SLOT);

		D3DRenderer::Get().MakeFullpass();

		ShaderResourceView::ClearPS(TX_LDR_FXAA_SLOT);
	}


	constexpr uint8_t BLOOM_MODE_DOWNSAMPLE = 0;	//Used in bloomThreshold_CS.hlsl
	constexpr uint8_t BLOOM_MODE_UPSAMPLE = 1;		//Used in bloomThreshold_CS.hlsl
	constexpr uint8_t BLOOM_FLAG_GAUSSIAN_HORIZONTAL = 1;  //Used gaussianBlur_CS.hlsl,
	constexpr uint8_t BLOOM_FLAG_GAUSSIAN_COMPOSITE = 2;   //Used gaussianBlur_CS.hlsl,



	void PostProcess::resolveBloom_CoD_Siggraph(
		const ShaderResourceView& src,
		uint16_t width,
		uint16_t height,
		uint8_t numMips,
		const UnorderedAccessView* targetUAVs,
		const ShaderResourceView* targetSRVs)
	{
		//1st- Downsample src into MIP 0
//2nd- Gaussian MIP N, using MIP N-1 as SRV
//3nd- Mix MIP N, using MIP N+1 as SRV



//{
//	m_bloomDownsample_CS.bind();
//	cbBloom* map = static_cast<cbBloom*>(m_postprocessUniform.Map());

//	cbBloom gpuCnfg(width, height, m_bloomConfig, MODE_DOWNSAMPLE);
//	memcpy(map, &gpuCnfg, sizeof(cbBloom));
//	m_postprocessUniform.Unmap();

//	m_postprocessUniform.BindCS(cb_DS_BLOOM_HDR_SLOT);
//	src.BindCS(TX_HDR_RESOLVE_SLOT);
//	targetUAVs[0].BindCS(RW_DS_BLOOM_HDR_SLOT);

//	ComputeShader::dispatch((width + 7) / 8, (height + 7) / 8, 1);
//}




		m_bloomGaussian_CS.bind();



		//for (uint8_t N = 1; N < numMips; ++N)
		//{
		//	UnorderedAccessView::ClearCS(RW_DS_BLOOM_HDR_SLOT);

		//	if (N == 1)
		//	{
		//		targetSRVs[N - 1].BindCS(TX_HDR_RESOLVE_SLOT);
		//	}
		//	else
		//	{
		//		scrapSRVs[N - 1].BindCS(TX_HDR_RESOLVE_SLOT);
		//	}
		//	scrapUAVs[N].BindCS(RW_DS_BLOOM_HDR_SLOT);


		//	uint16_t mipWidth = static_cast<uint16_t>(std::max(1, width >> N));
		//	uint16_t mipHeight = static_cast<uint16_t>(std::max(1, height >> N));

		//	//HORIZONTAL PASS
		//	cbBloomGaussian gpuCnfg(mipWidth, mipHeight, MODE_GAUSSIAN_HORIZONTAL);

		//	{
		//		cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
		//		memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
		//		m_postprocessUniform.Unmap();
		//	}


		//	ComputeShader::dispatch((mipWidth + 128 - 1) / 128, mipHeight, 1);

		//	//VERTICAL PASS
		//	gpuCnfg.mode = MODE_GAUSSIAN_VERTICAL;

		//	UnorderedAccessView::ClearCS(RW_DS_BLOOM_HDR_SLOT);

		//	scrapSRVs[N].BindCS(TX_HDR_RESOLVE_SLOT);
		//	targetUAVs[N].BindCS(RW_DS_BLOOM_HDR_SLOT);

		//	{
		//		cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
		//		memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
		//		m_postprocessUniform.Unmap();
		//	}

		//	ComputeShader::dispatch(mipWidth, (mipHeight + 128 - 1) / 128, 1);
		//}

		//m_bloomDownsample_CS.bind();
		//for (uint8_t N = numMips - 1; N > 0 ; --N)
		//{
		//	UnorderedAccessView::ClearCS(RW_DS_BLOOM_HDR_SLOT);

		//	targetUAVs[N - 1].BindCS(RW_DS_BLOOM_HDR_SLOT);
		//	targetSRVs[N].BindCS(TX_HDR_RESOLVE_SLOT);

		//	uint16_t mipWidth = static_cast<uint16_t>(std::max(1, width >> (N - 1)));
		//	uint16_t mipHeight = static_cast<uint16_t>(std::max(1, height >> (N - 1)));

		//	cbBloomGaussian gpuCnfg(mipWidth, mipHeight, MODE_UPSAMPLE);

		//	cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
		//	memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
		//	m_postprocessUniform.Unmap();



		//	ComputeShader::dispatch((mipWidth + 7) / 8, (mipHeight + 7) / 8, 1);
		//}

		ShaderResourceView::ClearCS(TX_HDR_RESOLVE_SLOT);
		UnorderedAccessView::ClearCS(RW_DS_BLOOM_HDR_SLOT);
	}

	void PostProcess::resolveBloom_downsampleGaussian_upsampleBilinear(
		const ShaderResourceView& src,
		uint16_t width,
		uint16_t height,
		uint8_t numMips,
		const UnorderedAccessView* targetUAVs,
		const ShaderResourceView* targetSRVs,
		const UnorderedAccessView* gaussUAVs,
		const ShaderResourceView* gaussSRVs)
	{

	}

	void PostProcess::resolveBloom_downsampleCoD_upsampleBilinearGaussian(
		const ShaderResourceView& src,
		uint16_t width,
		uint16_t height,
		uint8_t numMips,
		const UnorderedAccessView* targetUAVs,
		const ShaderResourceView* targetSRVs,
		const UnorderedAccessView* downsampleUAVs,
		const ShaderResourceView* downsampleSRVs,
		const UnorderedAccessView* gaussUAVs,
		const ShaderResourceView* gaussSRVs)
	{

		m_bloomDownsample_CS.bind();
		for (uint8_t N = 0; N < numMips; ++N)
		{
			if (N == 0)
			{
				src.BindCS(SRV_PP_SLOT0);
			}
			else
			{
				downsampleSRVs[N - 1].BindCS(SRV_PP_SLOT0);
			}
			downsampleUAVs[N].BindCS(UAV_PP_COMPUTE_SLOT0);

			uint16_t mipWidth = static_cast<uint16_t>(std::max(1, width >> N));
			uint16_t mipHeight = static_cast<uint16_t>(std::max(1, height >> N));

			cbBloom* map = static_cast<cbBloom*>(m_postprocessUniform.Map());

			cbBloom gpuCnfg(mipWidth, mipHeight, m_bloomConfig, BLOOM_MODE_DOWNSAMPLE);
			memcpy(map, &gpuCnfg, sizeof(cbBloom));
			m_postprocessUniform.Unmap();

			m_postprocessUniform.BindCS(CB_PP_SLOT0);

			ComputeShader::dispatch((mipWidth + 7) / 8, (mipHeight + 7) / 8, 1);
			UnorderedAccessView::ClearCS(UAV_PP_COMPUTE_SLOT0);

		}

		m_bloomGaussian_CS.bind();


		for (uint8_t N = numMips - 1; N > 0; --N)
		{

			//1st horizontal pass, no composite
			if (N == numMips - 1)
			{
				downsampleSRVs[N].BindCS(SRV_PP_SLOT0);
				gaussUAVs[N - 1].BindCS(UAV_PP_COMPUTE_SLOT0);
			}
			else
			{
				targetSRVs[N].BindCS(SRV_PP_SLOT0);
				gaussUAVs[N - 1].BindCS(UAV_PP_COMPUTE_SLOT0);
			}



			uint16_t mipWidth = static_cast<uint16_t>(std::max(1, width >> (N - 1)));
			uint16_t mipHeight = static_cast<uint16_t>(std::max(1, height >> (N - 1)));

			//HORIZONTAL PASS
			uint32_t flags{ BLOOM_FLAG_GAUSSIAN_HORIZONTAL };
			cbBloomGaussian gpuCnfg(mipWidth, mipHeight, flags);

			{
				cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
				memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
				m_postprocessUniform.Unmap();
			}


			ComputeShader::dispatch((mipWidth + 128 - 1) / 128, mipHeight, 1);

			//VERTICAL PASS
			UnorderedAccessView::ClearCS(UAV_PP_COMPUTE_SLOT0);

			gpuCnfg.mode &= ~(BLOOM_FLAG_GAUSSIAN_HORIZONTAL);
			gpuCnfg.mode |= BLOOM_FLAG_GAUSSIAN_COMPOSITE;

			gaussSRVs[N - 1].BindCS(SRV_PP_SLOT0);
			downsampleSRVs[N - 1].BindCS(SRV_PP_SLOT1);
			targetUAVs[N - 1].BindCS(UAV_PP_COMPUTE_SLOT0);



			{
				cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
				memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
				m_postprocessUniform.Unmap();
			}

			ComputeShader::dispatch(mipWidth, (mipHeight + 128 - 1) / 128, 1);

			//Make one last pass to blur last mip
			if (N == 1)
			{
				UnorderedAccessView::ClearCS(UAV_PP_COMPUTE_SLOT0);

				targetSRVs[N - 1].BindCS(SRV_PP_SLOT0);
				gpuCnfg.mode = BLOOM_FLAG_GAUSSIAN_HORIZONTAL;
				gaussUAVs[N - 1].BindCS(UAV_PP_COMPUTE_SLOT0);

				{
					cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
					memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
					m_postprocessUniform.Unmap();
				}

				ComputeShader::dispatch((mipWidth + 128 - 1) / 128, mipHeight, 1);

				UnorderedAccessView::ClearCS(UAV_PP_COMPUTE_SLOT0);

				gpuCnfg.mode = 0;
				gaussSRVs[N - 1].BindCS(SRV_PP_SLOT0);
				targetUAVs[N - 1].BindCS(UAV_PP_COMPUTE_SLOT0);

				{
					cbBloomGaussian* map = static_cast<cbBloomGaussian*>(m_postprocessUniform.Map());
					memcpy(map, &gpuCnfg, sizeof(cbBloomGaussian));
					m_postprocessUniform.Unmap();
				}

				ComputeShader::dispatch(mipWidth, (mipHeight + 128 - 1) / 128, 1);
			}
			UnorderedAccessView::ClearCS(UAV_PP_COMPUTE_SLOT0);
		}
		ShaderResourceView::ClearCS(SRV_PP_SLOT0);
		ShaderResourceView::ClearCS(SRV_PP_SLOT1);


	}

	void PostProcess::resolveBloom_downsampleGaussian(
		const ShaderResourceView& src,
		uint16_t width,
		uint16_t height,
		uint8_t numMips,
		const UnorderedAccessView* targetUAVs,
		const ShaderResourceView* targetSRVs,
		const UnorderedAccessView* gaussUAVs,
		const ShaderResourceView* gaussSRVs)
	{

	}


	//TODO
	void PostProcess::resolveBloom_FFT_convPSF(
		const ShaderResourceView& src,
		const ShaderResourceView& psf,
		uint16_t width,
		uint16_t height,
		uint8_t numMips
		/**/)
	{

	}
}



