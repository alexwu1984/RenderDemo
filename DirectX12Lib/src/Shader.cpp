#include "Shader.h"
#include "D3D12RHI.h"

FShader::FShader(const std::wstring& fileName, ShaderType shaderType)
	:m_name(fileName)
{
	if (shaderType ==  GraphicsShader)
	{
		m_vertexShader = D3D12RHI::Get().CreateShader(fileName, "vs_main", "vs_5_1");
		m_pixelShader = D3D12RHI::Get().CreateShader(fileName, "ps_main", "ps_5_1");
	}
	else if (shaderType == ComputeShader)
	{
		m_computeShader = D3D12RHI::Get().CreateShader(fileName, "cs_main", "cs_5_1");
	}

}

FShader::FShader(const std::wstring& fileName, const std::string& entryVSPoint, const std::string& entryPSPoint)
	:m_name(fileName)
{
	m_vertexShader = D3D12RHI::Get().CreateShader(fileName, entryVSPoint, "vs_5_1");
	m_pixelShader = D3D12RHI::Get().CreateShader(fileName, entryPSPoint, "ps_5_1");

}

FShader::FShader(const std::wstring& fileName, const std::string& entryCSPoint)
{
	m_computeShader = D3D12RHI::Get().CreateShader(fileName, entryCSPoint, "cs_5_1");
}

FShader::~FShader()
{

}

FShaderMgr& FShaderMgr::Get()
{
	static FShaderMgr mgr;
	return mgr;
}

std::shared_ptr<FShader> FShaderMgr::CreateShader(const std::string& keyName, const std::wstring& fullname, FShader::ShaderType shaderType)
{
	auto itFind = m_mapShader.find(keyName);
	if (itFind != m_mapShader.end())
	{
		return itFind->second;
	}

	std::shared_ptr<FShader> shader = std::make_shared<FShader>(fullname, shaderType);
	m_mapShader.insert({ keyName,shader });
	return shader;
}

std::shared_ptr<FShader> FShaderMgr::CreateShader(const std::string& keyName, const std::string& entryPoint, const std::wstring& fullname)
{
	auto itFind = m_mapShader.find(keyName);
	if (itFind != m_mapShader.end())
	{
		return itFind->second;
	}

	std::shared_ptr<FShader> shader = std::make_shared<FShader>(fullname,entryPoint);
	m_mapShader.insert({ keyName,shader });
	return shader;
}

std::shared_ptr<FShader> FShaderMgr::CreateShader(const std::string& keyName, const std::string& entryVSPoint, const std::string& entryPSPoint, const std::wstring& fullname)
{
	auto itFind = m_mapShader.find(keyName);
	if (itFind != m_mapShader.end())
	{
		return itFind->second;
	}

	std::shared_ptr<FShader> shader = std::make_shared<FShader>(fullname, entryVSPoint,entryPSPoint);
	m_mapShader.insert({ keyName,shader });
	return shader;
}

std::shared_ptr<FShader> FShaderMgr::CreateShaderDirect(const std::wstring& fullname, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> shader = std::make_shared<FShader>(fullname, entryVSPoint, entryPSPoint);
	return shader;
}

std::shared_ptr<FShader> FShaderMgr::FindShader(const std::string& keyName)
{
	auto itFind = m_mapShader.find(keyName);
	if (itFind != m_mapShader.end())
	{
		return itFind->second;
	}
	return nullptr;
}
