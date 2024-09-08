#include "PhysXEngine.h"

#include "PxPhysicsAPI.h"

#include <Debug.h>

#include "PhysXScene.h"
#include "PhysXShapes.h"
#include "PhysXObject.h"
#include "PhysXMaterial.h"
#include "PhysXSimulationCallbacks.h"
#include "PhysXSimulationFilterShader.h"
#include "PhysXSimulationCallbacks.h"
#include "../Data/SimulationEventData.h"

#include <algorithm>

leap::physics::PhysXEngine::PhysXEngine()
    : m_pDefaultAllocatorCallback{ std::make_unique<physx::PxDefaultAllocator>() }
    , m_pDefaultErrorCallback{ std::make_unique<physx::PxDefaultErrorCallback>() }
    , m_pSimulationCallbacks{ std::make_unique<PhysXSimulationCallbacks>() }
    , m_pSimulationFilterCallback{ std::make_unique<PhysXSimulationFilterCallback>() } 
{
    m_pSimulationFilterCallback->OnSimulationEvent.AddListener(this, &PhysXEngine::OnSimulationEvent);
    m_pSimulationCallbacks->OnSimulationEvent.AddListener(this, &PhysXEngine::OnSimulationEvent);

    // Create foundation
    m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_pDefaultAllocatorCallback, *m_pDefaultErrorCallback);
    if (!m_pFoundation)
    {
        Debug::LogError("PhysXEngine Error : PxCreateFoundation failed");
        return;
    }

    // Create top-level physics object
    bool recordMemoryAllocations{};
#if _DEBUG
    recordMemoryAllocations = true;
#endif
    physx::PxTolerancesScale scaling{}; // Default scaling (size = 1 & velocity = gravity * 1s = 9.81)
    scaling.length = 1.0f;
    scaling.speed = 9.81f;
    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, scaling, recordMemoryAllocations, nullptr);
    if (!m_pPhysics)
    {
        Debug::LogError("PhysXEngine Error : PxCreatePhysics failed");
        return;
    }

    // Create cooking object
    m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxCookingParams(m_pPhysics->getTolerancesScale()));
    if (!m_pCooking)
    {
        Debug::LogError("PhysXEngine Error : PxCreateCooking failed");
        return;
    }

    // Create a CPU dispatcher
    m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
}

leap::physics::PhysXEngine::~PhysXEngine()
{
    m_pSimulationFilterCallback->OnSimulationEvent.RemoveListener(this);
    m_pSimulationCallbacks->OnSimulationEvent.RemoveListener(this);

    m_pScene = nullptr;
    m_pObjects.clear();

    m_pDispatcher->release();
    m_pCooking->release();
    m_pPhysics->release();
    m_pFoundation->release();
}

void leap::physics::PhysXEngine::SetSyncFunc(const std::function<void(void*, const glm::vec3&, const glm::quat&)>& setFunc, const std::function<std::pair<glm::vec3, glm::quat>(void*)> getFunc)
{
    m_SyncGetFunc = getFunc;
    m_SyncSetFunc = setFunc;
}

void leap::physics::PhysXEngine::Update(float fixedDeltaTime)
{
    // Update all the physics objects and apply updates
    for (auto& pObject : m_pObjects)
    {
        pObject.second->Update(this, m_pScene.get());
    }

    // Erase all objects that are not connected anymore to a game object
    std::erase_if(m_pObjects, [](const auto& pObject) { return !pObject.second->IsValid(); });

    // Simulate the physics scene
    m_pScene->Simulate(fixedDeltaTime);

    // Apply poses
    for (auto& pObject : m_pObjects)
    {
        pObject.second->Apply(m_SyncSetFunc, m_SyncGetFunc);
    }

    m_pSimulationFilterCallback->NotifyStayingPairs(); // OnCollisionStay
    m_pSimulationCallbacks->NotifyStayingPairs(); // OnTriggerStay
}

