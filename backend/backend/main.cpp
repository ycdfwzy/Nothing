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

using namespace std;
using namespace Nothing;

char diskName = 'E';
wstring path(L"E:\\大四上\\云数据管理(2)\\智能数据分析\\智能数据分析大作业");

int main() {
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
		wcout << L"Please input keyword:" << endl;
		WCHAR keyword[256];
		wcin.getline(keyword, 256, L'\n');
		vector<SearchResult> res;
		res.clear();
		start = clock();
		/*if (manager->search_content(wstring(keyword), L"E:\\", res)
				== Result::SUCCESS) {
			wcout << "result: " << res.size() << endl;
		}*/
		if (manager->search_name(wstring(keyword), res, diskName)
				== Result::SUCCESS) {
			for (auto& p : res) {
				wcout << p.get_path() << endl;
			}
			wcout << "result: " << res.size() << endl;
		}
		end = clock();
		wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	}

	return 0;
}