#include "FileContent.h"
#include <fstream>

using namespace std;
using namespace Nothing;

CONTENT_SEARCH_RESULT get_pred_succ(const wstring& raw, int start, int end, int len = 3) {
	PREDECESSOR_CONTENT pred;
	for (int i = 1; i <= len; i++) {
		if (start - i >= 0)
			pred = raw[start - i] + pred;
		else
			break;
	}
	if (start > len)
		pred = L"..." + pred;

	SUCCESSOR_CONTENT succ;
	for (int i = 1; i <= len; i++) {
		if (end + i < raw.length())
			succ = succ + raw[end + i];
		else
			break;
	}
	if (end + len > raw.length())
		succ = succ + L"...";
	return make_pair(pred, succ);
}

Result FileContent::next(const wstring& keyword,
						const wstring& content,
						SearchResult& sr) {
	if (this->empty()) return Result::FILEPOOL_EMPTY;

	DWORDLONG ref = files.back().first;
	wstring path = files.back().second;
	files.pop_back();
	wstring name;
	int t = path.size() - 1;
	while (t >= 0) {
		if (path[t] == L'\\')
			break;
		name = path[t] + name;
		t--;
	}

	if (isText(path)) { // plain text
		// open file
		wifstream fin(path);
		if (!fin.is_open())
			return Result::CANNOT_OPEN_FILE;
		locale loc("zh_CN.UTF-8");
		fin.imbue(loc);
		// get all lines
		wstring line;
		while (!fin.eof()) {
			getline(fin, line);
			// check every line
			auto p = line.find(content);
			while (p != line.npos) {
				if (sr.get_reference() != ref) {
					sr = SearchResult(ref, name, path, keyword, content);
				}
				auto cont_rst = get_pred_succ(line, p, p + content.length() - 1);
				sr.add_content(cont_rst);
				p = line.find(content, p + content.length());
			}
		}
		fin.close();
	}
	else if (isPDF(path)) { // pdf
		PDFReader* reader = PDFReader::getInstance();
		reader->setPath(path);
		wstring cont;
		Result r = reader->getContent(cont);
		if (r == Result::SUCCESS) {
			auto p = cont.find(content);
			while (p != cont.npos) {
				if (sr.get_reference() != ref) {
					sr = SearchResult(ref, name, path, keyword, content);
				}
				auto cont_rst = get_pred_succ(cont, p, p + content.length() - 1);
				sr.add_content(cont_rst);
				p = cont.find(content, p + content.length());
			}
		}
		return r;
	}
	
	return Result::SUCCESS;
}