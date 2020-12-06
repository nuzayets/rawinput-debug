#pragma once
#include <windows.h>

struct DeviceProperties {
	const TCHAR* pstrDeviceInterfaceId;
	const TCHAR* pstrDeviceInstanceId;
	const TCHAR* pstrDeviceName;
};

DeviceProperties get_device_props(HANDLE hDevice);