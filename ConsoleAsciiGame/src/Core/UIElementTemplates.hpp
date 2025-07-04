#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>

namespace AssetManagement { class AssetManager; }
namespace Templates
{
	class EntityTemplateManager
	{
	private:
		//std::unordered_map<std::string, std::function<
	public:

	private:
	public:
		EntityTemplateManager();
	};
}

class EntityData;
class UITransformData;
class UIToggleComponent;
namespace Templates
{
	void Init(AssetManagement::AssetManager& assetManager);
	std::tuple<EntityData*, UITransformData*, UIToggleComponent*> CreateCheckboxTemplate(EntityData& parent, const std::string& name);
}