#include "pch.h"

#pragma once
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/D3DRenderer.h"


namespace fth
{
	namespace renderer
	{
		void GeometryShader::LoadShader(std::wstring_view shaderName)
		{
			DxResPtr<ID3DBlob> pBlob;

			const std::wstring gsFullPath(D3DRenderer::shadersPath + std::wstring(shaderName));

			DX_HRCALL(D3DReadFileToBlob(gsFullPath.c_str(), pBlob.reset()));
			DX_HRCALL(device->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, m_geometryShader.reset()));

			pBlob.release();
		}

		void GeometryShader::Bind()
		{
			DX_CALL(devcon->GSSetShader(m_geometryShader, nullptr, 0));
		}
		

		void GeometryShader::Clear()
		{
			m_geometryShader.reset();
		}
	}
}