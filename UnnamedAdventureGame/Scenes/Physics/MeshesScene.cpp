#include "MeshesScene.h"
#include <SceneGraph/Scene.h>
#include <SceneGraph/GameObject.h>

#include <Components/RenderComponents/CameraComponent.h>
#include <Components/Transform/Transform.h>

#include "../../Components/FreeCamMovement.h"

#include "Components/Physics/BoxCollider.h"
#include "Components/Physics/MeshCollider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/RenderComponents/MeshRenderer.h"
#include "Graphics/Mesh.h"

void unag::MeshesScene::Load(leap::Scene& scene)
{
	leap::GameObject* pCameraObj{ scene.CreateGameObject("Extended Camera Test") };
	const leap::CameraComponent* pMainCamera{ pCameraObj->AddComponent<leap::CameraComponent>() };
	pMainCamera->SetAsActiveCamera(true);
	pCameraObj->GetTransform()->SetWorldPosition(0.0f, 0.0f, -5.0f);
	FreeCamMovement* freecam = pCameraObj->AddComponent<FreeCamMovement>();
	freecam->SetSpeed(1.f);

	leap::GameObject* pFloorObj{ scene.CreateGameObject("Floor") };
	pFloorObj->GetTransform()->SetWorldPosition(0.0f, -5.0f, 0.0f);
	leap::BoxCollider* pFloorCollider{ pFloorObj->AddComponent<leap::BoxCollider>() };
	pFloorCollider->SetSize(10, 2, 10);

	leap::Mesh mesh{ "Data/bunny.obj" };

	leap::GameObject* pMeshObj{ scene.CreateGameObject("Mesh") };

	leap::MeshRenderer* pMeshRenderer{ pMeshObj->AddComponent<leap::MeshRenderer>() };
	pMeshRenderer->SetMesh(mesh);

	leap::MeshCollider* pMeshCollider{ pMeshObj->AddComponent<leap::MeshCollider>() };
	pMeshCollider->SetMesh(mesh);

	pMeshObj->AddComponent<leap::Rigidbody>();
}
