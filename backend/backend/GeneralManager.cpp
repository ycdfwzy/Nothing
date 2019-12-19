#include "GeneralManager.h"
#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <comdef.h>

using namespace std;
using namespace Nothing;

GeneralManager* GeneralManager::singleton = nullptr;

Result checkNTFS(const string& diskName) {
	TCHAR volumeName[MAX_PATH + 1] = { 0 };
	TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
	DWORD serialNumber = 0;
	DWORD maxComponentLen = 0;
	DWORD fileSystemFlags = 0;
	if (GetVolumeInformation(
		_T(diskName.c_str()),
		volumeName,
		ARRAYSIZE(volumeName),
		&serialNumber,
		&maxComponentLen,
		&fileSystemFlags,
		fileSystemName,
		ARRAYSIZE(fileSystemName))) {
		return _tcscmp(fileSystemName, _T("NTFS")) == 0 ?
			Result::SUCCESS : Result::NOT_NTFS;
	}
	return Result::GETVOLUMEINFORMATION_FALIED;
}

GeneralManager* GeneralManager::getInstance() {
	if (singleton == nullptr) {
		singleton = new GeneralManager();
	}
	return singleton;
}

GeneralManager::~GeneralManager() {
	for (auto p: disk_base)
		delete p.second;
	for (auto p: file_base)
		delete p.second;
	disk_base.clear();
	file_base.clear();
}

Result GeneralManager::addDisk(char diskName) {
	if (disk_base.find(diskName) != disk_base.end()) {
		return Result::NO_DISK;
	}
	string volume = "@:\\";
	volume[0] = diskName;
	Result r = checkNTFS(volume);
	if (r != Result::SUCCESS) {
		return r;
	}
	
	file_base[diskName] = new FileBase(diskName);
	disk_base[diskName] = new DiskController(diskName, file_base[diskName]);
	r = disk_base[diskName]->loadFilenames();
	if (r != Result::SUCCESS) {
		disk_base.erase(diskName);
		file_base.erase(diskName);
	}
	else {
		disk_base[diskName]->startWatching();
		file_base[diskName]->count_files();
		file_base[diskName]->preprocess();
	}
	return r;
}

Result GeneralManager::save(char diskName) const {
	wstring path = L"D:\\x_Allfiles.txt";
	if (diskName == 0) {
		Result r = Result::SUCCESS;
		for (const auto& p : file_base) {
			path[3] = p.first;
			r = p.second->save(path);
		}
		return r;
	}
	if (file_base.find(diskName) == file_base.end())
		return Result::NO_DISK;
	path[3] = diskName;
	return file_base.at(diskName)->save(path);
}

Result GeneralManager::search(vector<SearchResult>& res,
							  const std::wstring& keyword,
							  const std::wstring& content,
							  const std::wstring& path) {
	if (keyword == L"" && content == L"" && path == L"")
		return Result::SUCCESS;

	Result r = Result::SUCCESS;
	res.clear();
	if (path == L"") {
		for (const auto& p : file_base) {
			r = p.second->search_by_name(keyword, res, false);
		}
	}
	else
	{
		char diskName = path[0];
		if (file_base.find(diskName) == file_base.end())
			return Result::NO_DISK;
		DWORDLONG path_ref;
		if (file_base.at(diskName)->getReference(path, path_ref)
				== Result::SUCCESS)
			file_base.at(diskName)->search_by_name(keyword, res, false, path_ref);
		else
			return Result::PATH_INVALID;
	}

	if (content != L"") {
		if (contentSearch != nullptr) {
			delete contentSearch;
		}
		contentSearch = new FileContent();
		for (const auto& p : res) {
			r = contentSearch->add_file(p.get_reference(), p.get_path());
		}
		res.clear();
	}

	return Result::SUCCESS;
}

Result GeneralManager::search_name(const wstring& keyword,
							vector<SearchResult>& res,
							CHAR diskName) const {
	if (diskName == 0) {
		res.clear();
		Result r = Result::SUCCESS;
		for (const auto& p : file_base) {
			r = p.second->search_by_name(keyword, res, false);
		}
		return r;
	}
	if (file_base.find(diskName) == file_base.end())
		return Result::NO_DISK;

	return file_base.at(diskName)->search_by_name(keyword, res);
}

Result GeneralManager::search_content(const wstring& keyword,
									const wstring& path,
									vector<SearchResult>& res) {
	// precheck
	if (path.empty()) {
		res.clear();
		for (const auto& p : file_base) {
			wstring path_ = L"X:";
			vector<SearchResult> res_;
			path_[0] = p.first;
			if (search_content(keyword, path_, res_) == Result::SUCCESS) {
				res.insert(res.end(), res_.begin(), res_.end());
			}
		}
		return Result::SUCCESS;
	}
	CHAR diskName = path[0];
	if (path[0] > L'z' || file_base.find(diskName) == file_base.end()) {
		return Result::NO_PATH;
	}

	contentSearch = new FileContent();	
	FileBase* fb = file_base.at(diskName);
	Result r = Result::SUCCESS;

	// get path reference
	DWORDLONG pathRef;
	r = fb->getReference(path, pathRef);
	if (r != Result::SUCCESS) {
		delete contentSearch;
		contentSearch = nullptr;
		return r;
	}

	vector<DWORDLONG> allFiles;
	r = fb->getAllFiles(pathRef, allFiles);
	if (r != Result::SUCCESS) {
		delete contentSearch;
		contentSearch = nullptr;
		return r;
	}
	for (DWORDLONG fileRef : allFiles) {
		wstring path;
		if (fb->getPath(fileRef, path) == Result::SUCCESS) {
			contentSearch->add_file(fileRef, path);
		}
	}

	/*int cnt = 0;
	wstring curpath;*/
	res.clear();
	SearchResult tmp_result;
	while (contentSearch->next(keyword, keyword, tmp_result) != Result::FILEPOOL_EMPTY) {
		if (tmp_result.get_content_results().size() > 0) {
			wcout << tmp_result.get_content_results().size() << L" times in " << tmp_result.get_path() << endl;
			res.push_back(tmp_result);
			tmp_result = SearchResult();
		}
	}
	
	delete contentSearch;
	contentSearch = nullptr;

	// disk_base[diskName]->loadFilenames(fb);
	return Result::SUCCESS;
}