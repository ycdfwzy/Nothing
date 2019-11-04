#pragma once
#include <Windows.h>
#include <string>
#include <unordered_set>
#include <unordered_map>

class DiskController
{
public:
	DiskController(char diskName);

	char DiskName() const { return this->diskName; }

private:
	bool getUSNJournalInfo();

private:
	static const unsigned BUFFER_LEN = 1 << 16;

	char diskName;

	HANDLE hDsk;
	CREATE_USN_JOURNAL_DATA cujd;
	DELETE_USN_JOURNAL_DATA dujd;
	USN_JOURNAL_DATA ujd;

	std::unordered_map<DWORDLONG, std::wstring> ref2name;
	std::unordered_map<DWORDLONG, DWORDLONG> parent;
};
