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
	// DWORDLONG ref;			// reference number
	DWORDLONG parent;		// reference number of parent

	FileName(const std::wstring& name = L"", DWORDLONG parent = 0) {
		this->name = name;
		this->parent = parent;
	}
};

typedef std::pair<size_t, size_t> INTERVAL;

class FileBase {
public:
	FileBase(char diskName) : diskName(diskName) {}

	bool getPath(DWORDLONG, std::wstring&) const;

	bool save(std::wstring&) const;

	void add_file(const std::wstring&, DWORDLONG, DWORDLONG);

	bool search_by_name(const std::wstring&, std::vector<std::wstring>&, bool need_clear_res = true) const;

	void count_files() const {
		std::wcout << L"total files in volume " << diskName << L": " << this->refmap.size() << std::endl;

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

	bool preprocess();

	bool getAllFiles(DWORDLONG, std::vector<DWORDLONG>&, bool no_directory = true) const;

	bool makeIndex();

	bool getReference(const std::wstring&, DWORDLONG&) const;

private:
	bool removeFiles(DWORDLONG);
	bool splitPath(const std::wstring&, std::vector<std::wstring>&) const;
	void DFS(DWORDLONG, bool, const std::vector<DWORDLONG>&,
		std::unordered_map<DWORDLONG, DWORDLONG>*,
		std::unordered_map<DWORDLONG, DWORDLONG>*);

private:
	char diskName;

	std::unordered_map<DWORDLONG, FileName> refmap;
	std::vector<DWORDLONG> DFSseq; // dfs sequence
	std::unordered_map<DWORDLONG, INTERVAL> interval;

	std::unordered_multimap<WCHAR, DWORDLONG> index;
};

