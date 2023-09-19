#pragma once
#include "include/ClientApp.h"
#include "vendor/ImGUI/include/imgui.h"
#include "vendor/ImGUI/include/imgui_impl_dx11.h"
#include "vendor/ImGUI/include/imgui_impl_win32.h"
#include "vendor/DirectXTex/Include/DirectXTex.h"

using namespace fth;
using namespace DirectX;

constexpr ImGuiKey _TranslateToImGuiKey(fthKey key);


ClientApp::ClientApp(const EngineAppSpecs& specs):
	EngineApp(specs),
    m_Dragger(nullptr),
	m_tDistanceDrag(std::numeric_limits<float>::infinity()),
	m_BaseCameraSpeed(5.0f),
	m_CameraSpeedMultiplier(1.0f),
	m_CurrentCameraSpeed(m_BaseCameraSpeed),
	m_TranslationDirection(math::Vector3::Zero),
	m_RotationSpeedVector(math::Vector3::Zero),
	m_RotationVector(math::Vector3::Zero),
	m_TranslationVector(math::Vector3::Zero),
	m_PitchSpeed(0.0f),
	m_YawSpeed(0.0f),
	m_RollSpeed(XM_PIDIV2),
	m_oldMousePos({ 0, 0 }),
    m_oldDragPos(math::Vector3::Zero),
	m_dissolutionSpawnDistance(0.5f),
	m_dissolutionDuration(2.0f),
    m_dissolutionInitialTime(0.0f),
	discardUpdate(false)
{

	
}	
ClientApp::~ClientApp()
{
	
}



void ClientApp::PostInit()
{
	LoadTexturesAndModels();


	m_oldMousePos = { GetBaseWindow().GetWindowWidth() / 2, GetBaseWindow().GetWindowHeight() / 2 };
	renderer::Camera& camera = CameraManager::GetActiveCamera();

	camera.SetWorldPosition({ 0.0f, 1.0f, -5.0f });
	camera.SetWorldLookAtDirection(math::Vector3::Forward, math::Vector3::Up);
	camera.UpdateMatrices();
	
	CreateShowcaseScene();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(GetBaseWindow().GetHWND());
	ImGui_ImplDX11_Init(s_device, s_devcon);



}

