#ifdef IODRIVER_GIVEIO
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>

// GiveIO
#define READ_PORT_UCHAR(Port)                  _inp (Port)
#define READ_PORT_USHORT(Port)                 _inpw (Port)
#define READ_PORT_ULONG(Port)                  _inpd (Port)
#define WRITE_PORT_UCHAR(Port, Value)          _outp ((Port), (Value))
#define WRITE_PORT_USHORT(Port, Value)         _outpw ((Port), (Value))
#define WRITE_PORT_ULONG(Port, Value)          _outpd ((Port), (Value))

#define IODriver_Exit()

#else
// Buttio
#include "buttio.h"

extern IOHandler buttioHand;
__forceinline
VOID
WRITE_PORT_UCHAR (
    USHORT Port,
    UCHAR Value
    )

{
	buttio_wu8(&buttioHand, (Port), (Value));
    return;
}

__forceinline
UCHAR
READ_PORT_UCHAR (
    USHORT Port
    )

{
	BYTE b;
	buttio_ru8(&buttioHand, (Port), &b);
	return b;
}
void IODriver_Exit(void);

#pragma comment(lib, "buttio.lib")
#endif

BOOL IODriver_Init(USHORT first, USHORT last);
