#include "FileContent.h"
#include <fstream>

using namespace std;

bool FileContent::next(const wstring& keyword,
					   int& cnt, std::wstring& path) {
	if (this->empty()) return false;

	path = files.back();
	files.pop_back();
	// open file
	wifstream fin(path);
	if (!fin.is_open())
		return false;
	locale loc("zh_CN.UTF-8");
	fin.imbue(loc);
	// get all lines
	wstring line;
	cnt = 0;
	while (!fin.eof()) {
		getline(fin, line);
		// check every line
		auto p = line.find(keyword);
		while (p != line.npos) {
			cnt++;
			p = line.find(keyword, p + keyword.length());
		}
	}
	fin.close();
	return true;
}