// #include <winioctl.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <cstring>
#include "DiskControllerManager.h"

using namespace std;

int main() {
	DiskControllerManager* manager = DiskControllerManager::getInstance();
	if (manager->addDisk('D')) {
		cout << "OK" << endl;
	}
	return 0;
}