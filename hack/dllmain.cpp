#include "main.h"
#include <string>

#include <windows.h>
#include <winternl.h>
#include <stdio.h>

DWORD g_dwSAMP_Addr = NULL;
stTextdrawPool* g_Textdraws = NULL;
/* 
 * PEB Functions
 */
void DbgPrint(const char* lpcFmt, ...){
	char szBuf[0x4000];
	va_list vaArgs;
	va_start(vaArgs, lpcFmt);
	_vsnprintf(szBuf, sizeof(szBuf), lpcFmt, vaArgs);
	va_end(vaArgs);
	OutputDebugStringA (szBuf);
}

typedef struct _PEB_LDR_DATA_C
{
     ULONG Length;
     UCHAR Initialized;
     PVOID SsHandle;
     LIST_ENTRY InLoadOrderModuleList;
     LIST_ENTRY InMemoryOrderModuleList;
     LIST_ENTRY InInitializationOrderModuleList;
     PVOID EntryInProgress;
} PEB_LDR_DATA_C, *PPEB_LDR_DATA_C;

typedef struct _LDR_MODULE { 

  LIST_ENTRY              InLoadOrderModuleList; 
  LIST_ENTRY              InMemoryOrderModuleList; 
  LIST_ENTRY              InInitializationOrderModuleList; 
  PVOID                   BaseAddress; 
  PVOID                   EntryPoint; 
  ULONG                   SizeOfImage; 
  UNICODE_STRING          FullDllName; 
  UNICODE_STRING          BaseDllName; 
  ULONG                   Flags; 
  SHORT                   LoadCount; 
  SHORT                   TlsIndex; 
  LIST_ENTRY              HashTableEntry; 
  ULONG                   TimeDateStamp; 

} LDR_MODULE, *PLDR_MODULE; 

DWORD GetPEB() 
{ 
    DWORD* dwPebBase = NULL; 
    /* Return PEB address for current process 
       address is located at FS:0x30 */ 
        __asm  
        { 
            push eax 
            mov eax, FS:[0x30] 
            mov [dwPebBase], eax 
            pop eax 
        } 
    return (DWORD)dwPebBase; 
}  

/*  Walks one of the three modules double linked lists referenced by the  
PEB  (error check stripped) 
ModuleListType is an internal flag to determine on which list to operate : 
LOAD_ORDER_TYPE <---> InLoadOrderModuleList //does not exist
MEM_ORDER_TYPE  <---> InMemoryOrderModuleList 
INIT_ORDER_TYPE <---> InInitializationOrderModuleList //does not exist
*/ 
void HideModule(HMODULE hHideModule) 
{ 
    PPEB pPeb = (PPEB) GetPEB();
    PPEB_LDR_DATA_C pLdr = (PPEB_LDR_DATA_C)pPeb->Ldr;
    PLDR_MODULE pModule = (PLDR_MODULE) pLdr->InLoadOrderModuleList.Flink;
    PLDR_MODULE pFirstModule = (PLDR_MODULE) &pLdr->InLoadOrderModuleList;
    LIST_ENTRY le;

    do 
    {
        if (pModule->BaseAddress == (PVOID)hHideModule)
        {
            memcpy(&le,&pModule->InInitializationOrderModuleList,sizeof(le));
            pModule->InInitializationOrderModuleList.Blink->Flink = le.Flink;
            pModule->InInitializationOrderModuleList.Flink->Blink = le.Blink;

            memcpy(&le,&pModule->InLoadOrderModuleList,sizeof(le));
            pModule->InLoadOrderModuleList.Blink->Flink = le.Flink;
            pModule->InLoadOrderModuleList.Flink->Blink = le.Blink;

            memcpy(&le,&pModule->InMemoryOrderModuleList,sizeof(le));
            pModule->InMemoryOrderModuleList.Blink->Flink = le.Flink;
            pModule->InMemoryOrderModuleList.Flink->Blink = le.Blink;
        }

        pModule = (PLDR_MODULE) pModule->InLoadOrderModuleList.Flink;
    } while(pFirstModule != pModule);
}  

void SAMP()
{
	if ( !g_renderSAMP_initSAMPstructs )
	{
		g_SAMP = stGetSampInfo();
		if ( isBadPtr_writeAny(g_SAMP, sizeof(stSAMP)) )
			return;

		if ( isBadPtr_writeAny(g_SAMP->pPools, sizeof(stSAMPPools)) )
			return;

		g_Textdraws = g_SAMP->pPools->pPool_Textdraw;
		if ( isBadPtr_writeAny(g_Textdraws, sizeof(stPlayerPool)) )
			return;

		g_Players = g_SAMP->pPools->pPool_Player;
		if ( isBadPtr_writeAny(g_Players, sizeof(stPlayerPool)) )
			return;

		g_Vehicles = g_SAMP->pPools->pPool_Vehicle;
		if ( isBadPtr_writeAny(g_Vehicles, sizeof(stVehiclePool)) )
			return;

		g_Chat = stGetSampChatInfo();
		if ( isBadPtr_writeAny(g_Chat, sizeof(stChatInfo)) )
			return;

		g_Input = stGetInputInfo();
		if ( isBadPtr_writeAny(g_Input, sizeof(stInputInfo)) )
			return;

		g_DeathList = stGetKillInfo();
		if ( isBadPtr_writeAny(g_DeathList, sizeof(stKillInfo)) )
			return;

		////init modCommands
		////if ( set1.mod_commands_activated )
		//	init_samp_chat_cmds();

		// patch
		memcpy_safe((void *)(g_dwSAMP_Addr + SAMP_PATCH_NOCARCOLORRESETTING), "\xC3", 1);

		g_renderSAMP_initSAMPstructs = 1;
	}
}

DWORD WINAPI processHack()
{
	getSamp();
retry:
	SAMP();
	if(g_renderSAMP_initSAMPstructs == 0)
	{
		goto retry;
	}

	while(true)
	{

		SAMP();
		for(int i = 0; i< MAX_TEXTDRAW;i++)
		{
			if(g_Textdraws->textdraw[i] != NULL)
			{
				//if(std::string(g_Textdraws->textdraw[i]->text).find("code") != -1)
				{
					MessageBox(NULL,g_Textdraws->textdraw[i]->text,"Textdraw",0);
				}
			}
		}
	}
	return 0;
}


DWORD WINAPI OnAttach (LPVOID lpReserved)
{
	/*HMODULE hModule = (HMODULE)lpReserved;
	DbgPrint ("Thread Created. Base Address: %X\n", hModule);
	UnlinkModule ("GameMon.dll");
	DbgPrint ("Module should now be unlinked.\n");
	UnlinkModule ("GameMon.dll");*/
	
	/*getSampAddress();
	getSamp();
	stSAMP* samp = stGetSampInfo();*/

	while (true)
	{
		/*for(int i = 0; i < 2048;i++)
		{
			if(samp->pPools->pPool_Textdraw->textdraw[i] != NULL)
				memcpy(samp->pPools->pPool_Textdraw->textdraw[i]->text," ",1);
		}*/
		Sleep (1);
	}
	return 0;
}

BOOL WINAPI DllMain (HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			HideModule(hModule);
			CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)processHack,NULL,NULL,NULL); // start the hack thread
			break;
		case DLL_PROCESS_DETACH:
			DbgPrint ("Module is being detached now.\n");
			break;
	}

	return TRUE;
}