#include "pch.h"
#pragma once
#include "include/Managers/TextureManager.h"
#include "include/Render/Model.h"
#include "vendor/DirectXTex/DDSTextureLoader/DDSTextureLoader11.h"
#include "include/Utils/Conversions.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include <dxgiformat.h>
#include "vendor/DirectXTex/Include/DirectXTex.h"
#include "vendor/DirectXTex/Include/DDS.h"
#include "include/Specifications.h"


namespace fth
{
	extern ID3D11Device5* s_device;
	extern ID3D11DeviceContext4* s_devcon;


	std::shared_ptr<Texture>           TextureManager::defaultTextures[DefaultTextures::NUM];

	

	const std::string  TextureManager::defaultUVTx    = "UVGrid.dds";
	const std::string  TextureManager::defaultUVColor = "UVColorGrid.dds";
	const std::string  TextureManager::defaultCubemap = "skyCubemap.dds";
	const std::string  TextureManager::defaultWhiteTx = "fullWhite.dds";
	const std::string  TextureManager::defaultBlackTx = "fullBlack.dds";
	const std::string  TextureManager::defaultHalfGrayTx = "halfGray.dds";
	const std::string  TextureManager::defaultWorleyTx = "worley_noise.dds";
	const std::string  TextureManager::facingNormalTx = "facingNormal.dds";

	TextureManager::TextureManager() {}
	TextureManager::~TextureManager() {}



	//constexpr DirectX::DDS_LOADER_FLAGS TranslateLoaderFlags(TextureManager::LOADER_FLAGS flag);
	constexpr DXGI_FORMAT _TranslateDDSFileFormat(TextureManager::FileFormat fmt);
	constexpr TextureManager::FileFormat _TranslateDDSFileFormatInv(DXGI_FORMAT fmt);
	bool isCompressed(TextureManager::FileFormat fmt);

	void TextureManager::Init()
	{
		defaultTextures[DefaultTextures::UVTEXTURE] = LoadTextures("Textures/" + TextureManager::defaultUVTx);
		defaultTextures[DefaultTextures::UVCOLOR] = LoadTextures("Textures/" + TextureManager::defaultUVColor);
		defaultTextures[DefaultTextures::SKYCUBEMAP] = LoadCubemaps("Textures/Cubemaps/" + TextureManager::defaultCubemap);
		defaultTextures[DefaultTextures::WHITE] = LoadTextures("Textures/" + TextureManager::defaultWhiteTx);
		defaultTextures[DefaultTextures::BLACK] = LoadTextures("Textures/" + TextureManager::defaultBlackTx);
		defaultTextures[DefaultTextures::HALFGRAY] = LoadCubemaps("Textures/" + TextureManager::defaultHalfGrayTx);
		defaultTextures[DefaultTextures::WORLEY_NOISE] = LoadTextures("Textures/" + TextureManager::defaultWorleyTx);
		defaultTextures[DefaultTextures::FACING_NORMAL] = LoadTextures("Textures/" + TextureManager::facingNormalTx);
	}

	void TextureManager::Shutdown()
	{

	}

	std::shared_ptr<Texture> TextureManager::LoadTextures(std::string_view texturePath)
	{
		std::string  txName{ FindLastChar('/', texturePath.data(), 1) };
		std::wstring fullpath{ std::wstring(Config::wAssetsPath) + ConvertUtf8ToWide(texturePath.data()) };


		{
			std::shared_ptr<Texture> alreadyLoaded = FindTexture(txName);
			if (alreadyLoaded != TextureManager::UVTexture())
				return alreadyLoaded;
		}

		std::shared_ptr<Texture> texture = std::make_shared<Texture>();


		LoadTextures(
			*texture,
			fullpath.c_str());

		MapTexture(texture, txName);

		return texture;
	}



	std::shared_ptr<Texture> TextureManager::LoadCubemaps(std::string_view cubemapPath)
	{
		std::string  txName{ FindLastChar('/', cubemapPath.data(), 1) };
		std::wstring fullpath{ std::wstring(Config::wAssetsPath) + ConvertUtf8ToWide(cubemapPath.data()) };

		{
			std::shared_ptr<Texture> alreadyLoaded = FindCubemap(txName);
			if (alreadyLoaded != TextureManager::DefaultCubemap())
				return alreadyLoaded;
		}

		std::shared_ptr<Texture> texture = std::make_shared<Texture>();

		LoadTextures
		(
			*texture,
			fullpath.c_str()
		);


		MapCubemap(texture, txName);

		return texture;
	}



