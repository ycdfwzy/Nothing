#pragma once
#include "utils.h"
#include <Windows.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

namespace Nothing {

class FileContent {
public:
	FileContent() {}

	Result add_file(const std::wstring& path) {
		files.push_back(path);
		return Result::SUCCESS;
	}

	bool empty() const { return files.empty(); }
	Result next(const std::wstring&, int&, std::wstring&);

private:
	std::vector<std::wstring> files;
};

} // namespace Nothing