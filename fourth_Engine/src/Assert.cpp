#include "pch.h"

#pragma once
#include "include/Assert.h"
#include "include/win_def.h"
#include <comdef.h>
#include "include/win_undef.h"

namespace fth
{
	std::string TranslateHRESULT(HRESULT hr)
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		const int size = WideCharToMultiByte(CP_UTF8, 0, errMsg, (int)wcslen(errMsg), NULL, 0, NULL, NULL);
		std::string returnString{};
		returnString.resize(size);
		WideCharToMultiByte(CP_UTF8, 0, errMsg, (int)wcslen(errMsg), returnString.data(), size, NULL, NULL);

		return returnString;
	}
}