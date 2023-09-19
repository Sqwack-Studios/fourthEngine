#pragma once

struct ID3D11BlendState;
struct D3D11_BLEND_DESC;

namespace fth::renderer
{
	enum class BSTYPES
	{
		DEFAULT_A2C = 0,
		//COLOR: src * srcAlpha + dst * invSrcAlpha; ALPHA: src * 1 + dst * invSrc
		GENERAL_TRANSLUCENCY = 1,
		ADDITIVE_TRANSLUCENCY,
		GENERAL_TRANSLUCENCY_ALBEDO_ROUGHMETAL_EMISSION,
		DEFAULT,
		NUM
	};

	class BlendState
	{
	public:
		static BlendState CreateBlendState(BSTYPES bsType);
		
		void BindState(const float* blendFactor, uint32_t mask);

	private:

		static BlendState CreateBlendState(const D3D11_BLEND_DESC& bsDsc);
		static BlendState CreateDefaultBlendState();
		static BlendState CreateDefaultA2C();
		static BlendState CreateGeneralTranslucency();
		static BlendState CreateAdditiveTranslucency();
		static BlendState CreateGeneralTranslucencyGBuffer();
		DxResPtr<ID3D11BlendState>   m_blendState;
	};
}