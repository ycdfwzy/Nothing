#pragma once
#include "FileBase.h"
#include "DiskController.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

class GeneralManager
{
public:
	static GeneralManager* getInstance();

	bool addDisk(char diskName);

	bool save(char diskName = 0) const;

	bool search(const std::wstring&, std::vector<std::wstring>&, CHAR diskName = 0) const;

private:
	GeneralManager() {}

	static GeneralManager* singleton;

	std::unordered_map<char, DiskController*> disk_base;
	std::unordered_map<char, FileBase*> file_base;
};

