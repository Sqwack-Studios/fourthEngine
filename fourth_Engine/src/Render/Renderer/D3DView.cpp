#include "pch.h"
#pragma once
#include "include/Render/Renderer/D3DView.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include <dxgiformat.h>

namespace fth
{
	extern ID3D11Device5* s_device;
	extern ID3D11DeviceContext4* s_devcon;
}

namespace fth::renderer
{
	void RenderTargetView::CreateRenderTarget(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t mipLevel, uint16_t txSlice, uint16_t arraySize)
	{
		D3D11_RENDER_TARGET_VIEW_DESC dsc;

		dsc.Format = fmt;
		bool isArray{ arraySize > 1 };

		//Technically it's an union, we could just paste inputs and specify dimension
		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY,
				dsc.Texture1DArray.MipSlice = mipLevel,
				dsc.Texture1DArray.FirstArraySlice = txSlice,
				dsc.Texture1DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D,
				dsc.Texture1D.MipSlice = mipLevel;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
				dsc.Texture2DArray.MipSlice = mipLevel,
				dsc.Texture2DArray.FirstArraySlice = txSlice,
				dsc.Texture2DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
				dsc.Texture2D.MipSlice = mipLevel;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
			dsc.Texture3D.MipSlice = mipLevel;
			dsc.Texture3D.FirstWSlice = txSlice;
			dsc.Texture3D.WSize = arraySize;
			break;
		}
		default:
			LOG_ENGINE_WARN("D3DView", "Unknow dimension passed to create RTV");
			break;
		}
		DX_CALL(s_device->CreateRenderTargetView(res.ptr(), &dsc, reinterpret_cast<ID3D11RenderTargetView**>(m_view.reset())));
	}
	void RenderTargetView::CreateRenderTarget2D_MS(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t txSlice, uint16_t arraySize)
	{
		D3D11_RENDER_TARGET_VIEW_DESC dsc;

		dsc.Format = fmt;
		bool isArray{ arraySize > 1 };

		if (isArray)
		{
			dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
			dsc.Texture2DMSArray.FirstArraySlice = txSlice;
			dsc.Texture2DMSArray.ArraySize = arraySize;
		}
		else
		{
			dsc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		}
		DX_CALL(s_device->CreateRenderTargetView(res.ptr(), &dsc, reinterpret_cast<ID3D11RenderTargetView**>(m_view.reset())));

	}
	void RenderTargetView::CreateRenderTargetBuffer(const DxResPtr<ID3D11Resource>& res, ViewBufferDsc bufferDsc)
	{
		D3D11_RENDER_TARGET_VIEW_DESC dsc;
		dsc.Buffer.ElementOffset = bufferDsc.elementOffset;
		dsc.Buffer.ElementWidth = bufferDsc.elementWidth;

		DX_CALL(s_device->CreateRenderTargetView(res.ptr(), &dsc, reinterpret_cast<ID3D11RenderTargetView**>(m_view.reset())));
	}

	void RenderTargetView::Clear(const math::Color& clear)
	{
		DX_CALL(s_devcon->ClearRenderTargetView(getView(), clear));
	}


	void RenderTargetView::BindRenderTargetView(DepthStencilView* dsv) const
	{
		if (dsv && dsv->getView().valid())
		{
			DX_CALL(s_devcon->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(&m_view), dsv->getView()));
		}
		else
		{
			DX_CALL(s_devcon->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(&m_view), nullptr));
		}
	}

	void RenderTargetView::BindRenderTargetViews(RenderTargetView * const rtv, uint16_t numRTV, DepthStencilView* dsv)
	{
		if (dsv && dsv->getView().valid())
		{
			DX_CALL(s_devcon->OMSetRenderTargets(numRTV, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv->getView()), dsv->getView()));
		}
		else
		{
			DX_CALL(s_devcon->OMSetRenderTargets(numRTV, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv->getView()), nullptr));
		}
	}


	void DepthStencilView::CreateDepthStencilTarget(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t mipLevel, uint16_t arraySlice, uint16_t arraySize, uint16_t flags)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsc;
		dsc.Format = fmt;
		dsc.Flags = flags;

		bool isArray{arraySize > 1};

		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY,
				dsc.Texture1DArray.MipSlice = mipLevel,
				dsc.Texture1DArray.FirstArraySlice = arraySlice,
				dsc.Texture1DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D,
				dsc.Texture1D.MipSlice = mipLevel;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
				dsc.Texture2DArray.MipSlice = mipLevel,
				dsc.Texture2DArray.FirstArraySlice = arraySlice,
				dsc.Texture2DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
				dsc.Texture2D.MipSlice = mipLevel;
			break;
		}
		default:
			LOG_ENGINE_WARN("D3DView", "Unknow dimension passed to create DSV");
			break;
		}
		
		DX_CALL(s_device->CreateDepthStencilView(res.ptr(), &dsc, reinterpret_cast<ID3D11DepthStencilView**>(m_view.reset())));

	}
	void  DepthStencilView::CreateDepthStencilTarget2D_MS(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t txSlice, uint16_t arraySize, uint16_t flags)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsc;
		dsc.Flags = flags;
		dsc.Format = fmt;
		
		bool isArray{ arraySize > 1 };
		if (isArray)
		{
			dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
			dsc.Texture2DMSArray.FirstArraySlice = txSlice;
			dsc.Texture2DMSArray.ArraySize = arraySize;
		}
		else
		{
			dsc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		DX_CALL(s_device->CreateDepthStencilView(res.ptr(), &dsc, getView().reset()));


	}

	void DepthStencilView::BindDepthStencilView(RenderTargetView* rtv) const
	{
		DX_CALL(s_devcon->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(&rtv), getView().ptr()));
	}

	void DepthStencilView::BindDepthOnly() const
	{
		DX_CALL(s_devcon->OMSetRenderTargets(0, nullptr, getView().ptr()));
	}

	void DepthStencilView::Clear(uint16_t flag, float depth, uint8_t stencil) const
	{
		DX_CALL(s_devcon->ClearDepthStencilView(getView().ptr(), flag, depth, stencil));
	}

	void ShaderResourceView::CreateShaderResourceView(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t detailedMip, uint16_t numMips, uint16_t arraySlice, uint16_t arraySize)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsc;
		dsc.Format = fmt;
		
		bool isArray{ arraySize > 1 };

		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY,
				dsc.Texture1DArray.MostDetailedMip = detailedMip,
				dsc.Texture1DArray.MipLevels = numMips,
				dsc.Texture1DArray.FirstArraySlice = arraySlice,
				dsc.Texture1DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D,
				dsc.Texture1D.MostDetailedMip = detailedMip,
				dsc.Texture1D.MipLevels = numMips;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
				dsc.Texture2DArray.MostDetailedMip = detailedMip,
				dsc.Texture2DArray.MipLevels = numMips,
				dsc.Texture2DArray.FirstArraySlice = arraySlice,
				dsc.Texture2DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
				dsc.Texture2D.MostDetailedMip = detailedMip,
				dsc.Texture2D.MipLevels = numMips;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			dsc.Texture3D.MostDetailedMip = detailedMip;
			dsc.Texture3D.MipLevels = numMips;
			break;
		}
		default:
			LOG_ENGINE_WARN("D3DView", "Unknow dimension passed to create SRV");
			break;
		}
		DX_CALL(s_device->CreateShaderResourceView(res.ptr(), &dsc, reinterpret_cast<ID3D11ShaderResourceView**>(m_view.reset())));
	}

	void ShaderResourceView::CreateShaderResourceView2D_MS(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t arraySlice, uint16_t arraySize)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsc;
		dsc.Format = fmt;
		bool isArray{ arraySize > 1 };

		if (isArray)
		{
			dsc.Texture2DMSArray.FirstArraySlice = arraySlice;
			dsc.Texture2DMSArray.ArraySize = arraySize;
			dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;

		}
		else
		{
			dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		DX_CALL(s_device->CreateShaderResourceView(res.ptr(), &dsc, reinterpret_cast<ID3D11ShaderResourceView**>(m_view.reset())));
	}

	void ShaderResourceView::CreateShaderResourceViewCubemap(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t detailedMip, uint16_t numMips, uint16_t first2DIndex, uint16_t numCubes)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsc;
		dsc.Format = fmt;

		bool isArray{ numCubes > 1 };

		if (isArray)
		{
			dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
			dsc.TextureCubeArray.MipLevels = numMips;
			dsc.TextureCubeArray.MostDetailedMip = detailedMip;
			dsc.TextureCubeArray.First2DArrayFace = first2DIndex;
			dsc.TextureCubeArray.NumCubes = numCubes;
		}
		else
		{
			dsc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			dsc.TextureCube.MipLevels = numMips;
			dsc.TextureCube.MostDetailedMip = detailedMip;
		}

		DX_CALL(s_device->CreateShaderResourceView(res.ptr(), &dsc, reinterpret_cast<ID3D11ShaderResourceView**>(m_view.reset())));

	}

	void ShaderResourceView::CreateShaderResourceBuffer(DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc inDsc)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsc;
		dsc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		dsc.Format = fmt;
		dsc.Buffer.ElementOffset = inDsc.elementOffset;
		dsc.Buffer.ElementWidth = inDsc.elementWidth;

		DX_CALL(s_device->CreateShaderResourceView(res.ptr(), &dsc, reinterpret_cast<ID3D11ShaderResourceView**>(m_view.reset())));

	}

	void ShaderResourceView::CreateRawShaderResourceBuffer(DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc inDsc)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsc;
		dsc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		dsc.Format = fmt;
		dsc.BufferEx.FirstElement = inDsc.firstElement;
		dsc.BufferEx.NumElements = inDsc.numElements;
		dsc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG::D3D11_BUFFEREX_SRV_FLAG_RAW;

		DX_CALL(s_device->CreateShaderResourceView(res.ptr(), &dsc, reinterpret_cast<ID3D11ShaderResourceView**>(m_view.reset())));

	}


	void ShaderResourceView::BindPS(uint16_t slot) const
	{
		if (slot > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindPS", "PS slot binding is {0}, when last slot is {1}", slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
		DX_CALL(s_devcon->PSSetShaderResources(slot, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(&m_view)));
	}

	void ShaderResourceView::BindVS(uint16_t slot) const
	{
		if (slot > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindVS", "VS slot binding is {0}, when last slot is {1}", slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
			DX_CALL(s_devcon->VSSetShaderResources(slot, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(&m_view)));
	}

	void ShaderResourceView::BindCS(uint16_t slot) const
	{
		if (slot > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindCS", "CS slot binding is {0}, when last slot is {1}", slot, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
		DX_CALL(s_devcon->CSSetShaderResources(slot, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(&m_view)));
	}

	void ShaderResourceView::ClearPS(uint16_t slot)
	{
		ID3D11ShaderResourceView* null[1] = { NULL };
		DX_CALL(s_devcon->PSSetShaderResources(slot, 1, null));
	}
	void ShaderResourceView::ClearVS(uint16_t slot)
	{
		ID3D11ShaderResourceView* null[1] = { NULL };
		DX_CALL(s_devcon->VSSetShaderResources(slot, 1, null));
	}

	void ShaderResourceView::ClearCS(uint16_t slot)
	{
		ID3D11ShaderResourceView* null[1] = { NULL };
		DX_CALL(s_devcon->CSSetShaderResources(slot, 1, null));
	}

	void ShaderResourceView::ClearPS(uint16_t startSlot, uint16_t numSlots)
	{
		ID3D11ShaderResourceView* null[1] = { NULL };
		DX_CALL(s_devcon->PSSetShaderResources(startSlot, numSlots, null));
	}
	void ShaderResourceView::ClearVS(uint16_t startSlot, uint16_t numSlots)
	{
		ID3D11ShaderResourceView* null[1] = { NULL };
		DX_CALL(s_devcon->VSSetShaderResources(startSlot, numSlots, null));
	}


	void ShaderResourceView::BindPS(ShaderResourceView* srv, uint16_t numViews, uint16_t startSlot)
	{
		if (numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
		{
			LOG_ENGINE_WARN("SRV::BindPS", "PS tried to bind {0} views, but maximum is {1}", numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		}

		if (startSlot + numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindPS", "PS last slot would be {0}, last allowed slot is {1}", startSlot + numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
		DX_CALL(s_devcon->PSSetShaderResources(startSlot, numViews, reinterpret_cast<ID3D11ShaderResourceView* const*>(srv)));
	}

	void ShaderResourceView::BindVS(ShaderResourceView* srv, uint16_t numViews, uint16_t startSlot)
	{
		if (numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
		{
			LOG_ENGINE_WARN("SRV::BindVS", "VS tried to bind {0} views, but maximum is {1}", numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		}

		if (startSlot + numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindVS", "PS last slot would be {0}, last allowed slot is {1}", startSlot + numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
		DX_CALL(s_devcon->VSSetShaderResources(startSlot, numViews, reinterpret_cast<ID3D11ShaderResourceView* const*>(&srv)));
	}

	void ShaderResourceView::BindCS( ShaderResourceView* const srv, uint16_t numViews, uint16_t startSlot)
	{
		if (numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
		{
			LOG_ENGINE_WARN("SRV::BindCS", "CS tried to bind {0} views, but maximum is {1}", numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		}

		if (startSlot + numViews > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)
		{
			LOG_ENGINE_WARN("SRV::BindCS", "CS last slot would be {0}, last allowed slot is {1}", startSlot + numViews, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1);
		}
		DX_CALL(s_devcon->CSSetShaderResources(startSlot, numViews, reinterpret_cast<ID3D11ShaderResourceView* const*>(srv)));
	}


	void UnorderedAccessView::CreateBufferUAV(const DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc inDsc, uint16_t uavFlag)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC dsc;
		dsc.Format = fmt;
		dsc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		dsc.Buffer.FirstElement = inDsc.firstElement;
		dsc.Buffer.NumElements = inDsc.numElements;
		dsc.Buffer.Flags = uavFlag;
		DX_CALL(s_device->CreateUnorderedAccessView(res, &dsc, getView().reset()));
	}

	void UnorderedAccessView::createTextureUAV(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t targetMip, uint16_t arraySlice, uint16_t arraySize)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC dsc;
		dsc.Format = fmt;
		bool isArray{ arraySize > 1 };
		switch (dimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY,
				dsc.Texture1DArray.MipSlice = targetMip,
				dsc.Texture1DArray.FirstArraySlice = arraySlice,
				dsc.Texture1DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D,
				dsc.Texture1D.MipSlice = targetMip;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			isArray ?
				dsc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
				dsc.Texture2DArray.MipSlice = targetMip,
				dsc.Texture2DArray.FirstArraySlice = arraySlice,
				dsc.Texture2DArray.ArraySize = arraySize :

				dsc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
				dsc.Texture2D.MipSlice = targetMip;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			dsc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
			dsc.Texture3D.FirstWSlice = arraySlice;
			dsc.Texture3D.MipSlice = targetMip;
			dsc.Texture3D.WSize = arraySize;
			break;
		}
		default:
			LOG_ENGINE_WARN("D3DView", "Unknow dimension passed to create SRV");
			break;
		}
		DX_CALL(s_device->CreateUnorderedAccessView(res.ptr(), &dsc, reinterpret_cast<ID3D11UnorderedAccessView**>(m_view.reset())));
	}

	void UnorderedAccessView::BindCS(uint16_t slot) const
	{
		DX_CALL(s_devcon->CSSetUnorderedAccessViews(slot, 1, reinterpret_cast<ID3D11UnorderedAccessView* const *>(&getView()), nullptr));
	}
	void UnorderedAccessView::BindCS(UnorderedAccessView* const srv, uint16_t numViews, uint16_t startSlot)
	{
		DX_CALL(s_devcon->CSSetUnorderedAccessViews(startSlot, numViews, reinterpret_cast<ID3D11UnorderedAccessView* const*>(srv), nullptr));
	}

	void UnorderedAccessView::ClearCS(uint16_t slot)
	{
		ID3D11UnorderedAccessView* null[1] = { NULL };

		DX_CALL(s_devcon->CSSetUnorderedAccessViews(slot, 1, null, nullptr));
	}

}