void ClientApp::Update(float DeltaTime)
{
	HandleTranslation();
	HandleRotation();

	{

		renderer::D3DRenderer& renderer{ renderer::D3DRenderer::Get() };
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		discardUpdate = HandleImGUI_Input();

		ImGui::NewFrame();

		ImGui::Begin("Config");

		bool renderSpecular{ renderer::D3DRenderer::Get().isSpecularEnabled() };
		bool renderDiffuse{ renderer::D3DRenderer::Get().isDiffuseEnabled() };
		bool renderIBL{ renderer::D3DRenderer::Get().isIBLEnabled() };

		ImGui::Checkbox("Enable Specular", &renderSpecular);
		ImGui::Checkbox("Enable Diffuse", &renderDiffuse);
		ImGui::Checkbox("Enable Environmental IBL", &renderIBL);

		renderer.enableSpecular(renderSpecular);
		renderer.enableDiffuse(renderDiffuse);
		renderer.enableIBL(renderIBL);

		bool shouldOverride{ renderer::D3DRenderer::Get().roughnessOverride() };
		ImGui::Checkbox("Override roughness", &shouldOverride);
		float roughness{ renderer::D3DRenderer::Get().roughnessOverrideValue() };
		ImGui::SliderFloat("Roughness Value", &roughness, 0.0f, 1.0f);

		const int items{ 4 };
		constexpr char* combo[items]{ "Karis", "Carpentier - Iteration", "Carpentier - Non Iteration", "None" };
		static const char* current_item = combo[items - 1];
		static SpecularMRP specularMRP_Flags{ SpecularMRP::NONE }; 

		renderer::PostProcess::FXAAConfig fxaaConfg = renderer::PostProcess::Get().getFXAAConfiguration();
		
		ImGui::SliderFloat("FXAA-Quality Subpixel", &fxaaConfg.qualitySubpixel, renderer::PostProcess::fxaa_qualitySubpixel_lower, renderer::PostProcess::fxaa_qualitySubpixel_upper);
		ImGui::SliderFloat("FXAA-Quality Edge Threshold", &fxaaConfg.qualityEdgeThreshold, renderer::PostProcess::fxaa_qualityEdgeThreshold_lower, renderer::PostProcess::fxaa_qualityEdgeThreshold_upper);
		ImGui::SliderFloat("FXAA-Quality Edge ThresholdMin", &fxaaConfg.qualityEdgeThresholdMin, renderer::PostProcess::fxaa_qualityEdgeThresholdMin_lower, renderer::PostProcess::fxaa_qualityEdgeThresholdMin_upper);

		if (ImGui::Button("Reset FXAA-Subpixel"))
		{
			fxaaConfg.qualitySubpixel = renderer::PostProcess::fxaa_qualityEdgeThresholdMin_default;
		}
		if (ImGui::Button("Reset FXAA-Edge Threshold"))
		{
			fxaaConfg.qualityEdgeThreshold = renderer::PostProcess::fxaa_qualitySubpixel_default;
		}
		if (ImGui::Button("Reset FXAA-Edge ThresholdMin"))
		{
			fxaaConfg.qualityEdgeThresholdMin = renderer::PostProcess::fxaa_qualityEdgeThreshold_default;
		}



		renderer::PostProcess::Get().setFXAAConfiguration(fxaaConfg);

		ImGui::Text("Specular shape implementation");
		if (ImGui::BeginCombo("##", current_item))
		{
			for (uint16_t n = 0; n < items; ++n)
			{
				bool selected = (current_item == combo[n]);
				if (ImGui::Selectable(combo[n], selected))
				{
					current_item = combo[n];
					//no good, temporary
					if(n == items - 1)
					{
						specularMRP_Flags = SpecularMRP::NONE;
					}
					else
					{
						specularMRP_Flags = static_cast<SpecularMRP>(1u << n);
					}
				}
			}
			ImGui::EndCombo();
		}

		
		ImGui::End();

		renderer.SetRoughnessOverride(shouldOverride);
		renderer.SetRoughnessValue(roughness);
		renderer.setSpecularMRPFlags(specularMRP_Flags);

		const Texture& smTx{ fth::LightSystem::Get().GetDirectionalShadowmap().texture };
		const Texture& spotTx{ fth::LightSystem::Get().GetSpotlightShadowmap().texture };

		ImGui::Begin("Depth tests");
		ImGui::Text("Depth from directional light");
		ImGui::Image((void*)smTx.GetSRV().getView(), ImVec2(512, 512));
		ImGui::Text("Depth from spotlight");
		ImGui::Image((void*)spotTx.GetSRV().getView(), ImVec2(512, 512));
		ImGui::End();
	}

	if (discardUpdate)
	{

		discardUpdate = false;
		return;
	}


	if (InputController::Get().KeyIsDown(fthKey::fthKey_Shift))
		m_CameraSpeedMultiplier = 5.0f;
	else
		m_CameraSpeedMultiplier = 1.0f;

	if (InputController::Get().GetMouse().GetDeltaWheel() > 0)
		m_CurrentCameraSpeed *= 1.10f;
	else if (InputController::Get().GetMouse().GetDeltaWheel() < 0)
		m_CurrentCameraSpeed *= .90f;


	if (InputController::Get().KeyIsPressed(fthKey::fthKey_K))
	{
		renderer::D3DRenderer::Get().ToggleWireframe();
	}
	if (InputController::Get().KeyIsPressed(fthKey::fthKey_N))
	{
		MeshSystem::Get().ToggleNormalDebugging();
	}
	m_BitsetController.FillSpaceMovementDirection(m_TranslationDirection);

	if (InputController::Get().KeyIsPressed(fthKey::fthKey_F))
	{
		m_attatchedFlashlight = !m_attatchedFlashlight;

		lights::SpotSphere& spot{ LightSystem::Get().QuerySpotLight(m_flashlightID) };
		if (m_attatchedFlashlight)
		{
			spot.position = math::Vector3::Zero;
			spot.direction = math::Vector3::Forward;
			spot.parentID = CameraManager::GetActiveCamera().GetIDInverseView();
		}
		else
		{
			const math::Matrix& parentTransform{ spot.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(spot.parentID) : math::Matrix::Identity };
			spot.position = parentTransform.Translation();
			spot.direction = parentTransform.Forward();
			spot.parentID = Handle<math::Matrix>::Invalid();
		}
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_Plus))
	{
		float currentEV100{ renderer::PostProcess::Get().GetEV100() };
		LOG_CLIENT_INFO("Client::Update", "Current EV100 is {0}", renderer::PostProcess::Get().SetEV100(currentEV100 + DeltaTime));
	}
	else if (InputController::Get().KeyIsDown(fthKey::fthKey_Minus))
	{
		float currentEV100{ renderer::PostProcess::Get().GetEV100() };
	    LOG_CLIENT_INFO("Client::Update", "Current EV100 is {0}", renderer::PostProcess::Get().SetEV100(currentEV100 - DeltaTime));
	}

	if (InputController::Get().KeyIsPressed(fthKey::fthKey_V))
	{
		ToggleLuminanceDebugging();
	}

	if (InputController::Get().KeyIsPressed(fthKey::fthKey_M))
	{
		//Add new dissolution instance when pressing M
		CreateDissolutionInstance(dissolutionModelHandles[SAMURAI_HANDLE_IDX], dissolutionMaterialHandles[SAMURAI_HANDLE_IDX].data(), static_cast<uint32_t>(dissolutionMaterialHandles[SAMURAI_HANDLE_IDX].size()));
	}

	if (InputController::Get().KeyIsPressed(fthKey::fthKey_G))
	{
		SpawnDecal();
	}
	if (InputController::Get().KeyIsPressed(fthKey::fthKey_Delete))
	{
		IncinerateInstance();
	}
	m_TranslationVector = m_CurrentCameraSpeed * m_CameraSpeedMultiplier * m_TranslationDirection * DeltaTime;
	m_RotationVector = m_RotationSpeedVector * DeltaTime;


	MoveCamera(m_TranslationVector, m_RotationVector);
	DragObjects();


	uint32_t dissolutionSize{ static_cast<uint32_t>(m_dissolutionInstances.size()) };

	for (uint32_t i = 0; i < dissolutionSize;)
	{
		DissolutionInstance& dis = m_dissolutionInstances[i];

		if (dis.HasFinishedAnimation())
		{
			//Add instance to opaque
			Handle<math::Matrix> transformid = dis.getTransformHandle();
			const std::shared_ptr<Model>& model = MeshSystem::Get().getModelByHandle(dis.getModelInstanceHandle());

			shading::OpaqueGroup::InstanceData instanceData;

			instanceData.handle_modelToWorld = transformid;

			if (model == ModelManager::Get().FindModel("Samurai"))
			{
				MeshSystem::Get().addOpaqueInstances(opaqueModelHandles[SAMURAI_HANDLE_IDX], opaqueMaterialHandles[SAMURAI_HANDLE_IDX].data(), &instanceData, 1, model->GetNumMeshes());
			}
			else
			{
				MeshSystem::Get().addOpaqueInstances(opaqueModelHandles[HORSE_HANDLE_IDX], opaqueMaterialHandles[HORSE_HANDLE_IDX].data(), &instanceData, 1, model->GetNumMeshes());
			}
			//Remove this item from the vector
			dis.Destroy();

			//Move last instance to removed instance slot, resize vector by one, keep iterating
			--dissolutionSize;

			m_dissolutionInstances[i] = std::move(m_dissolutionInstances[dissolutionSize]);
			continue;
		}
		++i;
	}
	m_dissolutionInstances.resize(dissolutionSize);


	uint32_t incinerateSize{ static_cast<uint32_t>(m_incinerationInstances.size()) };

	for (uint32_t i = 0; i < incinerateSize;)
	{
		IncinerationInstance& dis = m_incinerationInstances[i];

		if (dis.HasFinishedAnimation())
		{
			//Remove this item from the vector
			dis.Destroy();

			//Move last instance to removed instance slot, resize vector by one, keep iterating
			--incinerateSize;

			m_incinerationInstances[i] = std::move(m_incinerationInstances[incinerateSize]);
			continue;
		}
		++i;
	}
	m_incinerationInstances.resize(incinerateSize);
	//---------------------------------------
	for (DissolutionInstance& dis : m_dissolutionInstances)
		dis.dataIsUpdated = false;
	for (IncinerationInstance& inc : m_incinerationInstances)
		inc.isUpdated = false;
}


void ClientApp::OnRender()
{
	renderer::D3DRenderer::Get().BindLDR_Target();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	//renderer::D3DRenderer::Get().BindHDR_Target();
}

void ClientApp::MoveCamera(const math::Vector3& offsetTranslation, const math::Vector3& offsetRotation)
{
	fth::renderer::Camera& camera = CameraManager::GetActiveCamera();
	camera.AddRelativePos(offsetTranslation);
	camera.AddRelativeAngles(offsetRotation.x, offsetRotation.y, offsetRotation.z);
	camera.UpdateMatrices();

}

void ClientApp::OnResize(uint16_t newWidth, uint16_t newHeight)
{
	Win32Window& targetWindow = GetBaseWindow();
	Win32Window::Resize(targetWindow, newWidth, newHeight);

	renderer::D3DRenderer::Get().ShutdownViews();
	targetWindow.ResizeBackBuffers(newWidth, newHeight);
	renderer::D3DRenderer::Get().AttachLDR_Target(targetWindow.GetTexture());

}

void ClientApp::OnShutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ClientApp::SetMovementDirection(const SimpleMath::Vector3& direction)
{
	m_TranslationDirection = direction;
}

