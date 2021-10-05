#include "GltfModel.h"
#include <windows.h>

static inline std::string WcharToUTF8(const std::wstring& wstr) {
	int str_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(),
		nullptr, 0, NULL, NULL);
	std::string str(str_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &str[0],
		(int)str.size(), NULL, NULL);
	return str;
}

FGLTFMode::FGLTFMode(const std::wstring& FileName)
{

	std::string err;
	std::string warn;
	std::string utf8FileName = WcharToUTF8(FileName);
	m_GltfCtx.LoadBinaryFromFile(&m_GltfMode, &err, &warn, utf8FileName);
}

FGLTFMode::FGLTFMode()
{

}

FGLTFMode::~FGLTFMode()
{

}

