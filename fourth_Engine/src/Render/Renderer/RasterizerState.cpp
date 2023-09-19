#include "pch.h"
#pragma once
#include "include/Render/Renderer/RasterizerState.h"
#include "include/Render/Renderer/D3D11Headers.h"

namespace fth
{
	namespace renderer
	{
		extern ID3D11Device5* device;
		extern ID3D11DeviceContext4* devcon;


		RasterizerState RasterizerState::CreateRasterizerState(const D3D11_RASTERIZER_DESC1& rsDsc)
		{
			RasterizerState rs;
			
			DX_HRCALL(device->CreateRasterizerState1(&rsDsc, rs.m_rasterizerState.reset()));

			return rs;
		}

		RasterizerState RasterizerState::CreateDefault()
		{
			D3D11_RASTERIZER_DESC1 rsDsc;

			rsDsc.FillMode = D3D11_FILL_SOLID;
			rsDsc.CullMode = D3D11_CULL_BACK;
			rsDsc.FrontCounterClockwise = false;
			rsDsc.DepthBias = 0;
			rsDsc.DepthBiasClamp = 0;
			rsDsc.SlopeScaledDepthBias = 0;
			rsDsc.DepthClipEnable = true;
			rsDsc.ScissorEnable = false;
			rsDsc.MultisampleEnable = false;
			rsDsc.AntialiasedLineEnable = false;
			rsDsc.ForcedSampleCount = 0;

			return CreateRasterizerState(rsDsc);
		}

		RasterizerState RasterizerState::CreateNoCullWireframe()
		{
			D3D11_RASTERIZER_DESC1 rsDsc;

			rsDsc.FillMode = D3D11_FILL_WIREFRAME;
			rsDsc.CullMode = D3D11_CULL_NONE;
			rsDsc.FrontCounterClockwise = false;
			rsDsc.DepthBias = 0;
			rsDsc.DepthBiasClamp = 0;
			rsDsc.SlopeScaledDepthBias = 0;
			rsDsc.DepthClipEnable = true;
			rsDsc.ScissorEnable = false;
			rsDsc.MultisampleEnable = false;
			rsDsc.AntialiasedLineEnable = false;
			rsDsc.ForcedSampleCount = 0;

			return CreateRasterizerState(rsDsc);
		}

		RasterizerState RasterizerState::CreateNoCullSolid()
		{
			D3D11_RASTERIZER_DESC1 rsDsc;

			rsDsc.FillMode = D3D11_FILL_SOLID;
			rsDsc.CullMode = D3D11_CULL_NONE;
			rsDsc.FrontCounterClockwise = false;
			rsDsc.DepthBias = 0;
			rsDsc.DepthBiasClamp = 0;
			rsDsc.SlopeScaledDepthBias = 0;
			rsDsc.DepthClipEnable = true;
			rsDsc.ScissorEnable = false;
			rsDsc.MultisampleEnable = false;
			rsDsc.AntialiasedLineEnable = false;
			rsDsc.ForcedSampleCount = 0; 

			return CreateRasterizerState(rsDsc);
		}

		RasterizerState RasterizerState::CreateDefaultDepthBias()
		{
			D3D11_RASTERIZER_DESC1 rsDsc;

			rsDsc.FillMode = D3D11_FILL_SOLID;
			rsDsc.CullMode = D3D11_CULL_BACK;
			rsDsc.FrontCounterClockwise = false;
			//
			rsDsc.DepthBias = -1.0f;
			rsDsc.DepthBiasClamp = 0.0f;
			rsDsc.SlopeScaledDepthBias = -1.0f;
			//
			rsDsc.DepthClipEnable = true;
			rsDsc.ScissorEnable = false;
			rsDsc.MultisampleEnable = false;
			rsDsc.AntialiasedLineEnable = false;
			rsDsc.ForcedSampleCount = 0;

			return CreateRasterizerState(rsDsc);
		}

		RasterizerState RasterizerState::CreateRasterizerState(RSTYPES rsType)
		{
			switch (rsType)
			{
			case RSTYPES::RSTYPE_NOCULL_WIREFRAME:
				return CreateNoCullWireframe();
			case RSTYPES::RSTYPE_NOCULL_SOLID:
				return CreateNoCullSolid();
			case RSTYPES::RSTYPE_DEFAULT_DEPTH_BIAS:
				return CreateDefaultDepthBias();
			}
			return CreateDefault();

		}

		void RasterizerState::BindState()
		{
			if (m_rasterizerState.valid())
			{
				DX_CALL(devcon->RSSetState(m_rasterizerState));
			}
		}
	}
}