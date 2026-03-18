#include "memory.hpp"

namespace memory {

DWORD GetProcessId(const char* process_name) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, process_name) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return pid;
}

uintptr_t GetModuleBase(DWORD pid, const char* module_name) {
    uintptr_t base = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szModule, module_name) == 0) {
                base = (uintptr_t)entry.modBaseAddr;
                break;
            }
        } while (Module32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return base;
}

bool Initialize(HWND game_window) {
    DWORD pid;
    GetWindowThreadProcessId(game_window, &pid);
    process_id = pid;

    process_handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!process_handle) return false;

    module_base = GetModuleBase(pid, "RainbowSix.exe");
    if (!module_base) {
        CloseHandle(process_handle);
        return false;
    }

    attached = true;
    return true;
}

bool Read(uintptr_t address, void* buffer, size_t size) {
    SIZE_T bytes_read;
    return ReadProcessMemory(process_handle, (LPCVOID)address, buffer, size, &bytes_read) && bytes_read == size;
}

bool Write(uintptr_t address, void* buffer, size_t size) {
    SIZE_T bytes_written;
    return WriteProcessMemory(process_handle, (LPVOID)address, buffer, size, &bytes_written) && bytes_written == size;
}

}
