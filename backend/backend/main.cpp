// #include <winioctl.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <cstring>
#include <tchar.h>

using namespace std;

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

int main() {
	string diskName = "@:\\";
	for (char c = 'A'; c <= 'Z'; c++) {
		diskName[0] = c;
		if (checkNTFS(diskName)) {
			cout << c << endl;
		}
	}
	return 0;
}