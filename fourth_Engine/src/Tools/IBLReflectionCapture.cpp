#include "pch.h"

#pragma once
#include "include/Tools/IBLReflectionCapture.h"
#include "include/Managers/TextureManager.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "Shaders/RegisterSlots.hlsli"

namespace fth::ibl
{

	void UpdateViewport(uint32_t width, uint32_t height)
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.Width =  static_cast<float>(width);
		viewport.Height = static_cast<float>(height);

		s_devcon->RSSetViewports(1, &viewport);

	}

	//We could just use outWidth and outHeight, but it feels that the intentions are clearer if you have to specify initial values
	void ComputeMipLevelSize(uint32_t width, uint32_t height, uint16_t level, uint32_t& outWidth, uint32_t& outHeight)
	{
		if (level == 0)
		{
			outWidth = width;
			outHeight = height;
			return;
		}

		outWidth  =  static_cast<uint32_t>(std::floor(static_cast<float>(width) / 2.0f));
		outHeight =  static_cast<uint32_t>(std::floor(static_cast<float>(height) / 2.0f));
		if (outWidth == 1 || outHeight == 1)
		{
			return;
		}

		ComputeMipLevelSize(outWidth, outHeight, level - 1, outWidth, outHeight);
	}

	void IBLReflectionCapture::Init()
	{
		m_cubemapGenVS.LoadShader(L"IBLSampleCubemap_VS.cso", nullptr, 0);
		m_diffuseIrradiancePS.LoadShader(L"IBLDiffuseIrradiance_PS.cso");
		m_specularIrradiancePS.LoadShader(L"IBLSpecularIrradiance_PS.cso");
		m_specularReflectancePS.LoadShader(L"IBLSpecularReflectance_PS.cso");
		m_faceSelectorGS.LoadShader(L"IBLCubemapFaceSelector_GS.cso");

		m_uniform.CreateGPUBuffer(sizeof(UniformData), 1, nullptr);
	}
	//Face slices
//0 : +X
//1 : -X
//2 : +Y
//3 : -Y
//4 : +Z
//5 : -Z
	void IBLReflectionCapture::GenerateCubemapDiffuseIrradiance(const Texture& src, std::string_view filePath, uint32_t width, uint32_t height, uint32_t samples, Texture& dst)
	{
		TextureDsc txDsc;
		txDsc.width = width;
		txDsc.height = height;
		txDsc.depth = 0;
		txDsc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		txDsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txDsc.numTextures = 6;
		txDsc.arraySize = 6;
		txDsc.numMips = 1;
		txDsc.isCubemap = true;

		dst.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET);
		Capture(src, DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, samples, m_diffuseIrradiancePS, dst);

		TextureManager::Get().ExportTexture(dst, TextureManager::FileFormat::BC6H_UF16, std::string(filePath), false);
	}

	void IBLReflectionCapture::GenerateCubemapSpecularIrradiance(const Texture& src, std::string_view filePath, uint32_t width, uint32_t height, uint32_t samples, Texture& dst)
	{
		TextureDsc txDsc;
		txDsc.width = width;
		txDsc.height = height;
		txDsc.depth = 0;
		txDsc.format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		txDsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txDsc.numTextures = 6;
		txDsc.arraySize = 6;
		txDsc.numMips = static_cast<uint16_t>(1.0f + std::ceilf(std::fmaxf(std::log2f((float)width), std::log2f((float)height))));
		txDsc.isCubemap = true;

		dst.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET);
		Capture(src, DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, txDsc.numMips, samples, m_specularIrradiancePS, dst);

		TextureManager::Get().ExportTexture(dst, TextureManager::FileFormat::BC6H_UF16, std::string(filePath), false);

	}

	void IBLReflectionCapture::GenerateGGXSpecularReflectanceLUT(uint32_t width, std::string_view filePath, uint32_t height, uint32_t samples, Texture& dst)
	{
		TextureDsc txDsc;
		txDsc.width = width;
		txDsc.height = height;
		txDsc.depth = 0;
		txDsc.format = DXGI_FORMAT_R16G16_FLOAT;
		txDsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txDsc.numTextures = 1;
		txDsc.arraySize = 1;
		txDsc.numMips = 1;
		txDsc.isCubemap = false;

		dst.Init(txDsc, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET);
		
		m_cubemapGenVS.Bind();
		m_specularReflectancePS.Bind();
		renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DEFAULT);
		renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_NOCULL_SOLID);

		DxResPtr<ID3D11RenderTargetView> RTV;
		D3D11_RENDER_TARGET_VIEW_DESC rtvDsc;
		rtvDsc.Format = DXGI_FORMAT_R16G16_FLOAT;
		rtvDsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDsc.Texture2D.MipSlice = 0;
		DX_CALL(s_device->CreateRenderTargetView(dst.GetResource(), &rtvDsc, RTV.reset()));
		UpdateViewport(width, height);

		{
			UniformData* map = static_cast<UniformData*>(m_uniform.Map());
			map[0] = { samples };
			m_uniform.Unmap();
			m_uniform.BindPS(IBL_GEN_SLOT);
		}
		renderer::D3DRenderer::Get().SetRenderTargets(RTV, 1);
		renderer::D3DRenderer::Get().MakeFullpass();


		TextureManager::Get().ExportTexture(dst, TextureManager::FileFormat::BC5_UNORM, std::string(filePath), false);
	}

	void IBLReflectionCapture::Capture(const Texture& src, DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mips, uint32_t samples, const renderer::PixelShader& targetPS, Texture& dst)
	{
		{
			m_cubemapGenVS.Bind();
			m_faceSelectorGS.Bind();
			targetPS.Bind();
			src.BindPS(TX_ALBEDO_SLOT);

			renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DEFAULT);
			renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_NOCULL_SOLID);
		}


		DxResPtr<ID3D11RenderTargetView> RTV;
		D3D11_RENDER_TARGET_VIEW_DESC rtvDsc;
		rtvDsc.Format = format;
		rtvDsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDsc.Texture2DArray.ArraySize = 6;
		rtvDsc.Texture2DArray.FirstArraySlice = 0;

		for (uint16_t level = 0; level < mips; ++level)
		{
			rtvDsc.Texture2DArray.MipSlice = level;
			DX_CALL(s_device->CreateRenderTargetView(dst.GetResource(), &rtvDsc, RTV.reset()));


			uint32_t mipWidth;
			uint32_t mipHeight;
			ComputeMipLevelSize(width, height, level, mipWidth, mipHeight);
			UpdateViewport(mipWidth, mipHeight);

			{
				float roughness = std::fmaxf(static_cast<float>(level) / (static_cast<float>(mips) - 1.0f), 0.001f);
				UniformData* map = static_cast<UniformData*>(m_uniform.Map());
				map[0] = { samples, roughness * roughness };
				m_uniform.Unmap();
				m_uniform.BindPS(IBL_GEN_SLOT);
			}

			renderer::D3DRenderer::Get().SetRenderTargets(RTV, 1);
			renderer::D3DRenderer::Get().MakeFullpass();
		}
		renderer::D3DRenderer::Get().ClearGeometryStage();
	}


}