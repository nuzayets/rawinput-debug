#pragma once
#include <map>
#include <windows.h>

#pragma comment (lib, "Cfgmgr32.lib")
#include <cfgmgr32.h>
#include <initguid.h> 
#include <devpkey.h>


struct DeviceProperties {
	const TCHAR* pstrDeviceInterfaceId;
	const TCHAR* pstrDeviceInstanceId;
	const TCHAR* pstrDeviceName;
};

DeviceProperties get_device_props(HANDLE hDevice);