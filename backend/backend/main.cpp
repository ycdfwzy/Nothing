// #include <winioctl.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <locale.h>
#include <ctime>
#include <unordered_map>
#include <vector>
#include "GeneralManager.h"

using namespace std;

char diskName = 'E';
wstring path(L"E:\\大四上\\云数据管理(2)\\智能数据分析\\智能数据分析大作业");

int main() {
	locale loc("chs");
	wcin.imbue(loc);
	wcout.imbue(loc);
	//wstring s;
	//getline(wcin, s);
	//wcout << s << endl;
	GeneralManager* manager = GeneralManager::getInstance();
	clock_t start = clock();
	if (manager->addDisk(diskName)) {
		wcout << L"OK" << endl;
	}
	clock_t end = clock();
	wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;

	while (true) {
		wcout << L"Please input keyword:" << endl;
		WCHAR keyword[256];
		wcin.getline(keyword, 256, L'\n');
		vector<wstring> res;
		res.clear();
		start = clock();
		if (manager->search_content(wstring(keyword), L"E:\\", res)) {
			wcout << "result: " << res.size() << endl;
			/*for (const wstring& s : res) {
				wcout << s << endl;
			}*/
		}
		end = clock();
		wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	}

	// 281474976715340
	return 0;
}

