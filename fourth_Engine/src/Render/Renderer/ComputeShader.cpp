#include "pch.h"
#pragma once
#include "include/Render/Renderer/ComputeShader.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth::renderer
{
	void ComputeShader::loadShader(std::wstring_view shaderName)
	{
		DxResPtr<ID3DBlob> pBlob;

		const std::wstring psFullPath(D3DRenderer::shadersPath + std::wstring(shaderName));

		DX_HRCALL(D3DReadFileToBlob(psFullPath.c_str(), pBlob.reset()));
		DX_HRCALL(s_device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_compute.reset()));

		pBlob.release();
	}

	void ComputeShader::bind()
	{
		DX_CALL(s_devcon->CSSetShader(m_compute, nullptr, 0));
	}
	void ComputeShader::dispatch(uint16_t x, uint16_t y, uint16_t z)
	{
		DX_CALL(s_devcon->Dispatch(x, y, z));
	}
	void ComputeShader::reset()
	{
		m_compute.reset();
	}
}