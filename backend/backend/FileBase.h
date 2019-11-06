#pragma once
#include <Windows.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct FileName {
	std::wstring name;		// filename
	DWORDLONG ref;			// reference number
	DWORDLONG parent;		// reference number of parent
};

class FileBase {
public:
	FileBase(char diskName) : diskName(diskName) {}

	bool getPath(DWORDLONG, std::wstring&) const;

	bool save(std::wstring&) const;

	void add_file(const std::wstring&, DWORDLONG, DWORDLONG);

	bool search_by_name(const std::wstring&, std::vector<std::wstring>&, bool need_clear_res = true) const;

	void count_files() const {
		std::wcout << L"total files in volume " << diskName << L": " << this->ref2name.size() << std::endl;

		/*int cnt = 0;
		int max_cnt = 0;
		WCHAR max_arg;
		auto I = index.begin();
		while (I != index.end()) {
			cnt++;
			WCHAR key = (*I).first;
			auto p = index.equal_range(key);
			I = p.second;
			int tmp = index.count(key);
			if (max_cnt < tmp) {
				max_cnt = tmp;
				max_arg = key;
			}
		}
		std::wcout << index.size() << L" values" << std::endl;
		std::wcout << cnt << L" different keys" << std::endl;
		std::wcout << max_arg << L" maximum counts: " << max_cnt << std::endl;*/
	}

private:
	char diskName;

	std::unordered_map<DWORDLONG, std::wstring> ref2name;
	std::unordered_map<DWORDLONG, DWORDLONG> parent;

	std::unordered_multimap<WCHAR, DWORDLONG> index;
};
