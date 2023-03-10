#include "iodriver.h"

#ifdef IODRIVER_GIVEIO
// GiveIO
BOOL IODriver_Init(USHORT first, USHORT last)
{
	HANDLE h;

	h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);

	return h != INVALID_HANDLE_VALUE;
}
#else
// ButtIO
IOHandler buttioHand = {0};


BOOL IODriver_Init(USHORT first, USHORT last)
{
	if (!buttio_init(&buttioHand, NULL, BUTTIO_MET_IOPM))
		return FALSE;
	iopm_fillRange(buttioHand.iopm, first, last, TRUE);
	buttio_flushIOPMChanges(&buttioHand);
	return TRUE;
}

void IODriver_Exit(void)
{
	buttio_shutdown(&buttioHand);
}

#endif
