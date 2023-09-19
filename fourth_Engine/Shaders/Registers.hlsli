#include "RegisterSlots.hlsli"
#include "Macros.hlsli"

#ifndef _REGISTERS_HLSL
#define _REGISTERS_HLSL

#define JOIN(a, b) a##b

//UNIFORM REGISTERS
//Registers: HLSLI ONLY

#define cb_PER_FRAME           JOIN(b, PER_FRAME_SLOT)
#define cb_PER_VIEW            JOIN(b, PER_VIEW_SLOT)
#define cb_PER_DRAW            JOIN(b, PER_DRAW_SLOT)
#define cb_LIGHTS              JOIN(b, LIGHTS_SLOT)
#define cb_MATERIAL            JOIN(b, PER_MATERIAL_SLOT)
#define cb_IBL_GEN             JOIN(b, IBL_GEN_SLOT)
#define cb_SHADOW_MAPS         JOIN(b, SHADOW_MAP_SLOT)
#define cb_SHADOW_PASS_CUBE    JOIN(b, SHADOW_PASS_CUBE_SLOT)
#define cb_SMOKE_TILED_ATLAS   JOIN(b, SMOKE_TILED_ATLAS_SLOT)			    
#define cb_PP_RESOLVE          JOIN(b, POSTPROCESS_UNIFORM_SLOT)
#define cb_FXAA_RESOLVE        JOIN(b, CB_FXAA_RESOLVE_SLOT)
//TEXTURE REGISTERS	    
#define srv_ALBEDO             JOIN(t, TX_ALBEDO_SLOT)
#define srv_NORMAL             JOIN(t, TX_NORMAL_SLOT)
#define srv_ROUGH              JOIN(t, TX_ROUGH_SLOT)
#define srv_METAL              JOIN(t, TX_METAL_SLOT)

//DECAL RENDERING
#define srv_DECAL_DEPTH        JOIN(t, TX_DECAL_DEPTH_SLOT)
#define srv_DECAL_NORMAL_ALPHA JOIN(t, TX_DECAL_NORMAL_ALPHA_SLOT)
#define srv_DECAL_NORMAL_GB    JOIN(t, TX_DECAL_NORMAL_GB_SLOT)
#define srv_DECAL_OBJECTID_GB  JOIN(t, TX_DECAL_OBJECTID_GB_SLOT)
//TRANSLUCENCY RENDERING
#define srv_DIS_NOISE          JOIN(t, TX_DIS_NOISE_SLOT)
#define srv_INC_NOISE          JOIN(t, TX_INC_NOISE_SLOT)
#define srv_LM_RLU             JOIN(t, TX_PARTICLES_RLU_SLOT)
#define srv_LM_DBF             JOIN(t, TX_PARTICLES_DBF_SLOT)
#define srv_LM_EMVA            JOIN(t, TX_PARTICLES_EMVA_SLOT)

#define srv_RANGE_BUFFER       JOIN(t, BUF_RANGE_RENDER_SLOT)
#define srv_PARTICLE_BUFFER    JOIN(t, BUF_PARTICLE_RENDER_SLOT)
#define srv_PARTICLE_MASK      JOIN(t, PARTICLE_MASK_SLOT)
//PARTICLE SIMULATION
#define srv_PSIM_DEPTH		         JOIN(t, TX_PART_SIM_DEPTH_SLOT)
#define srv_PSIM_NORMALS_GB	         JOIN(t, TX_PART_SIM_NORMAL_GB_SLOT)
#define uav_PSIM_RING_BUFFER         JOIN(u, RW_PART_SIM_RING_BUFF_SLOT)
#define uav_PSIM_RANGE_BUFFER        JOIN(u, RW_PART_SIM_RANGE_BUFF_SLOT)
#define uav_PSIM_RANGE_UPDATE        JOIN(u, RW_PART_RANGE_UPDATE_SLOT)

//DEFERRED GBUFFER RESOLUTION
#define srv_GB_ALBEDO          JOIN(t, TX_GB_ALBEDO_SLOT)
#define srv_GB_ROUGH_METAL     JOIN(t, TX_GB_ROUGH_METAL_SLOT)
#define srv_GB_NORMAL          JOIN(t, TX_GB_NORMAL_SLOT)
#define srv_GB_OBJECTID        JOIN(t, TX_GB_OBJECTID_SLOT)
#define srv_GB_DEPTH           JOIN(t, TX_GB_DEPTH_SLOT)
#define srv_GB_EMISSION        JOIN(t, TX_GB_EMISSION_SLOT)


