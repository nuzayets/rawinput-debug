#pragma once
#include <vector>
#include <windows.h>
#include <dinput.h>

VOID InitializeDirectInput();

VOID DIPrintControllers();
VOID DIRegisterControllerEvents();
VOID DIRegisterControllerPolling();
VOID DIUnregisterControllers();

struct DIController {
	LPDIRECTINPUTDEVICE8 dev = NULL;
	PTP_WAIT ptpWait = NULL;
	PTP_TIMER ptpTimer = NULL;
	ULONG count = 0;
};