bool ClientApp::HandleImGUI_Input()
{
	ImGuiIO& io = ImGui::GetIO();

	bool shouldDiscard{ io.NavActive };

	for (uint8_t mk = 0; mk < static_cast<uint8_t>(MouseKey::NUM); ++mk)
	{
		io.AddMouseButtonEvent(mk, InputController::Get().KeyIsDown(static_cast<MouseKey>(mk)));
	}
	io.AddMouseWheelEvent(0.0f, InputController::Get().GetMouse().GetDeltaWheel());

	for (uint32_t k = 0; k < static_cast<uint32_t>(fthKey::NUM); ++k)
	{
		io.AddKeyEvent(_TranslateToImGuiKey(static_cast<fthKey>(k)), InputController::Get().KeyIsDown(static_cast<fthKey>(k)));
	}

	return shouldDiscard;
}


void ClientApp::HandleTranslation()
{
	if (InputController::Get().KeyIsDown(fthKey::fthKey_W))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::BitsetController::MoveDir::FORWARD);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::FORWARD);
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_A))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::MoveDir::LEFT);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::LEFT);
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_S))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::MoveDir::BACKWARD);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::BACKWARD);
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_D))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::BitsetController::MoveDir::RIGHT);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::RIGHT);
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_E))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::BitsetController::MoveDir::UP);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::UP);
	}

	if (InputController::Get().KeyIsDown(fthKey::fthKey_Q))
	{
		m_BitsetController.SetBitsetMovementDirection(BitsetController::BitsetController::MoveDir::DOWN);
	}
	else
	{
		m_BitsetController.UnsetBitsetMovementDirection(BitsetController::MoveDir::DOWN);
	}

}

void ClientApp::HandleRotation()
{
	//Roll

	m_RotationSpeedVector.x = 0.f;

	if (InputController::Get().KeyIsDown(MouseKey::LMB))
	{

		if (m_oldMousePos.xPos == GetBaseWindow().GetWindowWidth() / 2 && m_oldMousePos.yPos == GetBaseWindow().GetWindowHeight() / 2)
		{
			m_oldMousePos = InputController::Get().GetMouse().GetMousePos();
		}
		else
		{
			//Calculate delta from old position
			MouseDelta offsetFromOldMousePos;
			offsetFromOldMousePos.deltaX = InputController::Get().GetMouse().GetMousePos().xPos - m_oldMousePos.xPos;
			offsetFromOldMousePos.deltaY = InputController::Get().GetMouse().GetMousePos().yPos - m_oldMousePos.yPos;

			m_PitchSpeed = (XM_2PI / static_cast<float>(GetBaseWindow().GetWindowHeight())) * offsetFromOldMousePos.deltaY;
			m_YawSpeed = (XM_2PI / static_cast<float>(GetBaseWindow().GetWindowWidth())) * offsetFromOldMousePos.deltaX;
		}

	}
	else if(InputController::Get().KeyIsReleased(MouseKey::LMB))
	{
		m_PitchSpeed = 0.0f;
		m_YawSpeed = 0.0f;
		m_oldMousePos.xPos = GetBaseWindow().GetWindowWidth() / 2;
		m_oldMousePos.yPos = GetBaseWindow().GetWindowHeight() / 2;
	}

	m_RotationSpeedVector.y = m_PitchSpeed;
	m_RotationSpeedVector.z = m_YawSpeed;


}

void ClientApp::DragObjects()
{


	if (InputController::Get().KeyIsDown(MouseKey::RMB))
	{
		Ray ray;
		MousePos mousePos{ InputController::Get().GetMouse().GetMousePos() };
		CameraManager::GetActiveCamera().BuildRayFromScreenPixel((uint16_t)mousePos.xPos, (uint16_t)mousePos.yPos, GetBaseWindow().GetWindowWidth(), GetBaseWindow().GetWindowHeight(), ray);

		if (m_Dragger)
		{

			float offsetT{ m_tDistanceDrag / CameraManager::GetActiveCamera().GetForward().Dot(ray.direction) };

			math::Vector3 offset{ ray.at(offsetT) - m_oldDragPos };
			m_Dragger.get()->drag(offset);

			m_oldDragPos = ray.at(offsetT);
		}
		else
		{
			MeshSystem::HitResult query{};
			query.hitResult.ResetDistance();

			if (MeshSystem::Get().FindIntersection(ray, query))
			{
				m_Dragger.reset(new ModelDragger(query.handle_transform));
				m_tDistanceDrag = query.hitResult.tDistance * CameraManager::GetActiveCamera().GetForward().Dot(ray.direction);
				m_oldDragPos = query.hitResult.hitPoint ;
			}
		}
	}
	else if (InputController::Get().KeyIsReleased(MouseKey::RMB))
	{
		m_tDistanceDrag = std::numeric_limits<float>::infinity();
		m_oldDragPos = math::Vector3::Zero;
		m_Dragger.reset();
	}


}

void ClientApp::CreateDissolutionInstance(Handle<shading::DissolutionGroup::PerModel> modelHandle, const Handle<shading::DissolutionGroup::Material>* materialHandles, uint32_t numMeshes)
{
	//Instance needs to be created at fixed position in fornt of the camera
	const renderer::Camera& camera{ CameraManager::GetActiveCamera() };
	math::Transform transform{ camera.GetPosition() , math::Vector3::One, camera.GetRotation()};

	transform.position += m_dissolutionSpawnDistance * camera.GetForward();
	shading::DissolutionGroup::InstanceData instance;
	instance.animationTime = m_dissolutionInitialTime;
	instance.animationDuration = m_dissolutionDuration;
	instance.handle_modelToWorld = TransformSystem::Get().AddTransform(transform);

	MeshSystem::Get().addDissolutionInstances(modelHandle, materialHandles, &instance, 1, numMeshes);

	m_dissolutionInstances.emplace_back(instance);
}

