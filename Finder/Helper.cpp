#include <Windows.h>
#include <malloc.h>

#include "Helper.h"

COORD consoleCoords = { 0, 0 };

std::string strip(char* string, char stripChar, int sizeOfString) {
	std::string passedString(string), newString = "";
	int i = 0, lastIndex = 0;

	while (i < passedString.length())
	{
		if (passedString.at(i) == stripChar) {
			newString += passedString.substr(lastIndex, i - lastIndex);
			lastIndex = i + 1;
		}

		i++;
	}

	if (newString.compare("") == 0)
		return passedString;

	return newString;
}

std::vector<std::wstring> split(const wchar_t* string, wchar_t splitChar) {
	std::wstring newString(string);
	
	std::vector<std::wstring> indices;
	int lastIndex = 0;

	for (int i = 0; i < newString.length(); i++) {
		if (newString.at(i) == splitChar) {
			indices.push_back(newString.substr(lastIndex, i - lastIndex));
			lastIndex = i + 1;
		}

		if (i == newString.length() - 1 && newString.at(i) != splitChar)
			indices.push_back(newString.substr(lastIndex, i - lastIndex + 1));
	}

	return indices;
}

int indexOf(char* string, char character) {
	int i = 0;
	int len = strlen(string);
	
	while (i < len)
	{
		if (*(string + i) == character)
			break;

		i = i + 1;
	}

	return i;
}

int strlen(char* string) {
	int i = 0;
	while (*(string + i) != '\0')
		i = i + 1;
	return i;
}

int strlen(const wchar_t* string) {
	int i = 0;
	while (*(string + i) != '\0')
		i = i + 1;
	return i;
}

bool hasSuffix(const std::wstring &str, const std::wstring &suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}


HANDLE clearScreenAndResetCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	
	#if !DEBUG
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;
	
	GetConsoleScreenBufferInfo(consoleHandle, &screen);
	FillConsoleOutputCharacter(consoleHandle, L' ', screen.dwSize.X * screen.dwSize.Y, consoleCoords, &written);
	SetConsoleCursorPosition(consoleHandle, consoleCoords);
	#endif

	return consoleHandle;
}

void listIndexedFiles(std::multimap<std::wstring, std::wstring> &indexedFiles) {
	auto consoleHandle = clearScreenAndResetCursor();
	std::wstring list = L"Listing indexed files: \n\n";
	WriteConsole(consoleHandle, list.c_str(), strlen(list.c_str()), NULL, NULL);
	for (auto i = indexedFiles.begin(); i != indexedFiles.end(); i++) {
		WriteConsole(consoleHandle, i->first.c_str(), strlen(i->first.c_str()), NULL, NULL);
		WriteConsole(consoleHandle, L"\n", 1, NULL, NULL);
	}
	WriteConsole(consoleHandle, L"\n", 1, NULL, NULL);
}

void listIndexDirectories(std::vector<std::wstring> directories) {
	auto consoleHandle = clearScreenAndResetCursor();
	std::wstring list = L"Listing indexed directories: \n\n";
	WriteConsole(consoleHandle, list.c_str(), strlen(list.c_str()), NULL, NULL); 
	for (auto i = directories.begin(); i != directories.end(); i++) {
		WriteConsole(consoleHandle, i->data(), strlen(i->data()), NULL, NULL);
		WriteConsole(consoleHandle, L"\n", 1, NULL, NULL);
	}
	WriteConsole(consoleHandle, L"\n", 1, NULL, NULL);
}

void listPossibilities(std::vector<std::multimap<const std::wstring, std::wstring>::iterator> *possibilities, const std::string *input) {
	auto consoleHandle = clearScreenAndResetCursor();
	std::wstring list = L"Program options based on: " + s2ws(input->c_str()) + L"\n\n";
	WriteConsole(consoleHandle, list.c_str(), strlen(list.c_str()), NULL, NULL);
	for (int i = 0; i < possibilities->size(); i++) {
		WriteConsole(consoleHandle, possibilities->at(i)->first.c_str(), strlen(possibilities->at(i)->first.c_str()), NULL, NULL);
		WriteConsole(consoleHandle, L"\n", 1, NULL, NULL);
	}
}


std::vector<std::multimap<std::wstring, std::wstring>::iterator> listIndexedPossibilities(std::multimap<std::wstring, std::wstring> *indexedFiles, const std::wstring *input) {
	std::vector<std::multimap<std::wstring, std::wstring>::iterator> possibilities;
	possibilities.reserve(MAX_POSSIBILITIES);

	for (auto i = indexedFiles->begin(); i != indexedFiles->end(); i++) {
		if (i->first.find(*input) != std::wstring::npos)
			possibilities.push_back(i);
	}

	return possibilities;
}

std::string getAppdataLocation() {
	char* appdata;
	size_t len;
	_dupenv_s(&appdata, &len, "APPDATA");
	std::string appData(appdata);

	return appData;
}

//Used to convert string to wstring -> https://codereview.stackexchange.com/questions/419/converting-between-stdwstring-and-stdstring
//Need to maybe profile this.
std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

//Used to convert wstring to string -> https://codereview.stackexchange.com/questions/419/converting-between-stdwstring-and-stdstring
//Need to maybe profile this.
std::string ws2s(const std::wstring& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	char* buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
	std::string r(buf);
	delete[] buf;
	return r;
}