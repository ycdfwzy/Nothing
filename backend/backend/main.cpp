#include <Windows.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <locale.h>
#include <ctime>
#include <unordered_map>
#include <vector>
#include "SearchResult.h"
#include "GeneralManager.h"

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

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

char diskName = 'E';
wstring path(L"E:\\THU");

int main(int argc, char** argv) {
	// Initialize python
	Py_OptimizeFlag = 1;
	Py_SetProgramName(L"PythonFileReader");

	locale loc("chs");
	wcin.imbue(loc);
	wcout.imbue(loc);
	GeneralManager* manager = GeneralManager::getInstance();
	clock_t start = clock();
	if (manager->addDisk(diskName) == Result::SUCCESS) {
		wcout << L"OK" << endl;
	}
	clock_t end = clock();
	wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;

	while (true) {
		// input keyword to search
		wcout << L"Please input keyword:" << endl;
		WCHAR keyword[256];
		wcin.getline(keyword, 256, L'\n');

		wcout << L"Please input content:" << endl;
		WCHAR content[256];
		wcin.getline(content, 256, L'\n');

		//wcout << L"Please input path:" << endl;
		//WCHAR path[MAX_PATH];
		//wcin.getline(path, MAX_PATH, L'\n');

		vector<SearchResult> res;
		res.clear();

		start = clock();
		// search
		if (manager->search(res, keyword, content, path)
				== Result::SUCCESS) {
			if (wstring(content) == L"") {
				/*for (auto& p : res) {
					wcout << p.get_path() << endl;
				}*/
				wcout << "result: " << res.size() << endl;
			}
			else {
				FileContent* contentSearch = manager->getContentSearch();
				SearchResult tmp_result;
				while (contentSearch->next(keyword, content, tmp_result) != Result::FILEPOOL_EMPTY) {
					if (tmp_result.get_content_results().size() > 0) {
						wcout << tmp_result.get_content_results().size() << L" times in " << tmp_result.get_path() << endl;
						res.push_back(tmp_result);
						tmp_result = SearchResult();
					}
				}
			}
		}
		end = clock();
		wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	}

	return 0;
}