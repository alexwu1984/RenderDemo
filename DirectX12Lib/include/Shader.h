#pragma once
#include "Common.h"
#include <map>
#include <memory>

class FShader
{
public:
	enum ShaderType
	{
		GraphicsShader,
		ComputeShader
	};
public:
	FShader(const std::wstring& fileName, ShaderType shaderType = GraphicsShader);
	FShader(const std::wstring& fileName, const std::string& entryCSPoint);
	FShader(const std::wstring& fileName, const std::string& entryVSPoint, const std::string& entryPSPoint);
	~FShader();

	ComPtr<ID3DBlob> GetVertexShader() { return m_vertexShader; }
	ComPtr<ID3DBlob> GetPixelShader() { return m_pixelShader; }
	ComPtr<ID3DBlob> GetComputeShader() { return m_computeShader; }
private:
	std::wstring m_name;

	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;
	ComPtr<ID3DBlob> m_computeShader;
};


class FShaderMgr
{
public:
	FShaderMgr() {}
	~FShaderMgr() {}

	static FShaderMgr& Get();

	std::shared_ptr<FShader> CreateShader(const std::string& keyName, const std::wstring& fullname, FShader::ShaderType shaderType = FShader::GraphicsShader);
	std::shared_ptr<FShader> CreateShader(const std::string& keyName, const std::string& entryPoint, const std::wstring& fullname);
	std::shared_ptr<FShader> CreateShader(const std::string& keyName, const std::string& entryVSPoint, const std::string& entryPSPoint, const std::wstring& fullname);
	std::shared_ptr<FShader> CreateShaderDirect(const std::wstring& fullname, const std::string& entryVSPoint, const std::string& entryPSPoint);

	std::shared_ptr<FShader> FindShader(const std::string& keyName);

private:
	std::map<std::string, std::shared_ptr<FShader>> m_mapShader;
};