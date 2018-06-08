#pragma once
#include <string>
#include <map>
#include <iostream>
#include <vector>

#include "Finder.h"

#define MAX_POSSIBILITIES 128

std::string strip(char* string, char stripChar, int sizeOfString);

std::vector<std::wstring> split(const wchar_t* string, wchar_t splitChar);
int indexOf(char* string, char character);

int strlen(char* string);
int strlen(const wchar_t* string);

bool hasSuffix(const std::wstring &str, const std::wstring &suffix);
HANDLE clearScreenAndResetCursor();
void listIndexedFiles(std::multimap<std::wstring, std::wstring> &indexedFiles);
void listPossibilities(std::vector<std::multimap<std::wstring, std::wstring>::iterator> *possibilities, const std::string *input);
void listIndexDirectories(std::vector<std::wstring> directories);
std::vector<std::multimap<std::wstring, std::wstring>::iterator> listIndexedPossibilities(std::multimap<std::wstring, std::wstring> *indexedFiles, const std::wstring *input);
std::string getAppdataLocation();
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& s);