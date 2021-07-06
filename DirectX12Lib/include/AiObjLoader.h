#pragma once
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#include <iostream>
#include "MathLib.h"

class FAiMeshData;

class FAiObjLoader
{
public:
	FAiObjLoader();
	~FAiObjLoader();

public:
	static FAiObjLoader& GetLoader();
	
	std::shared_ptr<FAiMeshData> LoadObj(const std::wstring& FilePath);

private:
	const aiScene* m_Scene;
	std::wstring m_Directory;
};                                                                                                                                                                                                                                                                 