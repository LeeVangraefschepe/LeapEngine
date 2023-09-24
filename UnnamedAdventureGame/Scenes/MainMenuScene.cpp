#include "MainMenuScene.h"
#include "SceneGraph/Scene.h"
#include "Components/RenderComponents/CameraComponent.h"
#include "Camera.h"

#include "InputManager.h"
#include "LambdaCommand.h"
#include "Components/Transform/Transform.h"

#include "ServiceLocator/ServiceLocator.h"
#include "Interfaces/IRenderer.h"

#include "GameContext/GameContext.h"

#include <Components/RenderComponents/MeshRenderer.h>
#include "../Components/Transformator.h"

#include "Shaders/Pos3D.h"

void unag::MainMenuScene::Load(leap::Scene& scene)
{
	leap::GameObject* pCameraObj{ scene.CreateGameObject("Main Camera") };
	leap::CameraComponent* pMainCamera{ pCameraObj->AddComponent<leap::CameraComponent>() };
	pMainCamera->SetAsActiveCamera(true);
	pCameraObj->GetTransform()->SetWorldPosition(0.0f, 0.0f, -5.0f);

	leap::GameObject* pCameraObj2{ scene.CreateGameObject("Other Camera") };
	leap::CameraComponent* pOtherCamera{ pCameraObj2->AddComponent<leap::CameraComponent>() };
	pOtherCamera->GetData()->SetColor(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
	pCameraObj2->GetTransform()->SetWorldPosition(0.0f, 0.0f, -5.0f);

	const auto pMaterial{ leap::ServiceLocator::GetRenderer().CreateMaterial(leap::graphics::shaders::Pos3D::GetShader())};

	auto mesh{ scene.CreateGameObject("Mesh") };
	mesh->AddComponent<leap::MeshRendererComponent>()->SetMaterial(pMaterial);
	mesh->AddComponent<Transformator>();

	leap::input::InputManager::GetInstance().AddCommand(
		std::make_shared<leap::LambdaCommand>([=]() 
			{ 
				leap::graphics::IRenderer& renderer{ leap::ServiceLocator::GetRenderer() };
				if (renderer.GetCamera() == pMainCamera->GetData())
				{
					pOtherCamera->SetAsActiveCamera(true);
				}
				else
				{
					pMainCamera->SetAsActiveCamera(true);
				}
			}),
		leap::input::InputManager::InputType::EventPress, 
		leap::input::InputManager::KeyboardInput::KeyB
	);

	leap::input::InputManager::GetInstance().AddCommand(
		std::make_shared<leap::LambdaCommand>([=]()
			{
				pCameraObj->GetTransform()->Translate(-10.0f * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime(), 0.0f, 0.0f);
			}),
		leap::input::InputManager::InputType::EventRepeat,
		leap::input::InputManager::KeyboardInput::KeyA
	);
	leap::input::InputManager::GetInstance().AddCommand(
		std::make_shared<leap::LambdaCommand>([=]()
			{
				pCameraObj->GetTransform()->Translate(10.0f * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime(), 0.0f, 0.0f);
			}),
		leap::input::InputManager::InputType::EventRepeat,
		leap::input::InputManager::KeyboardInput::KeyD
	);
	leap::input::InputManager::GetInstance().AddCommand(
		std::make_shared<leap::LambdaCommand>([=]()
			{
				pCameraObj->GetTransform()->Translate(0.0f, 0.0f, 10.0f * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
			}),
		leap::input::InputManager::InputType::EventRepeat,
		leap::input::InputManager::KeyboardInput::KeyW
	);
	leap::input::InputManager::GetInstance().AddCommand(
		std::make_shared<leap::LambdaCommand>([=]()
			{
				pCameraObj->GetTransform()->Translate(0.0f, 0.0f, -10.0f * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
			}),
		leap::input::InputManager::InputType::EventRepeat,
		leap::input::InputManager::KeyboardInput::KeyS
	);
}