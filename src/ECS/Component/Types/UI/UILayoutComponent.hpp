#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/NormalizedVec2.hpp"

namespace Engine::ECS { class EntityData; }
namespace Engine::UI
{
	enum class LayoutType : std::uint8_t
	{
		Vertical,
		Horizontal,
		Grid,
	};
	constexpr LayoutType DEFAULT_LAYOUT_TYPE = LayoutType::Vertical;

	enum class SizingType : std::uint8_t
	{
		/// <summary>
		/// Will ignore any sizing and the elements will be rendered based on the type with
		/// no scale factor applied in order to fit them. If they surpass 
		/// </summary>
		None,
		/// <summary>
		/// If the amount of elements in the layout can be EXPANDED to fit the total area
		/// an appropriate scale factor will be applied while maintaining proportions
		///  Even of the size can be shrunk, it will be left as default
		/// </summary>
		ExpandOnly,
		// <summary>
		/// If the amount of elements in the layout can be SHRUNK to fit the total area
		/// an appropriate scale factor will be applied while maintaining proportions. 
		/// Even of the size can be enlarged, it will be left as default
		/// </summary>
		ShrinkOnly,
		// <summary>
		/// If the amount of elements in the layout can be expanded to fit the total area
		/// OR shrunk an appropriate scale factor will be applied while maintaining proportions
		/// depending on if it needs to be shrunk/enlarged
		/// </summary>
		ExpandAndShrink,
	};
	constexpr SizingType DEFAULT_SIZING_TYPE = SizingType::None;

	class UITransformComponent;
	class UILayoutComponent : public Engine::ECS::Component
	{
	private:
		LayoutType m_type;
		SizingType m_sizingType;
		NormalizedVec2 m_spacing;
	public:

	private:
		void LayoutUpdate();
	public:
		UILayoutComponent(const LayoutType type = DEFAULT_LAYOUT_TYPE, const SizingType sizing = DEFAULT_SIZING_TYPE, const NormalizedVec2 spacing = {});

		void AddLayoutElement(ECS::EntityData& element);
		std::tuple<ECS::EntityData*, UITransformComponent*> CreateLayoutElement(const std::string& name);
		void RemoveLayoutElements(const size_t& childStartIndex, const size_t& count);
		Vec2 GetTotalSizeUsed() const;

		void Update(const float deltaTime);

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}


