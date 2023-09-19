#pragma once
#include "include/Render/Renderer/D3DBuffer.h"

//Tbh I don't think creating a template is necessary. We could allocate void* data of requested size with stride sizeof(structure)

namespace fth
{
	namespace renderer
	{
		template<typename T>
		struct IndexBuffer
		{
			using DataType = T;

			IndexBuffer() = default;
			~IndexBuffer() = default;
			IndexBuffer(IndexBuffer&& other) = default;
			IndexBuffer& operator=(IndexBuffer&& other) = default;
			IndexBuffer(const IndexBuffer&) = delete;
			IndexBuffer& operator=(const IndexBuffer&) = delete;
			
			inline void CreateGPUBuffer()
			{
				m_gpuData.CreateIndexBuffer(sizeof(DataType), static_cast<uint32_t>(m_cpuData.size()), m_cpuData.data());
			}
			inline void Bind() const { m_gpuData.BindIndexBuffer(); };
			inline void ClearAll() { ClearCPU(); ClearGPU(); };
			inline void ClearGPU() { m_gpuData.ClearGPU(); };
			inline void ClearCPU() { m_cpuData.clear(); };

			inline const std::vector<DataType>& GetCpuData() const { return m_cpuData; }
			


			std::vector<DataType>        m_cpuData;
		private:
			D3DBuffer                    m_gpuData;
		};





	}
}