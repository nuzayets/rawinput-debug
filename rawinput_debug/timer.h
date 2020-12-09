#pragma once
#include <windows.h>

FILETIME _get_ft_for_ms(DWORD delay_ms);
VOID SetTimerDelay(PTP_TIMER t, DWORD delay_ms);
VOID CancelTimer(PTP_TIMER t);