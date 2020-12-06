#include <iostream>
#include <map>

#include <windows.h>

#include "dummy_window.h"
#include "device_props.h"
#include "rawinput.h"
#include "directinput.h"

HWND CreateDummyWindow() {
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Dummy";

	WNDCLASS wc = { };

	HINSTANCE hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&wc)) return NULL;

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		CLASS_NAME,					    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	return hwnd;
}

static const char* toggle_strings[] = { "OFF", "ON" };

VOID toggle_joystick_rawinput_sink() {
	static BOOL toggle = false;
	toggle = !toggle;
	if (toggle) RegisterRawInputSink(0x1, 0x4, hDummy);
	else UnregisterRawInput(0x1, 0x4);
	std::cout << "Capturing joystick raw input (as sink): " << toggle_strings[toggle] << std::endl;
}

VOID toggle_joystick_rawinput() {
	static BOOL toggle = false;
	toggle = !toggle;
	if (toggle) RegisterRawInput(0x1, 0x4, hDummy);
	else UnregisterRawInput(0x1, 0x4);
	std::cout << "Capturing joystick raw input (not as sink): " << toggle_strings[toggle] << std::endl;
}


std::map<HANDLE, ULONG> eventCounts;
VOID count_raw_events(HANDLE hDevice) {
	if (eventCounts.find(hDevice) != eventCounts.end()) eventCounts[hDevice]++;
	else eventCounts[hDevice] = 1;
}

VOID print_raw_events_stats() {
	for (auto it = eventCounts.begin(); it != eventCounts.end(); ++it) {
		auto props = get_device_props(it->first);
		std::wcout << "Name: " << props.pstrDeviceName << std::endl;
		std::wcout << "Interface ID: " << props.pstrDeviceInterfaceId << std::endl;
		std::wcout << "Instance ID: " << props.pstrDeviceInstanceId << std::endl;
		std::wcout << "Events Received: " << it->second << std::endl;
		std::wcout << std::endl;
	}
}

VOID print_directinput_stats() {
	DIPrintControllers();
}

VOID toggle_joystick_directinput_events() {
	static BOOL toggle = false;
	toggle = !toggle;
	if (toggle) DIRegisterControllerEvents();
	else DIUnregisterControllers();
}

VOID toggle_joystick_directinput_polling() {
	static BOOL toggle = false;
	toggle = !toggle;
	if (toggle) DIRegisterControllerPolling();
	else DIUnregisterControllers();
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INPUT:
		{
			UINT dwSize;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE lpb = new BYTE[dwSize];
			if (!lpb) return 0;
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
				std::cout << "GRID lied about size" << std::endl;

			RAWINPUT* raw = (RAWINPUT*)lpb;
			switch (raw->header.dwType) {
			case RIM_TYPEKEYBOARD:
				if (raw->data.keyboard.Flags == RI_KEY_MAKE) { // on key down
					switch (raw->data.keyboard.VKey) {
					case VK_F1:
						toggle_joystick_rawinput_sink();
						break;
					case VK_F2:
						toggle_joystick_rawinput();
						break;
					case VK_F3:
						toggle_joystick_directinput_events();
						break;
					case VK_F4:
						toggle_joystick_directinput_polling();
						break;
					case VK_F9:
						print_raw_events_stats();
						break;
					case VK_F10:
						print_directinput_stats();
						break;
					case VK_F12:
						PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
						break;
					default:
						break;
					}
				}
				break;
			default:
				count_raw_events(raw->header.hDevice);
				break;
			}

			delete[] lpb;

			// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-input
			// RIM_INPUT: Input occurred while the application was in the foreground
			//		The application must call DefWindowProc so the system can perform cleanup.
			if (GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT) return DefWindowProc(hwnd, uMsg, wParam, lParam);
			else return 0;
		}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

}

HWND hDummy = CreateDummyWindow();