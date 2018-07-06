#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <map>
#include <iostream>
#include <strsafe.h>
#include <Commctrl.h>
#include <algorithm>

#include "resource.h"
#include "Finder.h"

//TODO: Make listPossibilties ignore case
//TODO: Sort program options based on Last Accessed time maybe?
//TODO: Add a way to remove index and filetype
//TODO: Add a timer to readIndexFile?
//TODO: Create the Finder folder in appdata if no other one exists
//TODO: Make -list take a parameter like start of .exe name. i.e., -list Stea returns Steam.exe, SteamReporter.exe...
//      Still can do -list and return all files.
//TODO: Make more Helper functions use strings instead of char*?

//TODO: Make WriteFile have Unicode support. Currently wstrings are being converted back to strings.

std::vector<std::wstring> filetypes;
std::vector<std::wstring> indexDirectories;
std::vector<std::map<std::wstring, std::wstring>::iterator> possiblities;
std::multimap<std::wstring, std::wstring> indexedFiles;

static bool windowHidden = false;
static bool keepRunning;
static bool needClear = true;

// Used for System Tray Icon
static UUID guid;


int createIndexFile() {
	// Have to use s2ws anywhere that needs an LPCWSTR. Freaking Unicode.
	
	HANDLE file;
	DWORD error;

	std::string indexpath = getAppdataLocation();
	std::string filetypepath = getAppdataLocation();

	indexpath += "\\Finder\\index.txt";
	filetypepath += "\\Finder\\filetypes.txt";
	
	file = CreateFileW(s2ws(filetypepath).c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	error = GetLastError();
	CloseHandle(file);

	file = CreateFile(s2ws(indexpath).c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	error = GetLastError();

	CloseHandle(file);

	// FILE_ALREADY_EXISTS
	if (error != 183) {
		return 0;
	}

	return 1;
}

std::vector<std::wstring> readIndexFile() {
	// Read index.txt so that we can search the directories that the user specified.

	std::string filepath = getAppdataLocation();
	filepath += "\\Finder\\index.txt";

	HANDLE index = CreateFile(s2ws(filepath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileSize = GetFileSize(index, NULL);
	DWORD error = GetLastError();

	char* indexLocations = (char*) malloc(sizeof(char*) * fileSize);
	int indexRead = ReadFile(index, indexLocations, fileSize, NULL, NULL);
	
	CloseHandle(index);

	std::vector<std::wstring> indicies;

	if (indexLocations == NULL)
		return indicies;

	if (indexRead == 0)
	{
		OutputDebugString(L"Read Index Error\n");		
		return indicies;
	}

	*(indexLocations + fileSize) = L'\0';

	std::string indexList;
	if (strcmp(indexLocations, "") != 0) {
		indexList = strip(indexLocations, L'\r', fileSize);
		indicies = split(s2ws(indexList).c_str(), L'\n');

		free(indexLocations);
	}

	return indicies;
}

std::vector<std::wstring> readFiletypesFile() {
	std::string filepath = getAppdataLocation();
	filepath += "\\Finder\\filetypes.txt";

	HANDLE filetype = CreateFile(s2ws(filepath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileSize = GetFileSize(filetype, NULL);
	DWORD error;

	char* filetypes = (char*)malloc(sizeof(char*) * fileSize);
	int filetypeRead = ReadFile(filetype, filetypes, fileSize, NULL, NULL);

	CloseHandle(filetype);

	std::vector<std::wstring> filetypesVector;

	if (filetypes == NULL)
		return filetypesVector;

	if (filetypeRead == 0)
	{
		OutputDebugString(L"Read Filetypes Error");
		error = GetLastError();

		return filetypesVector;
	}

	*(filetypes + fileSize) = '\0';

	std::string filetypesList;
	if (strcmp(filetypes, "") != 0) {
		filetypesList = strip(filetypes, L'\r', fileSize);
		filetypesVector = split(s2ws(filetypesList).c_str(), L'\n');

		free(filetypes);
	}

	return filetypesVector;
}

void appendIndexFile(std::wstring append) {

	if (std::find(indexDirectories.begin(), indexDirectories.end(), append) != indexDirectories.end()) {
		std::wstring open = L"Directory is already indexed\n\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)open.c_str(), open.length(), NULL, NULL);
		
		// TODO: Figure out a way to make this not clear everytime
		//      Should not be this hard!

		return;
	}

	WIN32_FIND_DATA findData;
	auto hFind = FindFirstFile(append.c_str(), &findData);
	if (INVALID_HANDLE_VALUE == hFind) {
		// File does not exist
		std::wstring open = L"Folder does not exist\n\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)open.c_str(), open.length(), NULL, NULL);

		// TODO: Figure out a way to make this not clear everytime
		//      Should not be this hard!

		return;
	}

	HANDLE file;
	DWORD error;
	DWORD dwBytesWritten;
	
	std::string filepath = getAppdataLocation();
	filepath += "\\Finder\\index.txt";

	file = CreateFile(s2ws(filepath).c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	error = GetLastError();

	if (file == INVALID_HANDLE_VALUE)
		return;

	append += L"\r\n";

	DWORD dwPos = SetFilePointer(file, 0, NULL, FILE_END);
	
	int a = strlen(append.c_str());

	bool success = WriteFile(file, ws2s(append).c_str(), strlen(ws2s(append).c_str()), &dwBytesWritten, NULL);

	CloseHandle(file);

	indexDirectories = readIndexFile();
}

void appendFiletypes(std::wstring filetype) {
	HANDLE file;
	DWORD error;
	DWORD dwBytesWritten;

	std::string filepath = getAppdataLocation();
	filepath += "\\Finder\\filetypes.txt";

	file = CreateFile(s2ws(filepath).c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	error = GetLastError();

	if (file == INVALID_HANDLE_VALUE)
		return;

	filetype += L"\r\n";

	DWORD dwPos = SetFilePointer(file, 0, NULL, FILE_END);

	WriteFile(file, ws2s(filetype).c_str(), strlen(ws2s(filetype).c_str()), &dwBytesWritten, NULL);
	error = GetLastError();

	CloseHandle(file);

	filetypes = readFiletypesFile();
}

void readDirectory(std::wstring directory) {
	WIN32_FIND_DATA findData;
	HANDLE hFind = NULL;
	DWORD dwError;

	hFind = FindFirstFile(directory.c_str(), &findData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
#if DEBUG
		OutputDebugString(L"Unable to look in directory: ");
		OutputDebugString(directory.c_str());
		OutputDebugString(L"\n");
		dwError = GetLastError();
#endif
	}

	int i = 0, j = 0;
	int isAnotherFile = 0;

	do {
		if (i == 0 || i == 1) {
			i = i + 1;
		}
		else {
			std::wstring filename(findData.cFileName);
			std::wstring fullPath = directory.substr(0, directory.length() - 1) + findData.cFileName;

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
#if DEBUG
				OutputDebugString(L"FOUND A DIR ");
				OutputDebugString(findData.cFileName);
				OutputDebugString(L"\n");
#endif			

				fullPath = fullPath + L"/*";
				readDirectory(fullPath);
			}
			else {
				for (auto i = filetypes.begin(); i != filetypes.end(); i++) {
					if (findData.dwFileAttributes && hasSuffix(findData.cFileName, i->data())) {
#if DEBUG
						OutputDebugString(L"FOUND AN IMPORTANT FILE ");
						OutputDebugString(findData.cFileName);
						OutputDebugString(L"\n");
#endif	
						std::wstring filename(findData.cFileName);
						filename = filename.substr(0, filename.length() - strlen(i->data()));

						indexedFiles.insert(std::make_pair(filename, fullPath));
					}
				}
			}
		}

		isAnotherFile = FindNextFile(hFind, &findData);
	} while (isAnotherFile);

	FindClose(hFind);
}


void readDirectory(std::vector<std::wstring> directories) {
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	std::wstring statusText = L"Reading directory: ";
	WriteConsole(console, (CHAR_INFO*)statusText.c_str(), statusText.length(), NULL, NULL);

	COORD newCoords;

	std::wstring spaces = L" ";

	for (int i = 0; i < directories.size(); i++) {
		std::string	statusNumber = std::to_string(i) + " of " + std::to_string(directories.size());

		CONSOLE_SCREEN_BUFFER_INFO screen;
		GetConsoleScreenBufferInfo(console, &screen);
		newCoords.X = screen.dwCursorPosition.X - directories.size() - 4 - i; // Resets x of line to point to the space at the end of statusText
		newCoords.Y = screen.dwCursorPosition.Y;


		WriteConsole(console, (CHAR_INFO*)s2ws(statusNumber).c_str(), statusNumber.length(), NULL, NULL);
		resetCurosr(newCoords);

		readDirectory(directories.at(i));
		// TODO: Find a way to reset cursor back so I can update the text for each new directory scanned.
		//		 Still an issue with this.
	}
}

INDEXED_FILE *setFileData(WIN32_FIND_DATA &findData) {
	INDEXED_FILE *help = new INDEXED_FILE();

	help->filename = (char*) findData.cFileName;
	help->fileAttributes = findData.dwFileAttributes;

	return help;
}

HINSTANCE readConsoleInputAndLaunchProgram() {
	std::wstring open = L"Enter a program to open: \n";
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)open.c_str(), open.length(), NULL, NULL);
	std::string input;
	std::getline(std::cin, input);

	HINSTANCE success = 0;
	// Handles commands
	if (strcmp(input.c_str(), "") == 0) {
		std::wstring launch(possiblities.at(0)->second);
		success = ShellExecute(NULL, NULL, launch.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		return success;
	}
	else if (strcmp(input.c_str(), "-list") == 0) {
		clearScreenAndResetCursor();
		listIndexedFiles(indexedFiles);
		needClear = false;
		return NULL;
	}
	else if (strcmp(input.c_str(), "-reload") == 0) {
		
		std::wstring reloading = L"Reading Filetypes File...\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)reloading.c_str(), reloading.length(), NULL, NULL);
		filetypes = readFiletypesFile();
		
		reloading = L"Reading Index File...\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)reloading.c_str(), reloading.length(), NULL, NULL);
		indexDirectories = readIndexFile();
		
		reloading = L"Reading Directories...\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)reloading.c_str(), reloading.length(), NULL, NULL);
		readDirectory(indexDirectories);
		needClear = true;
		return NULL;
	}
	else if (input.find("-add index") != std::string::npos) {
		auto append = split(s2ws(input.substr(11, input.length() - 1)).c_str(), ';'); // 11 is the amount of characters before we want to start checking args.

		for (int i = 0; i < append.size(); i++)
			appendIndexFile(append.at(i));
		
		return NULL;
	} 
	else if (input.find("-add filetype") != std::string::npos) {
		auto append = split(s2ws(input.substr(14, input.length() - 1)).c_str(), ';'); // 14 is the amount of characters before we want to start checking args.

		for (int i = 0; i < append.size(); i++)
			appendFiletypes(append.at(i));

		return NULL;
	}
	else if (strcmp(input.c_str(), "-quit") == 0) {
		keepRunning = false;
		return NULL;
	}
	else if (strcmp(input.c_str(), "-index") == 0) {
		listIndexDirectories(indexDirectories);
		needClear = false;
		return NULL;
	}

	bool couldNotLaunch = false;

	possiblities = listIndexedPossibilities(&indexedFiles, &s2ws(input));
	if (possiblities.size() == 1) {
		ShowWindow(GetConsoleWindow(), 0);
		windowHidden = true;
		std::wstring launch(possiblities.at(0)->second);
		success = ShellExecute(NULL, NULL, launch.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}
	else if (possiblities.size() >= 2) {
		listPossibilities(&possiblities, &input);
		readConsoleInputAndLaunchProgram();
	}
	else {
		OutputDebugString(L"Could not launch program\n");
		// TODO: Add a better way to handle the program not existing
		std::wstring error = s2ws(input) + L" does not exist. Please search again.\n";
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*) error.c_str(), error.length(), NULL, NULL);
		couldNotLaunch = true;
		needClear = false;
	}
	
	if (!windowHidden && !couldNotLaunch) {
		ShowWindow(GetConsoleWindow(), 0);
		windowHidden = true;
	}

	return success;
}


void addSystemTray() {
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetConsoleWindow();
	nid.uFlags = NIF_ICON | NIF_GUID;
	nid.guidItem = (GUID) guid;
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"Finder");

	// Add icon
	LoadIconMetric(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), LIM_SMALL, &nid.hIcon);

	Shell_NotifyIcon(NIM_ADD, &nid) ? S_OK : E_FAIL;
}

int main() {
	SetConsoleTitle(L"Finder");
	if (RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'F'))
		OutputDebugString(L"Registered hot key\n");
	else
		OutputDebugString(L"Hotkey could not be registered\n");

	UuidCreate(&guid);
	addSystemTray();

	filetypes.reserve(64);

	int create = createIndexFile();

	wchar_t* reading = L"Reading Filetypes File...\n";
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)reading, strlen(reading), NULL, NULL);
	filetypes = readFiletypesFile();

	reading = L"Reading Index File...\n";
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), (CHAR_INFO*)reading, strlen(reading), NULL, NULL);
	indexDirectories = readIndexFile();

	readDirectory(indexDirectories);

	#if DEBUG
	listIndexedFiles(indexedFiles);
	#endif

	keepRunning = true;
	while (keepRunning) {
		if (windowHidden) {
			MSG msg = { 0 };
			while (GetMessage(&msg, NULL, 0, 0) != 0)
			{
				if (msg.message == WM_HOTKEY)
				{
					OutputDebugString(L"CTRL+ALT+F received\n");
					ShowWindow(GetConsoleWindow(), SW_SHOW);
					SetForegroundWindow(GetConsoleWindow());
					SetFocus(GetConsoleWindow());
					windowHidden = false;
					break;
				}

				if (msg.message == WM_QUIT)
					keepRunning = false;
			}
		}
		else {
			if (needClear) {
				clearScreenAndResetCursor();
			}

			needClear = true;
			readConsoleInputAndLaunchProgram();
		}
	}

	return 0;
}