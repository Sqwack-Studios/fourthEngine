#include "pch.h"
#pragma once
#include "include/Render/Skybox.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Managers/TextureManager.h"
#include "Shaders/RegisterSlots.hlsli"

namespace fth
{
	void Skybox::Init()
	{
		m_vertexShader.LoadShader(L"Skybox_VS.cso", nullptr, 0);
		m_pixelShader.LoadShader(L"Skybox_PS.cso");
		m_RS = renderer::RSTYPES::RSTYPE_NOCULL_SOLID;
		m_DSS = renderer::DSSTYPES::DSS_DEPTH_DISABLE_STENCIL_RO_OPEQUAL;
	}

	void Skybox::Draw()
	{
		if(!m_cubemap)
			m_cubemap = TextureManager::DefaultCubemap();

		m_vertexShader.Bind();
		m_pixelShader.Bind();
		m_cubemap->BindPS(TX_SKYMAP_SLOT);
		//Set Readonly DepthTest. Set comparison to greater equal so we can place skybox exactly at 1;
		renderer::D3DRenderer::Get().BindDepthStencilState(m_DSS, renderer::StencilValues::SKY);

		if (!renderer::D3DRenderer::Get().WireframeEnabled())
		{
			renderer::D3DRenderer::Get().BindRasterizerState(m_RS);
		}
		else
		{
			renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_NOCULL_WIREFRAME);
		}
		
		renderer::D3DRenderer::Get().MakeFullpass();
	}

	void Skybox::SetCubemap(std::string_view cubemapName)
	{
		m_cubemap = TextureManager::Get().FindCubemap((std::string)cubemapName);
	}
}
