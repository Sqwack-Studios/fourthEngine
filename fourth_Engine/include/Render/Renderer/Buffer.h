#pragma once
#include "include/Render/Renderer/D3DBuffer.h"


namespace fth::renderer
{

	template<typename T>
	class StructuredBuffer: public D3DBuffer
	{
	public:
		using DataStruct = T;
		void init(uint32_t count, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, void* data)
		{
			D3DBuffer::init(sizeof(DataStruct), count, usage, bindFlags, cpuFlags, miscFlags | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, data);
		}


	};

	class ByteAddressBuffer: public D3DBuffer
	{
	public:
		void init(uint32_t count, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, void* data);

		
	};
}
