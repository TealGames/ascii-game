#include "pch.hpp"
#include "ECS/Component/Types/Editor/ColorGradientEditorComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::Editor::UI
{
	ColorGradientEditorComponent::ColorGradientEditorComponent() : ColorGradientEditorComponent(ColHDRGradient{}) {}
	ColorGradientEditorComponent::ColorGradientEditorComponent(const ColHDRGradient& gradient) : m_gradient(gradient) {}

	void ColorGradientEditorComponent::SetGradient(const ColHDRGradient& gradient)
	{
		m_gradient = gradient;
	}
	const ColHDRGradient& ColorGradientEditorComponent::GetGradient() const
	{
		return m_gradient;
	}

	void ColorGradientEditorComponent::Update(const float deltaTime)
	{
	}
	void ColorGradientEditorComponent::InitFields()
	{
		m_Fields = {};
	}
	void ColorGradientEditorComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void ColorGradientEditorComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	//RenderInfo ColorGradientEditorComponent::ElementRender(const RenderInfo& renderInfo)
	//{
	//	//TODO: finish implementing
	//	Color startColor = m_gradient.GetFirstColor(false);
	//	Color endColor = m_gradient.GetLastColor(false);
	//	DrawRectangleGradientH(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_X, 
	//		renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, startColor, endColor);
	//
	//	return {};
	//}
}
