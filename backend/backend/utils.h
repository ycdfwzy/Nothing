#pragma once
#include <string>
#include <codecvt>

namespace Nothing {

enum class Result {
	SUCCESS = 0,
	UNKNOWN_FAILED,
	/*-----------------------*/
	GETVOLUMEINFORMATION_FALIED,
	NOT_NTFS,
	NO_DISK,
	SAVE_FAILED,
	NO_PATH,
	/*-----------------------*/
	REFERENCE_NOT_FOIUND,
	CANNOT_OPEN_FILE,
	KEYWORD_EMPTY,
	PATH_INVALID,
	/*-----------------------*/
	CREATEHANDLE_FAILED,
	CREATEUSNJOURNAL_FAILED,
	QUERYUSNJOURNAL_FAILED,
	GETUSNJOURNALINFO_FAILED,
	DELETEUSNJOURNAL_FAILED,
	/*-----------------------*/
	FILEPOOL_EMPTY,
	PDF_READ_FAILED
};

std::wstring string2wstring(const std::string& str);
std::string wstring2string(const std::wstring& wstr);

bool endsWith(const std::string& str, const std::string& suffix);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::wstring& wstr, const std::wstring& suffix);
bool startsWith(const std::wstring& wstr, const std::wstring& prefix);

bool isPDF(const std::wstring& wstr);
bool isText(const std::wstring& wstr);
bool isWord(const std::wstring& wstr);
bool isExcel(const std::wstring& wstr);
bool isPowerPoint(const std::wstring& wstr);

class PDFReader {
public:
	static PDFReader* getInstance();
	bool setPath(const std::wstring& path) { this->path = path; return true; }
	Result getContent(std::wstring&) const;

	~PDFReader();

private:
	PDFReader();
	

	static PDFReader* singleton;

	std::wstring path;
};

} // namespace Nothing