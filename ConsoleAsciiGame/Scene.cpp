#include <vector>
#include <string>
#include <filesystem>
#include <optional>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include "raylib.h"
#include "Point2DInt.hpp"
#include "Scene.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "StringUtil.hpp"
#include "Globals.hpp"

const std::string Scene::m_SCENE_FILE_PREFIX = "scene_";

Scene::Scene(const std::filesystem::path& scenePath) :
	m_Layers{}, m_sceneName(""),
	m_SceneName(m_sceneName), m_ScenePath(scenePath),
	m_entities{}
{
	if (!Utils::Assert(std::filesystem::exists(scenePath), std::format("Tried to create a scene at path: {} "
		"but that path does not exist", scenePath.string()))) 
		return;

	if (!Utils::Assert(scenePath.has_filename(), std::format("Tried to create a scene at path: {} "
		"but that path does not lead to a file", scenePath.string()))) 
		return;

	std::vector<std::vector<TextCharPosition>> layerText = {};

	m_sceneName = scenePath.stem().string().substr(m_SCENE_FILE_PREFIX.size() - 1);
	std::replace(m_sceneName.begin(), m_sceneName.end(), '_', ' ');
	std::ifstream fstream(scenePath);
	
	/*if (!Utils::Assert(!!fstream, std::format("Tried to create scene at: {} "
		"but it could not open file at that path", scenePath.string())))
	{
		std::cout << "RETURN" << std::endl;
		return;
	}*/

	ParseSceneFile(fstream, layerText);

	//Utils::Log("Creating new layer in scene");
	const RenderLayer newLayer = RenderLayer(layerText, TEXT_BUFFER_FONT, CHAR_SPACING);
	int newLayerW = newLayer.m_SquaredTextBuffer.GetWidth();
	int newLayerH = newLayer.m_SquaredTextBuffer.GetHeight();
	//TODO: right now these are global constants, but might have to be later parsed from data
	//for each scene, allowing each scene to have its own settings

	//TODO: right now we only have one layer, but later on we should add multiple
	//Note: right now we make two layers one for background and one for player, but this should
	//get abstracted more with ids and text file parsing of scene data
	m_Layers.push_back(newLayer);

	const RenderLayer playerLayer = RenderLayer(TextBuffer{ newLayerW, newLayerH, TextChar()}, TEXT_BUFFER_FONT, CHAR_SPACING);
	m_Layers.push_back(playerLayer);
}

void Scene::ParseSceneFile(std::ifstream& fstream, 
	std::vector<std::vector<TextCharPosition>>& layerText) const
{
	int r = 0;
	std::string currentLine = "";
	std::unordered_map<std::string, Color> colorAliases = {};
	const std::string keyHeader = "key:";
	const std::string sceneHeader = "scene:";
	const char charColorAliasStart = '[';
	const char charColorAliasEnd = ']';
	bool isParsingKey = false;

	int lineIndex = -1;
	const Color defaultColor = BLACK;
	Color currentColor = defaultColor;
	while (std::getline(fstream, currentLine))
	{
		lineIndex++;
		if (currentLine.empty()) continue;

		if (currentLine == keyHeader) isParsingKey = true;
		else if (currentLine == sceneHeader) isParsingKey = false;

		else if (isParsingKey)
		{
			int equalsSignIndex = currentLine.find('=');
			std::string colorAlias = currentLine.substr(0, equalsSignIndex);
			std::string hexString = currentLine.substr(equalsSignIndex + 1);
			std::optional<uint32_t> maybeConvertedHex = Utils::TryParseHex<uint32_t>(hexString);
			if (!Utils::Assert(maybeConvertedHex.has_value(), std::format("Tried to parse scene data: {}, but encountered "
				"unparsable hex: '{}' at line: {}", m_SceneName, hexString, std::to_string(lineIndex)))) continue;

			Color convertedColor = RaylibUtils::GetColorFromHex(maybeConvertedHex.value());
			Utils::Log(std::format("Found the color: {} from hex: {}", RaylibUtils::ToString(convertedColor), hexString));
			colorAliases.emplace(colorAlias, convertedColor);
		}
		else
		{
			layerText.push_back({});
			//std::cout << "ALLOC with line: "<<currentLine<< std::endl;
			//if (currentLine.size() > maxLineChars) maxLineChars = currentLine.size();

			for (int i = 0; i < currentLine.size(); i++)
			{
				if (currentLine[i] == '\t' || currentLine[i]==' ') continue;
				//We need to make sure there is at least 2 chars in front for at least one for alias and one for ending symbol
				if (currentLine[i] == charColorAliasStart && i< currentLine.size()-2)
				{
					int colorAliasEndIndex = currentLine.find(charColorAliasEnd, i + 1);
					if (!Utils::Assert(colorAliasEndIndex != std::string::npos, std::format("Tried to parse a color alias for scene data: {} at line: {} "
						"but did not find color alias end at color alias start at index: {}", 
						m_SceneName, std::to_string(lineIndex), std::to_string(i)))) continue;

					std::string colorAlias = currentLine.substr(i + 1, colorAliasEndIndex - (i + 1));
					if (!Utils::Assert(colorAliases.find(colorAlias) != colorAliases.end(), std::format("Tried to parse a color alias for scene data: {} at line: {} "
						"but color alias: {} starting at index:{} has no color data defined in KEY section",
						m_SceneName, std::to_string(lineIndex), colorAlias, std::to_string(i + 1))))
					{
						i = colorAliasEndIndex;
						continue;
					}

					Utils::Log(std::format("Found good color alias: {}", colorAlias));
					currentColor = colorAliases[colorAlias];
					i = colorAliasEndIndex;
					continue;
				}

				//TODO: what is the best way of doing this? putting in text chars and putting empty chars 
				//which would work fine for init but hard to create collision bound 
				//OR do we leave empty spots and put them in with positions?
				Utils::Log(std::format("Added char: {} with color: {}", Utils::ToString(currentLine[i]), RaylibUtils::ToString(currentColor)));
				layerText.back().push_back(TextCharPosition{ Utils::Point2DInt(r, i), TextChar(currentColor, currentLine[i])});
			}
			r++;
		}
	}
}

