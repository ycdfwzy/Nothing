#pragma once

namespace Nothing {

enum class Result {
	SUCCESS = 0,
	UNKNOWN_FAILED,
	/*-----------------------*/
	GETVOLUMEINFORMATION_FALIED,
	NOT_NTFS,
	NO_DISK,
	SAVE_FAILED,
	NO_PATH,
	/*-----------------------*/
	REFERENCE_NOT_FOIUND,
	CANNOT_OPEN_FILE,
	KEYWORD_EMPTY,
	PATH_INVALID,
	/*-----------------------*/
	CREATEHANDLE_FAILED,
	CREATEUSNJOURNAL_FAILED,
	QUERYUSNJOURNAL_FAILED,
	GETUSNJOURNALINFO_FAILED,
	DELETEUSNJOURNAL_FAILED,
	/*-----------------------*/
	FILEPOOL_EMPTY
};

} // namespace Nothing