#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/HullShader.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	namespace renderer
	{
		void HullShader::LoadShader(std::wstring shaderName)
		{
			DxResPtr<ID3DBlob> pBlob;

			const std::wstring hsFullPath{ D3DRenderer::shadersPath + shaderName };

			DX_HRCALL(D3DReadFileToBlob(hsFullPath.c_str(), pBlob.reset()));
			DX_HRCALL(device->CreateHullShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_hullShader.reset()));

			pBlob.release();

		}

		void HullShader::Bind()
		{
			devcon->HSSetShader(m_hullShader, nullptr, 0);
		}

		void HullShader::Clear()
		{
			m_hullShader.release();
		}
	}
}