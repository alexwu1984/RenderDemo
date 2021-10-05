#pragma once
#include "tiny_gltf.h"
#include <memory>

class FGLTFMode
{
	friend struct FRenderItem;
public:
	FGLTFMode(const std::wstring& FileName);
	FGLTFMode();
	~FGLTFMode();

private:
	tinygltf::TinyGLTF m_GltfCtx ;
	tinygltf::Model m_GltfMode;
};