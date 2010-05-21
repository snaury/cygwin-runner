#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <signal.h>

TCHAR* str2tcs(const char* p)
{
#ifdef _UNICODE
    TCHAR* r;
    const char* pend = p + strlen(p);
    int rlen = MultiByteToWideChar(CP_ACP, 0, p, pend - p + 1, 0, 0);
    r = malloc(rlen * sizeof(TCHAR));
    MultiByteToWideChar(CP_ACP, 0, p, pend - p + 1, r, rlen);
    return r;
#else
    return strdup(p);
#endif
}

TCHAR* get_module_filename(HINSTANCE hInstance)
{
    TCHAR buffer[MAX_PATH+1];
    buffer[0] = _T('\0');
    GetModuleFileName(hInstance, buffer, MAX_PATH);
    buffer[MAX_PATH] = _T('\0');
    return _tcsdup(buffer);
}

TCHAR* dirname(const TCHAR* p)
{
    TCHAR* r;
    const TCHAR* p1 = _tcsrchr(p, _T('\\'));
    const TCHAR* p2 = _tcsrchr(p, _T('/'));
    const TCHAR* pend = p1 > p2 ? p1 : p2;
    if (!pend)
        return _tcsdup(_T(""));
    r = malloc((pend - p + 1) * sizeof(TCHAR));
    memcpy(r, p, (pend - p) * sizeof(TCHAR));
    r[pend - p] = '\0';
    return r;
}

TCHAR* join(const TCHAR* p1, const TCHAR* p2)
{
    TCHAR* r;
    int p1len, p2len;
    const TCHAR* p1end = p1 + _tcslen(p1);
    const TCHAR* p2end = p2 + _tcslen(p2);
    if (p1 == p1end)
        return _tcsdup(p2);
    if (p2 == p2end)
        return _tcsdup(p1);
    while (p1 != p1end && (p1end[-1] == _T('\\') || p1end[-1] == _T('/')))
        --p1end;
    while (p2 != p2end && (p2[0] == _T('\\') || p2[0] == _T('/')))
        ++p2;
    p1len = p1end - p1;
    p2len = p2end - p2;
    r = malloc((p1len + 1 + p2len + 1) * sizeof(TCHAR));
    memcpy(r, p1, p1len * sizeof(TCHAR));
    r[p1len] = _T('\\');
    memcpy(r + p1len + 1, p2, p2len * sizeof(TCHAR));
    r[p1len + 1 + p2len] = _T('\0');
    return r;
}

#ifdef CYGWIN_SUPPORT_REGISTRY
typedef struct {
    TCHAR* subkey;
    TCHAR* value;
} cygwin_registry_entry;

cygwin_registry_entry cygwin_registry_entries[] = {
#ifdef CYGWIN_1_7
    { _T("Software\\Cygwin\\setup"), _T("rootdir") },
#else
    { _T("Software\\Cygnus Solutions\\Cygwin\\mounts v2\\/"), _T("native") },
#endif
    { 0, 0 }
};
HKEY cygwin_registry_roots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };

TCHAR* find_cygwin_root_registry()
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
            lpData = malloc(cbData + sizeof(TCHAR));
            if (RegQueryValueEx(hkey, entry->value, NULL, &dwType, lpData, &cbData) || dwType != REG_SZ || !cbData) {
                free(lpData);
                RegCloseKey(hkey);
                continue;
            }
            *(TCHAR*)(lpData + cbData) = _T('\0');
            RegCloseKey(hkey);
            return (TCHAR*)lpData;
        }
    }
    return NULL;
}
#endif

TCHAR* find_cygwin_root(const TCHAR* module_filename)
{
    TCHAR* module_dir = dirname(module_filename);
    TCHAR* cygwin_root = dirname(module_dir);
    TCHAR* cygwin_dll = join(cygwin_root, _T("bin\\cygwin1.dll"));
    //_tprintf(_T("possible cygwin dll: %s\n"), cygwin_dll);
    if (GetFileAttributes(cygwin_dll) == INVALID_FILE_ATTRIBUTES) {
        free(cygwin_root);
#ifdef CYGWIN_SUPPORT_REGISTRY
        cygwin_root = find_cygwin_root_registry();
#else
        cygwin_root = NULL;
#endif
    }
    free(cygwin_dll);
    free(module_dir);
    return cygwin_root;
}

