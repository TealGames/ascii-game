#include "Core/Asset/Model3dAsset.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Utils/Platform/AssimpUtils.hpp"
#include "Utils/StringUtil.hpp"
#include "Math/PlatformMath.hpp"
#include "Core/Serialization/Model3DFileFormat.hpp"
#include "Utils/Debug.hpp"

static constexpr bool ADD_GLOBAL_SCALE = true;
//NOTE: Assimp by default assumes we want to convert m in modeling software
//coords to cm, so if we provide a global scale factor, that transform scale is ignored.
//NOTE: the following scale factor is in units RELATIVE to Blender UNITS (Blender is in meters)
static constexpr float IMPORT_TO_ENGINE_SCALE_FACTOR = 1.0f;
static constexpr bool BAKE_TRANSFORMS_IN_VERTICES = true;
//If true, will write all non-vtx formats to vtx to reduce file size
static constexpr bool WRITE_ANY_FORMAT_TO_CUSTOM = false;

static void ProcessSceneNode(Rendering::Model3d& model, const aiScene* modelScene, aiNode* node, const aiMatrix4x4* parentTransform)
{
	//NOTE: we do NOT need any conversion because Assimp converts models into +x -> right, +y ->up, -z -> forward, which match this engine coordinate system
	//BUT assimp also applies a scale factor of 100
	const aiMatrix4x4 globalTransform = parentTransform != nullptr ? *parentTransform * node->mTransformation : node->mTransformation;
	/*LogWarning(std::format("Found node transform : {}\nglobal transform:{}", 
		AssimpUtils::ToString(node->mTransformation), AssimpUtils::ToString(globalTransform)));*/
	/*LogWarning(std::format("Found {} global transform:{} parent:{} local:{}", node->mName.C_Str(), AssimpUtils::ToString(globalTransform),
		parentTransform == nullptr ? "NULL" : AssimpUtils::ToString(*parentTransform), AssimpUtils::ToString(node->mTransformation)));*/
	//LogWarning(std::format("For model found parent:{} transform:{}", parentTransform != nullptr? 
	//	AssimpUtils::ToString(*parentTransform) : "NULL", AssimpUtils::ToString(globalTransform)));
	if (node->mNumMeshes > 0)
	{
		Rendering::ModelObjectGroup* meshGroup = &(model.m_ObjectGroups.emplace_back(
			Rendering::ModelObjectGroup{ Mat4(&globalTransform.a1) }));
		//LogError(std::format("og global trans:{} stored:{}", AssimpUtils::ToString(globalTransform), meshGroup->m_GlobalTransform.ToString()));

		const aiMesh* currentImportMesh = nullptr;
		Rendering::ModelObject* currentEngineObj = nullptr;

		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			currentImportMesh = modelScene->mMeshes[node->mMeshes[i]];
			const size_t meshVertexCount = currentImportMesh->mNumVertices;

			currentEngineObj = &(model.m_Objects.emplace_back(Rendering::ModelObject{}));
			meshGroup->m_ObjectIndices.emplace_back(model.m_Objects.size() - 1);
			currentEngineObj->m_Mesh.m_Vertices.reserve(meshVertexCount);

			for (size_t j = 0; j < meshVertexCount; j++)
			{
				aiVector3D pos = currentImportMesh->mVertices[j];
				//TODO: do we really want global transform here or would it be okay if we had just the local?
				//if (BAKE_TRANSFORMS_IN_VERTICES) pos = globalTransform * pos;
				const aiVector3D normal = currentImportMesh->HasNormals() ? currentImportMesh->mNormals[j] : aiVector3D(0, 0, 0);
				const aiVector3D uv = currentImportMesh->HasTextureCoords(0) ? currentImportMesh->mTextureCoords[0][j] : aiVector3D(0, 0, 0);

				currentEngineObj->m_Mesh.m_Vertices.emplace_back(Rendering::Vertex{ WorldPosition3D(pos.x, pos.y, pos.z),
					UV(uv.x, uv.y), Vec3(normal.x, normal.y, normal.z) });
			}

			for (size_t j = 0; j < currentImportMesh->mNumFaces; j++)
			{
				const aiFace* face = &currentImportMesh->mFaces[j];
				if (face->mNumIndices != 3)
				{
					LogError(std::format("Attempted to read Assimp importer face with invalid face index count:{}", face->mNumIndices));
					continue;
				}
				currentEngineObj->m_Mesh.m_Indices.emplace_back(face->mIndices[0]);
				currentEngineObj->m_Mesh.m_Indices.emplace_back(face->mIndices[1]);
				currentEngineObj->m_Mesh.m_Indices.emplace_back(face->mIndices[2]);
			}
			
			currentEngineObj->m_Mesh.ConstructBLASTree(Rendering::BLAS_TREE_LEAF_COUNT);

			//TODO: also get roughness, normal map and albedo from the material
			aiMaterial* modelMaterial = modelScene->mMaterials[currentImportMesh->mMaterialIndex];
			aiColor4D baseColor;
			if (modelMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS 
				|| modelMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
			{
				currentEngineObj->m_Material.SetBaseColor(
					Color(baseColor.r, baseColor.g, baseColor.b, baseColor.a));
			}

			float metallic = 0;
			if (modelMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
			{
				currentEngineObj->m_Material.SetMetallic(metallic);
			}
			float roughness = 0;
			if (modelMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, metallic) == AI_SUCCESS)
			{
				currentEngineObj->m_Material.SetRoughness(metallic);
			}
		}
	}
	
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessSceneNode(model, modelScene, node->mChildren[i], &globalTransform);
	}
}

