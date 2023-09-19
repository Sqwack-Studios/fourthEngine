#ifndef _RT_SLOTS_HLSL
#define _RT_SLOTS_HLSL

//--------G-BUFFERS---------
#define TARGET_ALBEDO_SLOT          0
#define TARGET_ROUGH_METAL_SLOT     1
#define TARGET_TX_GM_NORMAL_SLOT    2
#define TARGET_OBJECTID_SLOT        3
#define TARGET_EMISSION_SLOT        4




//COMBINE FRAME TARGETS - FINAL FRAME PROCESSING
#define TARGET_COMBINE_HDR_SLOT     0//Bind at frame start, combine, unbind, use as SRV
//POST-PROCESSING
#define TARGET_BACKBUFFER_LDR_SLOT     0
#define TARGET_FXAAPASS_LDR_SLOT       0
#endif //_RT_SLOTS_HLSL