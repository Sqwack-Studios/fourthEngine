#pragma once
#include "include/Utils/DxRes.h"

//D3D11 resource structs
struct ID3D11View;
struct ID3D11Resource;
struct ID3D11Buffer;

struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11UnorderedAccessView;

//D3D11 resource descriptors
struct D3D11_RENDER_TARGET_VIEW_DESC;
struct D3D11_DEPTH_STENCIL_VIEW_DESC;
struct D3D11_SHADER_RESOURCE_VIEW_DESC;
struct D3D11_UNORDERED_ACCESS_VIEW_DESC;
enum DXGI_FORMAT;
enum D3D11_RESOURCE_DIMENSION;

namespace fth::renderer
{
	struct ViewBufferDsc
	{
		union
		{
			uint16_t firstElement;
			uint16_t elementOffset;
		};
		union
		{
			uint16_t numElements;
			uint16_t elementWidth;
		};
	};

	enum DSV_FLAGS
	{
		DSV_ALLOW_WRITE = 0,
		DSV_READ_ONLY_DEPT = 0x1L,
		DSV_READ_ONLY_STENCIL = 0x2L
	};

	class D3DView
	{
	protected:
		const DxResPtr<ID3D11View>&  getBaseView() const { return m_view; }
		DxResPtr<ID3D11View>& getBaseView() { return m_view; }

		DxResPtr<ID3D11View>  m_view;
	};

	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;

	class RenderTargetView: public D3DView
	{
	public:
		void CreateRenderTarget(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t mipLevel, uint16_t txSlice = 0, uint16_t arraySize = 1);
		void CreateRenderTarget2D_MS(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t txSlice = 0, uint16_t arraySize = 1);
		void CreateRenderTargetBuffer(const DxResPtr<ID3D11Resource>& res, ViewBufferDsc dsc);

		const DxResPtr<ID3D11RenderTargetView>& getView() const { return reinterpret_cast< const DxResPtr<ID3D11RenderTargetView>& > (getBaseView()); }
		DxResPtr<ID3D11RenderTargetView>& getView() {  return reinterpret_cast< DxResPtr<ID3D11RenderTargetView>& >  (getBaseView()); }

		void Clear(const math::Color& clear);

		void BindRenderTargetView(DepthStencilView* dsv = nullptr) const;
		static void BindRenderTargetViews(RenderTargetView * const rtv, uint16_t numRTV, DepthStencilView* dsv = nullptr);
	};

	class DepthStencilView : public D3DView
	{
	public:

		void CreateDepthStencilTarget(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t mipLevel, uint16_t arraySlice = 0, uint16_t arraySize = 1, uint16_t flags = 0);
		void CreateDepthStencilTarget2D_MS(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t txSlice = 0, uint16_t arraySize = 1, uint16_t flags = 0);

		const DxResPtr<ID3D11DepthStencilView>& getView() const { return reinterpret_cast<const DxResPtr<ID3D11DepthStencilView>&> (getBaseView()); }
		DxResPtr<ID3D11DepthStencilView>& getView() { return reinterpret_cast<DxResPtr<ID3D11DepthStencilView>&>  (getBaseView()); }

		void Clear(uint16_t flag, float depth, uint8_t stencil) const;
		void BindDepthStencilView(RenderTargetView* rtv) const;
		void BindDepthOnly() const;
	};

	class ShaderResourceView : public D3DView
	{
	public:
		void CreateShaderResourceView(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t detailedMip, uint16_t numMips, uint16_t arraySlice = 0, uint16_t arraySize = 1);
		void CreateShaderResourceView2D_MS(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t arraySlice = 0, uint16_t arraySize = 1);
		void CreateShaderResourceViewCubemap(DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, uint16_t detailedMip, uint16_t numMips, uint16_t first2DIndex = 0, uint16_t numCubes = 1);
		void CreateShaderResourceBuffer(DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc dsc);
		void CreateRawShaderResourceBuffer(DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc dsc);

		const DxResPtr<ID3D11ShaderResourceView>& getView() const { return reinterpret_cast<const DxResPtr<ID3D11ShaderResourceView>&> (getBaseView()); }
		DxResPtr<ID3D11ShaderResourceView>& getView() { return reinterpret_cast<DxResPtr<ID3D11ShaderResourceView>&>  (getBaseView()); }

		void BindPS(uint16_t slot) const;
		void BindVS(uint16_t slot) const;
		void BindCS(uint16_t slot) const;

		static void ClearPS(uint16_t slot);
		static void ClearVS(uint16_t slot);
		static void ClearCS(uint16_t slot);
		//THIS WILL CRASH, DO NOT USE
		static void ClearPS(uint16_t startSlot, uint16_t numSlots);
		//THIS WILL CRASH, DO NOT USE
		static void ClearVS(uint16_t startSlot, uint16_t numSlots);

		static void BindPS(ShaderResourceView* srv, uint16_t numViews, uint16_t startSlot);
		static void BindVS(ShaderResourceView* srv, uint16_t numViews, uint16_t startSlot);
		static void BindCS(ShaderResourceView* const srv, uint16_t numViews, uint16_t startSlot);
	};


	class UnorderedAccessView : public D3DView
	{
	public:
		const DxResPtr<ID3D11UnorderedAccessView>& getView() const { return reinterpret_cast<const DxResPtr<ID3D11UnorderedAccessView>&> (getBaseView()); }
		DxResPtr<ID3D11UnorderedAccessView>& getView() { return reinterpret_cast<DxResPtr<ID3D11UnorderedAccessView>&>  (getBaseView()); }

		void CreateBufferUAV(const DxResPtr<ID3D11Buffer>& res, DXGI_FORMAT fmt, ViewBufferDsc dsc, uint16_t uavFlag);
		void createTextureUAV(const DxResPtr<ID3D11Resource>& res, DXGI_FORMAT fmt, D3D11_RESOURCE_DIMENSION dimension, uint16_t targetMip, uint16_t arraySlice = 0, uint16_t arraySize = 1);
		void BindCS(uint16_t slot) const;
		static void BindCS(UnorderedAccessView* const srv, uint16_t numViews, uint16_t startSlot);
		static void ClearCS(uint16_t numViews);
	};

}