void ClientApp::IncinerateInstance()
{
	MousePos mousePos{ InputController::Get().GetMouse().GetMousePos() };
	Ray testRay;
	CameraManager::GetActiveCamera().BuildRayFromScreenPixel(
		static_cast<uint16_t>(mousePos.xPos), 
		static_cast<uint16_t>(mousePos.yPos),
		GetBaseWindow().GetWindowWidth(), 
		GetBaseWindow().GetWindowHeight(), 
		testRay);

	MeshSystem::HitResult result;
	if (MeshSystem::Get().FindIntersection(testRay, result))
	{
		//verify if we hit an opaque instance
		if (result.type != MeshSystem::IntersectedType::OpaqueGroup)
			return;
		//Now  we have model instance ID and objectID
		//1-Get the model
		//2-Get the material
		//3-Insert into Incineration group
		//4-Query matrix and invert to transform hit position into model space(remember that sphere pos is in model space)
		//5-Fill up instance data by randomizing color
		//6-Create the instance

		std::shared_ptr<Model> instanceModel{ MeshSystem::Get().getModelByHandle(result.handle_instance) };

		//TODO: replace stack arrays by inlined_vectors. Lookat HW12 (or 13, i dont remember). This isnt safe for models > 15 meshes
		shading::OpaqueGroup::Material targetOpaqueMaterials[15];

		uint32_t numMeshes{ instanceModel->GetNumMeshes() };

		if (MeshSystem::Get().queryInstanceMaterial(result.handle_instance, targetOpaqueMaterials))
		{
			shading::IncinerationGroup::Material incinerationMaterials[15];
			Handle<shading::IncinerationGroup::PerModel> modelHandle { MeshSystem::Get().addIncinerationModel(instanceModel) };
			Handle<shading::IncinerationGroup::Material> materialHandles[15];

			const std::shared_ptr<Texture>& noiseTx{ TextureManager::Get().FindTexture("Noise_10.dds") };

			uint32_t materialFlags{ shading::IncinerationGroup::BUILD_BLUE_CHANNEL };
			for (uint32_t i = 0; i < numMeshes; ++i)
			{
				incinerationMaterials[i] = shading::IncinerationGroup::Material(
					targetOpaqueMaterials[i].albedo,
					targetOpaqueMaterials[i].normal,
					targetOpaqueMaterials[i].roughness,
					targetOpaqueMaterials[i].metalness,
					noiseTx,
					materialFlags, targetOpaqueMaterials[i].f_roughness, targetOpaqueMaterials[i].f_metalness);
			}

			MeshSystem::Get().addIncinerationMaterial(incinerationMaterials, modelHandle, materialHandles);

			shading::IncinerationGroup::InstanceData data;

			math::Matrix worldToModel;
			math::InvertOrthogonalMatrix(TransformSystem::Get().QueryTransformMatrix(result.handle_transform), worldToModel);

			data.sphereToModelPos = math::Vector3::Transform(result.hitResult.hitPoint, worldToModel);
			data.radius = 0.0f;
			data.lastFrameRadius = 0.0f;
			data.emission = 10.0f * Random::randomVector3(math::Vector2::UnitY, math::Vector2::UnitY, math::Vector2::UnitY);
			data.handle_modelToWorld = result.handle_transform;
			data.radiusGrowthRate = 1.0f;
			float aabbDiagonal{ math::Vector3(instanceModel->getAABB().Extents).Length() * 2.0f };

			MeshSystem::Get().deleteInstance(result.handle_instance);
			MeshSystem::Get().addIncinerationInstances(modelHandle, materialHandles, &data, 1, numMeshes);

			m_incinerationInstances.emplace_back(data, aabbDiagonal);
		}

	}
}

void ClientApp::SpawnDecal()
{
	Ray ray;

	const math::Matrix& cameraTransform{ CameraManager::GetActiveCamera().GetInverseView() };

	ray.position = cameraTransform.Translation();
	ray.direction = cameraTransform.Forward();

	MeshSystem::HitResult result;
	result.hitResult.ResetDistance();

	if (MeshSystem::Get().FindIntersection(ray, result))
	{
		const math::Matrix& hitTransform{ result.handle_transform.isValid() ? TransformSystem::Get().QueryTransformMatrix(result.handle_transform) : math::Matrix::Identity };
		math::Transform transform;

		math::Matrix decalToModel{ math::Matrix::Identity };

		//scale -> rotate -> translate
		decalToModel.Right(cameraTransform.Right());
		decalToModel.Up(cameraTransform.Up());
		decalToModel.Forward(cameraTransform.Forward());
		decalToModel = decalToModel * math::Matrix::CreateFromQuaternion(math::Quaternion::CreateFromAxisAngle(cameraTransform.Forward(), Random::randomFloatUniformDistribution(0.0f, DirectX::XM_2PI)));
		decalToModel.Translation(result.hitResult.hitPoint);

		math::Matrix invHitTransform;
		math::InvertOrthogonalMatrix(hitTransform, invHitTransform);
		decalToModel *= invHitTransform;


		math::Vector3 targetSpaceHit { math::Vector3::Transform(result.hitResult.hitPoint, invHitTransform) };
		
		math::Color color = math::Color(Random::randomFloatUniformDistribution(0.0f, 1.0f), Random::randomFloatUniformDistribution(0.0f, 1.0f), Random::randomFloatUniformDistribution(0.0f, 1.0f));
		

		DecalMaterial material;
		material.invertNormals = false;
		material.buildBlueNormals = false;
		material.tx_normal = TextureManager::Get().FindTexture("splatter-512.dds");
		DecalSystem::Get().addDecal(material, TransformSystem::Get().AddTransform(decalToModel), result.handle_transform, result.handle_object, color);
	}
}

