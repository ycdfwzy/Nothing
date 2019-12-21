#include "FileContent.h"
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/embed.h>

#pragma GCC diagnostic pop

namespace py = pybind11;
using namespace py::literals;

using namespace std;
using namespace Nothing;

CONTENT_SEARCH_RESULT get_pred_succ(const wstring& raw, UINT start, UINT end, UINT len = 3) {
	PREDECESSOR_CONTENT pred;
	for (UINT i = 1; i <= len; i++) {
		if (start >= i)
			pred = raw[start - i] + pred;
		else
			break;
	}
	if (start > len)
		pred = L"..." + pred;

	SUCCESSOR_CONTENT succ;
	for (UINT i = 1; i <= len; i++) {
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
	/*
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
	}*/
	//else if (isPDF(path)) { // pdf
		/*PDFReader* reader = PDFReader::getInstance();
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
		return r;*/
	//}

	try {
		py::scoped_interpreter guard{};

		// Disable build of __pycache__ folders
		py::exec(R"(
            import sys
            sys.dont_write_bytecode = True
        )");

		auto example = py::module::import("reader.FileReader");

		const auto myExampleClass = example.attr("FileReader");
		auto myExampleInstance = myExampleClass(path);

		const auto cont = myExampleInstance.attr("getContent")().cast<std::wstring>();
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
	catch (std::exception & e) {
		std::cerr << "Python read file wrong: " << e.what() << std::endl;
		return Result::PYTHON_RUNTIME_ERROR;
	}

	return Result::SUCCESS;
}