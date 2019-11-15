#pragma once
#include <Windows.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

class FileContent {
public:
	FileContent() {}

	bool add_file(const std::wstring& path) {
		files.push_back(path);
		return true;
	}

	bool empty() const { return  files.empty(); }
	bool next(const std::wstring&, int&, std::wstring&);

private:
	std::vector<std::wstring> files;
};

