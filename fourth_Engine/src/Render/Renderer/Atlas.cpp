#include "pch.h"

#pragma once
#include "include/Render/Atlas.h"

namespace fth
{

	bool Atlas::createTiledAtlas(uint32_t width, uint32_t height, Sprite sprite, uint32_t numTiles)
	{
		//Try to fit by rows and then by cols
		uint32_t maxTilesByRow{ width / sprite.width };
		uint32_t maxTilesByCol{ height / sprite.height };

		uint32_t maxOccupancy{ maxTilesByRow * maxTilesByCol};


		bool fits { maxOccupancy >= numTiles };
		if (!fits)
		{
			LOG_ENGINE_WARN("createTiledAtlas", "Specified atlas size: ({0},{1}) is not big enough for tile count ({2}) and specified tile size({3}, {4})", width, height, numTiles, sprite.width, sprite.height);
			return false;
		}

		m_sprites.reserve(numTiles);

		uint32_t rowsToFill{ static_cast<uint32_t>(std::ceilf((float)numTiles / (float)maxTilesByRow)) };

		for (uint32_t row = 0; row < rowsToFill; ++row)
		{
			for (uint32_t col = 0; col < maxTilesByCol; ++col)
			{
				uint32_t xPos = col * sprite.width;
				uint32_t yPos = row * sprite.height;

				m_sprites.emplace_back(xPos, yPos, sprite.width, sprite.height);
			}
		}
		
		m_isTiled = true;
		m_atlasWidth = width;
		m_atlasHeight = height;
		return true;
	}
}