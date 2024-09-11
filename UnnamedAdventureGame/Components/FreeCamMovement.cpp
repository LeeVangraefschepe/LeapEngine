#include "FreeCamMovement.h"
#include "InputManager.h"
#include "Keyboard.h"
#include "LambdaCommand.h"
#include "Mouse.h"
#include "Components/Transform/Transform.h"
#include "GameContext/GameContext.h"
#include "GameContext/Timer.h"
#include "PhysX/PhysXObject.h"
#include "SceneGraph/GameObject.h"

void unag::FreeCamMovement::Awake()
{
	const auto newParent = GetGameObject()->GetParent()->CreateChild("Camera");
	newParent->GetTransform()->SetWorldPosition(GetGameObject()->GetTransform()->GetWorldPosition());
	GetGameObject()->SetParent(newParent);
}

void unag::FreeCamMovement::Start()
{
	const auto& input = leap::input::InputManager::GetInstance();
	const auto keyboard = input.GetKeyboard();
	const auto mouse = input.GetMouse();

	// Link keyboard input
	auto command = std::make_unique<leap::LambdaCommand>([this]()
		{
			GetGameObject()->GetParent()->GetTransform()->Translate(-GetTransform()->GetRight() * m_Speed * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
		});
	keyboard->AddCommand(command.get(), leap::input::InputManager::InputType::EventRepeat, leap::input::Keyboard::Key::KeyA);
	m_Commands.push_back(std::move(command));

	command = std::make_unique<leap::LambdaCommand>([this]()
		{
			GetGameObject()->GetParent()->GetTransform()->Translate(GetTransform()->GetRight() * m_Speed * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
		});
	keyboard->AddCommand(command.get(), leap::input::InputManager::InputType::EventRepeat, leap::input::Keyboard::Key::KeyD);
	m_Commands.push_back(std::move(command));

	command = std::make_unique<leap::LambdaCommand>([this]()
		{
			GetGameObject()->GetParent()->GetTransform()->Translate(GetTransform()->GetForward() * m_Speed * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
		});
	keyboard->AddCommand(command.get(), leap::input::InputManager::InputType::EventRepeat, leap::input::Keyboard::Key::KeyW);
	m_Commands.push_back(std::move(command));

	command = std::make_unique<leap::LambdaCommand>([this]()
		{
			GetGameObject()->GetParent()->GetTransform()->Translate(-GetTransform()->GetForward() * m_Speed * leap::GameContext::GetInstance().GetTimer()->GetDeltaTime());
		});
	keyboard->AddCommand(command.get(), leap::input::InputManager::InputType::EventRepeat, leap::input::Keyboard::Key::KeyS);
	m_Commands.push_back(std::move(command));

	// Link mouse input
	command = std::make_unique<leap::LambdaCommand>([this]()
		{
			const auto mouseDelta = static_cast<glm::vec2>(leap::input::InputManager::GetInstance().GetMouse()->GetDelta());
			const auto deltaTime = leap::GameContext::GetInstance().GetTimer()->GetDeltaTime();
			constexpr float mouseSpeed = 15.f;

			GetTransform()->Rotate(mouseDelta.y * deltaTime * mouseSpeed, 0.0f, 0.0f);
			GetGameObject()->GetParent()->GetTransform()->Rotate(0.0f, mouseDelta.x * deltaTime * mouseSpeed, 0.0f);
		});
	mouse->AddCommand(command.get(), leap::input::InputManager::InputType::EventRepeat, leap::input::Mouse::Button::LeftButton);
	m_Commands.push_back(std::move(command)); 
}

void unag::FreeCamMovement::OnDestroy()
{
	const auto& input = leap::input::InputManager::GetInstance();
	for (auto& command : m_Commands)
	{
		input.GetKeyboard()->RemoveCommand(command.get());
		input.GetMouse()->RemoveCommand(command.get());
	}
}

#include "ImGui/imgui.h"
void unag::FreeCamMovement::OnGUI()
{
	ImGui::SetNextWindowSize(ImVec2(250, 100));
	ImGui::Begin("PhysX offset");
	ImGui::InputFloat3("Offset", &leap::physics::PhysXObject::OFFSET.x);
	ImGui::End();
}
