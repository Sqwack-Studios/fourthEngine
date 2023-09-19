#pragma once
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ComputeShader.h"
#include "include/Render/Renderer/RasterizerState.h"
#include "include/Render/Renderer/DepthStencilState.h"
#include "include/Render/Renderer/BlendState.h"
#include "include/Render/Texture.h"
#include "include/Managers/CameraManager.h"


namespace fth
{
	extern ID3D11Device5*                 s_device;
	extern ID3D11DeviceContext4*          s_devcon;
	extern IDXGIFactory5*                 s_factory;
	extern ID3D11Debug*                   s_debug;
					
	struct PER_FRAME_CONSTANT_BUFFER;
	class Skybox;

	namespace textures
	{
		struct TEXTURE_DESC;
		class Texture2D;
		class Texture2DArray;
		class CubemapArray;
	}


	namespace ibl
	{
		struct EnvironmentalIBL
		{
			EnvironmentalIBL() = default;
			EnvironmentalIBL(const std::shared_ptr<Texture>& diffuseIrr, const std::shared_ptr<Texture>& specularIrr, const std::shared_ptr<Texture>& specularRef) :
				diffuseIrradiance(diffuseIrr), specularIrradiance(specularIrr), specularReflectance(specularRef) {};

			void Bind() const;
			bool valid() const { return diffuseIrradiance && specularIrradiance && specularReflectance; }
			std::shared_ptr<Texture> diffuseIrradiance;
			std::shared_ptr<Texture> specularIrradiance;
			std::shared_ptr<Texture> specularReflectance;
		};
	}

	enum SpecularMRP : uint8_t
	{
		NONE = 0,
		KARIS = 1 << 0,
		CARPENTIER_ITERATE = 1 << 1,
		CARPENTIER_NO_ITERATE = 1 << 2
	};

	//Gaussian techniques require extra mipchains, as they are separated in horizontal and vertical passes
	enum BloomTechnique
	{
		DOWNSAMPLE_16BILINEAR_UPSAMPLE_3x3TENT = 0,
		DOWNSAMPLE_GAUSSIAN_UPSAMPLE_BILINEARMEAN,
		DOWNSAMPLE_16BILINEAR_UPSAMPLE_GAUSSIANBILINEAR,
		DOWNSAMPLE_GAUSSIAN,
		NUM,
		DISABLED
	};


	namespace renderer						  
	{									  
		extern ID3D11Device5*             device;
		extern ID3D11DeviceContext4*      devcon;
		extern IDXGIFactory5*             factory;
		extern ID3D11Debug*               debug;


		struct cb_PER_VIEW;
		class VertexShader;
		class GeometryShader;
		class Grid;


		class D3DRenderer
		{
		public:
			static const std::wstring shadersPath;

			~D3DRenderer();
			D3DRenderer(const D3DRenderer&) = delete;
			D3DRenderer& operator= (const D3DRenderer&) = delete;

			static inline D3DRenderer& Get()
			{
				static D3DRenderer instance;
				return instance;
			};

			void Init(uint16_t width, uint16_t height, uint16_t multisamples);
			void Shutdown();


		    void SetClearColor(const math::Color color) { m_clearColor = color; }
			void Update(const PER_FRAME_CONSTANT_BUFFER& cb, uint16_t width, uint16_t height, uint16_t multisamples);
			void UpdatePerViewUniform(const renderer::cb_PER_VIEW& perView);
			void Render();
			void InitRenderPipeline(uint16_t width, uint16_t height, uint16_t multisamples);
			void AttachLDR_Target(const fth::Texture& texture);
			void BindLDR_Target();
			void BindHDR_Target();
			
			void SetMainViewport(D3D11_VIEWPORT viewport);
			void BindMainViewport();

			void ShutdownViews();
			
			void StartFrame();
			void EndFrame();
			void resolveMSDepthCopy(const ShaderResourceView& src, const RenderTargetView& dst);

			void BindRenderTargets(RenderTargetView * const rtv, uint16_t numRTV, DepthStencilView * dsv);
			void BindRenderTargetsAndUAV(RenderTargetView* const rtv, uint32_t numRTV, DepthStencilView const* dsv, UnorderedAccessView const* uav, uint32_t numUav, uint16_t startSlot);


			const Texture& computeDepthBufferCopy();
			const Texture& getDepthBufferCopy() const;
			const Texture& getGBufferNormalsCopy() const;
			const Texture& getGBufferObjectIDCopy() const;
			//---------PIPELINE STATES------------
		public:
			//Default blend factor is { 1.0f, 1.0f, 1.0f, 1.0f}. Default mask is 0xFF FF FF FF. ForceBind allows to bind the same state with different blend factors and mask
			void BindBlendState(BSTYPES state, bool forceBind = false, const FLOAT* blendFactors = nullptr, UINT sampleMask = 0xFFFFFFFF);
			void BindDepthStencilState(DSSTYPES state, StencilValues refStencil = StencilValues::SKY);
			void BindRasterizerState(RSTYPES state);

			void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

