#pragma once
#include <string>

namespace StringHelpers
{
	std::string WStringToString(const std::wstring& wideString);
	std::wstring StringToWString(const std::string& wideString);
}