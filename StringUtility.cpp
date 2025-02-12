#include <windows.h>
#include <stringapiset.h>
#include <string>
#include "StringUtility.h"

namespace StringUtility {
    std::wstring ConvertString(const std::string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }
   
    std::string ConvertString(const std::wstring& wstr)
    {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }
};

//std::wstring StringUtility::ConvertString(const std::string& str)
//{
//    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
//    std::wstring wstrTo(size_needed, 0);
//    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
//    return wstrTo;
//}
//
//std::string StringUtility::ConvertString(const std::wstring& wstr)
//{
//    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
//    std::wstring strTo(size_needed, 0);
//    MultiByteToWideChar(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed);
//    return strTo;
//}
