#pragma once
#include <fourthE.h>


struct fourth::ParallelExecutor;

class ClientApp : public fourth::EngineApp
{
public:
	ClientApp(const fourth::EngineAppSpecs& specs);
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

	void SetMovementDirection(const DirectX::SimpleMath::Vector3& direction);

	void MoveCamera(const fourth::math::Vector3& offset, const fourth::math::Vector3& offsetRotation);

private:

	virtual void OnResize(uint16_t newWidth, uint16_t newHeight) override;
	virtual void OnShutdown() override;

private:
	std::unique_ptr<fourth::IDragger>      m_Dragger;
	float                                  m_tDistanceDrag;
	float                                  m_BaseCameraSpeed;
	float                                  m_CameraSpeedMultiplier;
	float                                  m_CurrentCameraSpeed;

	fourth::math::Vector3                  m_TranslationDirection;
	fourth::math::Vector3                  m_RotationSpeedVector;
	fourth::math::Vector3                  m_RotationVector;
	fourth::math::Vector3                  m_TranslationVector;
	

	float                                  m_PitchSpeed;
	float                                  m_YawSpeed;
	float                                  m_RollSpeed;
	fourth::math::Vector3                  m_oldDragPos;
	fourth::MousePos                       m_oldMousePos;

	bool                                   m_attatchedFlashlight = false;
	uint32_t                               m_flashlightID;

	fourth::BitsetController               m_BitsetController;

	bool                                   discardUpdate;
};




std::unique_ptr<fourth::EngineApp> fourth::CreateApp(const fourth::EngineAppSpecs& specs)
{
	return std::make_unique<ClientApp>(specs);
}