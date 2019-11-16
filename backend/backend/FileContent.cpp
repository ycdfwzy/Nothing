#include "FileContent.h"
#include <fstream>

using namespace std;
using namespace Nothing;

Result FileContent::next(const wstring& keyword,
					   int& cnt, std::wstring& path) {
	if (this->empty()) return Result::FILEPOOL_EMPTY;

	path = files.back();
	files.pop_back();
	// open file
	wifstream fin(path);
	if (!fin.is_open())
		return Result::CANNOT_OPEN_FILE;
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
	return Result::SUCCESS;
}