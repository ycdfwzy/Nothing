#pragma once
#include "utils.h"
#include "FileBase.h"
#include <Windows.h>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <process.h>

namespace Nothing {

class FileBase;

class DiskController
{
public:
	DiskController(char diskName, FileBase* fb);
	~DiskController() {
		CloseHandle(hThread);
		deleteUSNJournal();
	}

	Result loadFilenames();

	char DiskName() const { return this->diskName; }

	void WatchChanges();

	void startWatching();

private:
	Result createHandle();
	Result createUSNJournal();
	Result queryUSNJournal();
	Result getUSNJournalInfo();
	Result deleteUSNJournal();

	Result WaitNextUsn(PREAD_USN_JOURNAL_DATA_V0);
	Result ReadChanges(USN);
	Result ReadJournalForChanges(USN, DWORD*);

private:
	bool isWatching;
	char diskName;

	HANDLE hDsk;
	CREATE_USN_JOURNAL_DATA cujd;
	DELETE_USN_JOURNAL_DATA dujd;
	USN_JOURNAL_DATA ujd;

	USN last_usn;
	DWORDLONG journal_id;

	HANDLE hThread;

	FileBase* fb;
};

} // namespace Nothing