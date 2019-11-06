#pragma once
#include <Windows.h>
#include <string>
#include <unordered_set>
#include <unordered_map>

class FileBase;

class DiskController
{
public:
	DiskController(char diskName);

	bool loadFilenames(FileBase*);

	char DiskName() const { return this->diskName; }

private:
	bool createHandle();
	bool createUSNJournal();
	bool queryUSNJournal();
	bool getUSNJournalInfo(FileBase*);
	bool deleteUSNJournal();

private:
	char diskName;

	HANDLE hDsk;
	CREATE_USN_JOURNAL_DATA cujd;
	DELETE_USN_JOURNAL_DATA dujd;
	USN_JOURNAL_DATA ujd;

};
