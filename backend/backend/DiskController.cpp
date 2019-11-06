#include "DiskController.h"
#include "FileBase.h"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <tchar.h>

using namespace std;

DiskController::DiskController(char diskName) {
	this->diskName = diskName;
}

bool DiskController::loadFilenames(FileBase* filebase) {
	if (createHandle() &&
		createUSNJournal() &&
		queryUSNJournal() &&
		getUSNJournalInfo(filebase) &&
		deleteUSNJournal())
		return true;
	return false;
}

bool DiskController::createHandle() {
	string filename = "\\\\.\\";
	filename.push_back(this->diskName);
	filename.push_back(':');
	this->hDsk = CreateFile(_T(filename.c_str()),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);
	if (this->hDsk == INVALID_HANDLE_VALUE) {
		wcerr << L"create file failed!" << endl;
		DWORD err = GetLastError();
		wcerr << L"error code: " << err << endl;
		return false;
	}
	return true;
}

bool DiskController::createUSNJournal() {
	DWORD br;
	this->cujd.AllocationDelta = 0;
	this->cujd.MaximumSize = 0;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_CREATE_USN_JOURNAL,
						&this->cujd,
						sizeof(this->cujd),
						NULL, 0, &br, NULL)) {
		wcerr << L"create usn failed!" << endl;
		DWORD err = GetLastError();
		wcerr << L"error code: " << err << endl;
		return false;
	}
	return true;
}

bool DiskController::queryUSNJournal() {
	DWORD br;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_QUERY_USN_JOURNAL,
						NULL, 0,
						&this->ujd,
						sizeof(this->ujd),
						&br, NULL)) {
		wcerr << L"get usn info failed!" << endl;
		DWORD err = GetLastError();
		wcerr << L"error code: " << err << endl;
		return false;
	}
	// wcout << ujd.MaxUsn << endl;
	return true;
}

constexpr auto BUFFER_LEN = 1 << 12;
CHAR buffer[BUFFER_LEN];

bool DiskController::getUSNJournalInfo(FileBase* filebase) {
	MFT_ENUM_DATA_V0 med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = this->ujd.FirstUsn;
	med.HighUsn = this->ujd.NextUsn;

	DWORD lpBytesReturned;
	PUSN_RECORD pusn_record;
	//int cnt = 0;
	memset(buffer, 0, sizeof(CHAR) * BUFFER_LEN);
	while (DeviceIoControl(this->hDsk,
							FSCTL_ENUM_USN_DATA,
							&med, sizeof(med),
							buffer, BUFFER_LEN,
							&lpBytesReturned,
							NULL)) {
		DWORD retBytes = lpBytesReturned - sizeof(USN);
		// first usn record
		pusn_record = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN));

		while (retBytes > 0) {
			PWCHAR filename = new WCHAR[pusn_record->FileNameLength / 2 + 1];
			for (int k = 0; k < pusn_record->FileNameLength / 2; k++)
				filename[k] = pusn_record->FileName[k];
			filename[pusn_record->FileNameLength / 2] = L'\0';
			filebase->add_file(filename,
								pusn_record->FileReferenceNumber,
								pusn_record->ParentFileReferenceNumber);
			delete[] filename;

			// next record
			retBytes -= pusn_record->RecordLength;
			pusn_record = (PUSN_RECORD)(((PCHAR)pusn_record) + pusn_record->RecordLength);
		}
		// next page
		med.StartFileReferenceNumber = *(USN*)& buffer;
		memset(buffer, 0, sizeof(CHAR) * BUFFER_LEN);
	}

	return true;
}

bool DiskController::deleteUSNJournal() {
	DWORD br;
	this->dujd.UsnJournalID = this->ujd.UsnJournalID;
	this->dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_DELETE_USN_JOURNAL,
						&this->dujd,
						sizeof(this->dujd),
						NULL, 0, &br, NULL)) {
		wcerr << L"delete usn failed!" << endl;
		DWORD err = GetLastError();
		wcerr << L"error code: " << err << endl;
		CloseHandle(this->hDsk);
		return false;
	}
	CloseHandle(this->hDsk);
	return true;
}