			//---------DEBUGGING (?)------------------
		public:
			inline bool WireframeEnabled() const { return m_enableWireframe; }
			void BindPSNormalDebug();
			void ToggleWireframe();
			void ToggleLuminanceDebugging() { m_debugLuminance = !m_debugLuminance; };
			void SetLuminanceDebugging(bool debugLuminance) { m_debugLuminance = debugLuminance; }
			void setBloomTechnique(BloomTechnique technique) { m_bloomTechnique = technique; }

			bool roughnessOverride() const           { return m_overrideRoughness; }
			float roughnessOverrideValue() const     { return m_overrideRoughValue; }
			void SetRoughnessOverride(bool override) { m_overrideRoughness  = override; }
			void SetRoughnessValue(float rough)      { m_overrideRoughValue = rough; }

			void enableDiffuse(bool enable)  { m_enableDiffuse = enable; }
			void enableSpecular(bool enable) { m_enableSpecular = enable; }
			void enableIBL(bool enable)      { m_enableIBL = enable; }

			bool isDiffuseEnabled() const    { return m_enableDiffuse; }
			bool isSpecularEnabled() const   { return m_enableSpecular; }
			bool isIBLEnabled() const        { return m_enableIBL; }

			void setSpecularMRPFlags(SpecularMRP flags) { m_MRPFlags = flags; }
			 
		public:

			Camera& GetActiveCamera() { return m_mainCamera; }
			const Camera& GetActiveCamera() const { return m_mainCamera; }
		public:

			void SetSkybox(std::string_view cubemapName);
			void MakeFullpass();
			void ClearGeometryStage();
			void ClearTesselationStage();
			void SetRenderTargets(const DxResPtr<ID3D11RenderTargetView>& rtv, uint16_t numTargets, DxResPtr<ID3D11DepthStencilView>* dsv = nullptr);
			void SetEnvironmentalIBL(const ibl::EnvironmentalIBL& env) { m_environment = env; }

			const VertexShader &  GetDepthPass_2D_VertexShader()     const { return m_depthPass2D_VS; }
			const VertexShader &  GetDepthPass_Cube_VertexShader()   const { return m_depthPassCube_VS; }
			const GeometryShader & GetDepthPass_Cube_GeometryShader() const { return m_depthPassCube_GS; }

		private:

			void InitContext();
			void SetupMainCamera();
			void RenderShadows();
		

			void CreateDepthStencilStates();
			void CreateRasterizerStates();
			void CreateBlendStates();
			void CreateSamplers();
			void createBloomResources(uint16_t frameWidth, uint16_t frameHeight);
			void bindGBuffer();
			void UnbindViews();
			void ClearViews();


			void InitConstantBuffers();

			void UpdatePerFrameConstantBuffer(const PER_FRAME_CONSTANT_BUFFER& cb);
			void LoadShaders();
	
		
		private:
			math::Color                                     m_clearColor;

		private:
			D3DRenderer();

			DxResPtr<IDXGIFactory>                          m_factory;
			DxResPtr<IDXGIFactory5>                         m_factory5;
			DxResPtr<ID3D11Device>                          m_device;
			DxResPtr<ID3D11Device5>                         m_device5;
			DxResPtr<ID3D11DeviceContext>                   m_devcon;
			DxResPtr<ID3D11DeviceContext4>                  m_devcon4;
			DxResPtr<ID3D11Debug>                           m_devdebug;


			DxResPtr<ID3D11BlendState>                      m_blendColorAlphaBS;


			RasterizerState                                 m_rasterizerStates[(uint8_t)RSTYPES::NUM];
			RSTYPES                                         m_currentRasterizerState;
			DepthStencilState                               m_depthStencilStates[(uint8_t)DSSTYPES::NUM];
			DSSTYPES                                        m_currentDepthStencilState;
			StencilValues                                   m_currentStencilRef;
			BlendState                                      m_blendStates[(uint8_t)BSTYPES::NUM];
			BSTYPES                                         m_currentBlendState;

			bool                                            m_enableWireframe;
			bool                                            m_debugLuminance;
			BloomTechnique                                  m_bloomTechnique;
			

		private:

			//add a vector of cameras and a index to the active camera. Add a helper class
			//to access these cameras from outside
			Camera                                          m_mainCamera;
			
		private:

			static const uint8_t NUM_G_TEXTURES = 5;
		struct GBuffer
		{
			Texture  gTextures [NUM_G_TEXTURES];
			RenderTargetView gRenderTargets[NUM_G_TEXTURES];
		};

		std::unique_ptr<GBuffer>                            m_gBuffer;
			
		RenderTargetView                                    m_clearObjectID_RTV;

		Texture                                             m_hdrBufferColor;
		renderer::RenderTargetView                          m_hdrRTV;
												           
		//DepthStencil							           
		Texture                                             m_bufferDepthStencil;
		renderer::DepthStencilView                          m_DSV;
		
		//GBuffer normals copy
		Texture                                             m_gbufferNormalsCopy;
		//Gbuffer objectID copy
		Texture                                             m_gbufferObjectIDCopy;

