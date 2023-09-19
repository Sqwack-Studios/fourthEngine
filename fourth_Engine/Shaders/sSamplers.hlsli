#include "Registers.hlsli"


#ifndef _SAMPLERS_HLSL
#define _SAMPLERS_HLSL
sampler g_pointWrap         : register(s_POINT_WRAP);
sampler g_trilinearWrap     : register(s_TRI_WRAP);
sampler g_anisotropicWrap   : register(s_ANIS_WRAP);
sampler g_pointClamp        : register(s_POINT_CLAMP);
sampler g_bilinearClamp     : register(s_BIL_CLAMP);
sampler g_trilinearClamp    : register(s_TRI_CLAMP);
sampler g_anisotropicClamp  : register(s_ANIS_CLAMP);
sampler g_pointBorder       : register(s_POINT_BORDER);
SamplerComparisonState g_bilinearCmpClamp : register(s_TRI_CMP_CLAMP);

#endif //_SAMPLERS_HLSL