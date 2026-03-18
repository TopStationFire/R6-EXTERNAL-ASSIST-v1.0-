#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

namespace memory {
    inline HANDLE process_handle = nullptr;
    inline DWORD process_id = 0;
    inline uintptr_t module_base = 0;
    inline bool attached = false;

    bool Initialize(HWND game_window);
    DWORD GetProcessId(const char* process_name);
    uintptr_t GetModuleBase(DWORD pid, const char* module_name);
    bool Read(uintptr_t address, void* buffer, size_t size);
    bool Write(uintptr_t address, void* buffer, size_t size);
    
    template<typename T>
    T Read(uintptr_t address) {
        T buffer{};
        Read(address, &buffer, sizeof(T));
        return buffer;
    }
    
    template<typename T>
    void Write(uintptr_t address, T value) {
        Write(address, &value, sizeof(T));
    }
}
