#include "Core/Asset/ShaderAsset.hpp"
#include "Utils/IOHandler.hpp"
#include "Core/Asset/AssetManager.hpp"
#include <string_view>
#include <optional>

static constexpr char const* FRAGMENT_SHADER_IDENTIFIER = "fragment";
static constexpr char const* VERTEX_SHADER_IDENTIFIER = "vertex";

ShaderAsset::ShaderAsset(const std::filesystem::path& path)
	: Asset(path, false), m_shader(Rendering::CreateShader("", "")) 
{
	const std::string pathStr = path.string();
	std::string_view pathView = std::string_view(pathStr);
	if (!Assert(IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a shader asset from path:{}"
		"but it does not have required shader extension:'{}'", pathStr, EXTENSION)))
		return;
	
	const size_t lastSeparatorIndex = pathStr.find(Asset::WORD_SEPARATOR);
	//If name has no separator it means it has no sahder identifier -> try to read as single file
	if (lastSeparatorIndex == std::string::npos)
	{
		ReadShaderFromSingleFile();
		return;
	}

	const std::string_view shaderTypeName = pathView.substr(lastSeparatorIndex + 1);
	const std::string_view shaderName = pathView.substr(0, lastSeparatorIndex);

	Rendering::ShaderType type = Rendering::ShaderType::Vertex;
	std::string otherShaderName = std::string(shaderName) + Asset::WORD_SEPARATOR;
	if (shaderTypeName == VERTEX_SHADER_IDENTIFIER)
	{
		otherShaderName += FRAGMENT_SHADER_IDENTIFIER;
	}
	else if (shaderTypeName == FRAGMENT_SHADER_IDENTIFIER)
	{
		type = Rendering::ShaderType::Fragment;
		otherShaderName += VERTEX_SHADER_IDENTIFIER;
	}
	//If the last segment can not be found as 
	// valid shader name -> it could be just a multi-word single file shader
	else
	{
		ReadShaderFromSingleFile();
		return;
	}

	//THIS POINT MEANS THERE ARE MULTIPLE FILES FOR THIS SHADER
	const std::filesystem::path otherShaderPath = GetPath().parent_path() / (otherShaderName + path.extension());
	//If we have split up shaders, we make the other asset path invalid for the assetmanager to set as asset so we do not
	//have two assets with the same shader data as initialized with the opposing shader type
	AssetManagement::AssetManager::SetAssetHiddenStatus(otherShaderPath, true);
	//We only want the non-shader type part of the name to be set as real asset name
	OverrideAssetName(shaderName);

	if (type == Rendering::ShaderType::Vertex) m_shader.SetSources(IO::TryReadFileFull(GetPath()), IO::TryReadFileFull(otherShaderPath));
	else if (type == Rendering::ShaderType::Fragment) m_shader.SetSources(IO::TryReadFileFull(otherShaderPath), IO::TryReadFileFull(GetPath()));
	else
	{
		LogError(std::format("Attempted to set shader asset at path:{} source code from multiple files, "
			"but current shader type could not get resolved:{}", pathStr, ::ToString(type)));
	}
}

void ShaderAsset::ReadShaderFromSingleFile() 
{
	std::optional<Rendering::ShaderType> maybeShaderMode = std::nullopt;
	//Index 0-> vertex, index 1-> fragment
	std::string shaderSource[2] = {};

	IO::TryExecuteOnFileByLine(GetPath(), 
		[this](std::string* line)-> void 
		{
			if (line->empty()) return;

			if (*line == '#' + FRAGMENT_SHADER_IDENTIFIER)
			{
				maybeShaderMode = Rendering::ShaderType::Fragment;
				return;
			}
			else if (*line == '#' + VERTEX_SHADER_IDENTIFIER)
			{
				maybeShaderMode = Rendering::ShaderType::Vertex;
				return;
			}

			if (!maybeShaderMode.has_value()) return;
			shaderSource[(ShaderTypeIntegralType)maybeShaderMode.value()] += *line + "\n";
		});

	if (shaderSource[0].empty() || shaderSource[1].empty())
	{
		LogError(std::format("Tried to read sahder from single file at path:{} but some shader type was not found. "
			"Vertex Found:{} Fragment found:{}", GetPath().c_str(), !shaderSource[0].empty(), !shaderSource[1].empty()));
		return;
	}
	m_shader.SetSources(std::move(shaderSource[0]), std::move(shaderSource[1]));
}

const Rendering::Shader& ShaderAsset::GetShader() const 
{
	return m_shader;
}
void ShaderAsset::UpdateAssetFromFile() 
{

}