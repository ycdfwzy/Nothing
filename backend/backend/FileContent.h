#pragma once
#include "utils.h"
#include "SearchResult.h"
#include <Windows.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

namespace Nothing {

class FileContent {
public:
	FileContent() {}

	Result add_file(DWORDLONG ref, const std::wstring& path) {
		WIN32_FIND_DATAW FindFileData;
		FindClose(FindFirstFileW(path.c_str(), &FindFileData));
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			files.emplace_back(ref, path);
		return Result::SUCCESS;
	}

	bool empty() const { return files.empty(); }
	Result next(const std::wstring&, const std::wstring&, SearchResult&);

private:
	std::vector<std::pair<DWORDLONG, std::wstring>> files;
};

} // namespace Nothing