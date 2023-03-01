#pragma once
#include <set>

class CmdOptions
{
public:
	static void Create(PWSTR pCmdLine);
	static bool Exists(const wchar_t* option);
private:
	CmdOptions() = default;
	void Parse(PWSTR pCmdLine);
	static CmdOptions* m_pInstance;
	std::set<std::wstring> m_options;
};