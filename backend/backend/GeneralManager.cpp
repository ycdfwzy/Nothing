#include "GeneralManager.h"
#include <Windows.h>
#include <iostream>
#include <tchar.h>

using namespace std;

GeneralManager* GeneralManager::singleton = nullptr;

bool checkNTFS(const string& diskName) {
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
		ARRAYSIZE(fileSystemName)))
		return _tcscmp(fileSystemName, _T("NTFS")) == 0;
	return false;
}

GeneralManager* GeneralManager::getInstance() {
	if (singleton == nullptr) {
		singleton = new GeneralManager();
	}
	return singleton;
}

bool GeneralManager::addDisk(char diskName) {
	if (disk_base.find(diskName) != disk_base.end()) {
		return false;
	}
	string volume = "@:\\";
	volume[0] = diskName;
	if (!checkNTFS(volume)) {
		return false;
	}
	disk_base[diskName] = new DiskController(diskName);
	file_base[diskName] = new FileBase(diskName);
	bool flag = disk_base[diskName]->loadFilenames(file_base[diskName]);
	if (!flag) {
		disk_base.erase(diskName);
		file_base.erase(diskName);
	}
	else {
		file_base[diskName]->count_files();
		file_base[diskName]->doDFS();
	}
	return flag;
}

bool GeneralManager::save(char diskName) const {
	wstring path = L"D:\\x_Allfiles.txt";
	if (diskName == 0) {
		bool flag = true;
		for (const auto& p : file_base) {
			path[3] = p.first;
			if (!p.second->save(path))
				flag = false;
		}
		return flag;
	}
	if (file_base.find(diskName) == file_base.end())
		return false;
	path[3] = diskName;
	return file_base.at(diskName)->save(path);
}

bool GeneralManager::search(const wstring& keyword,
							vector<wstring>& res,
							CHAR diskName) const {
	if (diskName == 0) {
		res.clear();
		bool flag = true;
		for (const auto& p : file_base) {
			if (!p.second->search_by_name(keyword, res, false))
				flag = false;
		}
		return flag;
	}
	if (file_base.find(diskName) == file_base.end())
		return false;

	//vector<DWORDLONG> res_;
	//if (file_base.at(diskName)->getAllFiles(281474976715340u, res_)) {
	//	wcout << res_.size() << endl;
	//	for (DWORDLONG ref : res_) {
	//		wstring fullpath;
	//		file_base.at(diskName)->getPath(ref, fullpath);
	//		wcout << fullpath << endl;
	//	}
	//}
	//else
	//{
	//	wcout << L"can't get all files!" << endl;
	//}

	return file_base.at(diskName)->search_by_name(keyword, res);
}