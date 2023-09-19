#include "pch.h"
#pragma once
#include "include/Render/Texture.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "vendor/DirectXTex/Include/DirectXTex.h"

namespace fth
{
	extern ID3D11Device5* s_device;
	extern ID3D11DeviceContext4* s_devcon;

	bool TextureDsc::operator==(const TextureDsc& rval)
	{
		return width     == rval.width     && height       == rval.height       && depth        == rval.depth  &&
			   format    == rval.format    && dimension    == rval.dimension    && numTextures  == rval.numTextures  && arraySize == rval.arraySize &&
			   numMips   == rval.numMips   && multisamples == rval.multisamples && msQuality    == rval.msQuality    && isCubemap == rval.isCubemap;
	}

	bool TextureDsc::operator!=(const TextureDsc& rval)
	{
		return !(* this == rval);
	}

	void Texture::Init(const TextureDsc& txDsc, D3D11_USAGE usage, uint32_t bindFlag, const DirectX::Image* data, uint32_t cpuFlag, uint32_t miscFlag)
	{
		//if there's data, call directX method, which will create subresource arrays and create the textures
		if (data)
		{
			InitTextureFromInitialData(txDsc, usage, bindFlag, data, cpuFlag, miscFlag);
			return;
		}
		//if there's no data and usage is non-staging, manually create.
		if (usage == D3D11_USAGE_STAGING)
		{
			BREAK_AND_LOG_ENGINE("CreateTextureRes", "Tried to create a staging resource with NULL data");
		}


		switch (txDsc.dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc{};
			desc.Width = static_cast<UINT>(txDsc.width);
			desc.MipLevels = static_cast<UINT>(txDsc.numMips);
			desc.ArraySize = static_cast<UINT>(txDsc.arraySize);
			desc.Usage = usage;
			desc.BindFlags = static_cast<UINT>(bindFlag);
			desc.CPUAccessFlags = static_cast<UINT>(cpuFlag);
			desc.MiscFlags = miscFlag & ~static_cast<uint32_t>(D3D11_RESOURCE_MISC_TEXTURECUBE);

			DX_CALL(s_device->CreateTexture1D(&desc, nullptr, reinterpret_cast<ID3D11Texture1D**>(GetResource().reset())));
		}
		break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = static_cast<UINT>(txDsc.width);
			desc.Height = static_cast<UINT>(txDsc.height);
			desc.MipLevels = static_cast<UINT>(txDsc.numMips);
			desc.ArraySize = static_cast<UINT>(txDsc.arraySize);
			desc.Format = txDsc.format;
			desc.SampleDesc.Count = txDsc.multisamples;
			desc.SampleDesc.Quality = txDsc.msQuality;
			desc.Usage = usage;
			desc.BindFlags = bindFlag;
			desc.CPUAccessFlags = cpuFlag;
			desc.MiscFlags = miscFlag | (txDsc.isCubemap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0);

			DX_CALL(s_device->CreateTexture2D(&desc, nullptr, reinterpret_cast<ID3D11Texture2D**>(GetResource().reset())));
		}
		break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc{};
			desc.Width = static_cast<UINT>(txDsc.width);
			desc.Height = static_cast<UINT>(txDsc.height);
			desc.Depth = static_cast<UINT>(txDsc.depth);
			desc.MipLevels = static_cast<UINT>(txDsc.numMips);
			desc.Format = txDsc.format;
			desc.Usage = usage;
			desc.BindFlags = bindFlag;
			desc.CPUAccessFlags = cpuFlag;
			desc.MiscFlags = miscFlag & ~static_cast<uint32_t>(D3D11_RESOURCE_MISC_TEXTURECUBE);

			DX_CALL(s_device->CreateTexture3D(&desc, nullptr, reinterpret_cast<ID3D11Texture3D**>(GetResource().reset())));
		}
		break;
		}
		SetDesc(txDsc);
	}

	void Texture::InitTextureFromInitialData(const TextureDsc& txDsc, D3D11_USAGE usage, uint32_t bindFlag, const DirectX::Image* data, uint32_t cpuFlag, uint32_t miscFlag)
	{
		const DirectX::TexMetadata meta{
	     txDsc.width,
	     txDsc.height,
	     txDsc.depth,
	     txDsc.arraySize,
	     txDsc.numMips,
	     txDsc.isCubemap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0,
	     0,
	     txDsc.format,
	     static_cast<DirectX::TEX_DIMENSION>(txDsc.dimension) };

		DX_CALL(DirectX::CreateTextureEx(s_device, data, txDsc.arraySize * txDsc.numMips, meta, usage, bindFlag, cpuFlag, miscFlag, DirectX::CREATETEX_DEFAULT, GetResource().reset()));
		SetDesc(txDsc);
	}

	
	void Texture::MakeShaderResource(DXGI_FORMAT format)
	{
		if (GetDesc().isCubemap)
		{
			m_srv.CreateShaderResourceViewCubemap(m_d3dTexture, format, 0, GetDesc().numMips, 0, GetDesc().numTextures);
		}
		else if (Multisamples() > 1)
		{
			m_srv.CreateShaderResourceView2D_MS(m_d3dTexture, format, 0, GetDesc().arraySize);
		}
		else
		{
			m_srv.CreateShaderResourceView(m_d3dTexture, format, GetDesc().dimension, 0, GetDesc().numMips, 0, GetDesc().arraySize);
		}
	}

	void Texture::BindPS(uint16_t slot) const
	{
		m_srv.BindPS(slot);
	}

	void Texture::BindVS(uint16_t slot) const
	{
		m_srv.BindVS(slot);
	}

	void Texture::MakeCopy(Texture& dst) const
	{
		DX_CALL(s_devcon->CopyResource(dst.GetResource(), this->GetResource()));
	}

	void Texture::MakeCopyNonMS(Texture& dst) const
	{
		if (m_texDsc.multisamples == 1)
		{
			return MakeCopy(dst);
			
		}

		//if we got here, source texture has to be multisampled, and we are resolving into a non multisampled target
		if (Multisamples() > 1)
		{
			for (UINT item = 0; item < m_texDsc.arraySize; ++item)
			{
				for (UINT level = 0; level < m_texDsc.numMips; ++level)
				{
					const UINT index = D3D11CalcSubresource(level, item, m_texDsc.numMips);
					DX_CALL(s_devcon->ResolveSubresource(dst.GetResource(), index, this->GetResource(), index, m_texDsc.format));
				}
			}
		}
	}

	void Texture::MakeCopyDepthNonMS(Texture& dst, const renderer::RenderTargetView& rtv) const
	{
		if (m_texDsc.multisamples == 1)
		{
			return MakeCopy(dst);
		}


		renderer::D3DRenderer::Get().resolveMSDepthCopy(GetSRV(), rtv);

	}


	
	void Texture::ClearPS(uint16_t slot)
	{
		renderer::ShaderResourceView::ClearPS(slot);
	}

	void Texture::ClearVS(uint16_t slot)
	{
		renderer::ShaderResourceView::ClearVS(slot);
	}



}