void ClientApp::CreateShowcaseScene()
{

	//moon luminance : ~5000 cd/sr. moon w ~ 6.418e-5
	math::Vector3 dir{ 1.0f, -1.0f, 1.0f };
	LightSystem::Get().AddDirectionalSphereLight(25000.0f * DirectX::Colors::White.v,  { 1.0f, -1.0f, 1.0f }, 6.418e-5f, Handle<math::Matrix>::Invalid());

	//1200 lumens (luminous flux, or phi light bulbs)
	{
		Handle<math::Matrix> emissiveID;
		math::Transform transform = math::Transform::Initial;
		float radius = 0.25f;
		transform.scale = math::Vector3(radius);

		math::Vector3 position{ math::Vector3::Zero };

		//Wrap this process into a Higher level light function
		constexpr uint8_t NUM_POINT{ 2 };
		shading::EmissionOnly::InstanceData emissiveInstance[NUM_POINT];
		const math::Vector3 radiance[NUM_POINT] = { 10000.0f * DirectX::Colors::OrangeRed.v , 10000.0f * DirectX::Colors::Red.v };

		Handle<shading::EmissionOnly::PerModel> emissionModelHandle = MeshSystem::Get().addEmissionOnlyModel(ModelManager::GetUnitSphereSmooth());
		shading::EmissionOnly::Material emissiveMat[10] = {};
		Handle<shading::EmissionOnly::Material> matHandles[10];
		MeshSystem::Get().addEmissionOnlyMaterial(emissiveMat, emissionModelHandle, matHandles);


		transform.position = { -5.0f, 5.0f, 0.0f };
		emissiveID = TransformSystem::Get().AddTransform(transform);

		LightSystem::Get().AddPointSphereLight(radiance[0], radius, position, emissiveID);

		emissiveInstance[0].emissionColor = radiance[0];
		emissiveInstance[0].handle_modelToWorld = emissiveID;

		transform.position = {  5.0f, 5.0f, 0.0f };
		emissiveID = TransformSystem::Get().AddTransform(transform);

		LightSystem::Get().AddPointSphereLight(radiance[1], radius, position, emissiveID);

		emissiveInstance[1].emissionColor = radiance[1];
		emissiveInstance[1].handle_modelToWorld = emissiveID;


		MeshSystem::Get().addEmissionOnlyInstances(emissionModelHandle, matHandles, emissiveInstance, 2, 1);




		constexpr math::Vector3 emitterPos{ 0.0f, 0.0f, 0.0f };
		const math::Color emitterTint{ DirectX::Colors::Black.v };
		constexpr float spawnRadius{ 0.5f };
		Handle<math::Matrix> emitterParentID{ emissiveID };
		constexpr math::Vector2 emitterXSpeedRange{ -0.25f, 0.25f };
		constexpr math::Vector2 emitterZSpeedRange{ -0.25f, 0.25f };
		constexpr math::Vector2 emitterInitialSize{ 1.0f, 1.0f };
		constexpr math::Vector2 emitterRotationRange{ 0.0f, DirectX::XM_PIDIV2 };
		constexpr float emitterSizeIncreaseRate{ 0.01f };
		constexpr float emitterYspeed{ 2.5f };
		constexpr float emitterParticleLifetime{ 2.0f };
		constexpr float emitterMaxParticles{ 100.0f };
		constexpr float spawnRate{ emitterMaxParticles / emitterParticleLifetime };

		ParticleSystem::Get().AddSmokeEmitter(emitterPos, spawnRate, emitterTint, spawnRadius, emitterParentID, emitterXSpeedRange, emitterZSpeedRange, emitterInitialSize, emitterRotationRange, emitterSizeIncreaseRate, emitterYspeed, emitterParticleLifetime, emitterMaxParticles);

		ParticleSystem::Get().setGPUParticlesMask(TextureManager::Get().FindTexture("dust.dds"));
	}



	//Flashlight->hook this into a class
	m_flashlightID = LightSystem::Get().AddSpotSphereLight(
		200.0f * DirectX::Colors::White.v,
		0.25f,
		{ 0.0f, 0.0f, 7.0f },
		math::Vector3::Forward, 
		DirectX::XM_PIDIV4 * 0.5f, 
		DirectX::XM_PIDIV2 * 0.5f,
		Handle<math::Matrix>::Invalid(),
		TextureManager::Get().FindTexture("flashlight_cookie.dds")
	);



	renderer::D3DRenderer::Get().SetSkybox("grass_field.dds");
	


	shading::OpaqueGroup::InstanceData opaqueInstanceData[20];

	constexpr uint32_t numHorseInstances{ 5 };

	for (uint32_t i = 0; i < numHorseInstances; ++i)
	{
		math::Transform transform{ math::Transform::Initial };
		transform.position.z += 3.0f * i;
		transform.scale *= (i * 0.33f + 1.0f);
		opaqueInstanceData[i].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);
	}

	MeshSystem::Get().addOpaqueInstances(opaqueModelHandles[HORSE_HANDLE_IDX], opaqueMaterialHandles[HORSE_HANDLE_IDX].data(), opaqueInstanceData, numHorseInstances, static_cast<uint32_t>(opaqueMaterialHandles[HORSE_HANDLE_IDX].size()));


	for (uint32_t i = 0; i < numHorseInstances; ++i)
	{
		math::Transform transform{ math::Transform::Initial };
	    transform.position.x += 2.0f * i + 2.0f;
		transform.scale *= (i * 0.33f + 1.0f);
	
		opaqueInstanceData[i].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);
	}
	MeshSystem::Get().addOpaqueInstances(opaqueModelHandles[SAMURAI_HANDLE_IDX], opaqueMaterialHandles[SAMURAI_HANDLE_IDX].data(), opaqueInstanceData, numHorseInstances, static_cast<uint32_t>(opaqueMaterialHandles[SAMURAI_HANDLE_IDX].size()));


	//Build Quad floors
	Handle<shading::OpaqueGroup::PerModel> opaqueModelHandle{ MeshSystem::Get().addOpaqueModel(ModelManager::GetQuad()) };
	shading::OpaqueGroup::Material opaqueQuadMat[2];
	opaqueQuadMat[0] = shading::OpaqueGroup::Material(
		TextureManager::Get().FindTexture("Mud_BaseColor.dds"),
		TextureManager::Get().FindTexture("Mud_Normal.dds"),
		TextureManager::Get().FindTexture("Mud_Roughness.dds"),
		nullptr,
		shading::OpaqueGroup::BUILD_BLUE_CHANNEL,
		1.0f, 0.0f);


	opaqueQuadMat[1] = shading::OpaqueGroup::Material(
		TextureManager::UVTexture(),
		nullptr,
		nullptr,
		nullptr,
		0,
		1.0f, 0.0f);

	Handle<shading::OpaqueGroup::Material> opaqueQuadMatHandles[2];

	MeshSystem::Get().addOpaqueMaterial(&opaqueQuadMat[0], opaqueModelHandle, &opaqueQuadMatHandles[0]);
	MeshSystem::Get().addOpaqueMaterial(&opaqueQuadMat[1], opaqueModelHandle, &opaqueQuadMatHandles[1]);

	shading::OpaqueGroup::InstanceData instanceData[4];
	math::Transform transform;
	transform.position = math::Vector3::Zero;
	transform.scale = math::Vector3::One * 20.0f;
	transform.rotation = math::Quaternion::CreateFromAxisAngle(math::Vector3::Right, DirectX::XM_PIDIV2);
	instanceData[0].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);

	MeshSystem::Get().addOpaqueInstances(opaqueModelHandle, opaqueQuadMatHandles, &instanceData[0], 1, 1);

	transform.position.y = 20.0f;
	transform.rotation = math::Quaternion::CreateFromAxisAngle(math::Vector3::Right, -1.0f * DirectX::XM_PIDIV2);
	instanceData[1].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);

	transform.position.x = -10.0f;
	transform.position.y = 10.0f;
	transform.rotation = math::Quaternion::CreateFromAxisAngle(math::Vector3::Up, -1.0f * DirectX::XM_PIDIV2);
	instanceData[2].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);

	transform.position.x = 10.0f;
	transform.position.y = 10.0f;
	transform.rotation = math::Quaternion::CreateFromAxisAngle(math::Vector3::Up, 1.0f * DirectX::XM_PIDIV2);
	instanceData[3].handle_modelToWorld = TransformSystem::Get().AddTransform(transform);
	MeshSystem::Get().addOpaqueInstances(opaqueModelHandle, &opaqueQuadMatHandles[1], &instanceData[1], 3, 1);

	m_attatchedFlashlight = false;

	if (m_attatchedFlashlight)
	{
		LightSystem::Get().QuerySpotLight(m_flashlightID).parentID = CameraManager::GetActiveCamera().GetIDInverseView();
	}


}


