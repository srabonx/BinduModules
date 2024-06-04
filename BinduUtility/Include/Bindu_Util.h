#pragma once
#include <comdef.h>
#include <string>
#include <locale>
#include <codecvt>
#include <exception>


#pragma warning(push)
#pragma warning(disable : 4996)
inline std::wstring StringToWstring(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> convertX;
	return convertX.from_bytes(str);

	/*std::wstring ws(str.size(), L'');
	ws.resize(std::mbstowcs(&ws[0], str.c_str(), str.size()));
	return ws;*/
}

inline std::string WstringToString(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
}
#pragma warning(pop)

namespace DX
{

	class DxException : public std::exception
	{
	public:
		DxException() = default;
		DxException(HRESULT hr, const std::string& functionName, const std::string& fileName, int lineNumber);

		const char* what() const noexcept override;

		HRESULT ErrorCode = S_OK;
		std::string FunctionName;
		std::string FileName;
		int LineNumber = -1;
	};


	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw DxException(hr, __func__, __FILE__, __LINE__);
	}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }
#endif
}

