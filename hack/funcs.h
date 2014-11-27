
bool								isBadPtr_handlerAny ( void *pointer, ULONG size, DWORD dwFlags );
bool								isBadPtr_readAny ( void *pointer, ULONG size );
bool								isBadPtr_writeAny ( void *pointer, ULONG size );


int									memcpy_safe ( void *dest, const void *src, uint32_t len, int check = NULL, const void *checkdata = NULL);
int									memset_safe ( void *_dest, int c, uint32_t len );
int									memcmp_safe ( const void *_s1, const void *_s2, uint32_t len );

void								*dll_baseptr_get ( const char *dll_name );

uint8_t								*hex_to_bin ( const char *str );