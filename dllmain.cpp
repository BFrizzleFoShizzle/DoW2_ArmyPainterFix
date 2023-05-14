#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <stdint.h>
#include <sstream>

// these use a hard-coded length-8 array to convert between dropdown box indices and race indices
// NOPing these stops races with index > 8 from crashing the army painter
// these are all 5-byte instructions - movzx (reg), byte [(reg) + (reg) + 4c]
uint8_t* const ArmyPainterRaceIndexRead1Ptr = (uint8_t*)0x00498005;
uint8_t* const ArmyPainterRaceIndexRead2Ptr = (uint8_t*)0x00496AF4;
uint8_t* const ArmyPainterRaceIndexRead3Ptr = (uint8_t*)0x00497E3C;

void NOP(uint8_t* addr, size_t count)
{
    DWORD oldProtect;
    // make writeable
    bool success = VirtualProtect(addr, count, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (!success)
    {
        DWORD error = GetLastError();
        std::stringstream str;
        str << "Error NOPing address: "  << std::hex << ((void*)addr) << " " << error << std::endl;
        MessageBoxA(NULL, str.str().c_str(), "Error", MB_OK | MB_SYSTEMMODAL | MB_ICONERROR);
        return;
    }
    // NOP
    for (size_t i = 0; i < count; ++i)
        addr[i] = 0x90;
    // revert protection
    DWORD oldProtect2;
    success = success && VirtualProtect(addr, count, oldProtect, &oldProtect2);
    if (!success)
    {
        DWORD error = GetLastError();
        std::stringstream str;
        str << "Could not revert protection at address: " << std::hex << ((void*)addr) << " " << error << std::endl;
        MessageBoxA(NULL, str.str().c_str(), "Error", MB_OK | MB_SYSTEMMODAL | MB_ICONERROR);
        return;
    }
}

DWORD WINAPI MainThread(LPVOID param)
{
    NOP(ArmyPainterRaceIndexRead1Ptr, 5);
    NOP(ArmyPainterRaceIndexRead2Ptr, 5);
    NOP(ArmyPainterRaceIndexRead3Ptr, 5);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, MainThread, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