void leap::physics::PhysXEngine::CreateScene()
{
    physx::PxSceneDesc sceneDesc{ m_pPhysics->getTolerancesScale() };
    sceneDesc.gravity = physx::PxVec3{ 0.0f, -9.81f, 0.0f };
    sceneDesc.filterShader = PhysXSimulationFilterShader;
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.simulationEventCallback = m_pSimulationCallbacks.get();
    sceneDesc.filterCallback = m_pSimulationFilterCallback.get();

    physx::PxScene* pPhysXScene{ m_pPhysics->createScene(sceneDesc) };

    m_pScene = std::make_unique<PhysXScene>(pPhysXScene);

    m_pScene->SetEnabledDebugDrawing(m_IsDebugDrawingEnabled);
}

leap::physics::IPhysicsObject* leap::physics::PhysXEngine::Get(void* pOwner)
{
    if (!m_pObjects.contains(pOwner))
    {
        m_pObjects[pOwner] = std::make_unique<PhysXObject>(pOwner);
    }

    return m_pObjects[pOwner].get();
}

std::unique_ptr<leap::physics::IShape> leap::physics::PhysXEngine::CreateShape(void* pOwner, physics::EShape shape, IPhysicsMaterial* pMaterial)
{
    if (!pMaterial)
    {
        pMaterial = GetDefaultMaterial();
    }

    switch (shape)
    {
    case EShape::Box:
        return std::make_unique<PhysXBoxShape>(this, pOwner, static_cast<PhysXMaterial*>(pMaterial));
    case EShape::Sphere:
        return std::make_unique<PhysXSphereShape>(this, pOwner, static_cast<PhysXMaterial*>(pMaterial));
    case EShape::Capsule:
        return std::make_unique<PhysXCapsuleShape>(this, pOwner, static_cast<PhysXMaterial*>(pMaterial));
    case EShape::Mesh:
        return std::make_unique<PhysXMeshShape>(this, pOwner, static_cast<PhysXMaterial*>(pMaterial));
    }

    return nullptr;
}

std::shared_ptr<leap::physics::IPhysicsMaterial> leap::physics::PhysXEngine::CreateMaterial()
{
    return std::make_shared<PhysXMaterial>(this);
}

void leap::physics::PhysXEngine::SetEnabledDebugDrawing(bool isEnabled)
{
    if(m_pScene) m_pScene->SetEnabledDebugDrawing(isEnabled);
    m_IsDebugDrawingEnabled = isEnabled;
}

std::vector<std::pair<glm::vec3, glm::vec3>> leap::physics::PhysXEngine::GetDebugDrawings()
{
    return m_pScene->GetDebugDrawings();
}

bool leap::physics::PhysXEngine::Raycast(const glm::vec3& start, const glm::vec3& direction, float distance, RaycastHit& hitInfo)
{
    return m_pScene->Raycast(start, direction, distance, hitInfo);
}

void leap::physics::PhysXEngine::OnSimulationEvent(const SimulationEvent& e)
{
    switch (e.type)
    {
    case SimulationEventType::OnCollisionEnter:
    {
        m_OnCollisionEnter.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    case SimulationEventType::OnCollisionStay:
    {
        m_OnCollisionStay.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    case SimulationEventType::OnCollissionExit:
    {
        m_OnCollisionExit.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    case SimulationEventType::OnTriggerEnter:
    {
        m_OnTriggerEnter.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    case SimulationEventType::OnTriggerStay:
    {
        m_OnTriggerStay.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    case SimulationEventType::OnTriggerExit:
    {
        m_OnTriggerExit.Notify(CollisionData{ e.pShape0->userData, e.pShape1->userData });
        break;
    }
    }
}

leap::physics::IPhysicsMaterial* leap::physics::PhysXEngine::GetDefaultMaterial()
{
    static std::unique_ptr<PhysXMaterial> pMaterial{};

    if (!pMaterial)
    {
        pMaterial = std::make_unique<PhysXMaterial>(this);
        
        constexpr float defaultFriction{ 0.6f };
        constexpr float defaultBounciness{ 0.0f };
        pMaterial->SetStaticFriction(defaultFriction);
        pMaterial->SetDynamicFriction(defaultFriction);
        pMaterial->SetBounciness(defaultBounciness);
    }

    return pMaterial.get();
}