#include "Button.h"

#include "../../../SceneGraph/GameObject.h"

#include "CanvasActions.h"
#include "Image.h"
#include "RectTransform.h"

glm::vec2 leap::Button::GetSize()
{
	return m_pTransform->GetSize();
}

glm::vec3 leap::Button::GetPosition()
{
	return m_pTransform->GetPosition();
}

glm::vec2 leap::Button::GetPivot()
{
	if (!m_pVisual)
	{
		// If no visuals is found on this component, return a centered pivot
		return { 0.5f, 0.5f };
	}

	return m_pVisual->GetPivot();
}

void leap::Button::SetHoverColor(const glm::vec4& color)
{
	m_HoverColor = color;
}

void leap::Button::SetClickColor(const glm::vec4& color)
{
	m_ClickColor = color;
}

void leap::Button::SetVisuals(Image* pVisual)
{
	m_pVisual = pVisual;
}

void leap::Button::Awake()
{
	m_pTransform = GetGameObject()->GetComponent<RectTransform>();
	if (m_pTransform == nullptr)
	{
		// Log error that Button component parent object doesn't have a RectTransform component
		return;
	}

	CanvasActions* pCanvas{ GetGameObject()->GetComponentInParent<CanvasActions>() };
	if (pCanvas == nullptr)
	{
		// Log warning that canvas doens't have a CanvasActions component
		return;
	}

	pCanvas->Add(this);

	m_pVisual = GetGameObject()->GetComponent<Image>();
	if (m_pVisual)
	{
		m_OriginalColor = m_pVisual->GetColor();
	}
}

void leap::Button::OnDestroy()
{
	CanvasActions* pCanvas{ GetGameObject()->GetComponentInParent<CanvasActions>() };
	if (pCanvas != nullptr) return;

	pCanvas->Remove(this);
}

void leap::Button::OnClickStart()
{
	m_pVisual->SetColor(m_OriginalColor * m_ClickColor);
}

void leap::Button::OnClickEnd()
{
	OnClicked.Notify(*this);

	if (!m_pVisual) return;

	if (m_IsHovering) OnHoverEnter();
	else OnHoverExit();
}

void leap::Button::OnHoverEnter()
{
	m_IsHovering = true;

	if (!m_pVisual) return;

	m_pVisual->SetColor(m_OriginalColor * m_HoverColor);
}

void leap::Button::OnHoverExit()
{
	m_IsHovering = false;

	if (!m_pVisual) return;

	m_pVisual->SetColor(m_OriginalColor);
}
