#include "pch.h"

#pragma once
#include "include/Render/Renderer/BlendState.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "Shaders/RenderTargets.hlsli"
namespace fth::renderer
{

	extern ID3D11Device5* device;
	extern ID3D11DeviceContext4* devcon;

	void BlendState::BindState(const float* blendFactor, uint32_t mask)
	{
		DX_CALL(devcon->OMSetBlendState(m_blendState, blendFactor, mask));
	}

	BlendState BlendState::CreateBlendState(BSTYPES bsType)
	{
		switch (bsType)
		{
		case fth::renderer::BSTYPES::DEFAULT_A2C:
			return CreateDefaultA2C();
		case fth::renderer::BSTYPES::GENERAL_TRANSLUCENCY:
			return CreateGeneralTranslucency();
		case fth::renderer::BSTYPES::ADDITIVE_TRANSLUCENCY:
			return CreateAdditiveTranslucency();
		case fth::renderer::BSTYPES::GENERAL_TRANSLUCENCY_ALBEDO_ROUGHMETAL_EMISSION:
			return CreateGeneralTranslucencyGBuffer();
		}
		return CreateDefaultBlendState();
	}


	BlendState BlendState::CreateBlendState(const D3D11_BLEND_DESC& bsDsc)
	{
		BlendState bs;

		DX_HRCALL(device->CreateBlendState(&bsDsc, bs.m_blendState.reset()));

		return bs;
	}

	BlendState BlendState::CreateDefaultBlendState()
	{
		D3D11_BLEND_DESC bsDsc;

		bsDsc.AlphaToCoverageEnable = false;
		bsDsc.IndependentBlendEnable = false;
		bsDsc.RenderTarget[0].BlendEnable = false;
		bsDsc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		bsDsc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bsDsc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		return CreateBlendState(bsDsc);
	}

	BlendState BlendState::CreateDefaultA2C()
	{
		D3D11_BLEND_DESC bsDsc;

		bsDsc.AlphaToCoverageEnable = true;
		bsDsc.IndependentBlendEnable = false;
		bsDsc.RenderTarget[0].BlendEnable = false;
		bsDsc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		bsDsc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bsDsc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		return CreateBlendState(bsDsc);
	}

	BlendState BlendState::CreateGeneralTranslucency()
	{
		D3D11_BLEND_DESC bsDsc;

		bsDsc.AlphaToCoverageEnable = false;
		bsDsc.IndependentBlendEnable = false;
		bsDsc.RenderTarget[0].BlendEnable = true;
		bsDsc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bsDsc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bsDsc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		bsDsc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		return CreateBlendState(bsDsc);
	}


	BlendState BlendState::CreateAdditiveTranslucency()
	{
		D3D11_BLEND_DESC bsDsc;

		bsDsc.AlphaToCoverageEnable = false;
		bsDsc.IndependentBlendEnable = false;
		bsDsc.RenderTarget[0].BlendEnable = true;
		bsDsc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bsDsc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		bsDsc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsDsc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		return CreateBlendState(bsDsc);
	}

	BlendState BlendState::CreateGeneralTranslucencyGBuffer()
	{
		D3D11_BLEND_DESC bsDsc;

		bsDsc.AlphaToCoverageEnable = false;
		bsDsc.IndependentBlendEnable = true;
		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDscEnable;
		rtBlendDscEnable.BlendEnable = true;
		rtBlendDscEnable.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtBlendDscEnable.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtBlendDscEnable.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDscEnable.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDscEnable.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		rtBlendDscEnable.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDscEnable.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


		D3D11_RENDER_TARGET_BLEND_DESC rtBlendDscDisable;
		rtBlendDscDisable.BlendEnable = false;
		rtBlendDscDisable.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtBlendDscDisable.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtBlendDscDisable.BlendOp = D3D11_BLEND_OP_ADD;
		rtBlendDscDisable.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtBlendDscDisable.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		rtBlendDscDisable.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtBlendDscDisable.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;


		bsDsc.RenderTarget[TARGET_ALBEDO_SLOT] = rtBlendDscEnable;
		bsDsc.RenderTarget[TARGET_ROUGH_METAL_SLOT] = rtBlendDscEnable;
		bsDsc.RenderTarget[TARGET_TX_GM_NORMAL_SLOT] = rtBlendDscDisable;
		bsDsc.RenderTarget[TARGET_OBJECTID_SLOT] = rtBlendDscDisable;
		bsDsc.RenderTarget[TARGET_EMISSION_SLOT] = rtBlendDscEnable;

		bsDsc.RenderTarget[5] = rtBlendDscDisable;
		bsDsc.RenderTarget[6] = rtBlendDscDisable;
		bsDsc.RenderTarget[7] = rtBlendDscDisable;

		return CreateBlendState(bsDsc);
	}
}