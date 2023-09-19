#pragma once

//Source: https://stackoverflow.com/questions/6693010/how-do-i-use-multibytetowidechar xd

std::string ConvertWideToANSI(const std::wstring& wstr);
std::wstring ConvertAnsiToWide(const std::string& str);
std::string ConvertWideToUtf8(const std::wstring& wstr);
std::wstring ConvertUtf8ToWide(const std::string& str);

const char* FindLastChar(char chop, const char* srcString, uint16_t offset);
