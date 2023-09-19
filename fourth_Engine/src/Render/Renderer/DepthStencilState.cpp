#include "pch.h"
#pragma once
#include "include/Render/Renderer/DepthStencilState.h"
#include "include/Render/Renderer/D3D11Headers.h"

namespace fth
{
	namespace renderer
	{
		extern ID3D11Device5* device;
		extern ID3D11DeviceContext4* devcon;


		DepthStencilState DepthStencilState::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dssDsc)
		{
			DepthStencilState dss;

			DX_HRCALL(device->CreateDepthStencilState(&dssDsc, dss.m_depthStencilState.reset()));

			return dss;
		}
		DepthStencilState DepthStencilState::CreateDepthStencilState(DSSTYPES rsType)
		{
			switch (rsType)
			{
			case DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED:
			{
				return CreateDepthStencilRWGreaterStencilDisabled();
				break;
			}
			case DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL:
			{
				return CreateDepthDisabledStencilRO_OP_Equal();
				break;
			}
			case DSSTYPES::DSS_DEPTH_RO_GREATEREQ_STENCIL_RO_OPEQUAL:
			{
				return CreateDepthROGreaterEqStencilRO_OP_Equal();
				break;
			}
			case DSSTYPES::DSS_DEPTH_RO_GREATER_STENCIL_DISABLED:
			{
				return CreateDepthROGreaterStencilDisabled();
				break;
			}
			case DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS:
			{
				return CreateDepthRWGreaterStencilRW_OP_Always();
				break;
			}
			case DSSTYPES::DISABLE:
			{
				return CreateDisabled();
				break;
			}
			case DSSTYPES::DEFAULT:
			default:
				break;
			}

			return CreateDefault();
		}

		DepthStencilState DepthStencilState::CreateDepthRWGreaterStencilRW_OP_Always()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			dsDesc.StencilEnable = TRUE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDepthStencilRWGreaterStencilDisabled()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
			dsDesc.StencilEnable = FALSE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDepthROGreaterEqStencilRO_OP_Equal()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
			dsDesc.StencilEnable = TRUE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDepthROGreaterStencilDisabled()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
			dsDesc.StencilEnable = FALSE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDepthDisabledStencilRO_OP_Equal()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = FALSE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
			dsDesc.StencilEnable = TRUE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDefault()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
			dsDesc.StencilEnable = FALSE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}

		DepthStencilState DepthStencilState::CreateDisabled()
		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;

			dsDesc.DepthEnable = FALSE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.StencilEnable = FALSE;
			dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

			return CreateDepthStencilState(dsDesc);
		}


		void DepthStencilState::BindState(StencilValues refStencil)
		{
			if (m_depthStencilState.valid())
			{
				DX_CALL(devcon->OMSetDepthStencilState(m_depthStencilState, refStencil));
			}
		}
	}
}
