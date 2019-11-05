#include "DiskControllerManager.h"
#include <tchar.h>

using namespace std;

DiskControllerManager* DiskControllerManager::singleton = nullptr;

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

DiskControllerManager* DiskControllerManager::getInstance() {
	if (singleton == nullptr) {
		singleton = new DiskControllerManager();
	}
	return singleton;
}

bool DiskControllerManager::addDisk(char diskName) {
	if (disk_base.find(diskName) != disk_base.end()) {
		return false;
	}
	string volume = "@:\\";
	volume[0] = diskName;
	if (!checkNTFS(volume)) {
		return false;
	}
	disk_base[diskName] = new DiskController(diskName);
	return disk_base[diskName]->initial();
}