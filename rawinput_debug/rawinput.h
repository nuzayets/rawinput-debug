#pragma once
#include <iostream>
#include <windows.h>

BOOL _register_raw_input(USHORT usUsagePage, USHORT usUsage, DWORD dwFlags, HWND hwndTarget);
BOOL RegisterRawInputSink(USHORT usUsagePage, USHORT usUsage, HWND hwndTarget);
BOOL RegisterRawInput(USHORT usUsagePage, USHORT usUsage, HWND hwndTarget);
BOOL UnregisterRawInput(USHORT usUsagePage, USHORT usUsage);