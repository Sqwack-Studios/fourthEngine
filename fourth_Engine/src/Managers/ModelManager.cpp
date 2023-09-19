#include "pch.h"

#pragma once
#include "include/Managers/ModelManager.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "include/Logging/AssimpLogger.h"
#include "include/Render/Model.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Managers/TextureManager.h"
#include "include/Utils/Conversions.h"
#include "include/Specifications.h"

namespace fth
{


	const uint32_t    ModelManager::defaultImportingFlags = { uint32_t(aiProcess_GenUVCoords | aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace) };
	std::shared_ptr<Model>           ModelManager::primitiveModels[PrimitiveTypes::NUM];


	ModelManager::ModelManager() {};
	ModelManager::~ModelManager() {};


	void ModelManager::Init()
	{
		InitializeQuad();
		InitializeUnitCube();
		InitializeUnitSphereFlat();
		InitializeUnitSphereSmooth();
	}


	void ModelManager::Shutdown()
	{
		for (auto& model : m_modelsMap)
			model.second->Shutdown();

		primitiveModels[PrimitiveTypes::QUAD]->Shutdown();
		primitiveModels[PrimitiveTypes::UNIT_CUBE]->Shutdown();
		primitiveModels[PrimitiveTypes::UNIT_SPHERE_FLAT]->Shutdown();
		primitiveModels[PrimitiveTypes::UNIT_SPHERE_SMOOTH]->Shutdown();


		m_modelsMap.clear();
	}

