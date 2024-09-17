#include <Windows.h>
#include "Leap.h"
#include "GameContext/GameContext.h"
#include "GameContext/Logger/ConsoleLogger.h"
#include "GameContext/Logger/NetworkLogger.h"
#include "SceneGraph/SceneManager.h"
#include <ServiceLocator/ServiceLocator.h>
#include <Interfaces/IPhysics.h>

#include "Scenes/MainScene.h"
#include "Scenes/ExtendedCameraTest.h"
#include "Scenes/ParticleTest.h"

#if _DEBUG
#include <vld.h>
#endif

int main()
{
	leap::GameContext::GetInstance().AddLogger<leap::ConsoleLogger>();
	leap::GameContext::GetInstance().AddLogger<leap::NetworkLogger>();

	leap::LeapEngine engine{ 1280, 720, "Leap game engine" };

	auto afterInitializing = []()
		{
			leap::ServiceLocator::GetPhysics().SetEnabledDebugDrawing(true);
			leap::SceneManager::GetInstance().AddScene("Main scene", unag::MainScene::Load);
			leap::SceneManager::GetInstance().AddScene("Extended camera test scene", unag::ExtendedCameraTest::Load);
			leap::SceneManager::GetInstance().AddScene("Particles test scene", unag::ParticleTest::Load);
			//leap::GameContext::GetInstance().GetWindow()->SetIcon("Data/Example.png");
		};

	engine.Run(afterInitializing, 60);

	return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return main();
}