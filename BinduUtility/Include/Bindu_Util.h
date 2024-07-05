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


inline std::string RelativeResourcePath(const char* filepath = nullptr, const std::string& rootFolderName = "")
{
	std::string ResourcePath(MAX_PATH, '\0');

	GetModuleFileNameA(nullptr, const_cast<LPSTR>(ResourcePath.data()), MAX_PATH);

	std::string delimeter = rootFolderName;

	size_t t = ResourcePath.find(delimeter);
	ResourcePath = ResourcePath.substr(0, t + delimeter.size() + 1);

	if (filepath)
		ResourcePath += filepath;

	return ResourcePath;
}

// Needs to be called every frame
// Returns fps = frames per second, mspf = milliseconds per frame 
// Returns true if a second has passed
inline bool CalculateFrameStats(int& fps, float& mspf, float totalTime)
{
	static int frameCount{ 0 };
	static float timeElapsed{ 0.0f };

	frameCount++;

	// Compute average over 1 second period
	if (totalTime - timeElapsed >= 1.0f)
	{
		fps = frameCount;
		mspf = 1000.0f / fps;

		frameCount = 0;
		timeElapsed += 1.0f;

		return true;
	}

	return false;
}

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
//#ifndef ThrowIfFailed
//#define ThrowIfFailed(x)																\
//{																						\
//		HRESULT hr__ = (x);																\
//		if (FAILED(hr__)) { throw DxException(hr__, L#x, __FILE__, __LINE__); }	\
//}
//#endif
 
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }
#endif
}



