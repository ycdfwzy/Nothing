// #include <winioctl.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <cstring>
#include <locale.h>
#include <ctime>
#include "DiskControllerManager.h"

using namespace std;

int main() {
	setlocale(LC_ALL, "chs");
	DiskControllerManager* manager = DiskControllerManager::getInstance();
	clock_t start = clock();
	if (manager->addDisk('D')) {
		cout << "OK" << endl;
	}
	clock_t end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}

//constexpr auto BUF_LEN = 4096;
//
//void main()
//{
//	HANDLE hVol;
//	CHAR Buffer[BUF_LEN];
//
//	USN_JOURNAL_DATA JournalData;
//	READ_USN_JOURNAL_DATA_V0 ReadData = { 0, 0xFFFFFFFF, FALSE, 0, 0 };
//	PUSN_RECORD UsnRecord;
//
//	DWORD dwBytes;
//	DWORD dwRetBytes;
//	int I;
//
//	hVol = CreateFile(TEXT("\\\\.\\d:"),
//		GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ | FILE_SHARE_WRITE,
//		NULL,
//		OPEN_EXISTING,
//		0,
//		NULL);
//
//	if (hVol == INVALID_HANDLE_VALUE)
//	{
//		printf("CreateFile failed (%d)\n", GetLastError());
//		return;
//	}
//
//	if (!DeviceIoControl(hVol,
//		FSCTL_QUERY_USN_JOURNAL,
//		NULL,
//		0,
//		&JournalData,
//		sizeof(JournalData),
//		&dwBytes,
//		NULL))
//	{
//		printf("Query journal failed (%d)\n", GetLastError());
//		return;
//	}
//
//	ReadData.UsnJournalID = JournalData.UsnJournalID;
//
//	printf("Journal ID: %I64x\n", JournalData.UsnJournalID);
//	printf("FirstUsn: %I64x\n\n", JournalData.FirstUsn);
//
//	for (I = 0; I <= 10; I++)
//	{
//		memset(Buffer, 0, BUF_LEN);
//
//		if (!DeviceIoControl(hVol,
//			FSCTL_READ_USN_JOURNAL,
//			&ReadData,
//			sizeof(ReadData),
//			&Buffer,
//			BUF_LEN,
//			&dwBytes,
//			NULL))
//		{
//			printf("Read journal failed (%d)\n", GetLastError());
//			return;
//		}
//
//		dwRetBytes = dwBytes - sizeof(USN);
//
//		// Find the first record
//		UsnRecord = (PUSN_RECORD)(((PUCHAR)Buffer) + sizeof(USN));
//
//		printf("****************************************\n");
//
//		// This loop could go on for a long time, given the current buffer size.
//		while (dwRetBytes > 0)
//		{
//			//printf("USN: %I64x\n", UsnRecord->Usn);
//			printf("File name: %.*S\n",
//				UsnRecord->FileNameLength / 2,
//				UsnRecord->FileName);
//			//printf("Reason: %x\n", UsnRecord->Reason);
//			//printf("Ref: %I64u\n", UsnRecord->FileReferenceNumber);
//			//printf("Pref: %I64u\n", UsnRecord->ParentFileReferenceNumber);
//			//printf("\n");
//
//			dwRetBytes -= UsnRecord->RecordLength;
//
//			// Find the next record
//			UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) +
//				UsnRecord->RecordLength);
//		}
//		// Update starting USN for next call
//		ReadData.StartUsn = *(USN*)&Buffer;
//	}
//
//	CloseHandle(hVol);
//
//}