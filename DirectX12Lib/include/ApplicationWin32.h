#pragma once
#include "WindowWin32.h"


class FGame;
class D3D12RHI;

class ApplicationWin32
{
public:
	static ApplicationWin32& Get();

	void Run(FGame* game);
	void SetCurrentWorkPath(const std::wstring& WorkPath);
	std::wstring GetCurrentWorkPath() const;
	
private:
	//ApplicationWin32() = default;
	bool Initialize(FGame* game);
	void Terminate();

private:
	std::wstring m_WorkPath = L"../";
};

