#include "stdafx.h"
#include "StringHelper.h"

namespace StringHelpers
{
    std::string WStringToString(const std::wstring& wideString)
    {
        size_t origsize = wideString.size() + 1;
        size_t convertedChars = 0;
        const size_t newsize = origsize * 2;

        std::string result;
        result.resize(newsize);
        wcstombs_s(&convertedChars, result.data(), newsize, wideString.c_str(), _TRUNCATE);

        return result;
    }

    std::wstring StringToWString(const std::string& narrowString)
    {
        size_t origsize = narrowString.length() + 1;
        size_t convertedChars = 0;
        const size_t newsize = origsize * 2;

        std::wstring result;
        result.resize(newsize);
        mbstowcs_s(&convertedChars, &result[0], newsize, narrowString.c_str(), _TRUNCATE);

        return result;
    }
}