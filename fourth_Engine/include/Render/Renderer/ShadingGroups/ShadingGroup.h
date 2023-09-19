 #pragma once
#include <stdint.h>
#include <memory>
#include <vector>
#include "include/Utils/SolidVector.h"


namespace fth
{
	class Model;
	struct Ray;
	namespace math
	{
		struct HitResult;
	}


//Each shading group mesh instance has 24 free bits to account for:
//32bits ID(not handle) : 
//RESERVED:
// 8bits  ---> shading identifier
//SHADING_GROUP SPECIFIC
//XXbits ---> model index
//XXbits ---> material index
//XXbits ---> material instance ID
//
//
//When adding an instance, a per MeshInstance data stream as big as
//numInstances * instanceMeshes is passed to the shading group to write each instance composedID, 
//and then MeshSystem will add it into SolidVector to create the instance Handle

//anonymous/TAG struct to create a RenderableObject handle
struct RenderableObject{};
//anonymous/TAG struct to create a combined instance identifier handle
struct CombinedID {};

	struct PerMeshInstance
	{
		PerMeshInstance(Handle<CombinedID> inComposed): composedID(inComposed) {}
		Handle<CombinedID> composedID;
	};

	struct ModelInstance
	{
		std::vector<PerMeshInstance> perMeshInstance;
	};

}


namespace fth::shading
{

	struct InstanceFinder
	{
		Handle<math::Matrix>     handle_transform;
		Handle<ModelInstance>    handle_instance;
		Handle<RenderableObject> handle_object;
		void invalidate() { handle_transform.invalidate() /*instanceID.invalidate(),*/, handle_transform.invalidate(); }
	};



	class IShadingGroup
	{
	public:
		virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits);
		virtual void UpdateInstanceBuffer() = 0;

		virtual void Render() = 0;
		virtual void RenderDebugNormals() = 0;

		void ToggleNormalDebugging() { m_debugNormals = !m_debugNormals; }
		bool ShouldDebugNormals() { return m_debugNormals; }

		//virtual void GetInstanceTransforms(const InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances ) = 0;


	protected:
		uint32_t computeModelIdx(uint32_t flag) const;
		uint32_t computeMaterialIdx(uint32_t flag) const;
		uint32_t computeInstanceHandle(uint32_t flag) const;

		//Num of bits destined to composite an instanceID
		uint32_t  instanceMask;
		uint8_t   modelBits;
		uint32_t  modelMask;
		uint32_t  materialMask;
		uint8_t   materialBits;
		uint8_t   instanceBits;

	private:


