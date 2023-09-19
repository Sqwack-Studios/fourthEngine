#pragma once
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ComputeShader.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/D3DView.h"


struct ID3D11DeviceContext4;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;


namespace fth::renderer
{


	class PostProcess
	{
	public:
		static constexpr float defaultEV100 = 0.0f;
		struct FXAAConfig;
		struct BloomConfig;


		static PostProcess& Get()
		{
			static PostProcess instance;
			return instance;
		}

		void Init();
		void Resolve(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst, const ShaderResourceView* bloom);
		void ResolveMSDepthTexture(const ShaderResourceView& src, const RenderTargetView& dst);
		void applyFXAA(float width, float height, const ShaderResourceView& src, RenderTargetView& dst);

		float SetEV100(float ev100) { EV100 = ev100; return EV100; };
		float GetEV100() const{ return EV100; }

		const FXAAConfig& getFXAAConfiguration() const{ return m_fxaaConfig; }
		void setFXAAConfiguration(const FXAAConfig& config) { m_fxaaConfig = config; }

		BloomConfig getBloomConfiguration() const { return m_bloomConfig; }
		void setBloomConfiguration(BloomConfig config) { m_bloomConfig = config;}

		void resolveBloom_CoD_Siggraph(
			const ShaderResourceView& src,
			uint16_t width,
			uint16_t height,
			uint8_t numMips,
			const UnorderedAccessView* targetUAVs,
			const ShaderResourceView* targetSRVs);

		void resolveBloom_downsampleGaussian_upsampleBilinear(
			const ShaderResourceView& src,
			uint16_t width,
			uint16_t height,
			uint8_t numMips,
			const UnorderedAccessView* targetUAVs,
			const ShaderResourceView*  targetSRVs,
			const UnorderedAccessView* gaussUAVs,
			const ShaderResourceView*  gaussSRVs);

		void resolveBloom_downsampleCoD_upsampleBilinearGaussian(
			const ShaderResourceView& src,
			uint16_t width,
			uint16_t height,
			uint8_t numMips,
			const UnorderedAccessView* targetUAVs,
			const ShaderResourceView* targetSRVs,
			const UnorderedAccessView* downsampleUAVs,
			const ShaderResourceView*  downsampleSRVs,
			const UnorderedAccessView* gaussUAVs,
			const ShaderResourceView* gaussSRVs);

		void resolveBloom_downsampleGaussian(
			const ShaderResourceView& src,
			uint16_t width,
			uint16_t height,
			uint8_t numMips,
			const UnorderedAccessView* targetUAVs,
			const ShaderResourceView* targetSRVs,
			const UnorderedAccessView* gaussUAVs,
			const ShaderResourceView* gaussSRVs);


		void resolveBloom_FFT_convPSF(
			const ShaderResourceView& src,
			const ShaderResourceView& psf,
			uint16_t width,
			uint16_t height,
			uint8_t numMips
			//
			);
		static constexpr float fxaa_qualitySubpixel_default = 0.75f;
		static constexpr float fxaa_qualitySubpixel_lower   = 0.0f;
		static constexpr float fxaa_qualitySubpixel_upper   = 1.0f;

		static constexpr float fxaa_qualityEdgeThreshold_default = 0.166f;
		static constexpr float fxaa_qualityEdgeThreshold_lower = 0.063f;
		static constexpr float fxaa_qualityEdgeThreshold_upper = 0.333f;

		static constexpr float fxaa_qualityEdgeThresholdMin_default = 0.0833f;
		static constexpr float fxaa_qualityEdgeThresholdMin_lower = 0.0312f;
		static constexpr float fxaa_qualityEdgeThresholdMin_upper = 0.0833f;

		static constexpr uint32_t POSTPROCESS_CBUFFER_SIZE = sizeof(float) * 16;

		struct FXAAConfig
		{
			static const FXAAConfig DEFAULT;

			float                qualitySubpixel;
			float                qualityEdgeThreshold;
			float                qualityEdgeThresholdMin;
		};

		static constexpr float bloom_lThreshold = 1.0f;
		static constexpr float bloom_uThreshold = 10.0f;

		struct BloomConfig
		{
			static const BloomConfig  DEFAULT;
			float lowerThresholdFactor;
			float upperThresholdFactor;
		};
	private:
		struct cbFXAA
		{
			math::Vector4   imageSize;// .xy = image_size, .zw = 1.0 / image_size

			FXAAConfig      config;
			float           pad[1];

		};

		struct cbBloom
		{
			cbBloom(uint16_t width, uint16_t height, BloomConfig cnfg, uint32_t mode):
				invResolution(1.0f/width, 1.0f/height),
				thresholdFactors(cnfg.lowerThresholdFactor, cnfg.upperThresholdFactor),
				mode(mode){}
			
			math::Vector2  invResolution;
			math::Vector2  thresholdFactors;
			
			uint32_t       mode;
			float          pad[3];
		};

		struct cbBloomGaussian
		{
			cbBloomGaussian(uint16_t width, uint16_t height, uint32_t mode) :
				resolution(static_cast<float>(width), static_cast<float>(height), 1.0f/width, 1.0f/height),
				mode(mode) {}
			
			math::Vector4 resolution;

			uint32_t      mode;
			float         pad[3];
		};
		PixelShader          m_resolveDepthTextureMS_PS;
		PixelShader          m_resolvePS;
		PixelShader          m_resolveFXAA_PS;
		ComputeShader        m_bloomDownsample_CS;
		ComputeShader        m_bloomGaussian_CS;
		UniformBuffer        m_postprocessUniform;

		BloomConfig          m_bloomConfig;

		//RESOLVE HDR
		float                EV100;
		//RESOLVE FXAA
		FXAAConfig           m_fxaaConfig;
	};
}