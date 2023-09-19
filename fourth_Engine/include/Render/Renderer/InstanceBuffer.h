#pragma once
#include "include/Render/Renderer/D3DBuffer.h"
namespace fth
{
	namespace renderer
	{
		struct InstanceBuffer
		{
		public: 

		public:
			InstanceBuffer() = default;
			~InstanceBuffer() = default;
			InstanceBuffer(InstanceBuffer&&) = default;
			InstanceBuffer(const InstanceBuffer&) = default;
			InstanceBuffer& operator=(const InstanceBuffer&) = default;
			InstanceBuffer& operator=(InstanceBuffer&&) = default;

		public:
			void CreateGPUBuffer(uint32_t dataSize, uint32_t count, void* dataPtr)
			{
				m_gpuData.CreateInstanceBuffer(dataSize, count, dataPtr);
			}

			void Bind(uint16_t slot) { m_gpuData.BindVertexBuffer(slot); }
			void ClearGPU() { m_gpuData.ClearGPU(); };
			void* Map() { return m_gpuData.Map(); };
			void Unmap() { m_gpuData.Unmap(); }
			inline uint32_t Size() const { return m_gpuData.Size(); }

		private:
			D3DBuffer             m_gpuData;
		};


	}
}