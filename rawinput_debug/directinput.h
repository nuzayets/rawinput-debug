#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#include "dummy_window.h"
#include "timer.h"

VOID InitializeDirectInput();

VOID DIPrintControllers();
VOID DIRegisterControllerEvents();
VOID DIRegisterControllerPolling();
VOID DIUnregisterControllers();

