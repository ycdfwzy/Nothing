#include "DiskController.h"
#include "FileBase.h"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <tchar.h>

using namespace std;
using namespace Nothing;

DWORD ALL_MASK = 0xFFFFFFFF;	// all changes
DWORD FILECHANGE_MASK = USN_REASON_RENAME_NEW_NAME   |
						USN_REASON_SECURITY_CHANGE   |
						USN_REASON_BASIC_INFO_CHANGE |
						USN_REASON_DATA_OVERWRITE    |
						USN_REASON_DATA_TRUNCATION   |
						USN_REASON_DATA_EXTEND       |
						USN_REASON_CLOSE;

void showLastError() {
	DWORD dw = GetLastError();
	wcerr << "Error code: " << dw << endl;
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	string message(messageBuffer, size);
	LocalFree(messageBuffer);
	cout << message << endl;
}

unsigned WINAPI spy(void* param) {
	((DiskController*)param)->WatchChanges();
	return 0;
}

DiskController::DiskController(char diskName, FileBase* fb) : 
	diskName(diskName), fb(fb) {
	this->isWatching = false;
	this->last_usn = 0;
}

Result DiskController::loadFilenames() {
	Result r = createHandle();
	if (r != Result::SUCCESS)
		return r;
	r = createUSNJournal();
	if (r != Result::SUCCESS)
		return r;
	r = queryUSNJournal();
	if (r != Result::SUCCESS)
		return r;
	r = getUSNJournalInfo();
	if (r != Result::SUCCESS)
		return r;
	// r = deleteUSNJournal();
	return r;
}

void DiskController::WatchChanges() {
	wcout << "WatchChanges" << endl;

	// wait for changes query
	PREAD_USN_JOURNAL_DATA_V0 query = new READ_USN_JOURNAL_DATA_V0;
	query->StartUsn = this->last_usn;
	query->ReasonMask = FILECHANGE_MASK;
	query->ReturnOnlyOnClose = FALSE;
	query->Timeout = 0;
	query->BytesToWaitFor = 1;
	query->UsnJournalID = this->journal_id;

	// Polling
	while (true) {
		Result res = WaitNextUsn(query);
		if (res != Result::SUCCESS) {
			wcerr << "Error when WaitNextUsn!" << endl;
			showLastError();
			continue;
		}
		if (!isWatching) {
			delete query;
			wcout << "end WaitNextUsn" << endl;
			return;
		}

		res = ReadChanges(query->StartUsn);
		query->StartUsn = this->last_usn;
		// break;
	}
	wcout << "end WaitNextUsn" << endl;
	delete query;
}

void DiskController::startWatching() {
	isWatching = true;
	UINT threadId;
	hThread = (HANDLE)_beginthreadex(NULL, 0, &spy, (void*)this, 0, &threadId);
}

constexpr auto BUFFER_LEN = 1 << 12;
CHAR buffer[BUFFER_LEN];
Result DiskController::ReadChanges(USN low_usn) {
	DWORD br;
	memset(buffer, 0, sizeof(CHAR) * BUFFER_LEN);
	if (ReadJournalForChanges(low_usn, &br) != Result::SUCCESS) {
		// something to do
		wcerr << "Error when ReadJournalForChanges!" << endl;
		showLastError();
		return Result::UNKNOWN_FAILED;
	}
	
	DWORD retBytes = br - sizeof(USN);
	PUSN_RECORD pusn_record = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN));
	while (retBytes > 0) {
		PWCHAR filename = new WCHAR[pusn_record->FileNameLength / 2 + 1];
		for (int k = 0; k < pusn_record->FileNameLength / 2; k++)
			filename[k] = pusn_record->FileName[k];
		filename[pusn_record->FileNameLength / 2] = L'\0';

		DWORD reason = pusn_record->Reason;
		// some system files will create and delete very soon
		if ((reason & USN_REASON_FILE_CREATE) && (reason & USN_REASON_FILE_DELETE)) {
			// wcout << "system file created & deleted: ";
			fb->delete_file_watching(pusn_record->FileReferenceNumber);
		} else
		if ((reason & USN_REASON_FILE_CREATE) && (reason & USN_REASON_CLOSE)) {
			// wcout << "file created: ";
			fb->add_file_watching(filename, pusn_record->FileReferenceNumber,
								  pusn_record->ParentFileReferenceNumber);
		} else
		if ((reason & USN_REASON_FILE_DELETE) && (reason & USN_REASON_CLOSE)) {
			// wcout << "file deleted: ";
			fb->delete_file_watching(pusn_record->FileReferenceNumber);
		} else
		if (reason & FILECHANGE_MASK) {
			// wcout << "file changed: ";
			fb->change_file_watching(filename, pusn_record->FileReferenceNumber,
									 pusn_record->ParentFileReferenceNumber);
		}
		// wcout << filename << endl;

		// next record
		retBytes -= pusn_record->RecordLength;
		pusn_record = (PUSN_RECORD)(((PCHAR)pusn_record) + pusn_record->RecordLength);
	}

	this->last_usn = *(USN*)buffer;

	return Result::SUCCESS;
}

