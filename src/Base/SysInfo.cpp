#include "Precompiled.h"
#include "SysInfo.h"
#include <Windows.h>

std::vector<std::string> GetGraphicCardList()
{
    std::vector<std::string> gpus;
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);

    DWORD deviceNum = 0;
    while (EnumDisplayDevices(NULL, deviceNum, &dd, 0))
    {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            gpus.push_back(std::string(dd.DeviceString));
        deviceNum++;
    }

    return gpus;
}

std::string GetProcessorInfo()
{
    char brand[0x40] = {};
    int cpui[4] = { -1 };

    __cpuidex(cpui, 0x80000002, 0);

    //unsigned int blocks = cpui[0];
    for (int i = 0; i <= 2; ++i)
    {
        __cpuidex(cpui, 0x80000002 + i, 0);
        *reinterpret_cast<int*>(brand + i * 16) = cpui[0];
        *reinterpret_cast<int*>(brand + 4 + i * 16) = cpui[1];
        *reinterpret_cast<int*>(brand + 8 + i * 16) = cpui[2];
        *reinterpret_cast<int*>(brand + 12 + i * 16) = cpui[3];
    }

    return std::string(brand, 0x40);
}