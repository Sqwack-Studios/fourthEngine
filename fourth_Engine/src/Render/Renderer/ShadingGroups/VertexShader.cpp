#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/D3DRenderer.h"
namespace fth
{
	namespace renderer
	{

		void VertexShader::LoadShader(std::wstring_view shaderName, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT descSize)
		{
			DxResPtr<ID3DBlob>  pBlob;

			const std::wstring vsFullPath{ D3DRenderer::shadersPath + std::wstring(shaderName) };

			DX_HRCALL(D3DReadFileToBlob(vsFullPath.c_str(), pBlob.reset()));
			DX_HRCALL(s_device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_vertexShader.reset()));

			if(layoutDesc)
			{
				DX_HRCALL(s_device->CreateInputLayout(layoutDesc, descSize, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), m_inputLayout.reset()));
			}

			pBlob.release();
		}

		void VertexShader::Bind()
		{
			DX_CALL(s_devcon->VSSetShader(m_vertexShader, nullptr, 0));

			if(m_inputLayout)
			{
				DX_CALL(devcon->IASetInputLayout(m_inputLayout));
			}
			else
			{
				DX_CALL(devcon->IASetInputLayout(nullptr));
			}
		}

		void VertexShader::Reset()
		{
			m_vertexShader.release();
		}
	}
}