#include "pch.h"

#pragma once
#include "include/Utils/Conversions.h"

std::string ConvertWideToANSI(const std::wstring& wstr)
{
    size_t count = static_cast<size_t>(WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL));
    std::string str(count, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

std::wstring ConvertAnsiToWide(const std::string& str)
{
    size_t count = static_cast<size_t>(MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0));
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], count);
    return wstr;
}

std::string ConvertWideToUtf8(const std::wstring& wstr)
{
    size_t count = static_cast<size_t>(WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL));
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

std::wstring ConvertUtf8ToWide(const std::string& str)
{
    size_t count = static_cast<size_t>(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0));
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
    return wstr;
}

const char* FindLastChar(char chop, const char* srcString, uint16_t offset)
{
    size_t found = std::string(srcString).find_last_of(chop);
    if (found != std::string::npos)
        return srcString + found + offset;

    return srcString;
}