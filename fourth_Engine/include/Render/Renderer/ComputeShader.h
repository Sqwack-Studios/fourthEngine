#pragma once
#include "include/Utils/DxRes.h"

struct ID3D11ComputeShader;

namespace fth::renderer
{
	class ComputeShader
	{
	public:
		void loadShader(std::wstring_view shaderName);
		void bind();
		static void dispatch(uint16_t x, uint16_t y, uint16_t z);
		void reset();

	private:
		DxResPtr<ID3D11ComputeShader> m_compute;
	};
}