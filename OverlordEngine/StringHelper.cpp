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
}