#include <Windows.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <iostream>

#pragma comment(lib, "shlwapi.lib")

#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5

//   Pietrek's macro
//
//   MakePtr is a macro that allows you to easily add to values (including
//   pointers) together without dealing with C's pointer arithmetic.  It
//   essentially treats the last two parameters as DWORDs.  The first
//   parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

//   This one is mine, but obviously..."adapted" from matt's original idea =p
#define MakeDelta(cast, x, y) (cast) ( (DWORD_PTR)(x) - (DWORD_PTR)(y))

bool MapRemoteModule(unsigned long, char *);

unsigned long GetProcessIdByName(char *);
HMODULE GetRemoteModuleHandle(unsigned long, char *);
FARPROC GetRemoteProcAddress(unsigned long, char *, char *);

bool FixImports(unsigned long, void *, IMAGE_NT_HEADERS *, IMAGE_IMPORT_DESCRIPTOR *);
bool FixRelocs(void *, void *, IMAGE_NT_HEADERS *, IMAGE_BASE_RELOCATION *, unsigned int);
bool MapSections(HANDLE, void *, void *, IMAGE_NT_HEADERS *);

PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD, PIMAGE_NT_HEADERS);
LPVOID GetPtrFromRVA(DWORD, PIMAGE_NT_HEADERS, PBYTE);