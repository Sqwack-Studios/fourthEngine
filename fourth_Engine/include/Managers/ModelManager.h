#pragma once

namespace fth
{
	class Model;
	struct Vertex;
	struct TriangleIndexed;
}

namespace fth
{
	struct ModelTextures;

	class ModelManager
	{
	public:
	static const uint32_t defaultImportingFlags;


		~ModelManager();

		inline static ModelManager& Get() 
		{
			static ModelManager instance;
			return instance;
		}

		enum class ModelFormats
		{
			OBJ = 0,
			FBX,
			STL,
			BLEND,
			_3DS,
			DAE,
			NUM
		};

		void Init();
		void Shutdown();

		std::shared_ptr<Model> LoadModel(std::string_view fullPath, ModelFormats format, uint32_t importFlags = defaultImportingFlags);
		std::shared_ptr<Model> FindModel(std::string_view name);

		static const std::shared_ptr<Model>& GetQuad() { return primitiveModels[PrimitiveTypes::QUAD]; }
		static const std::shared_ptr<Model>& GetUnitCube() { return primitiveModels[PrimitiveTypes::UNIT_CUBE]; }
		static const std::shared_ptr<Model>& GetUnitSphereFlat() { return primitiveModels[PrimitiveTypes::UNIT_SPHERE_FLAT]; }
		static const std::shared_ptr<Model>& GetUnitSphereSmooth() { return primitiveModels[PrimitiveTypes::UNIT_SPHERE_SMOOTH]; }

	private:
		const enum PrimitiveTypes : uint8_t
		{
			QUAD = 0,
			UNIT_CUBE,
			UNIT_SPHERE_FLAT,
			UNIT_SPHERE_SMOOTH,
			NUM
		};

		void InitializeUnitCube();
		void InitializeQuad();
		void InitializeUnitSphereFlat();
		void InitializeUnitSphereSmooth();
		void ComputeTangentSpace(Vertex* vertexStream, uint32_t numVertex);
		void ComputeTangentSpace(Vertex* vertexStream, TriangleIndexed* triangleStream, uint32_t numTriangles);
		void ComputeTangentSpace(Vertex& v0, Vertex& v1, Vertex& v2);

		//Primitive models
		static std::shared_ptr<Model>                   primitiveModels[PrimitiveTypes::NUM];
	private:
		ModelManager();

		
		//User-added models
		std::unordered_map<std::string, std::shared_ptr<Model>>        m_modelsMap;



	public:
		static constexpr std::string_view FormatToString(ModelFormats format)
		{
			switch (format)
			{
			case ModelManager::ModelFormats::OBJ:
			{
				return ".obj";
				break;
			}
			case ModelManager::ModelFormats::FBX:
			{
				return ".fbx";
				break;
			}
			case ModelManager::ModelFormats::STL:
			{
				return ".stl";
				break;
			}
			case ModelManager::ModelFormats::BLEND:
			{
				return ".blend";
				break;
			}
			case ModelManager::ModelFormats::_3DS:
			{
				return ".3ds";
				break;
			}
			case ModelManager::ModelFormats::DAE:
			{
				return ".dae";
				break;
			}
			default:
			{
				return "";
				break;
			}
			}
		}

		static ModelFormats StringToFormat(std::string_view string)
		{
			if (string == "obj")
				return ModelFormats::OBJ;
			else if (string == "fbx")
				return ModelFormats::FBX;
			else if (string == "stl")
				return ModelFormats::STL;
			else if (string == "blend")
				return ModelFormats::BLEND;
			else if (string == "3ds")
				return ModelFormats::_3DS;
			else if (string == "dae")
				return ModelFormats::DAE;

			return ModelFormats::NUM;
		}
	};
}