/* Taken from Tcl sources */
static BOOL HasConsole()
{
    HANDLE handle;

    handle = CreateFile(_T("CONOUT$"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
	return TRUE;
    } else {
        return FALSE;
    }
}

/* Based on Tcl sources */
static int run(TCHAR* command, int* orc)
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

TCHAR* skip_program_name(TCHAR* p)
{
    int dq = 0;
    while (*p > _T(' ') || (*p && dq)) {
        if (*p == _T('"'))
            dq = !dq;
        ++p;
    }
    while (*p && *p <= _T(' '))
        ++p;
    return p;
}

TCHAR* extract_cygwin_target(TCHAR* lpModuleFileName)
{
    TCHAR* result = NULL;
    HANDLE hFile = CreateFile(
        lpModuleFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwFileSize = GetFileSize(hFile, NULL);
        HANDLE hMapping = CreateFileMapping(
            hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMapping != NULL) {
            LPVOID lpExecutable = MapViewOfFile(
                hMapping, FILE_MAP_READ, 0, 0, dwFileSize);
            if (lpExecutable != NULL) {
                IMAGE_DOS_HEADER* dos = lpExecutable;
                if (dos->e_magic == IMAGE_DOS_SIGNATURE) {
                    IMAGE_NT_HEADERS* nt = lpExecutable + dos->e_lfanew;
                    if (nt->Signature == IMAGE_NT_SIGNATURE) {
                        DWORD dwNumSections = nt->FileHeader.NumberOfSections;
                        DWORD dwImageEnd = 0;
                        IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
                        while (dwNumSections--) {
                            DWORD dwSectionEnd = sec->PointerToRawData + sec->SizeOfRawData;
                            if (dwImageEnd < dwSectionEnd)
                                dwImageEnd = dwSectionEnd;
                            ++sec;
                        }
                        if (dwImageEnd <= dwFileSize) {
                            DWORD size = dwFileSize - dwImageEnd;
                            LPSTR data = malloc(size + 1);
                            if (size)
                                memcpy(data, lpExecutable + dwImageEnd, size);
                            data[size] = 0;
                            result = str2tcs(data);
                            free(data);
                        }
                        //else _ftprintf(stderr, _T("Sections end is past file end"));
                    }
                    //else _ftprintf(stderr, _T("File doesn't have PE signature"));
                }
                //else _ftprintf(stderr, _T("File doesn't have MZ signature"));
                UnmapViewOfFile(lpExecutable);
            }
            //else _ftprintf(stderr, _T("MapViewOfFile failed: 0x%08x\n"), GetLastError());
            CloseHandle(hMapping);
        }
        //else _ftprintf(stderr, _T("CreateFileMapping failed: 0x%08x\n"), GetLastError());
        CloseHandle(hFile);
    }
    //else _ftprintf(stderr, _T("Create failed: 0x%08x\n"), GetLastError());
    return result;
}

int main(int argc, char** argv)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    LPTSTR lpCmdLine = skip_program_name(GetCommandLine());
    HANDLE hFile;
    LPTSTR lpModuleFileName = get_module_filename(hInstance);
    LPTSTR lpCygwinRoot = find_cygwin_root(lpModuleFileName);
    LPTSTR lpCygwinRunner;
    LPTSTR lpCygwinTarget = NULL;
    LPTSTR lpFullCmd;
    DWORD dwFullCmdSize;
    int rc = 127;

    if (!lpCygwinRoot) {
        _ftprintf(stderr, _T("Error: Unable to find cygwin root\n"));
        return rc;
    }
    //_tprintf(_T("cygwin root: %s\n"), lpCygwinRoot);

    lpCygwinRunner = join(lpCygwinRoot, _T(CYGWIN_RUNNER));
    //_tprintf(_T("cygwin runner: %s\n"), lpCygwinRunner);

    lpCygwinTarget = extract_cygwin_target(lpModuleFileName);
    if (lpCygwinTarget == NULL) {
        _ftprintf(stderr, _T("Error: Unable to extract target from %s\n"), lpModuleFileName);
        return rc;
    }

    dwFullCmdSize = _tcslen(lpCygwinRunner) + (lpCygwinTarget ? 1 + _tcslen(lpCygwinTarget) : 0) + 1 + _tcslen(lpCmdLine);
    lpFullCmd = malloc((dwFullCmdSize + 1) * sizeof(TCHAR));
    _tcscpy(lpFullCmd, lpCygwinRunner);
    if (lpCygwinTarget) {
        _tcscat(lpFullCmd, _T(" "));
        _tcscat(lpFullCmd, lpCygwinTarget);
    }
    _tcscat(lpFullCmd, _T(" "));
    _tcscat(lpFullCmd, lpCmdLine);
    //_tprintf(_T("full command: %s\n"), lpFullCmd);

#ifdef CYGWIN_LOCALE
    putenv(CYGWIN_LOCALE);
#endif
    signal(SIGINT, SIG_IGN);
    if (run(lpFullCmd, &rc))
        _ftprintf(stderr, _T("Error executing: %s\n"), lpFullCmd);

    return rc;
}
