#pragma once
#include <Windows.h>
#include <string>
#include "Helper.h"

#define DEBUG 0

struct INDEXED_FILE {
	std::string filename = "";
	DWORD fileAttributes;
};

int createIndexFile();
std::vector<std::wstring> readIndexFile();
std::vector<std::wstring> readFiletypesFile();
void readDirectory(std::wstring directory);
void readDirectory(std::vector<std::wstring> directories);
INDEXED_FILE *setFileData(WIN32_FIND_DATA &findData);