#include "MeshesScene.h"
#include <SceneGraph/Scene.h>
#include <SceneGraph/GameObject.h>

#include <Components/RenderComponents/CameraComponent.h>
#include <Components/Transform/Transform.h>

#include "../../Components/FreeCamMovement.h"

#include "Components/Physics/BoxCollider.h"
#include "Components/Physics/MeshCollider.h"
#include "Components/Physics/Rigidbody.h"
#include "Components/RenderComponents/DirectionalLightComponent.h"
#include "Components/RenderComponents/MeshRenderer.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Shaders/Pos3D.h"

void unag::MeshesScene::Load(leap::Scene& scene)
{
	leap::GameObject* pCameraObj{ scene.CreateGameObject("Extended Camera Test") };
	const leap::CameraComponent* pMainCamera{ pCameraObj->AddComponent<leap::CameraComponent>() };
	pMainCamera->SetAsActiveCamera(true);
	pCameraObj->GetTransform()->SetWorldPosition(0.0f, 0.0f, -5.0f);
	FreeCamMovement* freecam = pCameraObj->AddComponent<FreeCamMovement>();
	freecam->SetSpeed(1.f);

	leap::GameObject* pDirLightObj{ scene.CreateGameObject("Directional light") };
	pDirLightObj->AddComponent<leap::DirectionalLightComponent>();
	pDirLightObj->GetTransform()->SetWorldRotation(50, -30, 0);
	pDirLightObj->GetTransform()->SetWorldPosition(0, 10, 0);

	leap::GameObject* pFloorObj{ scene.CreateGameObject("Floor") };
	pFloorObj->GetTransform()->SetWorldPosition(0.0f, -3.0f, 0.0f);

	leap::BoxCollider* pFloorCollider{ pFloorObj->AddComponent<leap::BoxCollider>() };
	pFloorCollider->SetSize(10, 2, 2);

	const leap::Mesh mesh{ "Data/bunny.obj" };
	const leap::Material meshMat{ leap::Material::Create<leap::graphics::shaders::Pos3D>("Mesh Material") };

	// Create static mesh collider
	{
		leap::GameObject* pMeshObj{ scene.CreateGameObject("Mesh") };
		pMeshObj->GetTransform()->SetWorldPosition(-5, 0, 0);

		leap::MeshRenderer* pMeshRenderer{ pMeshObj->AddComponent<leap::MeshRenderer>() };
		pMeshRenderer->SetMesh(mesh);
		pMeshRenderer->SetMaterial(meshMat);

		leap::MeshCollider* pMeshCollider{ pMeshObj->AddComponent<leap::MeshCollider>() };
		pMeshCollider->SetMesh(mesh);
	}

	// Create dynamic mesh collider
	{
		leap::GameObject* pMeshObj{ scene.CreateGameObject("MeshRB") };
		pMeshObj->GetTransform()->SetWorldPosition(0, 0, 0);

		leap::MeshRenderer* pMeshRenderer{ pMeshObj->AddComponent<leap::MeshRenderer>() };
		pMeshRenderer->SetMesh(mesh);
		pMeshRenderer->SetMaterial(meshMat);

		leap::MeshCollider* pMeshCollider{ pMeshObj->AddComponent<leap::MeshCollider>() };
		pMeshCollider->SetMesh(mesh);

		pMeshObj->AddComponent<leap::Rigidbody>();
	}

	// Create kinematic mesh collider
	{
		leap::GameObject* pMeshObj{ scene.CreateGameObject("MeshRB kinematic") };
		pMeshObj->GetTransform()->SetWorldPosition(5, 0, 0);

		leap::MeshRenderer* pMeshRenderer{ pMeshObj->AddComponent<leap::MeshRenderer>() };
		pMeshRenderer->SetMesh(mesh);
		pMeshRenderer->SetMaterial(meshMat);

		leap::MeshCollider* pMeshCollider{ pMeshObj->AddComponent<leap::MeshCollider>() };
		pMeshCollider->SetMesh(mesh);

		leap::Rigidbody* rb = pMeshObj->AddComponent<leap::Rigidbody>();
		rb->SetKinematic(true);
	}
}