	void TextureManager::LoadTextures(
		Texture& dst, 
		const std::wstring& textureFullPath)
	{

		DirectX::TexMetadata meta;
		DirectX::ScratchImage scratch;

		FTH_VERIFY_HR(DirectX::LoadFromDDSFile(textureFullPath.c_str(), DirectX::DDS_FLAGS_NONE, &meta, scratch));

		bool isCubemap{ (meta.miscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == D3D11_RESOURCE_MISC_TEXTURECUBE };

		dst.SetDesc(
			{
				static_cast<uint16_t>(meta.width),
				static_cast<uint16_t>(meta.height),
				static_cast<uint16_t>(meta.depth),
				meta.format,
				static_cast<D3D11_RESOURCE_DIMENSION>(meta.dimension),
				static_cast<uint16_t>(isCubemap ? 6 : 1),
				static_cast<uint16_t>(meta.arraySize),
				static_cast<uint16_t>(meta.mipLevels),
				isCubemap
			});

		DX_CALL(DirectX::CreateShaderResourceViewEx(
			s_device,
			scratch.GetImages(),
			scratch.GetImageCount(),
			meta,
			D3D11_USAGE_IMMUTABLE,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			meta.miscFlags,
			DirectX::CREATETEX_DEFAULT,
			dst.GetSRV().getView().reset()));

		dst.GetSRV().getView()->GetResource(dst.GetResource().reset());

	}

	void TextureManager::MapTexture(const std::shared_ptr<Texture>& texture, const std::string& textureName)
	{
		m_textureMap[textureName] = texture;
	}

	void TextureManager::MapCubemap(const std::shared_ptr<Texture>& cubemap, const std::string& cubemapName)
	{
		m_cubemapMap[cubemapName] = cubemap;
	}

	void TextureManager::ExportTexture(const Texture& src, FileFormat format, const std::string& fileName, bool generateMips)
	{
		bool isCompressed{ DirectX::IsCompressed(_TranslateDDSFileFormat(format))};

		if (isCompressed)
		{
			ExportToDDS(src, format, fileName, generateMips);
		}
		else
		{
			//NOT WORKING
			ExportNonCompressed(src, format, fileName, generateMips);
		}
	}

	void TextureManager::ExportToDDS(const Texture& src, FileFormat fmt, const std::string& fileName, bool generateMips)
	{
		DirectX::ScratchImage scratchImage;
		DirectX::CaptureTexture(s_device, s_devcon, src.GetResource(), scratchImage);

		const DirectX::ScratchImage* imagePtr = &scratchImage;

		DirectX::ScratchImage mipchain;
		if (generateMips)
		{
			const TextureDsc& txDsc{ src.GetDesc() };
			const DirectX::TexMetadata meta{
			   txDsc.width,
			   txDsc.height,
			   txDsc.depth,
			   txDsc.arraySize,
			   txDsc.numMips,
			   src.isCubemap() ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0,
			   0,
			   txDsc.format,
			   static_cast<DirectX::TEX_DIMENSION>(txDsc.dimension) };

			DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), meta, DirectX::TEX_FILTER_DEFAULT, 0, mipchain);
			imagePtr = &mipchain;
		}

		DirectX::ScratchImage compressed;

		if (FileFormat::BC6H_UF16 <= fmt && fmt <= FileFormat::BC7_UNORM_SRGB)
		{
			DX_CALL(DirectX::Compress(s_device, imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(),
				_TranslateDDSFileFormat(fmt), DirectX::TEX_COMPRESS_PARALLEL, 1.f, compressed));
		}
		else
		{
			DX_CALL(DirectX::Compress(imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(),
				_TranslateDDSFileFormat(fmt), DirectX::TEX_COMPRESS_PARALLEL, 1.f, compressed));
		}

