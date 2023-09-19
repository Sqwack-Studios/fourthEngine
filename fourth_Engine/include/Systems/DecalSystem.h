#pragma once
#include "include/Systems/TransformSystem.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"


namespace fth
{
	class Texture;
	struct RenderableObject;


	struct DecalMaterial
	{
		std::shared_ptr<Texture> tx_normal   ;

		bool operator==(const DecalMaterial rval) { return tx_normal == rval.tx_normal && invertNormals == rval.invertNormals && buildBlueNormals == rval.buildBlueNormals; };
		bool  invertNormals;
		bool  buildBlueNormals;

	};

	class DecalSystem
	{
	public:
		static DecalSystem& Get()
		{
			static DecalSystem instance;
			return instance;
		}
		~DecalSystem();

		void Init();
		void Shutdown();
		void addDecal(const DecalMaterial& material, Handle<math::Matrix> decalToModel, Handle<math::Matrix> parentID, Handle<RenderableObject> parentObjectID, const math::Color& color);
		void draw();
		void update();

	private:
		DecalSystem();

		struct DecalInstanceCPU
		{
			DecalInstanceCPU(const math::Color& color, Handle<math::Matrix> dMid, Handle<math::Matrix> pID, Handle<RenderableObject> pObID):
				color(color), decalToModelID(dMid), parentID(pID), parentObjectID(pObID){}

			math::Color  color;
			Handle<math::Matrix>         decalToModelID;
			Handle<math::Matrix>         parentID;
			Handle<RenderableObject>     parentObjectID;
		};

		struct DecalInstanceGPU
		{
			math::Matrix decalToWorld;
			math::Matrix worldToDecal;
			math::Color  color;
			uint32_t     parentObjectID;
		};

		struct PerMaterialGPU
		{
			uint32_t  invertNormal_buildBlue;
			float     pad[3];
		};

		struct PerMaterial
		{
			PerMaterial(const DecalMaterial& mat) :material(mat) {}
			DecalMaterial material;

			std::vector<DecalInstanceCPU> instances;
		};

		renderer::UniformBuffer    m_perMaterialUniform;
		renderer::InstanceBuffer   m_instanceBuffer;

		renderer::VertexShader     m_decalVS;
		renderer::PixelShader      m_decalPS;

		std::vector<PerMaterial>   m_perMaterial;
		uint32_t                   m_numInstances;
	};
}