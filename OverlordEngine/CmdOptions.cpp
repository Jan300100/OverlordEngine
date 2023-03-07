#include "stdafx.h"
#include <sstream>
#include <cwchar>
#include "CmdOptions.h"

CmdOptions* CmdOptions::m_pInstance = nullptr;

void CmdOptions::Create(PWSTR pCmdLine)
{
	if (m_pInstance == nullptr)
	{
		static CmdOptions opts{};
		m_pInstance = &opts;
	}

	m_pInstance->Parse(pCmdLine);
}

bool CmdOptions::Exists(const wchar_t* option)
{
	std::wstring opt = option;

	for (wchar_t& c : opt)
	{
		c = static_cast<wchar_t>(::tolower(c));
	}

	return m_pInstance->m_options.find(opt) != m_pInstance->m_options.cend();
}

void CmdOptions::Parse(PWSTR pCmdLine)
{
	std::wstringstream wss{ pCmdLine };
	
	std::wstring arg;
	while (wss.good())
	{
		std::getline(wss, arg, L' ');

		std::wstring option{};
		for (wchar_t c : arg)
		{
			if (c == '-')
			{
				continue;
			}
			option += static_cast<wchar_t>(::tolower(c));
		}

		m_options.insert(option);
	}
}
