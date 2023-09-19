#pragma once
#include "include/Render/Texture.h"
namespace fth
{
	struct Sprite
	{
		Sprite() = default;
		Sprite(uint32_t x, uint32_t y, uint32_t width, uint32_t height) :
			xPos(x), yPos(y), width(width), height(height) {}

		uint32_t xPos;
		uint32_t yPos;
		uint32_t width;
		uint32_t height;
	};

	class Atlas
	{
	public://TODO: add sprite name hashing to query a sprite by name
		//Build an Atlas from a .txt file describing sprite layout using fth::FileCommandLoader
		bool createAtlas(std::string_view atlasDescFile);
		//Build an Atlas using a string vector that contains sprite layout, i.e, each element describes sprite characteristics
		bool createAtlas(uint32_t width, uint32_t height, const std::vector<std::string>& descArray);
		//Build a tiled atlas from a src texture and a sample sprite
		bool createTiledAtlas(uint32_t width, uint32_t height, Sprite sprite, uint32_t numTiles);


		bool isTiled() const { return m_isTiled; }
		uint32_t getNumSprites() const { return static_cast<uint32_t>(m_sprites.size()); }
		const Sprite& getSprite(uint32_t index) const { return m_sprites[index]; }
		uint32_t getWidth() const { return  m_atlasWidth; }
		uint32_t getHeight() const { return m_atlasHeight; }


		math::Vector2 computeAtlasUVfromSpriteUV(const Sprite& sprite, math::Vector2 spriteUV);
		math::Vector2 computeAtlasUVfromSpriteUV(const uint32_t spriteIdx, math::Vector2 spriteUV);
	private:
		uint32_t                   m_atlasWidth = 0;
		uint32_t                   m_atlasHeight = 0;
		bool                       m_isTiled = false;
		std::vector<Sprite>        m_sprites; //each atlas section/sprite
	};
}