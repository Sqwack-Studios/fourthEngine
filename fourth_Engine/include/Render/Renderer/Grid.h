#pragma once
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/RasterizerState.h"
#include "include/Render/Renderer/DepthStencilState.h"
#include "include/Render/Renderer/BlendState.h"



struct ID3D11BlendState;
struct ID3D11DeviceContext4;
namespace fth
{
	namespace renderer
	{

		class Grid
		{
		public:
			void Init();
			void Draw();
			void Shutdown();

		private:

			VertexShader                              m_gridVS;
			PixelShader                               m_gridPS;
			RSTYPES                                   m_RS;
			DSSTYPES                                  m_DSS;
			BSTYPES                                   m_BSS;
		};
	}
}