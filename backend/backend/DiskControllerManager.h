#pragma once
#include "DiskController.h"
#include <unordered_map>
#include <string>
#include <cstring>
#include <algorithm>

class DiskControllerManager
{
public:
	static DiskControllerManager* getInstance();

	bool addDisk(char diskName);

private:
	DiskControllerManager() {}

	static DiskControllerManager* singleton;

	std::unordered_map<char, DiskController*> disk_base;
};

