#ifndef _REGISTER_SLOTS_HLSL
#define _REGISTER_SLOTS_HLSL
//UNIFORM REGISTERS
//Slots: C++ and HLSL code
#define PER_FRAME_SLOT             0
#define PER_VIEW_SLOT              1
#define PER_DRAW_SLOT              2
#define LIGHTS_SLOT                3
#define PER_MATERIAL_SLOT          4
#define IBL_GEN_SLOT               5
#define SHADOW_MAP_SLOT            6
#define SHADOW_PASS_CUBE_SLOT      3
#define SMOKE_TILED_ATLAS_SLOT     2
#define CB_FXAA_RESOLVE_SLOT	   2			   

//TEXTURE REGISTERS				   
//OPAQUE RENDERING				   
#define TX_ALBEDO_SLOT             0
#define TX_NORMAL_SLOT             1
#define TX_ROUGH_SLOT              2
#define TX_METAL_SLOT              3
								   
#define TX_DIS_NOISE_SLOT          4
#define TX_INC_NOISE_SLOT          4
//DECAL RENDERING
#define TX_DECAL_DEPTH_SLOT        1
#define TX_DECAL_NORMAL_ALPHA_SLOT 2
#define TX_DECAL_NORMAL_GB_SLOT    3
#define TX_DECAL_OBJECTID_GB_SLOT  4

//PARTICLE SIMULATION
#define TX_PART_SIM_DEPTH_SLOT          1
#define TX_PART_SIM_NORMAL_GB_SLOT      2
#define RW_PART_SIM_RING_BUFF_SLOT      7
#define RW_PART_SIM_RANGE_BUFF_SLOT     8
#define RW_PART_RANGE_UPDATE_SLOT       8

//TRANSLUCENCY RENDERING
#define TX_PARTICLES_RLU_SLOT    12
#define TX_PARTICLES_DBF_SLOT    13
#define TX_PARTICLES_EMVA_SLOT   14
#define TX_RESOLVE_DEPTH         0
#define BUF_RANGE_RENDER_SLOT    16
#define	BUF_PARTICLE_RENDER_SLOT 17
#define PARTICLE_MASK_SLOT       18
//DEFERRED GBUFFER RESOLUTION
#define TX_GB_ALBEDO_SLOT        0
#define TX_GB_ROUGH_METAL_SLOT   1
#define TX_GB_NORMAL_SLOT        2
#define TX_GB_OBJECTID_SLOT      3
#define TX_GB_EMISSION_SLOT      4
#define TX_GB_DEPTH_SLOT         5

#define TX_LIGHTMASK_SLOT        6
#define TX_IBL_D_IRR_SLOT        7
#define TX_IBL_S_IRR_SLOT        8
#define TX_IBL_S_REF_SLOT        9
#define TX_SM_DIRECT_SLOT        10
#define TX_SM_POINT_SLOT         11
#define TX_SM_SPOT_SLOT          12

//SKY RENDERING
#define TX_SKYMAP_SLOT           127

//POSTPROCESS

#define SRV_PP_SLOT0              64
#define SRV_PP_SLOT1              65
#define SRV_PP_SLOT2              66
#define SRV_PP_SLOT3              67
#define SRV_PP_SLOT4              68
#define SRV_PP_SLOT5              69

#define UAV_PP_COMPUTE_SLOT0	   0
#define UAV_PP_COMPUTE_SLOT1       1
#define UAV_PP_COMPUTE_SLOT2       2
#define UAV_PP_COMPUTE_SLOT3       3
#define UAV_PP_COMPUTE_SLOT4       4
#define UAV_PP_COMPUTE_SLOT5       5

#define UAV_PP_PIPELINE_SLOT0      7
#define UAV_PP_PIPELINE_SLOT1      8

#define CB_PP_SLOT0                2
#define CB_PP_SLOT1                3



#define TX_HDR_RESOLVE_SLOT      0
#define TX_RESOLVE_BLOOM_SLOT    1
#define RW_DS_BLOOM_HDR_SLOT     0
#define cb_DS_BLOOM_HDR_SLOT     2
#define POSTPROCESS_UNIFORM_SLOT 2
//----Resolve----
#define cb_HDR_RESOLVE      2
//----Resolve FXAA----
#define TX_LDR_FXAA_SLOT    0

//SAMPLER REGISTERS
#define POINT_WRAP_SLOT     0
#define TRI_WRAP_SLOT       1
#define ANIS_WRAP_SLOT      2
#define POINT_CLAMP_SLOT    3
#define BIL_CLAMP_SLOT      4
#define TRI_CLAMP_SLOT      5
#define ANIS_CLAMP_SLOT     6
#define POINT_BORDER_SLOT   7
#define BI_CLAMP_CMP_SLOT   8

//UAV REGISTERS(?)
#define RW_PARTICLES_BUFFER_SLOT  7
#define RW_RANGE_BUFFER_SLOT      8
//BUFFER OFFSET REGISTERS(?)

#endif //_REGISTERS_SLOTS_HLSL