		DirectX::SaveToDDSFile(imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(), DirectX::DDS_FLAGS(0), ConvertUtf8ToWide(fileName).c_str());
	}
	void TextureManager::ExportNonCompressed(const Texture& src, FileFormat fmt, const std::string& fileName, bool generateMips)
	{

	}



	std::shared_ptr<Texture> TextureManager::FindTexture(const std::string& texture)
	{
		auto iterator = m_textureMap.find(texture);
		if (iterator != m_textureMap.end())
		{
			return iterator->second;
		}

		return TextureManager::UVTexture();
	}

	std::shared_ptr<Texture> TextureManager::FindCubemap(const std::string& cubemap)
	{
		auto iterator = m_cubemapMap.find(cubemap);
		if (iterator != m_cubemapMap.end())
		{
			return iterator->second;
		}

		return TextureManager::DefaultCubemap();
	}



	//constexpr DirectX::DDS_LOADER_FLAGS TranslateLoaderFlags(TextureManager::LOADER_FLAGS flag)
	//{
	//	switch (flag)
	//	{

	//	case TextureManager::LOADER_FLAGS::FORCE_SRGB:
	//		return DirectX::DDS_LOADER_FORCE_SRGB;
	//	case TextureManager::LOADER_FLAGS::IGNORE_SRGB:
	//		return DirectX::DDS_LOADER_IGNORE_SRGB;
	//	case TextureManager::LOADER_FLAGS::DEFAULT:
	//	default:
	//		return DirectX::DDS_LOADER_DEFAULT;
	//	}
	//}


	constexpr DXGI_FORMAT _TranslateDDSFileFormat(TextureManager::FileFormat fmt)
	{
		switch (fmt)
		{
		case fth::TextureManager::FileFormat::BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM;
		case fth::TextureManager::FileFormat::BC1_UNORM_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case fth::TextureManager::FileFormat::BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM;
		case fth::TextureManager::FileFormat::BC2_UNORM_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case fth::TextureManager::FileFormat::BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM;
		case fth::TextureManager::FileFormat::BC3_UNORM_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		case fth::TextureManager::FileFormat::BC4_UNORM:
			return DXGI_FORMAT_BC4_UNORM;
		case fth::TextureManager::FileFormat::BC4_SNORM:
			return DXGI_FORMAT_BC4_SNORM;
		case fth::TextureManager::FileFormat::BC5_UNORM:
			return DXGI_FORMAT_BC5_UNORM;
		case fth::TextureManager::FileFormat::BC5_SNORM:
			return DXGI_FORMAT_BC5_SNORM;
		case fth::TextureManager::FileFormat::BC6H_UF16:
			return DXGI_FORMAT_BC6H_UF16;
		case fth::TextureManager::FileFormat::BC6H_SF16:
			return DXGI_FORMAT_BC6H_SF16;
		case fth::TextureManager::FileFormat::BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM;
		case fth::TextureManager::FileFormat::BC7_UNORM_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}

	}

	constexpr TextureManager::FileFormat _TranslateDDSFileFormatInv(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_UNORM:
			return TextureManager::FileFormat::BC1_UNORM;
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return TextureManager::FileFormat::BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_UNORM:
			return TextureManager::FileFormat::BC2_UNORM;
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return TextureManager::FileFormat::BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_UNORM:
			return TextureManager::FileFormat::BC3_UNORM;
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return TextureManager::FileFormat::BC3_UNORM_SRGB;
		case DXGI_FORMAT_BC4_UNORM:
			return TextureManager::FileFormat::BC4_UNORM;
		case DXGI_FORMAT_BC4_SNORM:
			return TextureManager::FileFormat::BC4_SNORM;
		case DXGI_FORMAT_BC5_UNORM:
			return TextureManager::FileFormat::BC5_UNORM;
		case DXGI_FORMAT_BC5_SNORM:
			return TextureManager::FileFormat::BC5_SNORM;
		case DXGI_FORMAT_BC6H_UF16:
			return TextureManager::FileFormat::BC6H_UF16;
		case DXGI_FORMAT_BC6H_SF16:
			return TextureManager::FileFormat::BC6H_SF16;
		case DXGI_FORMAT_BC7_UNORM:
			return TextureManager::FileFormat::BC7_UNORM;
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return TextureManager::FileFormat::BC7_UNORM_SRGB;
		}
	}

	bool isCompressed(TextureManager::FileFormat fmt)
	{
		switch (fmt)
		{
		case fth::TextureManager::FileFormat::PNG:
		case fth::TextureManager::FileFormat::TGA:
		case fth::TextureManager::FileFormat::HDR:
			return false;
		default:
			return true;
		}
	}
	
}