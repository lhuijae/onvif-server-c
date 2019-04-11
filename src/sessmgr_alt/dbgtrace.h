#pragma once

typedef struct
{
    int   nMaxLevel;
} *HDBG;

#define ASSERT(exp)
#define VERIFY(exp)                                     ((void)(exp))
#define TEXTASSERT(format, args...)
#define DBGINIT(szModule, phDbg)
#define DBGINITEX(pCfg, phDbg)
#define DBGUPDATE(hDbg)
#define DBGPRINT(hDbg, level, format, args...) DEBUG_MSG(format, ##args)
#define DBGPRINTEX(hDbg, level, format, args...) DEBUG_MSG(format, ##args)
#define DBGBUFFER(hDbg, level, Buffer, nSize, Title)
#define DBGSHUTDOWN(phDbg)
#define LOG_FATAL(format, args...)
#define DBGLOCK(hDbg)
#define DBGUNLOCK(hDbg)