void Scene::SortEntitiesByUpdatePriority()
{
	if (m_entities.empty()) return;

	std::sort(m_entities.begin(), m_entities.end(),
		//more priority=> means goes sooner
		[](const ECS::Entity* firstEn, const ECS::Entity* secndEnd) -> bool
		{
			return firstEn->GetUpdatePriority() > secndEnd->GetUpdatePriority();
		});
}

std::vector<RenderLayer*> Scene::GetLayersMutable()
{
	if (m_Layers.empty()) return {};

	std::vector<RenderLayer*> layers = {};
	for (auto& layer : m_Layers) layers.push_back(&layer);
	return layers;
}

std::vector<const RenderLayer*> Scene::GetLayers() const
{
	if (m_Layers.empty()) return {};

	std::vector<const RenderLayer*> layers = {};
	for (const auto& layer : m_Layers) layers.push_back(&layer);
	return layers;
}

//TODO: while this is fine, we should add version that allow us to add components to the entity
//and would probably be best if we create them on the heap probably to extend memory lifetime

//TODO: also maybe consider managing what objects are required to be in a scene, like a camera

//TODO: ideally, each entity should live on its own render layer rather than the whole scene
void Scene::AddEntity(ECS::Entity& entity)
{
	m_entities.push_back(&entity);
	SortEntitiesByUpdatePriority();
}

void Scene::Start()
{
	if (m_entities.empty()) return;
	for (auto& entity : m_entities) entity->Start();
}

void Scene::UpdateStart(float deltaTime)
{
	//We need to reset to default since previous changes were baked into the buffer
	//so we need to clear it for a fresh update
	for (int i=0; i<m_Layers.size(); i++)
	{
		//Utils::Log(std::format("Resetting to  default layer: {}/{}", std::to_string(i), std::to_string(m_Layers.size()-1)));
		m_Layers[i].ResetToDefault();
	}
	
	if (m_entities.empty()) return;
	for (auto& entity : m_entities)
	{
		//Utils::Log("Calling udpate on entity: " + entity->m_Name);
		entity->UpdateStart(deltaTime);
	}
}

void Scene::UpdateEnd(float deltaTime)
{
	if (m_entities.empty()) return;
	for (auto& entity : m_entities) entity->UpdateEnd(deltaTime);
}

std::string Scene::ToStringLayers() const
{
	//Utils::Log(std::format("Began to stirng alyers fro scene. first layer: {}", m_Layers[0].m_SquaredTextBuffer.GetSize().ToString()));
	std::string result = "\n" + m_SceneName + ":\n";

	for (const auto& layer : m_Layers)
	{
		result += "\nLAYER: ";
		Utils::Log(std::format("Display all scene layers at layer: {}", layer.ToString()));
		result += layer.ToString();
	}
	return result;
}
