#pragma once
#include <windows.h>
#include "device_props.h"
#include "rawinput.h"
#include "directinput.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateDummyWindow();
extern HWND hDummy;