		bool m_debugNormals;
		
	};

	namespace utility
	{
		//Adds a model into a shading group. Returns an index that locally identifies that model. Cache that index for faster data retrieval and addition operations.
		template<typename TGroupPerModel, typename TGroup>
		void addModel(TGroup& group, const std::shared_ptr<Model>& srcModel, Handle<TGroupPerModel>& outHandle)
		{
			uint32_t modelIndex{};
			{
				if (group.m_perModel.size() - 1 == group.modelMask)
				{
					outHandle = Handle<TGroupPerModel>::Invalid();
				}

				for (TGroup::PerModel& perModel : group.m_perModel)
				{
					if (perModel.srcModel == srcModel)
					{
						outHandle = modelIndex;
						return;
					}

					++modelIndex;
				}
			

				group.m_perModel.emplace_back(srcModel);
				
				outHandle = modelIndex;
			}
		}

		//Adds a model material. Note that each model is composited by as many materials as meshes, so materials stream is as big as Model->NumMeshes
        //Outputs an stream of material indices, which is as big as materials input stream. Cache material indices for faster data retrieval and addition operations.
		template<typename TGroup, typename TGroupMaterial, typename TGroupPerModel>
		void addMaterial(TGroup& group, const TGroupMaterial* materials, Handle<TGroupPerModel> modelIdx, Handle<TGroupMaterial>* outMaterialIdx)
		{
			uint32_t cmpMask{ (modelIdx << (group.instanceBits + group.materialBits) ) & group.modelMask };

			if (!modelIdx.isValid() || (cmpMask) == group.modelMask)
			{
				LOG_ENGINE_WARN("addMaterial", "provided Handle<PerModel> is invalid, material was ignored");
				return;
			}

			TGroup::PerModel& perModel = group.m_perModel[modelIdx];

			uint32_t numMeshes{ static_cast<uint32_t>(perModel.perMesh.size()) };
			for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
			{
				TGroup::PerMesh& perMesh{ perModel.perMesh[meshIdx] };
				//Search if this material exists
				const TGroup::Material& material = materials[meshIdx];

				uint32_t numMaterials{ static_cast<uint32_t>(perMesh.perMaterial.size()) };

				if (numMaterials - 1 == group.materialMask)
				{
					outMaterialIdx[meshIdx] = Handle<TGroup::Material>::Invalid();
					LOG_ENGINE_WARN("addMaterial", "budget materials for model handle{0}, mesh idx{1} is full", modelIdx.id, meshIdx);
					continue;
				}

				bool found{};


				for (uint32_t matIdx = 0; matIdx < numMaterials; ++matIdx)
				{
					TGroup::PerMaterial& perMaterial{ perMesh.perMaterial[matIdx] };

					if (perMaterial.material == material)
					{
						outMaterialIdx[meshIdx] = matIdx;
						found = true;
						break;
					}
					++matIdx;
				}
				
				if (!found)
				{
					TGroup::PerMaterial newPerMaterial;

					newPerMaterial.material = material;

					perMesh.perMaterial.emplace_back(newPerMaterial);
					outMaterialIdx[meshIdx] = numMaterials;
				}

			}
		}

		//Spawns a stream of instances provided you know model and material index for faster iteration. Outputs(optionally) instance ID stream
		template<typename TGroup, typename TGroupPerModel, typename TGroupMaterial, typename TGroupInstanceData>
		void addInstances(TGroup& group, Handle<TGroupPerModel> modelIdx, const Handle<TGroupMaterial>* materialIdx, const TGroupInstanceData* instanceData, uint32_t numInstances, PerMeshInstance* outIDs)
		{
			TGroup::PerModel& perModel = group.m_perModel[modelIdx];

			uint32_t numMeshes{ static_cast<uint32_t>(perModel.perMesh.size())};
			for (uint32_t instance = 0; instance < numInstances; ++instance)
			{
				const TGroup::InstanceData& data{ instanceData[instance] };

				for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
				{
					
					TGroup::PerMesh& perMesh = perModel.perMesh[meshIdx];

					//uint32_t numMaterials{ static_cast<uint32_t>(perMesh.perMaterial.size()) };

					uint32_t matIdx{ static_cast<uint32_t>(materialIdx[meshIdx].id)};
					TGroup::PerMaterial& perMaterial{ perMesh.perMaterial[matIdx]};
					//Verify if there's enough space in for specified mesh material
					uint32_t freeSpace{ (group.instanceMask - 1) - static_cast<uint32_t>(perMaterial.instances.size()) };

					Handle<TGroup::InstanceData> handle;

					if (freeSpace > numInstances)
					{
						handle = perMaterial.instances.insert(data);
					}
					else
					{
						LOG_ENGINE_WARN("addInstances", "not enough space for model handle {0}, mesh idx {1}, mat handle {2}. Added {3}/{4} instances",
							modelIdx, meshIdx, matIdx, instance, numInstances);

						handle = static_cast<uint32_t>(perMaterial.instances.size()) - 1;
					}
					//Build ID
					Handle<CombinedID>& id = outIDs[instance * numMeshes + meshIdx].composedID;

					id.id &= ~(group.modelMask | group.materialMask | group.instanceMask);
					id.id |= (modelIdx << (group.materialBits + group.instanceBits) ) | (matIdx << group.instanceBits) | handle.id;
					//id.id = (1 << group.instanceBits);

					++group.m_numInstances;
				}
			}

		}


		template<class TGroup, typename TGroupData>
		const TGroupData& getInstanceData(const TGroup& group, PerMeshInstance targetInstance)
		{
			//get which model, which material and which instance index
			uint32_t modelIdx   { group.computeModelIdx(targetInstance.composedID) };
			uint32_t materialIdx{ group.computeMaterialIdx(targetInstance.composedID)};
			Handle<TGroup::InstanceData> instanceID{ group.computeInstanceHandle(targetInstance.composedID)};

			const TGroup::PerModel& perModel{ group.m_perModel[modelIdx] };
			const TGroup::PerMesh& perMesh{ perModel.perMesh[0] };
			const TGroup::PerMaterial& perMaterial{ perMesh.perMaterial[materialIdx] };
			return perMaterial.instances[instanceID];
		}

		template<typename TGroup>
		const std::shared_ptr<Model>& getModelByHandle(const TGroup& group, Handle<CombinedID> modelHandle)
		{
			uint32_t modelIdx{ group.computeModelIdx(modelHandle.id) };

			return group.m_perModel[modelIdx].srcModel;
		}

		template<typename TGroup, typename TGMaterial>
		bool queryInstanceMaterial(const TGroup& group, const Handle<CombinedID>* meshInstances, uint32_t numMeshes, TGMaterial* outMaterials)
		{
			uint32_t modelIdx{ group.computeModelIdx(meshInstances[0].id) };

			const TGroup::PerModel& perModel{ group.m_perModel[modelIdx] };

			{
				uint32_t cachedMeshes{ static_cast<uint32_t>(perModel.perMesh.size()) };

				if (cachedMeshes != numMeshes)
				{
					LOG_ENGINE_WARN("queryMaterial", "provided numMeshes ({0}) != cachedMeshes ({1})", numMeshes, cachedMeshes);
					return false;
				}
			}

			for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
			{
				Handle<CombinedID> meshHandle{ meshInstances[meshIdx] };
				uint32_t matIdx{ group.computeMaterialIdx(meshHandle.id) };

				outMaterials[meshIdx] = perModel.perMesh[meshIdx].perMaterial[matIdx].material;
			}
			
			return true;
		}

		template<typename TGroup>
		bool deleteInstance(TGroup& group, const Handle<CombinedID>* instanceHandles, uint32_t numMeshes)
		{
			uint32_t modelIdx{ group.computeModelIdx(instanceHandles[0].id) };

      		uint32_t perMeshSize{ static_cast<uint32_t>(group.m_perModel[modelIdx].perMesh.size()) };

			if (perMeshSize != numMeshes)
				return false;

			for (uint32_t i = 0; i < numMeshes; ++i)
			{
				uint32_t materialIdx{ group.computeMaterialIdx(instanceHandles[i].id)};
				Handle<TGroup::InstanceData> instanceID{ group.computeInstanceHandle(instanceHandles[i].id)};

				group.m_perModel[modelIdx].perMesh[i].perMaterial[materialIdx].instances.erase(instanceID);
				--group.m_numInstances;
			}

			return true;
		}

		template<typename TGroup>
		bool findIntersectionInternal(TGroup& group, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder)
		{
			bool foundMeshIntersect{};
			uint32_t modelIdx = 0;
			uint32_t meshIdx = 0;
			uint32_t instanceIdx = 0;
			uint32_t materialIdx = 0;

			//Needed to verify world space distance
			MeshIntersection closestMesh;
			InstanceFinder   closestFinder;
			closestMesh.reset(0.0f);
			math::Matrix meshToWorldClosestMesh;

			for (const TGroup::PerModel& perModel : group.m_perModel)
			{
				meshIdx = 0;
				for (const TGroup::PerMesh& perMesh : perModel.perMesh)
				{
					const Mesh& mesh = perModel.srcModel->GetMeshes()[meshIdx];

					materialIdx = 0;
					for (const TGroup::PerMaterial& perMaterial : perMesh.perMaterial)
					{
						instanceIdx = 0;
						for (uint32_t i = 0; i < perMaterial.instances.size(); ++i)
						{
							const TGroup::InstanceData& instance = perMaterial.instances.at(i);
							math::Matrix worldToModel;
							math::Matrix worldToMesh;
							
							const math::Matrix instanceModelToWorld{ TransformSystem::Get().QueryTransformMatrix(instance.handle_modelToWorld) };
							math::InvertOrthogonalMatrix(instanceModelToWorld, worldToModel);
							worldToMesh = worldToModel * mesh.m_modelToMesh;

							Ray meshSpaceRay{ math::TransformRay(worldRay, worldToMesh) };

							float aabbT{};
							bool aabbIntersect{ mesh.m_AABB.Intersects(meshSpaceRay.position, meshSpaceRay.direction, aabbT) };

							if (aabbIntersect)
							{
								MeshIntersection testMeshInter;
								testMeshInter.reset(0.0f);
								if (mesh.m_TriangleOctree.intersect(meshSpaceRay, testMeshInter))
								{
									math::Matrix meshToWorld = mesh.m_meshToModel * instanceModelToWorld;
									math::Vector3 worldHitPos{ math::TransformPoint(testMeshInter.pos, meshToWorld) };
									float nearMeshT{ (worldHitPos - worldRay.position).Length() };

									if (nearMeshT < closestMesh.t)
									{
										meshToWorldClosestMesh = meshToWorld;
										closestMesh = testMeshInter;
										closestMesh.t = nearMeshT;
										closestMesh.pos = worldHitPos;

										foundMeshIntersect = true;
										//closestFinder.modelIndex = modelIdx;
										//closestFinder.meshIndex = meshIdx;
										//closestFinder.instanceIndex = instanceIdx;
										//closestFinder.materialIndex = materialIdx;
										closestFinder.handle_transform = instance.handle_modelToWorld;
										closestFinder.handle_object = instance.handle_object;
										closestFinder.handle_instance = instance.handle_modelInstance;
									}
								}
							}
							++instanceIdx;
						}
						++materialIdx;
					}
					++meshIdx;
				}
				++modelIdx;
			}

			if (foundMeshIntersect)
			{
				if (closestMesh.t < outHit.tDistance)
				{
					outHit.tDistance = closestMesh.t;
					outHit.hitPoint = closestMesh.pos;
					outHit.hitNormal = math::TransformVector(closestMesh.normal, meshToWorldClosestMesh);
					outHit.hitNormal.Normalize();
					outHit.hitObject = static_cast<TGroup*>(&group);
					outFinder = closestFinder;
				}
			}
			return foundMeshIntersect;
		}





		template<typename TGroup>
		bool findIntersection(TGroup& group, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder)
		{
			math::HitResult testHitResult;
			InstanceFinder  testFinder;
			testHitResult.ResetDistance();

			bool found{ findIntersectionInternal(group, worldRay, testHitResult, testFinder) };

			if (found && testHitResult.tDistance < outHit.tDistance)
			{
				outHit = testHitResult;
				outFinder = testFinder;
			}
			return found;
		}


	}
}