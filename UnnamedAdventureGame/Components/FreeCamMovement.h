#pragma once
#include "Components/Component.h"
#include <vector>
#include <memory>
#include "Command.h"

namespace unag
{
	class FreeCamMovement final : public leap::Component
	{
	public:
		FreeCamMovement() = default;
		virtual ~FreeCamMovement() = default;
		FreeCamMovement(const FreeCamMovement& other) = delete;
		FreeCamMovement(FreeCamMovement&& other) = delete;
		FreeCamMovement& operator=(const FreeCamMovement& other) = delete;
		FreeCamMovement& operator=(FreeCamMovement&& other) = delete;

		void SetSpeed(float speed) { m_Speed = speed; }

	private:
		virtual void Awake() override;
		virtual void Start() override;
		virtual void OnDestroy() override;
		virtual void OnGUI() override;

		float m_Speed{1.f};
		std::vector <std::unique_ptr<leap::Command>> m_Commands{};
	};
}