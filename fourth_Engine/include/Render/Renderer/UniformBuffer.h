#pragma once
#include "include/Render/Renderer/D3DBuffer.h"

namespace fth
{
	namespace renderer
	{
		struct UniformBuffer
		{


			UniformBuffer() = default;
			~UniformBuffer() = default;
			UniformBuffer(const UniformBuffer&) = delete;
			UniformBuffer(UniformBuffer&&) = default;
			UniformBuffer& operator=(const UniformBuffer&) = delete;
			UniformBuffer& operator=(UniformBuffer&&) = default;

			inline D3DBuffer& GetBuffer() { return m_gpuData; }
			inline void CreateGPUBuffer(uint32_t dataSize, uint16_t count, void* dataPtr) { m_gpuData.CreateUniformBuffer(dataSize, count, dataPtr); };
			inline void ClearGPU() { m_gpuData.ClearGPU(); };
			inline void* Map() { return m_gpuData.Map(); };
			inline void Unmap() { m_gpuData.Unmap(); };
			void BindPS (uint16_t slot) const{ m_gpuData.BindUniformBufferPS(slot); }
			void BindVS (uint16_t slot) const{ m_gpuData.BindUniformBufferVS(slot); }
			void BindHS (uint16_t slot) const{ m_gpuData.BindUniformBufferHS(slot); }
			void BindDS (uint16_t slot) const{ m_gpuData.BindUniformBufferDS(slot); }
			void BindGS (uint16_t slot) const{ m_gpuData.BindUniformBufferGS(slot); }
			void BindCS(uint16_t slot) const { m_gpuData.BindUniformBufferCS(slot); }


			inline uint32_t Size() const { return m_gpuData.Size(); }
		private:
			D3DBuffer     m_gpuData;
		};
		

	}

}
