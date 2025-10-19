#include "Core/Asset/Model3dAsset.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Utils/Platform/AssimpUtils.hpp"
#include "Utils/Debug.hpp"

static void ProcessSceneNode(Rendering::Model3d& model, const aiScene* modelScene, aiNode* node, const aiMatrix4x4* parentTransform)
{
	//NOTE: we do NOT need any conversion because Assimp converts models into +x -> right, +y ->up, -z -> forward, which match this engine coordinate system
	//BUT assimp also applies a scale factor of 100
	const aiMatrix4x4 globalTransform = parentTransform != nullptr ? *parentTransform * node->mTransformation : node->mTransformation;
	LogWarning(std::format("Found {} global transform:{} parent:{} local:{}", node->mName.C_Str(), AssimpUtils::ToString(globalTransform),
		parentTransform == nullptr ? "NULL" : AssimpUtils::ToString(*parentTransform), AssimpUtils::ToString(node->mTransformation)));
	if (node->mNumMeshes > 0)
	{
		Rendering::MeshGroup* meshGroup = &(model.m_MeshGroups.emplace_back(Rendering::MeshGroup{ Mat4(&globalTransform.a1) }));

		const aiMesh* currentImportMesh = nullptr;
		Rendering::Mesh* currentEngineMesh = nullptr;

		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			currentImportMesh = modelScene->mMeshes[node->mMeshes[i]];
			const size_t meshVertexCount = currentImportMesh->mNumVertices;

			currentEngineMesh = &(model.m_Meshes.emplace_back(Rendering::Mesh{}));
			meshGroup->m_MeshIndices.emplace_back(model.m_Meshes.size() - 1);
			currentEngineMesh->m_Vertices.reserve(meshVertexCount);

			for (size_t j = 0; j < meshVertexCount; j++)
			{
				const aiVector3D pos = currentImportMesh->mVertices[j];
				const aiVector3D normal = currentImportMesh->HasNormals() ? currentImportMesh->mNormals[j] : aiVector3D(0, 0, 0);
				const aiVector3D uv = currentImportMesh->HasTextureCoords(0) ? currentImportMesh->mTextureCoords[0][j] : aiVector3D(0, 0, 0);

				currentEngineMesh->m_Vertices.emplace_back(Rendering::Vertex{ WorldPosition3D(pos.x, pos.y, pos.z),
					UV(uv.x, uv.y), Vec3(normal.x, normal.y, normal.z) });
			}

			for (size_t j = 0; j < currentImportMesh->mNumFaces; j++)
			{
				const aiFace face = currentImportMesh->mFaces[j];
				for (size_t k = 0; k < face.mNumIndices; k++)
				{
					currentEngineMesh->m_Indices.emplace_back(face.mIndices[k]);
				}
			}

			//TODO: also get roughness, normal map and albedo from the material
			aiMaterial* modelMaterial = modelScene->mMaterials[currentImportMesh->mMaterialIndex];
			aiColor4D baseColor;
			if (modelMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
			{
				currentEngineMesh->m_Material.m_BaseColor =
					Utils::ConstructColorFromFloat(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
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
	Assimp::Importer importer;
	const aiScene* modelScene = importer.ReadFile(path.string(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	if (modelScene == nullptr || !modelScene->HasMeshes()) 
	{
		LogError(std::format("Tried to load 3d model at path: '{}' but could not find any meshes", path.string()));
		return;
	}

	Rendering::IndexType currentIndexOffset = 0;
	m_model.m_Meshes.reserve(modelScene->mNumMeshes);
	//NOTE: default assimp matrix creates identity
	ProcessSceneNode(m_model, modelScene, modelScene->mRootNode, nullptr);

	//LogError("FINSIHED MODEL: "+ m_model.ToString());
}

Model3dAsset::~Model3dAsset()
{

}

const Rendering::Model3d& Model3dAsset::GetModel() const { return m_model; }
Rendering::Model3d& Model3dAsset::GetModelMutable() { return m_model; }
void Model3dAsset::UpdateAssetFromFile()
{
	//TODO: implement
}

bool HasModel3dExtension(const std::string& extension)
{
	return extension == ".obj" || extension == ".fbx" || extension == ".glb";
}