Result DiskController::createHandle() {
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
		showLastError();
		return Result::CREATEHANDLE_FAILED;
	}
	return Result::SUCCESS;
}

Result DiskController::createUSNJournal() {
	DWORD br;
	this->cujd.AllocationDelta = 0;
	this->cujd.MaximumSize = 0;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_CREATE_USN_JOURNAL,
						&this->cujd,
						sizeof(this->cujd),
						NULL, 0, &br, NULL)) {
		wcerr << L"create usn failed!" << endl;
		showLastError();
		return Result::CREATEUSNJOURNAL_FAILED;
	}
	return Result::SUCCESS;
}

Result DiskController::queryUSNJournal() {
	DWORD br;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_QUERY_USN_JOURNAL,
						NULL, 0,
						&this->ujd,
						sizeof(this->ujd),
						&br, NULL)) {
		wcerr << L"get usn info failed!" << endl;
		showLastError();
		return Result::QUERYUSNJOURNAL_FAILED;
	}
	this->journal_id = this->ujd.UsnJournalID;
	return Result::SUCCESS;
}

Result DiskController::getUSNJournalInfo() {
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
			// wcout << filename << endl;
			fb->add_file(filename,
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

	this->last_usn = med.HighUsn;

	return Result::SUCCESS;
}

Result DiskController::deleteUSNJournal() {
	DWORD br;
	this->dujd.UsnJournalID = this->ujd.UsnJournalID;
	this->dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;
	if (!DeviceIoControl(this->hDsk,
						FSCTL_DELETE_USN_JOURNAL,
						&this->dujd,
						sizeof(this->dujd),
						NULL, 0, &br, NULL)) {
		wcerr << L"delete usn failed!" << endl;
		showLastError();
		CloseHandle(this->hDsk);
		return Result::DELETEUSNJOURNAL_FAILED;
	}
	CloseHandle(this->hDsk);
	return Result::SUCCESS;
}

Result DiskController::WaitNextUsn(PREAD_USN_JOURNAL_DATA_V0 query) {
	DWORD br;
	
	Result res = DeviceIoControl(this->hDsk,
								FSCTL_READ_USN_JOURNAL,
								query, sizeof(*query),
								// buffer, BUFFER_LEN,
								&(query->StartUsn), sizeof(query->StartUsn),
								&br, NULL) ?
							Result::SUCCESS : Result::UNKNOWN_FAILED;
	return res;
}

Result DiskController::ReadJournalForChanges(USN low_usn, DWORD* br) {
	READ_USN_JOURNAL_DATA_V0 query;
	query.StartUsn = low_usn;
	query.ReasonMask = ALL_MASK;
	query.ReturnOnlyOnClose = false;
	query.Timeout = 0;
	query.BytesToWaitFor = 0;
	query.UsnJournalID = this->journal_id;

	Result res = DeviceIoControl(this->hDsk,
								 FSCTL_READ_USN_JOURNAL,
								 &query, sizeof(query),
								 buffer, BUFFER_LEN,
								 br, NULL) ?
							Result::SUCCESS : Result::UNKNOWN_FAILED;
	return res;
}
