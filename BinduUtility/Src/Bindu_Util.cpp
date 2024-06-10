#include "../Include/Bindu_Util.h"

DX::DxException::DxException(HRESULT hr, const std::string& functionName, const std::string& fileName, int lineNumber) :
	ErrorCode(hr), FunctionName(functionName), FileName(fileName), LineNumber(lineNumber)
{
	what();
}

const char* DX::DxException::what() const noexcept
{
	_com_error err(ErrorCode);
	std::string msg = WstringToString(err.ErrorMessage());
	std::string str = FunctionName + " failed in " + FileName + "; line " + std::to_string(LineNumber) + "; error: " + msg;
	MessageBox(nullptr, StringToWstring(str).c_str(), L"", 0);
	return str.c_str();
}
