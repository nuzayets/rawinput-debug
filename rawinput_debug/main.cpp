#include "main.h"

void CALLBACK PrintIdleTime(PTP_CALLBACK_INSTANCE, PVOID, PTP_TIMER t) {
	LASTINPUTINFO info;
	info.cbSize = sizeof(info);
	GetLastInputInfo(&info);
	DOUBLE idleSeconds = (GetTickCount64() - info.dwTime) / 1000.0;
	std::cout << idleSeconds << "s since last input." << std::endl;
}

void PrintHelp() {
	std::cout << 
R"(rawinput_debug usage:
F1: Toggle joystick Raw Input input sink
F2: Toggle joystick Raw Input foreground hwnd (hidden wnd - never fg)
F3: Toggle joystick DirectInput event notifications
F4: Toggle joystick DirectInput polling

F9: Print count of raw input events received from device
F10: Print count of DirectInput events/polls received from device

F12: Exit
)" << std::endl;
}

int main(int argc, char* argv[]) {
	// monitor idle state
	PTP_TIMER t = CreateThreadpoolTimer(PrintIdleTime, NULL, NULL);
	if (!t) return GetLastError();
	SetTimerDelay(t, 1000);

	// send raw keyboard to window as input sink
	RegisterRawInputSink(0x1, 0x6, hDummy);

	PrintHelp();

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CancelTimer(t);
}