#define srv_LIGHTMASK          JOIN(t, TX_LIGHTMASK_SLOT)
#define srv_IBL_D_IRR          JOIN(t, TX_IBL_D_IRR_SLOT)
#define srv_IBL_S_IRR          JOIN(t, TX_IBL_S_IRR_SLOT)
#define srv_IBL_S_REF          JOIN(t, TX_IBL_S_REF_SLOT)
#define srv_SM_DIRECT          JOIN(t, TX_SM_DIRECT_SLOT)
#define srv_SM_POINT           JOIN(t, TX_SM_POINT_SLOT)
#define srv_SM_SPOT            JOIN(t, TX_SM_SPOT_SLOT)

//MULTISAMPLED DEPTH RESOLVING
#define srv_RESOLVE_DEPTH      JOIN(t, TX_RESOLVE_DEPTH)

//SKY RENDERING
#define srv_SKYMAP             JOIN(t, TX_SKYMAP_SLOT)
//POSTPROCESS

#define SRV_PP_0               JOIN(t, SRV_PP_SLOT0)
#define SRV_PP_1               JOIN(t, SRV_PP_SLOT1)
#define SRV_PP_2               JOIN(t, SRV_PP_SLOT2)
#define SRV_PP_3               JOIN(t, SRV_PP_SLOT3)
#define SRV_PP_4               JOIN(t, SRV_PP_SLOT4)
#define SRV_PP_5               JOIN(t, SRV_PP_SLOT5)

#define UAV_PP_COMPUTE_0       JOIN(u, UAV_PP_COMPUTE_SLOT0)
#define UAV_PP_COMPUTE_1       JOIN(u, UAV_PP_COMPUTE_SLOT1)
#define UAV_PP_COMPUTE_2       JOIN(u, UAV_PP_COMPUTE_SLOT2)
#define UAV_PP_COMPUTE_3       JOIN(u, UAV_PP_COMPUTE_SLOT3)
#define UAV_PP_COMPUTE_4       JOIN(u, UAV_PP_COMPUTE_SLOT4)
#define UAV_PP_COMPUTE_5       JOIN(u, UAV_PP_COMPUTE_SLOT5)

#define UAV_PP_PIPELLINE_0     JOIN(u, UAV_PP_PIPELINE_SLOT0)
#define UAV_PP_PIPELLINE_1     JOIN(u, UAV_PP_PIPELINE_SLOT1)

#define CB_PP_0                JOIN(b, CB_PP_SLOT0)
#define CB_PP_1                JOIN(b, CB_PP_SLOT1)

//Bloom
#define uav_DS_HDR             JOIN(u, RW_DS_BLOOM_HDR_SLOT)
#define cb_DS_BLOOM            JOIN(b, cb_DS_BLOOM_HDR_SLOT)
//----Resolve----
#define srv_HDR_RESOLVE        JOIN(t,TX_HDR_RESOLVE_SLOT)
#define srv_BLOOM_RESOLVE      JOIN(t,TX_RESOLVE_BLOOM_SLOT)
//----Resolve FXAA----

#define srv_LDR_FXAA           JOIN(t, TX_LDR_FXAA_SLOT)


//SAMPLER REGISTERS
#define s_POINT_WRAP    JOIN(s, POINT_WRAP_SLOT)
#define s_TRI_WRAP      JOIN(s, TRI_WRAP_SLOT)
#define s_ANIS_WRAP     JOIN(s, ANIS_WRAP_SLOT)
#define s_POINT_CLAMP   JOIN(s, POINT_CLAMP_SLOT)
#define s_BIL_CLAMP     JOIN(s, BIL_CLAMP_SLOT)
#define s_TRI_CLAMP     JOIN(s, TRI_CLAMP_SLOT)
#define s_ANIS_CLAMP    JOIN(s, ANIS_CLAMP_SLOT)
#define s_POINT_BORDER  JOIN(s, POINT_BORDER_SLOT)
#define s_TRI_CMP_CLAMP JOIN(s, BI_CLAMP_CMP_SLOT)


//UAV REGISTERS(?)

#define uav_RANGE_BUFFER      JOIN(u, RW_RANGE_BUFFER_SLOT)
#define uav_PARTICLES_BUFFER  JOIN(u, RW_PARTICLES_BUFFER_SLOT)
//BUFFER OFFSET REGISTERS(?)

#endif //_REGISTERS_HLSL