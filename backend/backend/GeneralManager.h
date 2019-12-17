#pragma once
#include "utils.h"
#include "FileBase.h"
#include "DiskController.h"
#include "FileContent.h"
#include "SearchResult.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

namespace Nothing {

class GeneralManager
{
public:
	static GeneralManager* getInstance();

	~GeneralManager();

	Result addDisk(char diskName);

	Result save(char diskName = 0) const;

	Result search(std::vector<SearchResult>&, const std::wstring& keyword = L"", const std::wstring& content = L"",const std::wstring& path = L"");

	Result search_name(const std::wstring&, std::vector<SearchResult>&, CHAR diskName = 0) const;

	Result search_content(const std::wstring&, const std::wstring&, std::vector<SearchResult>&);

	FileContent* getContentSearch() const { return contentSearch; }

private:
	GeneralManager() : contentSearch(nullptr) {}

	static GeneralManager* singleton;

	std::unordered_map<char, DiskController*> disk_base;
	std::unordered_map<char, FileBase*> file_base;

	FileContent* contentSearch;
};

} // namespace Nothing