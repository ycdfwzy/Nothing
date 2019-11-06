// #include <winioctl.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <locale.h>
#include <ctime>
#include <unordered_map>
#include "GeneralManager.h"

using namespace std;

int main() {
	locale loc("chs");
	wcin.imbue(loc);
	wcout.imbue(loc);
	GeneralManager* manager = GeneralManager::getInstance();
	clock_t start = clock();
	if (manager->addDisk('D')) {
		wcout << L"OK" << endl;
	}
	clock_t end = clock();
	wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;

	while (true) {
		wcout << L"Please input keyword:" << endl;
		wstring keyword;
		wcin >> keyword;
		wcout << keyword << endl;
		vector<wstring> res;
		res.clear();
		start = clock();
		if (manager->search(keyword, res, 'D')) {
			wcout << "result: " << res.size() << endl;
			/*for (const wstring& s : res) {
				wcout << s << endl;
			}*/
		}
		end = clock();
		wcout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	}
	return 0;
}

