#include "pch.h"

#pragma once
#include "include/Render/Renderer/Grid.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	namespace renderer
	{
		void Grid::Init()
		{
			m_gridVS.LoadShader(L"InfiniteGrid_VS.cso", nullptr, 0);
			m_gridPS.LoadShader(L"InfiniteGrid_PS.cso");
			m_RS = RSTYPES::DEFAULT;
			m_DSS = DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED;
			m_BSS = BSTYPES::GENERAL_TRANSLUCENCY;
			//Add blend states
		}

		void Grid::Draw()
		{
			m_gridVS.Bind();
			m_gridPS.Bind();

			D3DRenderer::Get().BindBlendState(m_BSS);

			renderer::D3DRenderer::Get().BindDepthStencilState(m_DSS);

			if (!renderer::D3DRenderer::Get().WireframeEnabled())
			{
				renderer::D3DRenderer::Get().BindRasterizerState(m_RS);
			}
			else
			{
				renderer::D3DRenderer::Get().BindRasterizerState(RSTYPES::RSTYPE_NOCULL_WIREFRAME);
			}

			DX_CALL(devcon->Draw(6, 0));

			//I think it would make more sense if each shading group binds their necessary BS
			D3DRenderer::Get().BindBlendState(BSTYPES::DEFAULT);
		}

		void Grid::Shutdown()
		{
			m_gridVS.Reset();
			m_gridPS.Reset();
		}
	}
}