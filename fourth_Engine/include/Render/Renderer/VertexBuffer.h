#pragma once
#include "include/Render/Renderer/D3DBuffer.h"


namespace fth
{
   namespace renderer
   {
		template<typename T>
		struct VertexBuffer
		{
		public:
			using DataType = T;
		public:

			VertexBuffer() = default;
			~VertexBuffer() = default;
			VertexBuffer(VertexBuffer&& other) = default;
			VertexBuffer& operator=(VertexBuffer&& other) = default;
			VertexBuffer(const VertexBuffer&) = delete;
			VertexBuffer& operator=(const VertexBuffer&) = delete;


		public:

			inline void CreateGPUBuffer()
			{ 
				m_gpuData.CreateVertexBuffer(sizeof(DataType), static_cast<uint32_t>(m_cpuData.size()), m_cpuData.data());
			}

			void Bind(uint16_t slot) const { m_gpuData.BindVertexBuffer(slot); };
			void ClearAll() { ClearCPU(); ClearGPU(); };
			void ClearGPU() { m_gpuData.ClearGPU(); };
			void ClearCPU() { m_cpuData.clear(); };


			
			inline void FillData(std::vector<DataType>&& data) { m_cpuData = std::move(data); }
			inline const std::vector<DataType>& GetCpuData() const { return m_cpuData; }

			std::vector<DataType>           m_cpuData;
		private:
			D3DBuffer                       m_gpuData;

		};



   }
}
