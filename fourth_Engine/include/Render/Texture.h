#pragma once
#include "include/Render/Renderer/D3DView.h"

struct ID3D11Texture2D;
struct ID3D11Resource;
struct ID3D11Device;
struct ID3D11DeviceContext4;
struct ID3D11ShaderResourceView;
struct D3D11_SUBRESOURCE_DATA;
enum D3D11_RESOURCE_DIMENSION;
enum D3D11_RESOURCE_MISC_FLAG;
enum DXGI_FORMAT;
enum D3D11_USAGE;

namespace DirectX
{
	struct Image;
}

namespace fth
{
		//Make a D3DSRV class, and then a D3DTexture, which inherits from SRV and incorporates texture desc 



	    struct TextureDsc
	    {
	    	uint16_t                   width;
	    	uint16_t                   height;
	    	uint16_t                   depth;
	    	DXGI_FORMAT                format;
			D3D11_RESOURCE_DIMENSION   dimension;
	    	uint16_t                   numTextures;//1 for single textures, 6 for cubemaps
			uint16_t                   arraySize;//needs to be multiple of 6 for cubemaps
	    	uint16_t                   numMips;
			uint16_t                   multisamples = 1;
			uint16_t                   msQuality = 0;
			bool                       isCubemap;

			bool operator==(const TextureDsc& rval);
			bool operator!=(const TextureDsc& rval);

	    };

		

		class Texture
		{
		public:

			DxResPtr<ID3D11Resource>&              GetResource() { return m_d3dTexture; };
			const DxResPtr<ID3D11Resource>&        GetResource() const { return m_d3dTexture; };

			renderer::ShaderResourceView&    GetSRV()      { return m_srv; };
			const renderer::ShaderResourceView& GetSRV() const { return m_srv; }

			//This method won't create MS textures for input data
			void Init(const TextureDsc& dsc, D3D11_USAGE usage, uint32_t bindFlags,const DirectX::Image* data = nullptr, uint32_t cpuFlags = 0, uint32_t miscFlags = 0);


			void MakeShaderResource(DXGI_FORMAT format);

			void BindPS(uint16_t slot) const;
			void BindVS(uint16_t slot) const;


;			void SetDesc(TextureDsc dsc) { m_texDsc = dsc; };
			const TextureDsc& GetDesc() const { return m_texDsc; }
			bool isCubemap() const { return m_texDsc.isCubemap; }

			uint32_t Width() const { return m_texDsc.width; }
			uint32_t Height() const { return m_texDsc.height; }
			uint16_t NumMips() const { return m_texDsc.numMips; }
			uint16_t Multisamples() const { return m_texDsc.multisamples; }
			DXGI_FORMAT Format() const { return m_texDsc.format; }

			//Creates an exact copy of the texture into dst texture. 
			void MakeCopy(Texture& dst) const;
			//Creates a non multisampled copy of a texture. 
			void MakeCopyNonMS(Texture& dst) const;
			//Creates a Non multisampled copy of a texture describing a depth buffer. Needs a valid render target.
			void MakeCopyDepthNonMS(Texture& dst, const renderer::RenderTargetView& rtv) const;



			//STATIC METHODS
		public:
			static void ClearVS(uint16_t slot);
			static void ClearPS(uint16_t slot);


		private:
			void InitTextureFromInitialData(const TextureDsc& dsc, D3D11_USAGE usage, uint32_t bindFlags, const DirectX::Image* data = nullptr, uint32_t cpuFlags = 0, uint32_t miscFlags = 0);

		private:
			DxResPtr<ID3D11Resource>               m_d3dTexture;
			renderer::ShaderResourceView           m_srv;

			TextureDsc                             m_texDsc;

		};
		using Cubemap = fth::Texture;

}