#include "utils.h"
#include <iostream>
#include <cstring>

// Important for PDF related manipulations
#include <PDF/PDFNet.h>
#include <PDF/PDFDoc.h>

// Important for document filesystem interaction and more
#include <SDF/ObjSet.h>

#include <PDF/TextSearch.h>
#include <PDF/ElementReader.h>
#include <PDF/Element.h>
#include <PDF/Annot.h>

using namespace pdftron;
using namespace PDF;
using namespace SDF;
using namespace Common;
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

PDFReader* PDFReader::singleton = nullptr;

PDFReader::PDFReader() {
	PDFNet::Initialize();
}

PDFReader::~PDFReader() {
	PDFNet::Terminate();
}

PDFReader* PDFReader::getInstance() {
	if (singleton == nullptr)
		singleton = new PDFReader;
	return singleton;
}

Result PDFReader::getContent(wstring& res) const {
	Result ret = Result::SUCCESS;
	try {
		PDFDoc doc(path);

		doc.InitSecurityHandler();

		PageIterator itr;
		ElementReader page_reader;

		res.clear();
		for (itr = doc.GetPageIterator(); itr.HasNext(); itr.Next())		//  Read every page
		{
			page_reader.Begin(itr.Current());
			for (Element element = page_reader.Next(); element; element = page_reader.Next()) {
				if (element.GetType() == Element::e_text) {
					res += element.GetTextString().ConvertToNativeWString();
				} /*else
				if (!endsWith(res, L"\n")) {
					res += L"\n";
				}*/
			}
			page_reader.End();
		}
		doc.Close();
	}
	catch (Exception & e)
	{
		wcout << "error: " << path << endl;
		cout << e << endl;
		ret = Result::PDF_READ_FAILED;
	}
	catch (...)
	{
		cout << "Unknown Exception" << endl;
		ret = Result::UNKNOWN_FAILED;
	}

	return ret;
}