#include "pch.hpp"
#include <filesystem>
#include <fstream>
#include "raylib.h"
#include "Point2DInt.hpp"
#include "Scene.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "Globals.hpp"
#include "Array2DPosition.hpp"
#include "CartesianPosition.hpp"
#include "VisualData.hpp"
#include "PhysicsBodyData.hpp"
#include "TransformData.hpp"

const std::string Scene::m_SCENE_FILE_PREFIX = "scene_";

Scene::Scene(const std::filesystem::path& scenePath, GlobalEntityManager& globalEntities) :
	m_Layers{}, m_sceneName(""),
	m_SceneName(m_sceneName), m_ScenePath(scenePath),
	m_localEntities(), //m_globalEntityLookup(globalEntities),
	m_currentFrameDirtyComponents(0), m_entityMapper(),
	m_mainCamera(nullptr), 
	m_globalEntities(globalEntities), m_GlobalEntities(m_globalEntities),
	m_PhysicsWorld()
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

	m_localEntities.reserve(MAX_ENTITIES);
	m_localEntityLookup.reserve(MAX_ENTITIES);
	m_Layers.emplace(RenderLayerType::Background, RenderLayer{});
	m_Layers.emplace(RenderLayerType::Player, RenderLayer{});

	//Utils::Log("Creating new layer in scene");
	const VisualData backgroundVisual = VisualData(layerText, GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE,
										VisualData::DEFAULT_CHAR_SPACING, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::PIVOT_CENTER);
	ECS::Entity& backgroundEntity = CreateEntity("Background", TransformData({ 0,0 }));
	EntityRendererData& backgroundRenderer= backgroundEntity.AddComponent<EntityRendererData>(EntityRendererData(backgroundVisual, RenderLayerType::Background));
	Utils::LogWarning(std::format("Created Backgorund: {}", backgroundRenderer.GetVisualData().m_Text.ToString()));
	Utils::LogWarning(std::format("Creating backgrounf entity: {} from rednerer: {}", backgroundEntity.m_Name, backgroundRenderer.m_Entity->m_Name));

	PhysicsBodyData& physicsBody= backgroundEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(backgroundVisual.GetWorldSize(), {0,0}));
	Utils::LogWarning(std::format("Created Physics body: {} visual size: {}", 
		physicsBody.GetAABB().ToString(backgroundEntity.m_Transform.m_Pos), backgroundVisual.m_Text.GetSize().ToString()));

	//Utils::Log(std::format("New layer w: {} h: {}", std::to_string(newLayerW), std::to_string(newLayerH)));
	//TODO: right now these are global constants, but might have to be later parsed from data
	//for each scene, allowing each scene to have its own settings

	//TODO: right now we only have one layer, but later on we should add multiple
	//Note: right now we make two layers one for background and one for player, but this should
	//get abstracted more with ids and text file parsing of scene data
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
			std::size_t equalsSignIndex = currentLine.find('=');
			std::string colorAlias = currentLine.substr(0, equalsSignIndex);
			std::string hexString = currentLine.substr(equalsSignIndex + 1);
			std::optional<uint32_t> maybeConvertedHex = Utils::TryParseHex<uint32_t>(hexString);
			if (!Utils::Assert(maybeConvertedHex.has_value(), std::format("Tried to parse scene data: {}, but encountered "
				"unparsable hex: '{}' at line: {}", m_SceneName, hexString, std::to_string(lineIndex)))) continue;

			Color convertedColor = RaylibUtils::GetColorFromHex(maybeConvertedHex.value());
			/*Utils::Log(std::format("Found the color: {} from hex: {}", RaylibUtils::ToString(convertedColor), hexString));*/
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

					//Utils::Log(std::format("Found good color alias: {}", colorAlias));
					currentColor = colorAliases[colorAlias];
					i = colorAliasEndIndex;
					continue;
				}

				//TODO: what is the best way of doing this? putting in text chars and putting empty chars 
				//which would work fine for init but hard to create collision bound 
				//OR do we leave empty spots and put them in with positions?
				layerText.back().push_back(TextCharPosition{ Array2DPosition(r, i), TextChar(currentColor, currentLine[i])});
			}
			r++;
		}
	}
}

