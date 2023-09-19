#pragma once

struct ID3D11DepthStencilState;
struct D3D11_DEPTH_STENCIL_DESC;
namespace fth
{
	namespace renderer
	{
		enum StencilValues : uint8_t
		{
			SKY = 0,
			PBR_SHADOWS,
			NO_LIGHTNING
		};

		enum class DSSTYPES
		{
			DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS = 0,
			DSS_DEPTH_RW_GREATER_STENCIL_DISABLED,
			DSS_DEPTH_RO_GREATER_STENCIL_DISABLED,
			DSS_DEPTH_RO_GREATEREQ_STENCIL_RO_OPEQUAL,
			DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL,
			DEFAULT,
			DISABLE,
			NUM
		};
		class DepthStencilState
		{
		public:
			static DepthStencilState CreateDepthStencilState(DSSTYPES rsType);

			void BindState(StencilValues refStencil);

		private:

			static DepthStencilState CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& rsDsc);
			static DepthStencilState CreateDepthStencilRWGreaterStencilDisabled();
			static DepthStencilState CreateDepthRWGreaterStencilRW_OP_Always();
			static DepthStencilState CreateDepthROGreaterEqStencilRO_OP_Equal();
			static DepthStencilState CreateDepthROGreaterStencilDisabled();

			static DepthStencilState CreateDepthDisabledStencilRO_OP_Equal();
			static DepthStencilState CreateDefault();
			static DepthStencilState CreateDisabled();


			DxResPtr<ID3D11DepthStencilState>      m_depthStencilState;

		};
	}
}