#include "pch.h"

#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"

namespace fth::shading
{
	void IShadingGroup::Init(uint8_t inModelBits, uint8_t inMaterialBits, uint8_t inInstanceBits)
	{
		//TODO: I've found another way (much more elegant) to build bitmasks. This is temp.
		auto generateBitmask = [](uint8_t bits, uint8_t startingBit) -> uint32_t
		{
			uint32_t mask{};

			for (uint32_t i = 0; i < bits; ++i)
			{
				mask |= 1 << (startingBit + i);
			}
			return mask;
		};
		instanceBits = inInstanceBits;
		instanceMask = generateBitmask(inInstanceBits, 0);

		modelBits = inModelBits;
		modelMask = generateBitmask(inModelBits, inMaterialBits + inInstanceBits);

		materialBits = inMaterialBits;
		materialMask = generateBitmask(inMaterialBits, inInstanceBits);

		
	}

	uint32_t IShadingGroup::computeModelIdx(uint32_t flag) const
	{
		return (flag & modelMask) >> (instanceBits + materialBits);
	}

	uint32_t IShadingGroup::computeMaterialIdx(uint32_t flag) const
	{
		return (flag & materialMask) >> instanceBits;
	}
	uint32_t IShadingGroup::computeInstanceHandle(uint32_t flag) const
	{
		return (flag & instanceMask);
	}
}