		//depth copy							           
		Texture                                             m_depthCopy;
		renderer::RenderTargetView                          m_depthCopyRTV;
												           
		Texture                                             m_resolvePassBuffer;
		RenderTargetView                                    m_resolvePassRTV;

		//LDR target (if there's any)			           
		renderer::RenderTargetView                          m_backBufferLDR_RTV;

		private: 									       
			//HDR scene rendering pipeline
			uint16_t                                        m_width;
			uint16_t                                        m_height;
			uint16_t                                        m_maxMSAA_qualitySupported;//Deprecated
													       
			D3D11_VIEWPORT                                  m_mainViewport;
		private:									       
			PixelShader                                     m_normalDebugPS;
			PixelShader                                     m_resolveLightningPS;
			PixelShader                                     m_resolveUnlitPS;
			VertexShader                                    m_fullPassPassthroughVS;

			//DepthPass
			VertexShader                                    m_depthPass2D_VS;
			//Cubemap					                    
			VertexShader                                    m_depthPassCube_VS;
			GeometryShader                                  m_depthPassCube_GS;

			//MipChain
			ComputeShader                                   m_fft_CS;

			std::unique_ptr<Grid>                           m_Grid = nullptr;
			std::unique_ptr<Skybox>                         m_Skybox = nullptr;
					
			static constexpr uint8_t MAX_MIPS_BLOOM = 6;
			//Bloom
			Texture                                         m_bloomTexture;
			ShaderResourceView                              m_bloomTextureSRV[MAX_MIPS_BLOOM];
			UnorderedAccessView                             m_bloomTextureUAV[MAX_MIPS_BLOOM];

			//Some bloom techniques need up to 3 mipchains
			Texture                                         m_optional1_BloomTexture;
			ShaderResourceView                              m_optional1_BloomTextureSRV[MAX_MIPS_BLOOM];
			UnorderedAccessView                             m_optional1_BloomTextureUAV[MAX_MIPS_BLOOM];

			Texture                                         m_optional2_BloomTexture;
			ShaderResourceView                              m_optional2_BloomTextureSRV[MAX_MIPS_BLOOM];
			UnorderedAccessView                             m_optional2_BloomTextureUAV[MAX_MIPS_BLOOM];

			Texture                                         m_output;
			Texture                                         m_realXYZimX;
			Texture                                         m_imYZ;
			UnorderedAccessView                             m_fftUAVs[3];

			UniformBuffer                                   m_uniformPerView;
			UniformBuffer                                   m_uniformPerFrame;
			UniformBuffer                                   m_fftUniform;

			DxResPtr<ID3D11SamplerState>                    m_samplerPointWrap;
			DxResPtr<ID3D11SamplerState>                    m_samplerTrilinearWrap;
			DxResPtr<ID3D11SamplerState>                    m_samplerAnisotropicWrap;

			DxResPtr<ID3D11SamplerState>                    m_samplerPointClamp;
			DxResPtr<ID3D11SamplerState>                    m_samplerBilinearClamp;
			DxResPtr<ID3D11SamplerState>                    m_samplerTrilinearClamp;
			DxResPtr<ID3D11SamplerState>                    m_samplerAnisotropicClamp;

			DxResPtr<ID3D11SamplerState>                    m_samplerPointBorder;
			DxResPtr<ID3D11SamplerState>                    m_samplerCmpBilinearBorder;


			ibl::EnvironmentalIBL                           m_environment;


			uint32_t                                        m_enableDiffuse       = 1;
			uint32_t                                        m_enableSpecular      = 1;
			uint32_t                                        m_enableIBL           = 1;
			uint32_t                                        m_overrideRoughness   = 0;
			float                                           m_overrideRoughValue  = 1.0f;
			SpecularMRP                                     m_MRPFlags = SpecularMRP::NONE;
		};

		struct cb_PER_VIEW
		{
				math::Matrix     worldToClip;		 //64bytes      
				math::Matrix     clipToWorld;	     //64bytes     
				math::Matrix     projection;	     //64bytes
				math::Matrix     cameraToWorld;	     //64bytes
				math::Matrix     clipToCamera;       //64bytes
				math::Matrix     worldToCamera;	     //64bytes
				math::Vector4    worldDirectionBL;   //16bytes
				math::Vector4    worldDirectionBR;   //16bytes
				math::Vector4    worldDirectionTL;   //16bytes

				float            nearPlane;			 //4 bytes
				float            farPlane;	         //4 bytes
				float            viewportWidth;      //4 bytes
				float            viewportHeight;      //4 bytes

				float            viewportAR;
				float            pad[3];
		};

		struct fftdata
		{
			int input_width;
			int input_height;
			int output_width;
			int output_height;

			int logtwo_width;
			int logtwo_height;
			int clz_width;
			int clz_height;

			int channel_no;
			int stage;
			float pad[2];
		};

		struct fftdata2
		{
			uint32_t fft_width;
			uint32_t fft_height;
			float    log2_fft_size;
			uint32_t forward;
		};
	}
	

}