#pragma once
#include "utils.h"
#include "SearchResult.h"
#include <Windows.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Nothing {

struct FileName {
	std::wstring name;		// filename
	// DWORDLONG ref;		// reference number
	DWORDLONG parent;		// reference number of parent

	FileName(const std::wstring& name = L"", DWORDLONG parent = 0) {
		this->name = name;
		this->parent = parent;
	}
};

typedef std::pair<size_t, size_t> INTERVAL;

class FileBase {
public:
	FileBase(char diskName) : diskName(diskName) {
		hMutex = CreateMutexW(NULL, FALSE, L"watch_mutex");
	}

	Result getPath(DWORDLONG, std::wstring&) const;

	Result save(std::wstring&) const;

	void add_file(const std::wstring&, DWORDLONG, DWORDLONG);
	void add_file_watching(const std::wstring&, DWORDLONG, DWORDLONG);
	void delete_file_watching(DWORDLONG);
	void change_file_watching(const std::wstring&, DWORDLONG, DWORDLONG);

	Result search_by_name(const std::wstring&, std::vector<SearchResult>&, bool need_clear_res = true) const;

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

	Result preprocess();

	Result getAllFiles(DWORDLONG, std::vector<DWORDLONG>&, bool no_directory = true) const;

	Result getReference(const std::wstring&, DWORDLONG&) const;

private:
	Result makeIndex();
	void addIndex(DWORDLONG);
	void removeIndex(DWORDLONG);
	bool splitPath(const std::wstring&, std::vector<std::wstring>&) const;
	void DFS(DWORDLONG, bool, const std::vector<DWORDLONG>&) const;
	void DFS(DWORDLONG, std::vector<DWORDLONG>&) const;
	void getAllSons(DWORDLONG, std::vector<DWORDLONG>&) const;
	void deleteTreeLink(DWORDLONG, DWORDLONG);
	void insertTreeLink(DWORDLONG ref_num, DWORDLONG par_num) {
		if (firstSon.find(par_num) == firstSon.end()) {
			firstSon.insert(std::make_pair(par_num, ref_num));
		}
		else {
			nextSibling.insert(std::make_pair(ref_num, firstSon.at(par_num)));
			firstSon.erase(par_num);
			firstSon.insert(std::make_pair(par_num, ref_num));
		}
	}

private:
	char diskName;

	HANDLE hMutex;

	std::unordered_map<DWORDLONG, FileName> refmap;
	//std::vector<DWORDLONG> DFSseq; // dfs sequence
	//std::unordered_map<DWORDLONG, INTERVAL> interval;

	std::unordered_multimap<WCHAR, DWORDLONG> index;

	/*------------------file tree-----------------*/
	std::unordered_set<DWORDLONG> roots;	// maybe a bit weird, but `$RmMetadata` is a strange directory

	// std::unordered_map<DWORDLONG, DWORDLONG> father;
	std::unordered_map<DWORDLONG, DWORDLONG> firstSon;
	std::unordered_map<DWORDLONG, DWORDLONG> nextSibling;
	/*--------------------------------------------*/

};

} // namespace Nothing