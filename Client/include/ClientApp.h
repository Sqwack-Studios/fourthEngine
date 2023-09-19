#pragma once
#include <fourthE.h>
#include "include/DissolutionInstance.h"
#include "include/IncinerationInstance.h"

struct fth::ParallelExecutor;

struct PBRMaterial
{
	std::shared_ptr<fth::Texture> albedo;
	std::shared_ptr<fth::Texture> normals;
	std::shared_ptr<fth::Texture> roughness;
	std::shared_ptr<fth::Texture> metalness;
};

class ClientApp : public fth::EngineApp
{
public:
	ClientApp(const fth::EngineAppSpecs& specs);
	virtual ~ClientApp();

	virtual void PostInit() override;
	virtual void Update(float DeltaTime) override;
	virtual void OnRender() override;

	void LoadTexturesAndModels();
	bool HandleImGUI_Input();
	void HandleTranslation();
	void HandleRotation();
	void DragObjects();
	void CreateShowcaseScene();
	void CreateDissolutionInstance(fth::Handle<fth::shading::DissolutionGroup::PerModel> modelHandle, const fth::Handle<fth::shading::DissolutionGroup::Material>* materialHandles, uint32_t numMeshes);
	void IncinerateInstance();

	void SetMovementDirection(const DirectX::SimpleMath::Vector3& direction);

	void MoveCamera(const fth::math::Vector3& offset, const fth::math::Vector3& offsetRotation);
	void SpawnDecal();
private:

	virtual void OnResize(uint16_t newWidth, uint16_t newHeight) override;
	virtual void OnShutdown() override;

private:
	std::unique_ptr<fth::IDragger>         m_Dragger;
	float                                  m_tDistanceDrag;
	float                                  m_BaseCameraSpeed;
	float                                  m_CameraSpeedMultiplier;
	float                                  m_CurrentCameraSpeed;

	fth::math::Vector3                     m_TranslationDirection;
	fth::math::Vector3                     m_RotationSpeedVector;
	fth::math::Vector3                     m_RotationVector;
	fth::math::Vector3                     m_TranslationVector;
	

	float                                  m_PitchSpeed;
	float                                  m_YawSpeed;
	float                                  m_RollSpeed;
	fth::math::Vector3                     m_oldDragPos;
	fth::MousePos                          m_oldMousePos;

	bool                                   m_attatchedFlashlight = false;
	uint32_t                               m_flashlightID;

	fth::BitsetController                  m_BitsetController;

	std::vector<DissolutionInstance>       m_dissolutionInstances;
	std::vector<IncinerationInstance>      m_incinerationInstances;

	float                                  m_dissolutionDuration;
	float                                  m_dissolutionInitialTime;
	float                                  m_dissolutionSpawnDistance;

	bool                                   discardUpdate;

	std::vector<PBRMaterial>               samuraiPBR;
	std::vector<PBRMaterial>               horsePBR;

	static constexpr uint8_t  HORSE_HANDLE_IDX = 0;
	static constexpr uint8_t  SAMURAI_HANDLE_IDX = 1;

	fth::Handle<fth::shading::DissolutionGroup::PerModel>     dissolutionModelHandles[2];
	fth::Handle<fth::shading::OpaqueGroup::PerModel>          opaqueModelHandles[2];

	std::vector<fth::Handle<fth::shading::OpaqueGroup::Material> >      opaqueMaterialHandles[2];
	std::vector<fth::Handle<fth::shading::DissolutionGroup::Material> > dissolutionMaterialHandles[2];


};




std::unique_ptr<fth::EngineApp> fth::CreateApp(const fth::EngineAppSpecs& specs)
{
	return std::make_unique<ClientApp>(specs);
}