	std::shared_ptr<Model> ModelManager::LoadModel(std::string_view inFullPath, ModelFormats format, uint32_t importFlags)
	{
		std::string name{ FindLastChar('/', inFullPath.data(), 1) };
		std::string fullPath{ std::string(Config::assetsPath) + std::string(inFullPath) + "/" + name  + std::string(FormatToString(format))};

		//First, try to find this model in the database
		{
			std::shared_ptr<Model> inDataBase = FindModel(name);
			if (inDataBase != GetUnitCube())
			{
				return inDataBase;
			}
		}

		AssimpLogger::Create();
		Assimp::Importer importer;


		const aiScene* assimpScene = importer.ReadFile(fullPath, importFlags);

		if (!assimpScene)
		{
			LOG_ENGINE_WARN("Model::LoadModel", "Model {0}{1} could not be imported", name, FormatToString(format));
			return nullptr;
		}
		static_assert(sizeof(math::Vector3) == sizeof(aiVector3D));
		static_assert(sizeof(TriangleIndexed) == sizeof(*aiFace::mIndices) * 3);
		static_assert(sizeof(math::Color) == sizeof(aiColor4D));

		Model model;

		
		uint32_t numMeshes{ assimpScene->mNumMeshes };

		

		{
			uint32_t totalVertices{};
			uint32_t totalIndices{};
			for (uint32_t i = 0; i < numMeshes; ++i)
			{
				totalVertices += assimpScene->mMeshes[i]->mNumVertices;
				totalIndices += assimpScene->mMeshes[i]->mNumFaces * 3;
			}
			LOG_ENGINE_INFO("LoadModel", "Loading model with {0} meshes, {1} vertices and {2} faces", numMeshes, totalVertices, totalIndices / 3);
			LOG_ENGINE_INFO("LoadModel", "Current model has {0} materials", assimpScene->mNumMaterials);

			model.m_name = name;
			model.m_meshes.resize(numMeshes);
			model.m_vertices.m_cpuData.reserve(totalVertices);
			model.m_trianglesIndexed.m_cpuData.reserve(totalIndices / 3);


		}

		uint32_t vertexOffset{};
		uint32_t facesOffset{};
		for (uint32_t i = 0; i < numMeshes; ++i)
		{

			const auto& srcMesh = assimpScene->mMeshes[i];
			Mesh& dstMesh = model.m_meshes[i];
			uint32_t numVertex{ srcMesh->mNumVertices };
			uint32_t numFaces{ srcMesh->mNumFaces };
			aiString texture_file;



			dstMesh.m_meshName = srcMesh->mName.C_Str();

			LOG_ENGINE_INFO("", "Loading {0}", dstMesh.m_meshName);

			dstMesh.m_meshRange.vertexNum = numVertex;
			dstMesh.m_meshRange.indexNum = numFaces * 3;
			dstMesh.m_meshRange.triangleNum = numFaces;

			dstMesh.m_meshRange.vertexOffset = vertexOffset;
			dstMesh.m_meshRange.indexOffset = facesOffset * 3;
			dstMesh.m_meshRange.triangleOffset = facesOffset;

			dstMesh.m_AABB = math::AABB::MinMaxToExtentsCenter(reinterpret_cast<DirectX::XMFLOAT3&>(srcMesh->mAABB.mMin), reinterpret_cast<DirectX::XMFLOAT3&>(srcMesh->mAABB.mMax));


			for (uint32_t v = 0; v < numVertex; ++v)
			{
				
				model.m_vertices.m_cpuData.emplace_back(
					reinterpret_cast<math::Vector3&>(srcMesh->mVertices[v]),
					reinterpret_cast<math::Vector3&>(srcMesh->mNormals[v]),
					reinterpret_cast<math::Vector3&>(srcMesh->mTangents[v]),
					reinterpret_cast<math::Vector3&>(srcMesh->mBitangents[v]) * -1.0f,
					reinterpret_cast<math::Vector2&>(srcMesh->mTextureCoords[0][v])
				);
			}

			for (uint32_t f = 0; f < numFaces; ++f)
			{

				const aiFace& face = srcMesh->mFaces[f];
				model.m_trianglesIndexed.m_cpuData.emplace_back(*reinterpret_cast<TriangleIndexed*>(face.mIndices));
			}
			vertexOffset += numVertex;
			facesOffset += numFaces;

		}
		std::function<void(aiNode*)> loadInstances;
		loadInstances = [&loadInstances, &model](aiNode* node)
		{
			const math::Matrix nodeToParent = reinterpret_cast<const math::Matrix&>(node->mTransformation.Transpose());
			const math::Matrix parentToNode = nodeToParent.Invert();

			// The same node may contain multiple meshes in its space, referring to them by indices
			for (uint32_t i = 0; i < node->mNumMeshes; ++i)
			{
				uint32_t meshIndex = node->mMeshes[i];
				model.m_meshes[meshIndex].m_nodeParentTransforms.push_back(nodeToParent);
				model.m_meshes[meshIndex].m_parentNodeTransforms.push_back(parentToNode);
				model.m_meshes[meshIndex].m_meshToModel *= nodeToParent;
				model.m_meshes[meshIndex].m_modelToMesh *= parentToNode;

			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i)
				loadInstances(node->mChildren[i]);
		};

		loadInstances(assimpScene->mRootNode);

		//compute model AABB
		math::Vector3 max { std::numeric_limits<float>::min()};
		math::Vector3 min { std::numeric_limits<float>::max() };
		for (uint32_t i = 0; i < numMeshes; ++i)
		{
			const Mesh& mesh{ model.m_meshes[i] };

			math::Vector3 meshAABBMax{ math::Vector3::Transform(math::Vector3(mesh.m_AABB.Center) + math::Vector3(mesh.m_AABB.Extents), mesh.m_meshToModel) };
			math::Vector3 meshAABBMin{ math::Vector3::Transform(math::Vector3(mesh.m_AABB.Center) - math::Vector3(mesh.m_AABB.Extents), mesh.m_meshToModel) };

			math::Vector3 auxMax{ math::Vector3::Max(meshAABBMin, meshAABBMax) };
			math::Vector3 auxMin{ math::Vector3::Min(meshAABBMin, meshAABBMax) };

			max = math::Vector3::Max(max, auxMax);
			min = math::Vector3::Min(min, auxMin);
		}
		model.m_aabb = math::AABB::MinMaxToExtentsCenter(min, max);
		LOG_ENGINE_INFO("LoadModel", "Model loaded");
		//loading is ok, point meshes to the source model


		std::shared_ptr <Model> modelPtr = m_modelsMap.insert(std::make_pair(
			std::string(name), 
			std::make_shared<Model>(std::move(model)))).first->second;

		modelPtr->Init(modelPtr);

		AssimpLogger::Clear();

		return modelPtr;

	}

