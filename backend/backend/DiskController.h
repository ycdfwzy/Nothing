#pragma once
#include "utils.h"
#include <Windows.h>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace Nothing {

class FileBase;

class DiskController
{
public:
	DiskController(char diskName);

	Result loadFilenames(FileBase*);

	char DiskName() const { return this->diskName; }

private:
	Result createHandle();
	Result createUSNJournal();
	Result queryUSNJournal();
	Result getUSNJournalInfo(FileBase*);
	Result deleteUSNJournal();

private:
	char diskName;

	HANDLE hDsk;
	CREATE_USN_JOURNAL_DATA cujd;
	DELETE_USN_JOURNAL_DATA dujd;
	USN_JOURNAL_DATA ujd;

};

} // namespace Nothing