Model3dAsset::Model3dAsset(const std::filesystem::path& path) : Asset(path, false), m_model()
{
	const std::string fileExtension = Utils::StringUtil(path.extension().string()).ToLowerCase().ToString();
	if (path.extension().string() == VTXConverter::MODEL_3D_FILE_EXTENSION)
	{
		ReadModelAsCompressedFormat();
		return;
	}
	
	Assimp::Importer importer;
	if (ADD_GLOBAL_SCALE) importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, IMPORT_TO_ENGINE_SCALE_FACTOR);

	// -> Triangulate:
	// -> SmoothNormals: will make normals smoothyl transition in neighboring areas
	// -> GlobalScale: applies the global scale factor property for import
	// -> FlipWindingOrder: will reverse the indices of vertices for all triangles (index0, index1, index2) -> (index0, index2, index1)
	std::uint32_t importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipWindingOrder;
	if (ADD_GLOBAL_SCALE) importFlags |= aiProcess_GlobalScale;
	if (BAKE_TRANSFORMS_IN_VERTICES) importFlags |= aiProcess_PreTransformVertices;
	if (ENGINE_FORWARD_SIGN_Z == ZForwardSign::Negative) importFlags |= aiProcess_ConvertToLeftHanded;

	const aiScene* modelScene = importer.ReadFile(path.string(), importFlags);
	if (modelScene == nullptr || !modelScene->HasMeshes()) 
	{
		LogError(std::format("Tried to load 3d model at path: '{}' but could not find any meshes", path.string()));
		return;
	}

	m_model.m_Objects.reserve(modelScene->mNumMeshes);
	ProcessSceneNode(m_model, modelScene, modelScene->mRootNode, nullptr);
	if (path.stem() == "plane") LogWarning(std::format("created model: {}", m_model.ToString()));

	//If we write any format to vtx, then after the first import from a non-vtx format we write as compressed
	if (WRITE_ANY_FORMAT_TO_CUSTOM && fileExtension != VTXConverter::MODEL_3D_FILE_EXTENSION)
	{
		WriteModelAsCompressedFormat();
	}
}

void Model3dAsset::WriteModelAsCompressedFormat() const
{
	const std::filesystem::path newPath = GetAbsolutePathCopy().replace_extension(VTXConverter::MODEL_3D_FILE_EXTENSION);
	if (!VTXConverter::TryWriteModelToPath(m_model, newPath))
	{
		LogError(std::format("Attempted to WRITE model3d asset:{} to vtx format but failed", ToString()));
	}
}
void Model3dAsset::ReadModelAsCompressedFormat()
{
	const std::filesystem::path newPath = GetAbsolutePathCopy().replace_extension(VTXConverter::MODEL_3D_FILE_EXTENSION);
	if (!VTXConverter::TryReadModelFromPath(m_model, newPath))
	{
		LogError(std::format("Attempted to READ model3d asset:{} from vtx format but failed", ToString()));
	}
}

const Rendering::Model3d& Model3dAsset::GetModel() const { return m_model; }
Rendering::Model3d& Model3dAsset::GetModelMutable() { return m_model; }
void Model3dAsset::UpdateAssetFromFile()
{
	//TODO: implement
}

bool HasModel3dExtension(const std::string& extension)
{
	return extension == ".fbx" || extension == VTXConverter::MODEL_3D_FILE_EXTENSION;
}