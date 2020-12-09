#include "device_props.h"

DeviceProperties get_device_props(HANDLE hDevice) {
	static std::map<HANDLE, DeviceProperties> cache;
	if (cache.find(hDevice) != cache.end()) return cache[hDevice];

	DeviceProperties props = { TEXT("Unknown"), TEXT("Unknown"), TEXT("Unknown") };

	UINT uSize;
	if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, NULL, &uSize) >= 0) {
		const TCHAR* pstrDeviceInterfaceId = new TCHAR[uSize];
		if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (LPVOID)pstrDeviceInterfaceId, &uSize) >= 0) {
			props.pstrDeviceInterfaceId = pstrDeviceInterfaceId;
			ULONG ulSize = 0;
			DEVPROPTYPE propertyType;
			if (CM_Get_Device_Interface_Property(pstrDeviceInterfaceId,
				&DEVPKEY_Device_InstanceId, &propertyType, NULL, &ulSize, 0) == CR_BUFFER_SMALL) {
				DEVINSTID pstrDeviceInstanceId = (DEVINSTID)new BYTE[ulSize];
				if (CM_Get_Device_Interface_Property(pstrDeviceInterfaceId,
					&DEVPKEY_Device_InstanceId, &propertyType, (PBYTE)pstrDeviceInstanceId, &ulSize, 0) == CR_SUCCESS) {
					props.pstrDeviceInstanceId = pstrDeviceInstanceId;
					DEVINST devInst;
					if (CM_Locate_DevNode(&devInst, pstrDeviceInstanceId, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS) {
						ulSize = 0;
						if (CM_Get_DevNode_Property(devInst, &DEVPKEY_NAME, &propertyType, NULL, &ulSize, 0) == CR_BUFFER_SMALL) {
							const TCHAR* pstrDeviceName = (TCHAR*)new BYTE[ulSize];
							if (CM_Get_DevNode_Property(devInst, &DEVPKEY_NAME, &propertyType, (PBYTE)pstrDeviceName, &ulSize, 0) == CR_SUCCESS) {
								props.pstrDeviceName = pstrDeviceName;
							}
						}
					}
				}
			}
		}
	}

	cache[hDevice] = props;
	return props;
}
