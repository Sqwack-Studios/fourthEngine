#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/DomainShader.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	namespace renderer
	{
		void DomainShader::LoadShader(std::wstring shaderName)
		{
			DxResPtr<ID3DBlob> pBlob;

			const std::wstring dsFullPath{ D3DRenderer::shadersPath + shaderName };
			DX_HRCALL(D3DReadFileToBlob(dsFullPath.c_str(), pBlob.reset()));
			DX_HRCALL(device->CreateDomainShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_domainShader.reset()));

			pBlob.release();
		}

		void DomainShader::Bind()
		{
			DX_CALL(devcon->DSSetShader(m_domainShader, nullptr, 0));
		}

		void DomainShader::Clear()
		{
			m_domainShader.release();
		}
	}
}