#include "main.h"

#define SAMP_DLL		"samp.dll"
#define SAMP_CMP_03DR1	"C70424C97E9C8F442430"
#define SAMP_CMP_03DR2	"0074201000A30B100073"

uint32_t						g_dwSAMP_Addr = NULL;
int								iIsSAMPSupported;
int								g_renderSAMP_initSAMPstructs = 0;
stSAMP							*g_SAMP = NULL;
stPlayerPool					*g_Players = NULL;
stVehiclePool					*g_Vehicles = NULL;
stChatInfo						*g_Chat = NULL;
stInputInfo						*g_Input = NULL;
stKillInfo						*g_DeathList = NULL;

void getSamp ()
{

	uint32_t	samp_dll = getSampAddress();

	if ( samp_dll != NULL )
	{
		g_dwSAMP_Addr = ( uint32_t ) samp_dll;

		if ( g_dwSAMP_Addr != NULL )
		{
			if ( memcmp_safe((uint8_t *)g_dwSAMP_Addr + 0xBABE, hex_to_bin(SAMP_CMP_03DR1), 10) )
			{
				//strcpy(g_szSAMPVer, "SA:MP 0.3d");
				//Log( "%s was detected. g_dwSAMP_Addr: 0x%p", g_szSAMPVer, g_dwSAMP_Addr );
				iIsSAMPSupported = 1;
			}
			else if ( memcmp_safe((uint8_t *)g_dwSAMP_Addr + 0xBABE, hex_to_bin(SAMP_CMP_03DR2), 10) )
			{
				//strcpy(g_szSAMPVer, "SA:MP 0.3d-R2");
				//Log( "%s was detected. g_dwSAMP_Addr: 0x%p", g_szSAMPVer, g_dwSAMP_Addr );

				// (0.3d-R2 temp) disable AC
				if(memcmp_safe((uint32_t *)(g_dwSAMP_Addr + 0x8F210), "\xE9\x34\x91\x24\x00", 5))
					memset_safe((uint32_t *)(g_dwSAMP_Addr + 0x8F210), 0xC3, 1);

				iIsSAMPSupported = 0;
				//set.basic_mode = true;
				g_dwSAMP_Addr = NULL;
			}
			else
			{
				//Log( "Unknown SA:MP version. Running in basic mode." );
				iIsSAMPSupported = 0;
				//set.basic_mode = true;

				g_dwSAMP_Addr = NULL;
			}
		}
	}
	else
	{
		iIsSAMPSupported = 0;
		//set.basic_mode = true;
		//Log( "samp.dll not found. Running in basic mode." );
	}

	return;
}
int load = 0;

uint32_t getSampAddress ()
{
	/*if ( set.run_mode == RUNMODE_SINGLEPLAYER )
		return 0x0;*/

	uint32_t	samp_dll;

	/*if ( set.run_mode == RUNMODE_SAMP )
	{*/
		if ( load )
		{
			HMODULE temp = LoadLibrary( SAMP_DLL );
			__asm mov samp_dll, eax
		}
		else
		{
			void	*temp = dll_baseptr_get( SAMP_DLL );
			__asm mov samp_dll, eax
		}
	//}

	if ( samp_dll == NULL )
		return 0x0;

	return samp_dll;
}


struct stSAMP *stGetSampInfo ( void )
{
	if ( g_dwSAMP_Addr == NULL )
		return NULL;

	uint32_t	info_ptr;
	info_ptr = ( UINT_PTR ) * ( uint32_t * ) ( (uint8_t *) (void *)((uint8_t *)g_dwSAMP_Addr + SAMP_INFO_OFFSET) );
	if ( info_ptr == NULL )
		return NULL;

	return (struct stSAMP *)info_ptr;
}

struct stChatInfo *stGetSampChatInfo ( void )
{
	if ( g_dwSAMP_Addr == NULL )
		return NULL;

	uint32_t	chat_ptr;
	chat_ptr = ( UINT_PTR ) * ( uint32_t * ) ( (uint8_t *) (void *)((uint8_t *)g_dwSAMP_Addr + SAMP_CHAT_INFO_OFFSET) );
	if ( chat_ptr == NULL )
		return NULL;

	return (struct stChatInfo *)chat_ptr;
}

struct stInputInfo *stGetInputInfo ( void )
{
	if ( g_dwSAMP_Addr == NULL )
		return NULL;

	uint32_t	input_ptr;
	input_ptr = ( UINT_PTR ) * ( uint32_t * ) ( (uint8_t *) (void *)((uint8_t *)g_dwSAMP_Addr + SAMP_CHAT_INPUT_INFO_OFFSET) );
	if ( input_ptr == NULL )
		return NULL;

	return (struct stInputInfo *)input_ptr;
}

struct stKillInfo *stGetKillInfo ( void )
{
	if ( g_dwSAMP_Addr == NULL )
		return NULL;

	uint32_t	kill_ptr;
	kill_ptr = ( UINT_PTR ) * ( uint32_t * ) ( (uint8_t *) (void *)((uint8_t *)g_dwSAMP_Addr + SAMP_KILL_INFO_OFFSET) );
	if ( kill_ptr == NULL )
		return NULL;

	return (struct stKillInfo *)kill_ptr;
}