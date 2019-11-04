#include "DiskController.h"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <tchar.h>

using namespace std;

DiskController::DiskController(char diskName) {
	this->diskName = diskName;

	string filename = "\\\\.\\";
	filename.push_back(diskName);
	filename.push_back(':');
	this->hDsk = CreateFile(_T(filename.c_str()),
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_READONLY,
							NULL);
	if (this->hDsk == INVALID_HANDLE_VALUE) {
		cerr << "create file failed!" << endl;
		DWORD err = GetLastError();
		cout << "error code: " << err << endl;
		return;
	}
	DWORD br;
	this->cujd.AllocationDelta = 0;
	this->cujd.MaximumSize = 0;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_CREATE_USN_JOURNAL,
						&this->cujd,
						sizeof(this->cujd),
						NULL, 0, &br, NULL)) {
		cerr << "create usn failed!" << endl;
		DWORD err = GetLastError();
		cout << "error code: " << err << endl;
		return;
	}
	if (!DeviceIoControl(this->hDsk,
						FSCTL_QUERY_USN_JOURNAL,
						NULL, 0,
						&this->ujd,
						sizeof(this->ujd),
						&br, NULL)) {
		cerr << "get usn info failed!" << endl;
		DWORD err = GetLastError();
		cout << "error code: " << err << endl;
		return;
	}
	this->getUSNJournalInfo();

	this->dujd.UsnJournalID = this->ujd.UsnJournalID;
	this->dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_DELETE_USN_JOURNAL,
						&this->dujd,
						sizeof(this->dujd),
						NULL, 0, &br, NULL)) {
		cerr << "delete usn failed!" << endl;
		DWORD err = GetLastError();
		cout << "error code: " << err << endl;
		return;
	}
	CloseHandle(this->hDsk);
}

char buffer[1<<16];

bool DiskController::getUSNJournalInfo() {
	MFT_ENUM_DATA med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = this->ujd.FirstUsn;
	med.HighUsn = this->ujd.NextUsn;

	
	DWORD lpBytesReturned;

	PUSN_RECORD pusn_record;

	while (DeviceIoControl(this->hDsk,
							FSCTL_ENUM_USN_DATA,
							&med, sizeof(med),
							buffer, BUFFER_LEN,
							&lpBytesReturned,
							NULL) != 0) {
		DWORD retBytes = lpBytesReturned - sizeof(USN);
		// first usn record
		pusn_record = (PUSN_RECORD)(((char*)buffer) + sizeof(USN));

		while (retBytes > 0) {
			this->ref2name[pusn_record->FileReferenceNumber] = pusn_record->FileName;
			this->parent[pusn_record->FileReferenceNumber] = pusn_record->ParentFileReferenceNumber;

			// next record
			retBytes -= pusn_record->RecordLength;
			pusn_record = (PUSN_RECORD)(((char*)pusn_record) + pusn_record->RecordLength);
		}
		// next page
		med.StartFileReferenceNumber = *(USN*)& buffer;
	}

	cout << this->ref2name.size() << endl;

	//delete[] buffer;
	return true;
}