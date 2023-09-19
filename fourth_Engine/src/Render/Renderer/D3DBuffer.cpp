#include "pch.h"

#pragma once
#include "include/Render/Renderer/D3DBuffer.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	namespace renderer
	{
		D3DBuffer::~D3DBuffer(){};

		BufferDesc::operator D3D11_BUFFER_DESC() const noexcept
		{
			D3D11_BUFFER_DESC bdsc;
			bdsc.ByteWidth = byteWidth;
			bdsc.Usage = usage;
			bdsc.BindFlags = bindFlags;
			bdsc.CPUAccessFlags = cpuFlags;
			bdsc.MiscFlags = miscFlags;
			bdsc.StructureByteStride = stride;

			return bdsc;
		}

		void D3DBuffer::init(uint32_t stride, uint32_t count, D3D11_USAGE usage, uint32_t bindFlags, uint32_t cpuFlags, uint32_t miscFlags, void* data)
		{
			m_dataSize = stride;
			m_dataCount = count;

			bufferDesc.byteWidth = stride * count;
			bufferDesc.stride = stride;
			bufferDesc.bindFlags = bindFlags;
			bufferDesc.cpuFlags = cpuFlags;
			bufferDesc.miscFlags = miscFlags;
			bufferDesc.usage = usage;
			D3D11_BUFFER_DESC bDsc = bufferDesc;

			if (!data)
			{
				DX_HRCALL(s_device->CreateBuffer(&bDsc, NULL, m_d3dBuffer.reset()));
				return;
			}

			D3D11_SUBRESOURCE_DATA subData {};
			subData.pSysMem = data;

			DX_HRCALL(s_device->CreateBuffer(&bDsc, &subData, m_d3dBuffer.reset()));

		}

		void D3DBuffer::CreateBuffer(const D3D11_BUFFER_DESC& bDsc, void* data)
		{
			if (data == nullptr)
			{
				DX_HRCALL(s_device->CreateBuffer(&bDsc, NULL, m_d3dBuffer.reset()));
			}
			else
			{
				D3D11_SUBRESOURCE_DATA bSd = {};
				bSd.pSysMem = data;

				DX_HRCALL(s_device->CreateBuffer(&bDsc, &bSd, m_d3dBuffer.reset()));
			}
		}

		void D3DBuffer::UpdateBufferData(uint32_t structSize, uint32_t count, void* data)
		{
			if (data)
			{
				void* dstData = Map();
				memcpy(dstData, data, structSize * count);
				Unmap();
			}
		}

		void D3DBuffer::CreateVertexBuffer(uint32_t structSize, uint32_t count, void* data)
		{
			//Don't create another buffer if the layout and size is the same
			if (EnoughCapacity(structSize, count))
			{
				if (m_d3dBuffer.valid())
				{
					UpdateBufferData(structSize, count, data);
				}
				return;
			}

			m_dataSize = structSize;
			m_dataCount = count;


			D3D11_BUFFER_DESC bDsc = {};
			bDsc.ByteWidth = m_dataSize * m_dataCount;
			bDsc.Usage = D3D11_USAGE_IMMUTABLE;
			bDsc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bDsc.CPUAccessFlags = 0;
			bDsc.StructureByteStride = m_dataSize;
			bDsc.MiscFlags = 0;

			CreateBuffer(bDsc, data);
		 
		}

		void D3DBuffer::CreateIndexBuffer(uint32_t structSize, uint32_t count, void* data)
		{
			if (EnoughCapacity(structSize, count))
			{
				if (m_d3dBuffer.valid())
				{
					UpdateBufferData(structSize, count, data);
				}
				return;
			}

			m_dataSize = structSize;
			m_dataCount = count;

			D3D11_BUFFER_DESC bDsc = {};
			bDsc.ByteWidth = m_dataSize * m_dataCount;
			bDsc.Usage = D3D11_USAGE_IMMUTABLE;
			bDsc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bDsc.CPUAccessFlags = 0;
			bDsc.StructureByteStride = m_dataSize;
			bDsc.MiscFlags = 0;

			CreateBuffer(bDsc, data);

		}

		void D3DBuffer::CreateInstanceBuffer(uint32_t structSize, uint32_t count, void* data)
		{
			if (EnoughCapacity(structSize, count))
			{
				if (m_d3dBuffer.valid())
				{
					UpdateBufferData(structSize, count, data);
				}
				return;
			}

			m_dataSize = structSize;
			m_dataCount = count;

			D3D11_BUFFER_DESC bDsc = {};
			bDsc.ByteWidth = m_dataSize * m_dataCount;
			bDsc.Usage = D3D11_USAGE_DYNAMIC;
			bDsc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bDsc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bDsc.StructureByteStride = m_dataSize;
			bDsc.MiscFlags = 0;

			CreateBuffer(bDsc, data);

		}

		void D3DBuffer::CreateUniformBuffer(uint32_t structSize, uint32_t count, void* data)
		{
			if (EnoughCapacity(structSize, count))
			{
				if (m_d3dBuffer.valid())
				{
					UpdateBufferData(structSize, count, data);
				}
				return;
			}

			m_dataSize = structSize;
			m_dataCount = count;

			D3D11_BUFFER_DESC bDsc = {};
			bDsc.ByteWidth = m_dataSize * m_dataCount;
			bDsc.Usage = D3D11_USAGE_DYNAMIC;
			bDsc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bDsc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bDsc.StructureByteStride = 0;
			bDsc.MiscFlags = 0;

			CreateBuffer(bDsc, data);

		}

		void D3DBuffer::BindVertexBuffer(uint16_t slot) const
		{
			const UINT stride = m_dataSize;
			const UINT offset = 0;
			DX_CALL(s_devcon->IASetVertexBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer), &stride, &offset));
		}

		void D3DBuffer::BindIndexBuffer() const
		{
			const UINT offset = 0;
			DX_CALL(s_devcon->IASetIndexBuffer(m_d3dBuffer, DXGI_FORMAT_R32_UINT, offset));
		}

		void D3DBuffer::BindUniformBufferPS(uint16_t slot) const
		{
			DX_CALL(s_devcon->PSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));

		}

		void D3DBuffer::BindUniformBufferPS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) 
		{
			DX_CALL(s_devcon->PSSetConstantBuffers(startSlot, numBuffers, reinterpret_cast<ID3D11Buffer* const*>(&buffer)));
		}

		void D3DBuffer::BindUniformBufferVS(uint16_t slot) const
		{
			DX_CALL(s_devcon->VSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));
		}

		void D3DBuffer::BindUniformBufferVS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) 
		{
			DX_CALL(s_devcon->VSSetConstantBuffers(startSlot, numBuffers, reinterpret_cast<ID3D11Buffer* const*>(&buffer)));
		}

		void D3DBuffer::BindUniformBufferDS(uint16_t slot) const
		{
			DX_CALL(s_devcon->DSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));
		}

		void D3DBuffer::BindUniformBufferDS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) 
		{
			DX_CALL(s_devcon->DSSetConstantBuffers(startSlot, numBuffers, reinterpret_cast<ID3D11Buffer* const*>(&buffer)));
		}

		void D3DBuffer::BindUniformBufferHS(uint16_t slot) const
		{
			DX_CALL(s_devcon->HSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));
		}

		void D3DBuffer::BindUniformBufferHS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) 
		{
			DX_CALL(s_devcon->HSSetConstantBuffers(startSlot, numBuffers, reinterpret_cast<ID3D11Buffer* const*>(&buffer)));
		}

		void D3DBuffer::BindUniformBufferGS(uint16_t slot) const
		{
			DX_CALL(s_devcon->GSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));
		}

		void D3DBuffer::BindUniformBufferGS(uint16_t startSlot, uint16_t numBuffers, ID3D11Buffer* buffer) 
		{
			DX_CALL(s_devcon->GSSetConstantBuffers(startSlot, numBuffers, reinterpret_cast<ID3D11Buffer* const*>(&buffer)));
		}

		void D3DBuffer::BindUniformBufferCS(uint16_t slot) const
		{
			DX_CALL(s_devcon->CSSetConstantBuffers(slot, 1, reinterpret_cast<ID3D11Buffer* const*>(&m_d3dBuffer)));
		}

		void* D3DBuffer::Map()
		{
			D3D11_MAPPED_SUBRESOURCE subRsc;

			DX_CALL(devcon->Map(m_d3dBuffer,0, D3D11_MAP_WRITE_DISCARD, NULL, &subRsc));
			return subRsc.pData;
		}

		void D3DBuffer::Unmap()
		{
			DX_CALL(devcon->Unmap(m_d3dBuffer, 0));

		}

	}
}