void Scene::InitScene()
{

	for (auto& entity : m_localEntities)
	{
		if (PhysicsBodyData* maybeBody = entity.TryGetComponent<PhysicsBodyData>())
		{
			m_PhysicsWorld.AddBody(*maybeBody);
		}
	}

	for (auto& entity : m_globalEntities.GetAllGlobalEntitiesMutable())
	{
		if (PhysicsBodyData* maybeBody = entity.TryGetComponent<PhysicsBodyData>())
		{
			m_PhysicsWorld.AddBody(*maybeBody);
		}
	}
}

std::vector<RenderLayer*> Scene::GetLayersMutable()
{
	if (m_Layers.empty()) return {};

	std::vector<RenderLayer*> layers = {};
	for (auto& layer : m_Layers) layers.push_back(&(layer.second));
	return layers;
}

std::vector<const RenderLayer*> Scene::GetLayers(const RenderLayerType& renderLayers) const
{
	if (m_Layers.empty()) return {};

	std::vector<const RenderLayer*> layers = {};
	for (const auto& layer : m_Layers)
	{
		if ((layer.first & renderLayers)!=0) layers.push_back(&(layer.second));
	}
	return layers;
}

std::vector<const RenderLayer*> Scene::GetAllLayers() const
{
	if (m_Layers.empty()) return {};

	std::vector<const RenderLayer*> layers = {};
	for (const auto& layer : m_Layers)
	{
		layers.push_back(&(layer.second));
	}
	return layers;
}

//std::vector<TextBuffer*> Scene::GetTextBuffersMutable(const RenderLayerType& renderLayers)
//{
//	if (m_Layers.empty()) return {};
//	//TODO: we could optimize this with the all choosen case by checking maxLayers & renderLayers == maxLayers
//
//	std::vector<TextBuffer*> buffers = {};
//	for (auto& layer : m_Layers)
//	{
//		if ((layer.first & renderLayers) == 0) continue;
//		buffers.push_back(&(layer.second.m_SquaredTextBuffer));
//	}
//	return buffers;
//}
//
//void Scene::SetLayers(const RenderLayerType& renderLayers, const std::vector<TextCharPosition>& positions)
//{
//	if (renderLayers == RenderLayerType::None) return;
//	if (positions.empty()) return;
//
//	for (auto& layer : m_Layers)
//	{
//		if ((layer.first & renderLayers) == 0) continue;
//		layer.second.m_SquaredTextBuffer.SetAt(positions);
//	}
//}
//
//void Scene::SetLayers(const RenderLayerType& renderLayers, const std::vector<ColorPosition>& positions)
//{
//	if (renderLayers == RenderLayerType::None) return;
//	if (positions.empty()) return;
//
//	for (auto& layer : m_Layers)
//	{
//		if ((layer.first & renderLayers) == 0) continue;
//		layer.second.m_SquaredTextBuffer.SetAt(positions);
//	}
//}

void Scene::AddToLayer(const RenderLayerType& layers, TextBufferPosition& textBufferPos)
{
	if (layers == RenderLayerType::None) return;

	for (auto& layer : m_Layers)
	{
		if ((layer.first & layers) == 0) continue;
		layer.second.AddText(textBufferPos);
		return;
	}
}

std::vector<TextBufferMixed*> Scene::GetLayerBufferMutable(const RenderLayerType& renderLayers)
{
	std::vector<TextBufferMixed*> buffers = {};
	if (renderLayers == RenderLayerType::None) return buffers;

	for (auto& layer : m_Layers)
	{
		if ((layer.first & renderLayers) == 0) continue;
		buffers.push_back(&layer.second.GetBufferMutable());
	}
	return buffers;
}

void Scene::ResetAllLayers()
{
	for (auto& layer : m_Layers) layer.second.ResetToDefault();
}

EntityCollection::iterator Scene::GetLocalEntityIterator(const ECS::EntityID& id)
{
	return m_localEntityLookup.find(id);
}

//EntityCollection::iterator Scene::GetEntityIterator(const EntityID& id)
//{
//	auto iterator = GetLocalEntityIterator(id);
//	if (iterator != m_localEntityLookup.end()) return iterator;
//	
//	return m_globalEntities.GetGlobalEntityIteratorMutable(id);
//}

