#include "rawinput.h"

BOOL _register_raw_input(USHORT usUsagePage, USHORT usUsage, DWORD dwFlags, HWND hwndTarget) {
	RAWINPUTDEVICE rid[1];
	rid[0].usUsagePage = usUsagePage;
	rid[0].usUsage = usUsage;
	rid[0].dwFlags = dwFlags;
	rid[0].hwndTarget = hwndTarget;

	BOOL ret = RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
	if (!ret) std::cout << "Unable to register raw device input: " << GetLastError() << std::endl;
	return ret;
}

BOOL RegisterRawInputSink(USHORT usUsagePage, USHORT usUsage, HWND hwndTarget) {
	return _register_raw_input(usUsagePage, usUsage, RIDEV_INPUTSINK, hwndTarget);
}

BOOL RegisterRawInput(USHORT usUsagePage, USHORT usUsage, HWND hwndTarget) {
	return _register_raw_input(usUsagePage, usUsage, 0, hwndTarget);
}

BOOL UnregisterRawInput(USHORT usUsagePage, USHORT usUsage) {
	return _register_raw_input(usUsagePage, usUsage, RIDEV_REMOVE, NULL);
}