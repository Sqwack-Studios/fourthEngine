#pragma once

namespace fth
{

	struct PER_FRAME_CONSTANT_BUFFER
	{
		math::Vector4  resolution;
		math::Vector4  mousePos;

		float          time;
		float          deltaTime;
		float          EV100;
		uint32_t       validateLuminance;

		uint32_t       enableDiffuse;
		uint32_t       enableSpecular;
		uint32_t       enableIBL;
		uint32_t       overrideRoughness ;

		float          overrideRoughValue;
		uint32_t       numMipsSpecularIrradianceIBL;
		uint32_t       specularMRP_Flags;
		uint32_t       mainRTVSubsamples;

	};


}