#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	namespace renderer
	{
		void PixelShader::LoadShader(std::wstring_view shaderName)
		{
			DxResPtr<ID3DBlob> pBlob;

			const std::wstring psFullPath(D3DRenderer::shadersPath + std::wstring(shaderName));

			DX_HRCALL(D3DReadFileToBlob(psFullPath.c_str(), pBlob.reset()));
			DX_HRCALL(s_device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_pixelShader.reset()));

			pBlob.release();
		}

		void PixelShader::Bind() const
		{
			DX_CALL(s_devcon->PSSetShader(m_pixelShader, nullptr, 0));
		}

		void PixelShader::Reset()
		{
			m_pixelShader.reset();
		}

		void PixelShader::ClearFromPipeline()
		{
			DX_CALL(s_devcon->PSSetShader(nullptr, nullptr, 0));
		}
	}
}