class CameraData;
void Scene::SetMainCamera(ECS::Entity& cameraEntity)
{
	if (!Utils::Assert(cameraEntity.HasComponent<CameraData>(), 
		std::format("Tried to set the non-camera entity: {} as the main camera for scene: {}",
			cameraEntity.m_Name, m_SceneName))) return;

	m_mainCamera = &cameraEntity;
}

bool Scene::HasMainCamera() const
{
	return m_mainCamera != nullptr;
}

ECS::Entity* Scene::TryGetMainCameraEntity()
{
	return m_mainCamera;
}

CameraData* Scene::TryGetMainCameraData()
{
	if (!HasMainCamera()) return nullptr;
	return m_mainCamera->TryGetComponent<CameraData>();
}

int Scene::GetEntityCount() const
{
	return m_localEntities.size() + m_globalEntities.GetCount();
	/*return m_entityMapper.va + m_globalEntities.GetCount();*/
}

bool Scene::HasEntities() const
{
	return GetEntityCount() > 0;
}

ECS::Entity& Scene::CreateEntity(const std::string& name, TransformData& transform)
{
	m_localEntities.emplace_back(name, m_entityMapper, transform);
	m_localEntityLookup.emplace(m_localEntities.back().m_Id, &(m_localEntities.back()));
	m_localEntities.back().m_Transform.m_Entity = &(m_localEntities.back());

	return m_localEntities.back();
}

ECS::Entity& Scene::CreateEntity(const std::string& name, TransformData&& transform)
{
	m_localEntities.emplace_back(name, m_entityMapper, std::move(transform));
	m_localEntityLookup.emplace(m_localEntities.back().m_Id, &(m_localEntities.back()));
	m_localEntities.back().m_Transform.m_Entity = &(m_localEntities.back());

	return m_localEntities.back();
}

bool Scene::HasEntity(const ECS::EntityID& id)
{
	return m_entityMapper.valid(id) || m_globalEntities.HasGlobalEntity(id);
	/*bool isLocal = GetLocalEntityIterator(id) != m_localEntityLookup.end();
	if (isLocal) return true;

	return m_globalEntities.HasGlobalEntity(id);*/
}

//TODO: while this is fine, we should add version that allow us to add components to the entity
//and would probably be best if we create them on the heap probably to extend memory lifetime

//TODO: also maybe consider managing what objects are required to be in a scene, like a camera
ECS::Entity* Scene::TryGetEntity(const ECS::EntityID& id)
{
	auto localIt = GetLocalEntityIterator(id);
	if (localIt != m_localEntityLookup.end()) return localIt->second;

	auto globalIt = m_globalEntities.GetGlobalEntityIteratorMutable(id);
	if (m_globalEntities.IsValidIterator(globalIt)) return globalIt->second;

	return nullptr;
}

std::string Scene::ToStringLayers() const
{
	//Utils::Log(std::format("Began to stirng alyers fro scene. first layer: {}", m_Layers[0].m_SquaredTextBuffer.GetSize().ToString()));
	std::string result = "\n" + m_SceneName + ":\n";

	for (const auto& layer : m_Layers)
	{
		result += "\nLAYER: ";
		Utils::Log(std::format("Display all scene layers at layer: {}", layer.second.ToString()));
		result += layer.second.ToString();
	}
	return result;
}

std::string Scene::ToStringEntityData() const
{
	return "NULL";
	/*return std::format("[Global: {} Local: {}]", 
		m_globalEntities.ToStringEntityData(), m_entityMapper.ToStringData());*/
}

void Scene::ResetFrameDirtyComponentCount()
{
	m_currentFrameDirtyComponents = 0;
}

void Scene::IncreaseFrameDirtyComponentCount()
{
	m_currentFrameDirtyComponents++;
}

int Scene::GetDirtyComponentCount() const
{
	return m_currentFrameDirtyComponents;
}

bool Scene::HasDirtyComponents() const
{
	return GetDirtyComponentCount() > 0;
}

const Physics::PhysicsWorld& Scene::GetPhysicsWorld() const
{
	return m_PhysicsWorld;
}

Physics::PhysicsWorld& Scene::GetPhysicsWorldMutable()
{
	return m_PhysicsWorld;
}
