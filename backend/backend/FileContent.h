#pragma once
#include "utils.h"
#include "SearchResult.h"
#include <Windows.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

namespace Nothing {

class FileContent {
public:
	FileContent() {}

	Result add_file(DWORDLONG ref, const std::wstring& path) {
		files.emplace_back(ref, path);
		return Result::SUCCESS;
	}

	bool empty() const { return files.empty(); }
	Result next(const std::wstring&, SearchResult&);

private:
	std::vector<std::pair<DWORDLONG, std::wstring>> files;
};

} // namespace Nothing