#pragma once
#include "include/Utils/DxRes.h"

struct ID3D11Buffer;
struct D3D11_BUFFER_DESC;
enum D3D11_USAGE;
namespace fth
{
	namespace renderer
	{

		struct BufferDesc
		{
			uint32_t byteWidth;
			D3D11_USAGE usage;
			uint32_t bindFlags;
			uint32_t cpuFlags;
			uint32_t miscFlags;
			uint32_t stride;

			operator D3D11_BUFFER_DESC() const noexcept;
		};

		struct D3DBuffer
		{


			D3DBuffer() = default;
			~D3DBuffer();
			D3DBuffer(D3DBuffer&& other) = default;
			D3DBuffer& operator=(D3DBuffer&& other) = default;

			void init(uint32_t stride, uint32_t count, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, void* data);


			void CreateVertexBuffer(uint32_t structSize, uint32_t count, void* data);
			void CreateIndexBuffer(uint32_t structSize, uint32_t count, void* data);
			void CreateInstanceBuffer(uint32_t structSize, uint32_t count, void* data);
			void CreateUniformBuffer(uint32_t structSize, uint32_t count, void* data);

			//TargetShader is only used by UniformBuffers
			void BindVertexBuffer(uint16_t slot) const;
			void BindIndexBuffer() const;
			void BindUniformBufferVS(uint16_t slot) const;
			void BindUniformBufferPS(uint16_t slot) const;
			void BindUniformBufferDS(uint16_t slot) const;
			void BindUniformBufferHS(uint16_t slot) const;
			void BindUniformBufferGS(uint16_t slot) const;
			void BindUniformBufferCS(uint16_t slot) const;

			static void BindUniformBufferVS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer);
			static void BindUniformBufferPS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) ;
			static void BindUniformBufferDS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) ;
			static void BindUniformBufferHS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) ;
			static void BindUniformBufferGS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) ;

			void UpdateBufferData(uint32_t structSize, uint32_t count, void* data);
			void* Map();
			void  Unmap();
			void ClearGPU() { m_d3dBuffer.reset(); m_dataSize = 0; m_dataCount = 0; };
			uint32_t Size() const { return m_dataCount; }


			DxResPtr<ID3D11Buffer>& operator->() { return m_d3dBuffer; }
		protected:
			bool EnoughCapacity(uint32_t structSize, uint32_t count) { return (m_dataSize * m_dataCount >= structSize * count); };
			void CreateBuffer(const D3D11_BUFFER_DESC& bDsc, void* data);


		private:
			DxResPtr<ID3D11Buffer> m_d3dBuffer;
			BufferDesc bufferDesc;
			uint32_t   m_dataSize = 0;
			uint32_t   m_dataCount = 0;

		};
	}
}