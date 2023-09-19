#pragma once
#include "include/Render/Texture.h"


struct ID3D11Resource;
struct ID3D11ShaderResourceView;
enum D3D11_USAGE;

namespace DirectX
{
	struct Image;
}
namespace fth
{
	class Model;

	class TextureManager
	{
	public:

		static const std::string  defaultUVTx   ;
		static const std::string  defaultUVColor;
		static const std::string  defaultCubemap;
		static const std::string  defaultWhiteTx;
		static const std::string  defaultBlackTx;
		static const std::string  defaultHalfGrayTx;
		static const std::string  defaultWorleyTx;
		static const std::string  facingNormalTx;

		enum class FileFormat
		{
			PNG,
			TGA,
			HDR,
			BC1_UNORM     ,
			BC1_UNORM_SRGB,
			BC2_UNORM     ,
			BC2_UNORM_SRGB,
			BC3_UNORM     ,
			BC3_UNORM_SRGB,
			BC4_UNORM     ,
			BC4_SNORM     ,
			BC5_UNORM     ,
			BC5_SNORM     ,
			BC6H_UF16     ,
			BC6H_SF16     ,
			BC7_UNORM     ,
			BC7_UNORM_SRGB
		};


		static TextureManager& Get()
		{
			static TextureManager instance;
			return instance;
		}

		void Init();
		void Shutdown();

		//Load a texture that gets mapped by name. Searchs in Assets/
		std::shared_ptr<Texture> LoadTextures(std::string_view texturePath);
		//Load a texture that gets mapped by name. Searchs in Assets/	
		std::shared_ptr<Texture> LoadCubemaps(std::string_view cubemapPath);


		void ExportTexture(const Texture& src, FileFormat format, const std::string& fileName, bool generateMips);


	
		std::shared_ptr<Texture> FindTexture(const std::string& texture);
		std::shared_ptr<Texture> FindCubemap(const std::string& cubemap);

		static const std::shared_ptr<Texture>& UVTexture()      { return defaultTextures[DefaultTextures::UVTEXTURE];}
		static const std::shared_ptr<Texture>& UVColorTexture() { return defaultTextures[DefaultTextures::UVCOLOR];}
		static const std::shared_ptr<Texture>& DefaultCubemap() { return defaultTextures[DefaultTextures::SKYCUBEMAP];}
		static const std::shared_ptr<Texture>& White()          { return defaultTextures[DefaultTextures::WHITE];}
		static const std::shared_ptr<Texture>& Black()          { return defaultTextures[DefaultTextures::BLACK];}
		static const std::shared_ptr<Texture>& HalfGray()       { return defaultTextures[DefaultTextures::HALFGRAY];}
		static const std::shared_ptr<Texture>& WorleyNoise()    { return defaultTextures[DefaultTextures::WORLEY_NOISE]; }
		static const std::shared_ptr<Texture>& FacingNormal()   { return defaultTextures[DefaultTextures::FACING_NORMAL]; }

	private:
		TextureManager();
		~TextureManager();

		enum DefaultTextures : uint8_t
		{
			UVTEXTURE = 0,
			UVCOLOR,
			WHITE,
			HALFGRAY,
			BLACK,
			SKYCUBEMAP,
			WORLEY_NOISE,
			FACING_NORMAL,
			NUM
		};

		static std::shared_ptr<Texture>        defaultTextures[(uint8_t)DefaultTextures::NUM];

		//Some default textures

		void MapTexture(const std::shared_ptr<Texture>& texture, const std::string& textureName);
		void MapCubemap(const std::shared_ptr<Texture>& cubemap, const std::string& cubemapName);

		void LoadTextures(Texture& dst, const std::wstring& textureFullPath);
		void ExportToDDS(const Texture& src, FileFormat fmt, const std::string& fileName, bool generateMips);
		void ExportNonCompressed(const Texture& src, FileFormat fmt, const std::string& fileName, bool generateMips);
	private:
		
		//Any texture is in this map
		std::unordered_map<std::string, std::shared_ptr<Texture>>   m_textureMap;
		//Any cubemap is in this map
		std::unordered_map<std::string, std::shared_ptr<Texture>>   m_cubemapMap;


	};

	
}