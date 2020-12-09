#include "directinput.h"

IDirectInput8* _dinput = NULL;

struct DIController {
	LPDIRECTINPUTDEVICE8 dev = NULL;
	PTP_WAIT ptpWait = NULL;
	PTP_TIMER ptpTimer = NULL;
	ULONG count = 0;
};
std::map<HANDLE, DIController> event_controller;

struct ControllerStats {
	DIDEVICEINSTANCE did;
	ULONG count;
};
std::vector<ControllerStats> stats;

IDirectInput8* DInput() {
	if (!_dinput) InitializeDirectInput();
	return _dinput;
}

VOID InitializeDirectInput() {
	if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&_dinput, NULL))) {
		std::cout << "Failed to initialize DirectInput." << std::endl;
		_dinput = NULL;
	}
}

VOID EnumerateControllers(LPDIENUMDEVICESCALLBACK lpCallback) {
	if (FAILED(DInput()->EnumDevices(DI8DEVCLASS_GAMECTRL, lpCallback, NULL, DIEDFL_ALLDEVICES))) {
		std::cout << "Failed to enumerate DirectInput controllers." << std::endl;
	}
}

BOOL DIEnumPrintDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	std::wcout << "Found DirectInput controller device: " << lpddi->tszProductName << std::endl;
	return DIENUM_CONTINUE;
}

// handle DirectInput events
VOID CALLBACK WaitCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WAIT Wait, TP_WAIT_RESULT WaitResult) {
	event_controller[(HANDLE)Context].count += 1;
	SetThreadpoolWait(Wait, (HANDLE)Context, NULL);
}

// handle own Timer events (to poll)
VOID CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE, PVOID Context, PTP_TIMER t) {
	auto dev = event_controller[(HANDLE)Context].dev;
	if (!FAILED(dev->Poll())) {
		DIJOYSTATE state;
		if (!FAILED(dev->GetDeviceState(sizeof(state), &state))) {
			event_controller[(HANDLE)Context].count += 1;
		}
	}
}

LPDIRECTINPUTDEVICE8 ConfigureDevice(LPCDIDEVICEINSTANCE lpddi) {
	LPDIRECTINPUTDEVICE8 dev = NULL;
	if (FAILED(DInput()->CreateDevice(lpddi->guidInstance, &dev, NULL))) {
		std::wcout << "Failed to create DirectInput controller device: " << lpddi->tszProductName << std::endl;
		return NULL;
	}

	if (FAILED(dev->SetCooperativeLevel(hDummy, DISCL_BACKGROUND | DISCL_EXCLUSIVE))) {
		std::wcout << "Failed to set cooperative level for device: " << lpddi->tszProductName << std::endl;
		dev->Release();
		return NULL;
	}

	if (FAILED(dev->SetDataFormat(&c_dfDIJoystick))) {
		std::wcout << "Failed to set data format for device: " << lpddi->tszProductName << std::endl;
		dev->Release();
		return NULL;
	}

	return dev;
}

BOOL DIEnumRegisterEvents(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	LPDIRECTINPUTDEVICE8 dev = ConfigureDevice(lpddi);
	if (dev) {
		// Let's handle some events.
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!hEvent) return DIENUM_STOP;

		HRESULT hr = dev->SetEventNotification(hEvent);
		if (FAILED(hr)) {
			if (hr == DI_POLLEDDEVICE) {
				std::wcout << "Cannot set notifications for device, it is a polled device: " << lpddi->tszProductName << std::endl;
			} else {
				std::wcout << "Failed to subscribe to notifications on device: " << lpddi->tszProductName << std::endl;
			}
			CloseHandle(hEvent);
			dev->Release();
		} else {
			if (FAILED(dev->Acquire())) {
				std::wcout << "Failed to acquire device: " << lpddi->tszProductName << std::endl;
				dev->SetEventNotification(NULL);
				CloseHandle(hEvent);
				dev->Release();
			} else {
				PTP_WAIT ptpWait = CreateThreadpoolWait(WaitCallback, hEvent, NULL);
				SetThreadpoolWait(ptpWait, hEvent, NULL);
				event_controller[hEvent] = { dev, ptpWait, NULL, 0 };
				std::wcout << "Listening for events on DirectInput device: " << lpddi->tszProductName << std::endl;
			}
		}
	}
	return DIENUM_CONTINUE;
}

BOOL DIEnumRegisterPolling(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	LPDIRECTINPUTDEVICE8 dev = ConfigureDevice(lpddi);
	if (dev) {
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // not used for polling, only as identifier...
		if (!hEvent) return DIENUM_STOP;

		if (FAILED(dev->Acquire())) {
			std::wcout << "Failed to acquire device: " << lpddi->tszProductName << std::endl;
			CloseHandle(hEvent);
			dev->Release();
		} else {
			PTP_TIMER ptpTimer = CreateThreadpoolTimer(TimerCallback, hEvent, NULL);
			SetTimerDelay(ptpTimer, 10);
			event_controller[hEvent] = { dev, NULL, ptpTimer, 0 };
			std::wcout << "Polling DirectInput device: " << lpddi->tszProductName << std::endl;
		}
	}
	return DIENUM_CONTINUE;
}

VOID DIRegisterControllerEvents() {
	if (event_controller.size() > 0) {
		DIUnregisterControllers();
	}
	EnumerateControllers(DIEnumRegisterEvents);
}

VOID DIRegisterControllerPolling() {
	if (event_controller.size() > 0) {
		DIUnregisterControllers();
	}
	EnumerateControllers(DIEnumRegisterPolling);
}

VOID collect_stats() {
	if (event_controller.size() > 0) {
		stats.clear();
		for (auto it = event_controller.begin(); it != event_controller.end(); ++it) {
			DIController controller = it->second;
			DIDEVICEINSTANCE did;
			did.dwSize = sizeof(did);
			controller.dev->GetDeviceInfo(&did);
			stats.push_back({ did, controller.count });
		}
	}
}

VOID DIUnregisterControllers() {
	collect_stats();
	for (auto it = event_controller.begin(); it != event_controller.end(); ++it) {
		HANDLE hEvent = it->first;
		DIController controller = it->second;
		DIDEVICEINSTANCE did;
		did.dwSize = sizeof(did);
		controller.dev->GetDeviceInfo(&did);
		std::wcout << "Closing DirectInput device: " << did.tszProductName << std::endl;
		if (controller.ptpWait) {
			SetThreadpoolWait(controller.ptpWait, NULL, NULL);
			controller.dev->SetEventNotification(NULL);
			CloseThreadpoolWait(controller.ptpWait);
		}
		if (controller.ptpTimer) {
			CancelTimer(controller.ptpTimer);
			CloseThreadpoolTimer(controller.ptpTimer);
		}
		
		CloseHandle(hEvent);
		controller.dev->Unacquire();
		controller.dev->Release();
	}
	event_controller.clear();
}

VOID DIPrintControllers() {
	collect_stats();
	for (auto it = stats.begin(); it != stats.end(); ++it) {
		std::wcout << "DirectInput Device Name: " << it->did.tszProductName << std::endl;
		std::wcout << "Event/Poll Count: " << it->count << std::endl;
	}
}
