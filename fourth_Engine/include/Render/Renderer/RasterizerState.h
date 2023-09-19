#pragma once

struct ID3D11RasterizerState1;
struct D3D11_RASTERIZER_DESC1;

namespace fth
{
	namespace renderer
	{
		enum class RSTYPES
		{
			RSTYPE_NOCULL_WIREFRAME = 0,
			RSTYPE_NOCULL_SOLID = 1,
			RSTYPE_DEFAULT_DEPTH_BIAS = 2,
			DEFAULT,
			NUM
		};

		class RasterizerState
		{
		public:
			static RasterizerState CreateRasterizerState(RSTYPES rsType);

			void BindState();

		private:
			static RasterizerState CreateRasterizerState(const D3D11_RASTERIZER_DESC1& rsDsc);
			static RasterizerState CreateDefault();
			static RasterizerState CreateNoCullWireframe();
			static RasterizerState CreateNoCullSolid();
			static RasterizerState CreateDefaultDepthBias();

			DxResPtr<ID3D11RasterizerState1>      m_rasterizerState;

		};
	}
}