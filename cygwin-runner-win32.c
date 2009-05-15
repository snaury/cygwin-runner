#include <windows.h>
#include <stdio.h>

char* get_module_filename(HINSTANCE hInstance)
{
    char buffer[MAX_PATH+1];
    buffer[0] = '\0';
    GetModuleFileName(hInstance, buffer, MAX_PATH);
    buffer[MAX_PATH] = '\0';
    return strdup(buffer);
}

typedef struct {
    char* subkey;
    char* value;
} cygwin_registry_entry;

cygwin_registry_entry cygwin_registry_entries[] = {
#ifdef CYGWIN_1_7
    { "Software\\Cygwin\\setup", "rootdir" },
#else
    { "Software\\Cygnus Solutions\\Cygwin\\mounts v2\\/", "native" },
#endif
    { 0, 0 }
};
HKEY cygwin_registry_roots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };

char* find_cygwin_root()
{
    int i;
    cygwin_registry_entry* entry;
    for (entry = cygwin_registry_entries; entry->subkey && entry->value; ++entry) {
        for (i = 0; i < 2; ++i) {
            HKEY hkey = 0;
            DWORD dwType, cbData;
            LPBYTE lpData;
            if (RegOpenKeyEx(cygwin_registry_roots[i], entry->subkey, 0, KEY_READ, &hkey))
                continue;
            if (RegQueryValueEx(hkey, entry->value, NULL, &dwType, NULL, &cbData) || dwType != REG_SZ || !cbData) {
                RegCloseKey(hkey);
                continue;
            }
            lpData = malloc(cbData + 1);
            if (RegQueryValueEx(hkey, entry->value, NULL, &dwType, lpData, &cbData) || dwType != REG_SZ || !cbData) {
                free(lpData);
                RegCloseKey(hkey);
                continue;
            }
            lpData[cbData] = 0;
            RegCloseKey(hkey);
            return (char*)lpData;
        }
    }
    return NULL;
}

/* Taken from Tcl sources */
static BOOL HasConsole()
{
    HANDLE handle;

    handle = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
	return TRUE;
    } else {
        return FALSE;
    }
}

/* Based on Tcl sources */
static int run(char* command, int* orc)
{
    HANDLE hProcess;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD status;

    hProcess = GetCurrentProcess();
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdInput  = INVALID_HANDLE_VALUE;
    si.hStdOutput = INVALID_HANDLE_VALUE;
    si.hStdError  = INVALID_HANDLE_VALUE;
    DuplicateHandle(hProcess, GetStdHandle(STD_INPUT_HANDLE),  hProcess, &si.hStdInput,  0, TRUE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(hProcess, GetStdHandle(STD_OUTPUT_HANDLE), hProcess, &si.hStdOutput, 0, TRUE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(hProcess, GetStdHandle(STD_ERROR_HANDLE),  hProcess, &si.hStdError,  0, TRUE, DUPLICATE_SAME_ACCESS);

    if (!CreateProcess(NULL, command, NULL, NULL, TRUE, HasConsole() ? 0 : DETACHED_PROCESS, NULL, NULL, &si, &pi))
        return -1;

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);
    if (si.hStdInput != INVALID_HANDLE_VALUE)  CloseHandle(si.hStdInput);
    if (si.hStdOutput != INVALID_HANDLE_VALUE) CloseHandle(si.hStdOutput);
    if (si.hStdError != INVALID_HANDLE_VALUE)  CloseHandle(si.hStdError);

    WaitForSingleObject(pi.hProcess, INFINITE);
    if (!GetExitCodeProcess(pi.hProcess, &status)) {
        CloseHandle(pi.hProcess);
        return -1;
    }
    CloseHandle(pi.hProcess);
    if (orc)
        *orc = (int)status;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HANDLE hFile;
    LPSTR lpModuleFileName = get_module_filename(hInstance);
    LPSTR lpCygwinRoot = find_cygwin_root();
    LPSTR lpCygwinRunner;
    LPSTR lpCygwinTarget = NULL;
    LPSTR lpFullCmd;
    DWORD dwFullCmdSize;
    int rc = 127;
    char* p;

    if (!lpCygwinRoot) {
        fprintf(stderr, "Error: Unable to find cygwin root\n");
        return rc;
    }
    //printf("cygwin root: %s\n", lpCygwinRoot);

    lpCygwinRunner = malloc(strlen(lpCygwinRoot) + strlen(CYGWIN_RUNNER) + 2);
    strcpy(lpCygwinRunner, lpCygwinRoot);
    if (CYGWIN_RUNNER[0] == '/' || CYGWIN_RUNNER[0] == '\\') {
        for (p = lpCygwinRunner + strlen(lpCygwinRunner); p != lpCygwinRunner; --p) {
            if (p[-1] == '/' || p[-1] == '\\')
                p[-1] = '\0';
            else
                break;
        }
    } else {
        p = lpCygwinRunner + strlen(lpCygwinRunner);
        if (p != lpCygwinRunner && (p[-1] != '/' && p[-1] != '\\')) {
            *p++ = '\\';
            *p = '\0';
        }
    }
    strcat(lpCygwinRunner, CYGWIN_RUNNER);
    for (p = lpCygwinRunner; *p; ++p)
        if (*p == '/')
            *p = '\\';
    //printf("cygwin runner: %s\n", lpCygwinRunner);

    if ((hFile = CreateFile(lpModuleFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to open %s\n", lpModuleFileName);
        return rc;
    }
    {
        DWORD lenoffset = SetFilePointer(hFile, -4, NULL, FILE_END);
        DWORD nread = 0;
        LONG size = 0;
        char* p;
        if (ReadFile(hFile, &size, 4, &nread, NULL) && nread == 4 && size > 0) {
            if (SetFilePointer(hFile, -size-4, NULL, FILE_END) == lenoffset - size) {
                p = malloc(size + 1);
                if (ReadFile(hFile, p, size, &nread, NULL) && nread == size) {
                    p[size] = '\0';
                    lpCygwinTarget = p;
                } else
                    free(p);
            }
        }
    }
    CloseHandle(hFile);

    dwFullCmdSize = strlen(lpCygwinRunner) + (lpCygwinTarget ? 1 + strlen(lpCygwinTarget) : 0) + 1 + strlen(lpCmdLine);
    lpFullCmd = malloc(dwFullCmdSize + 1);
    strcpy(lpFullCmd, lpCygwinRunner);
    if (lpCygwinTarget) {
        strcat(lpFullCmd, " ");
        strcat(lpFullCmd, lpCygwinTarget);
    }
    strcat(lpFullCmd, " ");
    strcat(lpFullCmd, lpCmdLine);
    //printf("full command: %s\n", lpFullCmd);

    if (run(lpFullCmd, &rc))
        fprintf(stderr, "Error executing: %s\n", lpFullCmd);

    return rc;
}
