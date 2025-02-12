#pragma once
#ifndef _AMD64_
#define _AMD64_
#endif

#include <stringapiset.h>
#include <string>

namespace StringUtility {

	std::wstring ConvertString(const std::string& str);

	std::string ConvertString(const std::wstring& wstr);
}