void ClientApp::LoadTexturesAndModels()
{
	//TODO: Create a texture loader for models: -> a file would contain
	//Texture name should be: MODEL_MESH_TYPE.dds
	//For each mesh, search for a texture that matches these characteristics

	
	TextureManager::Get().LoadTextures("Textures/Mud/Mud_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/Mud/Mud_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/Mud/Mud_Roughness.dds");


	TextureManager::Get().LoadTextures("Textures/MetalRose/MetalRose_BaseColor.dds");


	TextureManager::Get().LoadTextures("Textures/Rock/rockyGround_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/Rock/rockyGround_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/Rock/rockyGround_Roughness.dds");


	TextureManager::Get().LoadTextures("Textures/Stone/Stone_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/Stone/Stone_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/Stone/Stone_Roughness.dds");
	

	TextureManager::Get().LoadTextures("Textures/RedCrystal/redCrystal_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/RedCrystal/redCrystal_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/RedCrystal/redCrystal_Roughness.dds");

	TextureManager::Get().LoadTextures("Textures/YellowContainer/yellowContainer_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/YellowContainer/yellowContainer_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/YellowContainer/yellowContainer_Roughness.dds");

	TextureManager::Get().LoadTextures("Textures/Cobblestone/Cobblestone_BaseColor.dds");
	TextureManager::Get().LoadTextures("Textures/Cobblestone/Cobblestone_Normal.dds");
	TextureManager::Get().LoadTextures("Textures/Cobblestone/Cobblestone_Roughness.dds");
	

	TextureManager::Get().LoadTextures("Textures/bricks.dds");
	TextureManager::Get().LoadTextures("Textures/weirdGrass.dds");
	TextureManager::Get().LoadTextures("Textures/dafoe.dds");
	TextureManager::Get().LoadTextures("Textures/cubeFaces.dds");
	TextureManager::Get().LoadTextures("Textures/flashlight_cookie.dds");
	TextureManager::Get().LoadTextures("Textures/Noise_22.dds");
	TextureManager::Get().LoadTextures("Textures/Noise_10.dds");
	TextureManager::Get().LoadTextures("Textures/splatter-512.dds");
	TextureManager::Get().LoadTextures("Textures/FireNoise1.dds");
	TextureManager::Get().LoadTextures("Textures/spark.dds");
	TextureManager::Get().LoadTextures("Textures/dust.dds");





	TextureManager::Get().LoadCubemaps("Textures/Cubemaps/testSkybox.dds");
	TextureManager::Get().LoadCubemaps("Textures/Cubemaps/grass_field.dds");
	TextureManager::Get().LoadCubemaps("Textures/Cubemaps/lake_beach.dds");
	TextureManager::Get().LoadCubemaps("Textures/Cubemaps/mountains.dds");
	TextureManager::Get().LoadCubemaps("Textures/Cubemaps/night_street.dds");



	constexpr uint8_t MAX_MESHES_PER_MAT{ 10 };//TODO: temp

	shading::OpaqueGroup::Material opaqueMaterials[MAX_MESHES_PER_MAT];
	shading::DissolutionGroup::Material dissolutionMaterials[MAX_MESHES_PER_MAT];
	uint16_t opaqueMatFlag{ shading::OpaqueGroup::MaterialFlags::BUILD_BLUE_CHANNEL };
	uint16_t dissMatFlag{ shading::DissolutionGroup::MaterialFlags::BUILD_BLUE_CHANNEL };
	{
		std::shared_ptr<Model> horseModel{ ModelManager::Get().LoadModel("Models/KnightHorse", ModelManager::ModelFormats::FBX) };
		uint32_t numHorseMesh{ ModelManager::Get().LoadModel("Models/KnightHorse", ModelManager::ModelFormats::FBX)->GetNumMeshes() };

		dissolutionMaterialHandles[HORSE_HANDLE_IDX].resize(numHorseMesh);
		opaqueMaterialHandles[HORSE_HANDLE_IDX].resize(numHorseMesh);

		horsePBR.resize(numHorseMesh);
		horsePBR[0].albedo = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Armor_BaseColor.dds");
		horsePBR[0].normals = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Armor_Normal.dds");
		horsePBR[0].roughness = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Armor_Roughness.dds");
		horsePBR[0].metalness = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Armor_Metallic.dds");


		horsePBR[1].albedo = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_BaseColor.dds");
		horsePBR[1].normals = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Normal.dds");
		horsePBR[1].roughness = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Roughness.dds");

		horsePBR[2].albedo = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Tail_BaseColor.dds");
		horsePBR[2].normals = TextureManager::Get().LoadTextures("Models/KnightHorse/Textures/dds/Horse_Tail_Normal.dds");

		dissolutionModelHandles[HORSE_HANDLE_IDX] = MeshSystem::Get().addDissolutionModel(horseModel);
		opaqueModelHandles[HORSE_HANDLE_IDX] = MeshSystem::Get().addOpaqueModel(horseModel);

		const std::shared_ptr<Texture>& noiseTx{ TextureManager::Get().FindTexture("Noise_22.dds") };
		for (uint32_t meshIdx = 0; meshIdx < numHorseMesh; ++meshIdx)
		{
			shading::OpaqueGroup::Material& opaqueMaterial{ opaqueMaterials[meshIdx] };
			shading::DissolutionGroup::Material& dissolutionMaterial{ dissolutionMaterials[meshIdx] };

			opaqueMaterial = shading::OpaqueGroup::Material(
				horsePBR[meshIdx].albedo,
				horsePBR[meshIdx].normals,
				horsePBR[meshIdx].roughness,
				horsePBR[meshIdx].metalness,
				opaqueMatFlag,
				1.0f, 0.0f);

			dissolutionMaterial = shading::DissolutionGroup::Material(
				horsePBR[meshIdx].albedo,
				horsePBR[meshIdx].normals,
				horsePBR[meshIdx].roughness,
				horsePBR[meshIdx].metalness,
				noiseTx,
				dissMatFlag,
				1.0f, 0.0f);

		
		}
		MeshSystem::Get().addOpaqueMaterial(opaqueMaterials, opaqueModelHandles[HORSE_HANDLE_IDX], opaqueMaterialHandles[HORSE_HANDLE_IDX].data());
		MeshSystem::Get().addDissolutionMaterial(dissolutionMaterials, dissolutionModelHandles[HORSE_HANDLE_IDX], dissolutionMaterialHandles[HORSE_HANDLE_IDX].data());




	}

	{
		std::shared_ptr<Model> samuraiModel{ ModelManager::Get().LoadModel("Models/Samurai", ModelManager::ModelFormats::FBX) };
		uint32_t numSamuraiMesh{ ModelManager::Get().LoadModel("Models/Samurai", ModelManager::ModelFormats::FBX)->GetNumMeshes() };

		dissolutionMaterialHandles[SAMURAI_HANDLE_IDX].resize(numSamuraiMesh);
		opaqueMaterialHandles[SAMURAI_HANDLE_IDX].resize(numSamuraiMesh);

		samuraiPBR.resize(numSamuraiMesh);

		samuraiPBR[0].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Sword_BaseColor.dds");
		samuraiPBR[0].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Sword_Normal.dds");
		samuraiPBR[0].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Sword_Roughness.dds");
		samuraiPBR[0].metalness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Sword_Metallic.dds");


		samuraiPBR[1].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Head_BaseColor.dds");
		samuraiPBR[1].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Head_Normal.dds");
		samuraiPBR[1].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Head_Roughness.dds");

		samuraiPBR[2].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Eyes_BaseColor.dds");
		samuraiPBR[2].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Eyes_Normal.dds");

		samuraiPBR[3].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Helmet_BaseColor.dds");
		samuraiPBR[3].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Helmet_Normal.dds");
		samuraiPBR[3].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Helmet_Roughness.dds");
		samuraiPBR[3].metalness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Helmet_Metallic.dds");


		samuraiPBR[4].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Decor_BaseColor.dds");
		samuraiPBR[4].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Decor_Normal.dds");
		samuraiPBR[4].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Decor_Roughness.dds");
		samuraiPBR[4].metalness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Decor_Metallic.dds");


		samuraiPBR[5].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Pants_BaseColor.dds");
		samuraiPBR[5].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Pants_Normal.dds");
		samuraiPBR[5].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Pants_Roughness.dds");
		samuraiPBR[5].metalness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Pants_Metallic.dds");


		samuraiPBR[6].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Hands_BaseColor.dds");
		samuraiPBR[6].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Hands_Normal.dds");
		samuraiPBR[6].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Hands_Roughness.dds");

		samuraiPBR[7].albedo = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Torso_BaseColor.dds");
		samuraiPBR[7].normals = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Torso_Normal.dds");
		samuraiPBR[7].roughness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Torso_Roughness.dds");
		samuraiPBR[7].metalness = TextureManager::Get().LoadTextures("Models/Samurai/Textures/dds/Samurai_Torso_Metallic.dds");

		dissolutionModelHandles[SAMURAI_HANDLE_IDX] = MeshSystem::Get().addDissolutionModel(samuraiModel);
		opaqueModelHandles[SAMURAI_HANDLE_IDX] = MeshSystem::Get().addOpaqueModel(samuraiModel);

		const std::shared_ptr<Texture>& noiseTx{ TextureManager::Get().FindTexture("Noise_22.dds") };

		for (uint32_t meshIdx = 0; meshIdx < numSamuraiMesh; ++meshIdx)
		{
			shading::OpaqueGroup::Material& opaqueMaterial{ opaqueMaterials[meshIdx] };
			shading::DissolutionGroup::Material& dissolutionMaterial{ dissolutionMaterials[meshIdx] };

			opaqueMaterial = shading::OpaqueGroup::Material(
				samuraiPBR[meshIdx].albedo,
				samuraiPBR[meshIdx].normals,
				samuraiPBR[meshIdx].roughness,
				samuraiPBR[meshIdx].metalness,
				opaqueMatFlag,
				1.0f, 0.0f);

			dissolutionMaterial = shading::DissolutionGroup::Material(
				samuraiPBR[meshIdx].albedo,
				samuraiPBR[meshIdx].normals,
				samuraiPBR[meshIdx].roughness,
				samuraiPBR[meshIdx].metalness,
				noiseTx,
				dissMatFlag,
				1.0f, 0.0f);

		}
		MeshSystem::Get().addDissolutionMaterial(dissolutionMaterials, dissolutionModelHandles[SAMURAI_HANDLE_IDX], dissolutionMaterialHandles[SAMURAI_HANDLE_IDX].data());
		MeshSystem::Get().addOpaqueMaterial(opaqueMaterials, opaqueModelHandles[SAMURAI_HANDLE_IDX], opaqueMaterialHandles[SAMURAI_HANDLE_IDX].data());


	}
	{
		std::shared_ptr<Model> towerModel{ ModelManager::Get().LoadModel("Models/EastTower", ModelManager::ModelFormats::FBX) };
		uint32_t numTowerMesh{ ModelManager::Get().LoadModel("Models/EastTower", ModelManager::ModelFormats::FBX)->GetNumMeshes() };
	}
	{
		std::shared_ptr<Model> knightModel{ ModelManager::Get().LoadModel("Models/Knight", ModelManager::ModelFormats::FBX) };
		uint32_t numKnightMesh{ ModelManager::Get().LoadModel("Models/Knight", ModelManager::ModelFormats::FBX)->GetNumMeshes() };
	}

	std::shared_ptr<Texture> diff = TextureManager::Get().LoadCubemaps("Textures/Cubemaps/grass_field_diffuseIrradiance.dds");
	std::shared_ptr<Texture> specI = TextureManager::Get().LoadCubemaps("Textures/Cubemaps/grass_field_specularIrradiance.dds");
	std::shared_ptr<Texture> specR = TextureManager::Get().LoadTextures("Textures/Cubemaps/grass_field_specularReflectance.dds");

	renderer::D3DRenderer::Get().SetEnvironmentalIBL({ diff, specI, specR });

	_6Way_Lightmap lightmap;
	lightmap.RLUmultipliers = TextureManager::Get().LoadTextures("Textures/SmokeEffect/smoke_RLU.dds");
	lightmap.DBFmultipliers = TextureManager::Get().LoadTextures("Textures/SmokeEffect/smoke_DBF.dds");
	lightmap.EMVAfactors = TextureManager::Get().LoadTextures("Textures/SmokeEffect/smoke_MVEA.dds");

	uint32_t atlasWidth = lightmap.RLUmultipliers->Width();
	uint32_t atlasHeight = lightmap.RLUmultipliers->Height();
	constexpr uint32_t numSprites = 64;
	Sprite sprite;
	sprite.width = atlasWidth / 8;
	sprite.height = atlasHeight / 8;

	lightmap.atlas.createTiledAtlas(atlasWidth, atlasHeight, sprite, numSprites);

	ParticleSystem::Get().SetSmokeParticlesLightmap(lightmap);
}

