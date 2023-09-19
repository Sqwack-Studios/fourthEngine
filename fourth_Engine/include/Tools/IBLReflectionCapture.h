#pragma once
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"

#include "include/Render/Renderer/UniformBuffer.h"

namespace fth
{
	class Texture;
}

enum DXGI_FORMAT;

namespace fth::ibl
{
	class IBLReflectionCapture
	{
	public:
		static IBLReflectionCapture& Get()
		{
			static IBLReflectionCapture instance;
			return instance;
		}
		void Init();
	    void GenerateCubemapDiffuseIrradiance(const Texture& src, std::string_view filePath, uint32_t width, uint32_t height,  uint32_t samples, Texture& dst);
	    void GenerateCubemapSpecularIrradiance(const Texture& src, std::string_view filePath, uint32_t width, uint32_t height, uint32_t samples, Texture& dst);
	    void GenerateGGXSpecularReflectanceLUT(uint32_t width, std::string_view filePath, uint32_t height, uint32_t samples, Texture& dst);

	private:
		struct UniformData
		{
			uint32_t numSamples;
			float    roughness;
			float    pad[2];
		};

		void Capture(const Texture& src, DXGI_FORMAT format, uint32_t width, uint32_t height, uint16_t mips, uint32_t samples, const renderer::PixelShader& targetPS, Texture& dst);

		renderer::VertexShader     m_cubemapGenVS;
		renderer::PixelShader      m_diffuseIrradiancePS;
		renderer::PixelShader      m_specularIrradiancePS;
		renderer::PixelShader      m_specularReflectancePS;

		renderer::GeometryShader   m_faceSelectorGS;

		renderer::UniformBuffer    m_uniform;

		
	};
}