	std::shared_ptr<Model> ModelManager::FindModel(std::string_view name)
	{

		auto pos = m_modelsMap.find(std::string(name));
		if (pos == m_modelsMap.end())
			return GetUnitCube();


		return pos->second;
	}


	void ModelManager::InitializeUnitCube()
	{
		Model model;

		uint32_t numVertex{ 24 };
		uint32_t numTriangles{ 12 };
		uint32_t numIndices{ numTriangles * 3 };

		model.m_vertices.m_cpuData.reserve(numVertex);
		model.m_trianglesIndexed.m_cpuData.reserve(numTriangles);
		//Center of the cube defines the origin in model spac
		//Vertex: position, normal, tangent, bitangent, texture UV
		//Duplicate cube vertex to have sharp edges: this makes 8 vertices * 3 directions = 24 vertices

		//Y+ face
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, -0.5f }, math::Vector3{ -1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.375f });   //0
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, -0.5f }, math::Vector3{ 0.0f, 1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.375f });    //1
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, -0.5f }, math::Vector3{ 0.0f, 0.0f, -1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.75f, 0.375f });   //2

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, 0.5f }, math::Vector3{ -1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.0f, 0.375f });     //3
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, 0.5f }, math::Vector3{ 0.0f, 1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.125f });      //4
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, 0.5f, 0.5f }, math::Vector3{ 0.0f, 0.0f, 1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.375f });      //5

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, 0.5f }, math::Vector3{ 1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.75f, 0.375f });       //6
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, 0.5f }, math::Vector3{ 0.0f, 1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.125f });       //7
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, 0.5f }, math::Vector3{ 0.0f, 0.0f, 1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.375f });       //8

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, -0.5f }, math::Vector3{ 1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.375f });      //9
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, -0.5f }, math::Vector3{ 0.0f, 1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.375f });      //10
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, 0.5f, -0.5f }, math::Vector3{ 0.0f, 0.0f, -1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 1.0f, 0.375f });     //11

		//Y- face							   
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, -0.5f }, math::Vector3{ 1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.625f });	  //12
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, -0.5f }, math::Vector3{ 0.0f, -1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.625f });	  //13
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, -0.5f }, math::Vector3{ 0.0f, 0.0f, -1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 1.0, 0.625f });	  //14

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, 0.5f }, math::Vector3{ 1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.75f, 0.625f });	  //15
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, 0.5f }, math::Vector3{ 0.0f, -1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.875f });	  //16
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ 0.5f, -0.5f, 0.5f }, math::Vector3{ 0.0f, 0.0f, 1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.625f });	  //17

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, 0.5f }, math::Vector3{ -1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.0f, 0.625f });	  //18
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, 0.5f }, math::Vector3{ 0.0f, -1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.875f });	  //19
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, 0.5f }, math::Vector3{ 0.0f, 0.0f, 1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.50f, 0.625f });	  //20

		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, -0.5f }, math::Vector3{ -1.0f, 0.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.625f });    //21
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, -0.5f }, math::Vector3{ 0.0f, -1.0f, 0.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.25f, 0.625f });    //22
		model.m_vertices.m_cpuData.emplace_back(math::Vector3{ -0.5f, -0.5f, -0.5f }, math::Vector3{ 0.0f, 0.0f, -1.0f }, math::Vector3::Zero, math::Vector3::Zero, math::Vector2{ 0.75f, 0.625f });    //23



		//Setup indices
		//Y+
		model.m_trianglesIndexed.m_cpuData.emplace_back(1, 4, 7);
		model.m_trianglesIndexed.m_cpuData.emplace_back(1, 7, 10);
		//Y-
		model.m_trianglesIndexed.m_cpuData.emplace_back(13, 16, 19);
		model.m_trianglesIndexed.m_cpuData.emplace_back(13, 19, 22);
		//X+
		model.m_trianglesIndexed.m_cpuData.emplace_back(9, 6, 15);
		model.m_trianglesIndexed.m_cpuData.emplace_back(9, 15, 12);
		//Z+
		model.m_trianglesIndexed.m_cpuData.emplace_back(8, 5, 20);
		model.m_trianglesIndexed.m_cpuData.emplace_back(8, 20, 17);
		//X-
		model.m_trianglesIndexed.m_cpuData.emplace_back(3, 0, 21);
		model.m_trianglesIndexed.m_cpuData.emplace_back(3, 21, 18);
		//Z-
		model.m_trianglesIndexed.m_cpuData.emplace_back(2, 11, 14);
		model.m_trianglesIndexed.m_cpuData.emplace_back(2, 14, 23);

		ComputeTangentSpace(model.m_vertices.m_cpuData.data(), model.m_trianglesIndexed.m_cpuData.data(), numTriangles);

		model.m_name = "DefaultCube";
		model.m_meshes.resize(1);
		model.m_aabb = math::AABB{ math::Vector3::Zero, 0.5f * math::Vector3::One };
		Mesh& mesh{ model.m_meshes.back() };

		mesh.m_AABB = math::AABB{ math::Vector3::Zero, 0.5f * math::Vector3::One };
		mesh.m_meshName = "DefaultCubeMesh";
		mesh.m_meshToModel = mesh.m_modelToMesh = math::Matrix::Identity;

		mesh.m_meshRange.vertexNum = numVertex;
		mesh.m_meshRange.indexNum = numIndices;
		mesh.m_meshRange.triangleNum = numTriangles;
		mesh.m_meshRange.vertexOffset = 0;
		mesh.m_meshRange.indexOffset = 0;
		mesh.m_meshRange.triangleOffset = 0;

		primitiveModels[PrimitiveTypes::UNIT_CUBE] = std::make_shared<Model>(std::move(model));
		primitiveModels[PrimitiveTypes::UNIT_CUBE]->Init(GetUnitCube());
	}

	void ModelManager::InitializeQuad()
	{
		Model model;

		uint32_t numVertex{ 4 };
		uint32_t numTriangles{ 2 };
		uint32_t numIndices{ numTriangles * 3 };

		model.m_vertices.m_cpuData.reserve(numVertex);
		model.m_trianglesIndexed.m_cpuData.reserve(numTriangles);

		model.m_vertices.m_cpuData.emplace_back(math::Vector3(-0.5f, -0.5f, 0.0f), -math::Vector3::Forward, math::Vector3::Right,   -math::Vector3::Up, math::Vector2{ 0.0f, 1.0f });
		model.m_vertices.m_cpuData.emplace_back(math::Vector3(-0.5f, 0.5f, 0.0f), -math::Vector3::Forward,  math::Vector3::Right,   -math::Vector3::Up, math::Vector2{ 0.0f, 0.0f });
		model.m_vertices.m_cpuData.emplace_back(math::Vector3(0.5f, 0.5f, 0.0f), -math::Vector3::Forward,   math::Vector3::Right,   -math::Vector3::Up, math::Vector2{ 1.0f, 0.0f });
		model.m_vertices.m_cpuData.emplace_back(math::Vector3(0.5f, -0.5f, 0.0f), -math::Vector3::Forward,  math::Vector3::Right,   -math::Vector3::Up, math::Vector2{ 1.0f, 1.0f });


		model.m_trianglesIndexed.m_cpuData.emplace_back(0, 1, 2);
		model.m_trianglesIndexed.m_cpuData.emplace_back(2, 3, 0);

		ComputeTangentSpace(model.m_vertices.m_cpuData.data(), model.m_trianglesIndexed.m_cpuData.data(), numTriangles);

		model.m_name = "Quad";
		model.m_meshes.resize(1);
		model.m_aabb = math::AABB{ math::Vector3::Zero, {0.5f, 0.5f, math::EPSILON} };

		Mesh& mesh{ model.m_meshes.back() };


		mesh.m_AABB = math::AABB{ math::Vector3::Zero, {0.5f, 0.5f, math::EPSILON} };
		mesh.m_meshName = "Quad";
		mesh.m_meshToModel = mesh.m_modelToMesh = math::Matrix::Identity;

		mesh.m_meshRange.vertexNum = numVertex;
		mesh.m_meshRange.indexNum = numIndices;
		mesh.m_meshRange.triangleNum = numTriangles;
		mesh.m_meshRange.vertexOffset = 0;
		mesh.m_meshRange.indexOffset = 0;
		mesh.m_meshRange.triangleOffset = 0;

		primitiveModels[PrimitiveTypes::QUAD] = std::make_shared<Model>(std::move(model));
		primitiveModels[PrimitiveTypes::QUAD]->Init(GetQuad());
	}

	void ModelManager::InitializeUnitSphereFlat()
	{
		constexpr uint32_t SIDES = 6;
		constexpr uint32_t GRID_SIZE = 12;
		constexpr uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2;
		constexpr uint32_t VERT_PER_SIZE = 3 * TRIS_PER_SIDE;

		Model model;
		Mesh& mesh = model.m_meshes.emplace_back();


		model.m_name = "UNIT_SPHERE_FLAT";
		model.m_aabb = { math::Vector3::Zero, 1.0f * math::Vector3::One };
		mesh.m_meshName = "UNIT_SPHERE_FLAT";
		
		//model.box = engine::BoundingBox::empty();

		mesh.m_AABB = {math::Vector3::Zero, 1.0f * math::Vector3::One};
		mesh.m_meshToModel = { math::Matrix::Identity };
		mesh.m_modelToMesh = { math::Matrix::Identity };

		model.m_vertices.m_cpuData.resize(VERT_PER_SIZE * SIDES);
		model.m_trianglesIndexed.m_cpuData.clear();

		Vertex* vertex = model.m_vertices.m_cpuData.data();

		int sideMasks[6][3] =
		{
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 0, 2, 1 },
			{ 0, 2, 1 }
		};

		float sideSigns[6][3] =
		{
			{ +1, +1, +1 },
			{ -1, +1, +1 },
			{ -1, +1, -1 },
			{ +1, +1, -1 },
			{ +1, -1, -1 },
			{ +1, +1, +1 }
		};


		for (int side = 0; side < SIDES; ++side)
		{
			for (int row = 0; row < GRID_SIZE; ++row)
			{
				for (int col = 0; col < GRID_SIZE; ++col)
				{
					float left = (col + 0) / float(GRID_SIZE) * 2.f - 1.f;
					float right = (col + 1) / float(GRID_SIZE) * 2.f - 1.f;
					float bottom = (row + 0) / float(GRID_SIZE) * 2.f - 1.f;
					float top = (row + 1) / float(GRID_SIZE) * 2.f - 1.f;

					math::Vector3 quad[4] =
					{
						{ left, bottom, 1.f },
						{ left, top, 1.f },
						{ right, bottom, 1.f },
						{ right, top, 1.f }
					}; 

					vertex[0] = vertex[1] = vertex[2] = vertex[3] = Vertex::Zero;

					auto setPos = [sideMasks, sideSigns](int side, Vertex& dst, const math::Vector3& pos)
					{
						dst.position[sideMasks[side][0]] = pos.x * sideSigns[side][0];
						dst.position[sideMasks[side][1]] = pos.y * sideSigns[side][1];
						dst.position[sideMasks[side][2]] = pos.z * sideSigns[side][2];
						dst.position.Normalize();
			
					};


					setPos(side, vertex[0], quad[0]);
					setPos(side, vertex[1], quad[1]);
					setPos(side, vertex[2], quad[2]);

					{
						math::Vector3 AB = vertex[1].position - vertex[0].position;
						math::Vector3 AC = vertex[2].position - vertex[0].position;
						math::Vector3 cross{ AB.Cross(AC) };
						cross.Normalize();
						vertex[0].normal = vertex[1].normal = vertex[2].normal = cross;
					}

					vertex += 3;

					setPos(side, vertex[0], quad[1]);
					setPos(side, vertex[1], quad[3]);
					setPos(side, vertex[2], quad[2]);

					{
						math::Vector3 AB = vertex[1].position - vertex[0].position;
						math::Vector3 AC = vertex[2].position - vertex[0].position;
						math::Vector3 cross{ AB.Cross(AC) };
						cross.Normalize();
						vertex[0].normal = vertex[1].normal = vertex[2].normal = cross;
					}

					vertex += 3;
				}
			}
		}

		mesh.m_meshRange.indexOffset = 0;
		mesh.m_meshRange.triangleOffset = 0;
		mesh.m_meshRange.vertexOffset = 0;
		mesh.m_meshRange.vertexNum = SIDES * VERT_PER_SIZE;
		mesh.m_meshRange.triangleNum = SIDES * TRIS_PER_SIDE;
		mesh.m_meshRange.indexNum = 0;


		primitiveModels[UNIT_SPHERE_FLAT] = std::make_shared<Model>(std::move(model));
		primitiveModels[UNIT_SPHERE_FLAT]->Init(primitiveModels[UNIT_SPHERE_FLAT]);

	}

	void ModelManager::InitializeUnitSphereSmooth()
	{
		const uint32_t SIDES = 6;
		const uint32_t GRID_SIZE = 12;
		const uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2;
		const uint32_t VERT_PER_SIZE = (GRID_SIZE + 1) * (GRID_SIZE + 1);


		Model model;
		Mesh& mesh = model.m_meshes.emplace_back();


		model.m_name = "UNIT_SPHERE_SMOOTH";
		model.m_aabb = { math::Vector3::Zero, 1.0f * math::Vector3::One };
		mesh.m_meshName = "UNIT_SPHERE_SMOOTH";

		//model.box = engine::BoundingBox::empty();

		mesh.m_AABB = { math::Vector3::Zero, 1.0f * math::Vector3::One };
		mesh.m_meshToModel = { math::Matrix::Identity };
		mesh.m_modelToMesh = { math::Matrix::Identity };

		model.m_vertices.m_cpuData.resize(VERT_PER_SIZE * SIDES);

		Vertex* vertex = model.m_vertices.m_cpuData.data();

		int sideMasks[6][3] =
		{
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 2, 1, 0 },
			{ 0, 1, 2 },
			{ 0, 2, 1 },
			{ 0, 2, 1 }
		};

		float sideSigns[6][3] =
		{
			{ +1, +1, +1 },
			{ -1, +1, +1 },
			{ -1, +1, -1 },
			{ +1, +1, -1 },
			{ +1, -1, -1 },
			{ +1, +1, +1 }
		};

		for (int side = 0; side < SIDES; ++side)
		{
			for (int row = 0; row < GRID_SIZE + 1; ++row)
			{
				for (int col = 0; col < GRID_SIZE + 1; ++col)
				{
					math::Vector3 v;
					v.x = col / float(GRID_SIZE) * 2.f - 1.f;
					v.y = row / float(GRID_SIZE) * 2.f - 1.f;
					v.z = 1.f;

					vertex[0] = Vertex::Zero;

					vertex[0].position[sideMasks[side][0]] = v.x * sideSigns[side][0];
					vertex[0].position[sideMasks[side][1]] = v.y * sideSigns[side][1];
					vertex[0].position[sideMasks[side][2]] = v.z * sideSigns[side][2];

					vertex[0].position.Normalize();
					vertex[0].normal = vertex[0].position;

					vertex += 1;
				}
			}
		}

		model.m_trianglesIndexed.m_cpuData.resize(TRIS_PER_SIDE * SIDES);
		TriangleIndexed* triangle = model.m_trianglesIndexed.m_cpuData.data();

		for (int side = 0; side < SIDES; ++side)
		{
			uint32_t sideOffset = VERT_PER_SIZE * side;

			for (int row = 0; row < GRID_SIZE; ++row)
			{
				for (int col = 0; col < GRID_SIZE; ++col)
				{
					triangle[0].indexes[0] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 0;
					triangle[0].indexes[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
					triangle[0].indexes[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

					triangle[1].indexes[0] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
					triangle[1].indexes[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 1;
					triangle[1].indexes[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

					triangle += 2;
				}
			}
		}
		mesh.m_meshRange.indexOffset = 0;
		mesh.m_meshRange.triangleOffset = 0;
		mesh.m_meshRange.vertexOffset = 0;
		mesh.m_meshRange.vertexNum = SIDES * VERT_PER_SIZE;
		mesh.m_meshRange.triangleNum = TRIS_PER_SIDE * SIDES;
		mesh.m_meshRange.indexNum = 3 * TRIS_PER_SIDE * SIDES;

		primitiveModels[UNIT_SPHERE_SMOOTH] = std::make_shared<Model>(std::move(model));
		primitiveModels[UNIT_SPHERE_SMOOTH]->Init(primitiveModels[UNIT_SPHERE_SMOOTH]);
	}

	//source : http://foundationsofgameenginedev.com/FGED2-sample.pdf
	void ModelManager::ComputeTangentSpace(Vertex* vertexStream, uint32_t numVertex)
	{
		for (uint32_t currentVertex = 0; currentVertex < numVertex; ++currentVertex)
		{
			Vertex& v0 = *vertexStream;
			Vertex& v1 = *(vertexStream + 1);
			Vertex& v2 = *(vertexStream + 2);

			ComputeTangentSpace(v0, v1, v2);
			
			vertexStream += 3;
		}
	}

	void ModelManager::ComputeTangentSpace(Vertex* vertexStream, TriangleIndexed* triangleStream, uint32_t numTriangles)
	{
		for (uint32_t triangleIdx = 0; triangleIdx < numTriangles; ++triangleIdx)
		{
			Vertex& v0 = *(vertexStream + (triangleStream->triangle.index1));
			Vertex& v1 = *(vertexStream + (triangleStream->triangle.index2));
			Vertex& v2 = *(vertexStream + (triangleStream->triangle.index3));

			ComputeTangentSpace(v0, v1, v2);

			++triangleStream;
		}
	}

	void ModelManager::ComputeTangentSpace(Vertex& v0, Vertex& v1, Vertex& v2)
	{
		math::Vector2 UV0 = v0.textureUV;
		math::Vector2 UV1 = v1.textureUV;
		math::Vector2 UV2 = v2.textureUV;

		math::Vector3 deltaPos1 = v1.position - v0.position;
		math::Vector3 deltaPos2 = v2.position - v0.position;

		math::Vector2 deltaUV1 = UV1 - UV0;
		math::Vector2 deltaUV2 = UV2 - UV0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		math::Vector3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		math::Vector3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		math::Vector3 basisTangent0 = tangent - tangent.Dot(v0.normal) * v0.normal;
		math::Vector3 basisTangent1 = tangent - tangent.Dot(v1.normal) * v1.normal;
		math::Vector3 basisTangent2 = tangent - tangent.Dot(v2.normal) * v2.normal;

		basisTangent0.Normalize();
		basisTangent1.Normalize();
		basisTangent2.Normalize();

		v0.tangent = basisTangent0;
		v1.tangent = basisTangent1;
		v2.tangent = basisTangent2;

		v0.bitangent = v0.normal.Cross(basisTangent0);
		v1.bitangent = v0.normal.Cross(basisTangent1);
		v2.bitangent = v0.normal.Cross(basisTangent2);
	}


}