constexpr ImGuiKey _TranslateToImGuiKey(fthKey key)
{
	switch (key)
	{
	case fthKey::fthKey_A:            	 return ImGuiKey::ImGuiKey_A;
	case fthKey::fthKey_B:				 return ImGuiKey::ImGuiKey_B;
	case fthKey::fthKey_C:				 return ImGuiKey::ImGuiKey_C;
	case fthKey::fthKey_D:				 return ImGuiKey::ImGuiKey_D;
	case fthKey::fthKey_E:				 return ImGuiKey::ImGuiKey_E;
	case fthKey::fthKey_F:				 return ImGuiKey::ImGuiKey_F;
	case fthKey::fthKey_G:				 return ImGuiKey::ImGuiKey_G;
	case fthKey::fthKey_H:				 return ImGuiKey::ImGuiKey_H;
	case fthKey::fthKey_I:				 return ImGuiKey::ImGuiKey_I;
	case fthKey::fthKey_J:				 return ImGuiKey::ImGuiKey_J;
	case fthKey::fthKey_K:				 return ImGuiKey::ImGuiKey_K;
	case fthKey::fthKey_L:				 return ImGuiKey::ImGuiKey_L;
	case fthKey::fthKey_M:				 return ImGuiKey::ImGuiKey_M;
	case fthKey::fthKey_N:				 return ImGuiKey::ImGuiKey_N;
	case fthKey::fthKey_O:				 return ImGuiKey::ImGuiKey_O;
	case fthKey::fthKey_P:				 return ImGuiKey::ImGuiKey_P;
	case fthKey::fthKey_Q:				 return ImGuiKey::ImGuiKey_Q;
	case fthKey::fthKey_R:				 return ImGuiKey::ImGuiKey_R;
	case fthKey::fthKey_S:				 return ImGuiKey::ImGuiKey_S;
	case fthKey::fthKey_T:				 return ImGuiKey::ImGuiKey_T;
	case fthKey::fthKey_U:				 return ImGuiKey::ImGuiKey_U;
	case fthKey::fthKey_V:				 return ImGuiKey::ImGuiKey_V;
	case fthKey::fthKey_W:				 return ImGuiKey::ImGuiKey_W;
	case fthKey::fthKey_X:				 return ImGuiKey::ImGuiKey_X;
	case fthKey::fthKey_Y:				 return ImGuiKey::ImGuiKey_Y;
	case fthKey::fthKey_Z:				 return ImGuiKey::ImGuiKey_Z;
	case fthKey::fthKey_0:				 return ImGuiKey::ImGuiKey_0;
	case fthKey::fthKey_1:				 return ImGuiKey::ImGuiKey_1;
	case fthKey::fthKey_2:				 return ImGuiKey::ImGuiKey_2;
	case fthKey::fthKey_3:				 return ImGuiKey::ImGuiKey_3;
	case fthKey::fthKey_4:				 return ImGuiKey::ImGuiKey_4;
	case fthKey::fthKey_5:				 return ImGuiKey::ImGuiKey_5;
	case fthKey::fthKey_6:				 return ImGuiKey::ImGuiKey_6;
	case fthKey::fthKey_7:				 return ImGuiKey::ImGuiKey_7;
	case fthKey::fthKey_8:				 return ImGuiKey::ImGuiKey_8;
	case fthKey::fthKey_9:				 return ImGuiKey::ImGuiKey_9;
	case fthKey::fthKey_Left:			 return ImGuiKey::ImGuiKey_LeftArrow;
	case fthKey::fthKey_Right:			 return ImGuiKey::ImGuiKey_RightArrow;
	case fthKey::fthKey_Up:				 return ImGuiKey::ImGuiKey_UpArrow;
	case fthKey::fthKey_Down:			 return ImGuiKey::ImGuiKey_DownArrow;
	case fthKey::fthKey_PageUp:			 return ImGuiKey::ImGuiKey_PageUp;
	case fthKey::fthKey_PageDown:		 return ImGuiKey::ImGuiKey_PageDown;
	case fthKey::fthKey_Home:			 return ImGuiKey::ImGuiKey_Home;
	case fthKey::fthKey_End:			 return ImGuiKey::ImGuiKey_End;
	case fthKey::fthKey_Insert:			 return ImGuiKey::ImGuiKey_Insert;
	case fthKey::fthKey_Delete:			 return ImGuiKey::ImGuiKey_Delete;
	case fthKey::fthKey_Back:			 return ImGuiKey::ImGuiKey_Backspace;
	case fthKey::fthKey_Space:			 return ImGuiKey::ImGuiKey_Space;
	case fthKey::fthKey_Enter:			 return ImGuiKey::ImGuiKey_Enter;
	case fthKey::fthKey_Escape:			 return ImGuiKey::ImGuiKey_Escape;
	case fthKey::fthKey_Apostrophe:		 return ImGuiKey::ImGuiKey_Apostrophe;
	case fthKey::fthKey_Comma:			 return ImGuiKey::ImGuiKey_Comma;
	case fthKey::fthKey_Minus:			 return ImGuiKey::ImGuiKey_Minus;
	case fthKey::fthKey_Plus:			 return ImGuiKey::ImGuiKey_Equal;
	case fthKey::fthKey_Period:			 return ImGuiKey::ImGuiKey_Period;
	case fthKey::fthKey_Slash:			 return ImGuiKey::ImGuiKey_Slash;
	case fthKey::fthKey_Semicolon:		 return ImGuiKey::ImGuiKey_Semicolon;
	case fthKey::fthKey_LeftBracket:	 return ImGuiKey::ImGuiKey_LeftBracket;
	case fthKey::fthKey_Backslash:		 return ImGuiKey::ImGuiKey_Backslash;
	case fthKey::fthKey_RightBracket:	 return ImGuiKey::ImGuiKey_RightBracket;
	case fthKey::fthKey_GraveAccent:	 return ImGuiKey::ImGuiKey_GraveAccent;
	case fthKey::fthKey_CapsLock:		 return ImGuiKey::ImGuiKey_CapsLock;
	case fthKey::fthKey_ScrollLock:		 return ImGuiKey::ImGuiKey_ScrollLock;
	case fthKey::fthKey_PrintScreen:	 return ImGuiKey::ImGuiKey_PrintScreen;
	case fthKey::fthKey_Pause:			 return ImGuiKey::ImGuiKey_Pause;
	case fthKey::fthKey_KeyPad0:		 return ImGuiKey::ImGuiKey_Keypad0;
	case fthKey::fthKey_KeyPad1:		 return ImGuiKey::ImGuiKey_Keypad1;
	case fthKey::fthKey_KeyPad2:		 return ImGuiKey::ImGuiKey_Keypad2;
	case fthKey::fthKey_KeyPad3:		 return ImGuiKey::ImGuiKey_Keypad3;
	case fthKey::fthKey_KeyPad4:		 return ImGuiKey::ImGuiKey_Keypad4;
	case fthKey::fthKey_KeyPad5:		 return ImGuiKey::ImGuiKey_Keypad5;
	case fthKey::fthKey_KeyPad6:		 return ImGuiKey::ImGuiKey_Keypad6;
	case fthKey::fthKey_KeyPad7:		 return ImGuiKey::ImGuiKey_Keypad7;
	case fthKey::fthKey_KeyPad8:		 return ImGuiKey::ImGuiKey_Keypad8;
	case fthKey::fthKey_KeyPad9:		 return ImGuiKey::ImGuiKey_Keypad9;
	case fthKey::fthKey_KeyPadDecimal:	 return ImGuiKey::ImGuiKey_KeypadDecimal;
	case fthKey::fthKey_KeyPadDivide:	 return ImGuiKey::ImGuiKey_KeypadDivide;
	case fthKey::fthKey_KeyPadMultiply:	 return ImGuiKey::ImGuiKey_KeypadMultiply;
	case fthKey::fthKey_KeyPadSubtract:	 return ImGuiKey::ImGuiKey_KeypadSubtract;
	case fthKey::fthKey_KeyPadAdd:		 return ImGuiKey::ImGuiKey_KeypadAdd;
	case fthKey::fthKey_Tab:			 return ImGuiKey::ImGuiKey_Tab;
	case fthKey::fthKey_Shift:			 return ImGuiKey::ImGuiKey_LeftShift;
	case fthKey::fthKey_Control:		 return ImGuiKey::ImGuiKey_LeftCtrl;
		
	case fthKey::NUM:									  
	default:											  
		return ImGuiKey::ImGuiKey_None;					  
	}													  
}
