#include "AiObjLoader.h"
#include "AiMeshData.h"
#include "StringUnit.h"


FAiObjLoader::FAiObjLoader()
{

}

FAiObjLoader::~FAiObjLoader()
{

}

FAiObjLoader& FAiObjLoader::GetLoader()
{
	static FAiObjLoader loader;
	return loader;
}

std::shared_ptr<FAiMeshData> FAiObjLoader::LoadObj(const std::wstring& FilePath)
{
	Assimp::Importer ModelImpoter;
	
	std::shared_ptr<FAiMeshData> mesh = std::make_shared<FAiMeshData>(FilePath);

	m_Scene = ModelImpoter.ReadFile(core::ucs2_u8(FilePath), aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
	if (!m_Scene || !m_Scene->mRootNode || m_Scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cerr << "Error::Model:: " << ModelImpoter.GetErrorString() << std::endl;
		return mesh;
	}
	m_Directory = FilePath.substr(0, FilePath.find_last_of('/'));
	mesh->TraverserNodes(m_Scene);
	return mesh;
}
