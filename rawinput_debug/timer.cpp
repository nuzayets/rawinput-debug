#include "timer.h"

FILETIME _get_ft_for_ms(DWORD delay_ms) {
	// microsoft why are you like this
	LONGLONG llDelay = -(LONGLONG)delay_ms * 10000LL;
	return { (DWORD)llDelay, (DWORD)(llDelay >> 32) };
}

VOID SetTimerDelay(PTP_TIMER t, DWORD delay_ms) {
	FILETIME ftDueTime = _get_ft_for_ms(delay_ms);
	SetThreadpoolTimer(t, &ftDueTime, delay_ms, 0);
}

VOID CancelTimer(PTP_TIMER t) {
	SetThreadpoolTimer(t, NULL, 0, 0);
}
