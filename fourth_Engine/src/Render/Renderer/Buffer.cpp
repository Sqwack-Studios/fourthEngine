#include "pch.h"

#pragma once
#include "include/Render/Renderer/Buffer.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth::renderer
{
	void ByteAddressBuffer::init(uint32_t count, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, void* data)
	{
		D3DBuffer::init(sizeof(float), count, usage, bindFlags, cpuFlags, miscFlags | D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS, data);
	}
}