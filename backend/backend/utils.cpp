#include "utils.h"
#include <iostream>
#include <cstring>

using namespace std;
using namespace Nothing;

std::wstring Nothing::string2wstring(const std::string& str) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(str);
}

std::string Nothing::wstring2string(const std::wstring& wstr) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(wstr);
}

bool Nothing::endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length())
		return false;
	return str.substr(str.length() - suffix.length()) == suffix;
}

bool Nothing::startsWith(const std::string& str, const std::string& prefix) {
	if (str.length() < prefix.length())
		return false;
	return str.substr(0, prefix.length()) == prefix;
}

bool Nothing::endsWith(const std::wstring& wstr, const std::wstring& suffix) {
	if (wstr.length() < suffix.length())
		return false;
	return wstr.substr(wstr.length() - suffix.length()) == suffix;
}

bool Nothing::startsWith(const std::wstring& wstr, const std::wstring& prefix) {
	if (wstr.length() < prefix.length())
		return false;
	return wstr.substr(0, prefix.length()) == prefix;
}

bool Nothing::isPDF(const std::wstring& wstr) {
	return endsWith(wstr, L".pdf");
}

bool Nothing::isText(const std::wstring& wstr) {
	return endsWith(wstr, L".txt");
}

bool Nothing::isWord(const std::wstring& wstr) {
	return endsWith(wstr, L".doc") || endsWith(wstr, L".docx");
}

bool Nothing::isExcel(const std::wstring& wstr) {
	return endsWith(wstr, L".xls") || endsWith(wstr, L".xlsx");
}

bool Nothing::isPowerPoint(const std::wstring& wstr) {
	return endsWith(wstr, L".ppt") || endsWith(wstr, L".pptx");
}
