#pragma once
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/RasterizerState.h"
#include "include/Render/Renderer/DepthStencilState.h"

namespace fth
{
	class Texture;

	class Skybox
	{
	public:

		void Init();
		void Draw();
		void SetCubemap(std::string_view cubemap);
		inline const std::shared_ptr<Texture>& GetCubemapSlice() const { return m_cubemap; }
	private:
		renderer::VertexShader                    m_vertexShader;
		renderer::PixelShader                     m_pixelShader;
		std::shared_ptr<Texture>                  m_cubemap;
		renderer::RSTYPES                         m_RS;
		renderer::DSSTYPES                        m_DSS;
	};
}