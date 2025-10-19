/**
 * @file
 * @brief uGC - handle store implementation
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGCHandleStore.h"

int handlesCount = 0;
OBJECTHANDLE handles[65535];

void
uGCHandleStore::Uproot()
{
}

bool
uGCHandleStore::ContainsHandle(OBJECTHANDLE handle)
{
    return false;
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type)
{
    handles[handlesCount] = (OBJECTHANDLE__*)object;
    return (OBJECTHANDLE)&handles[handlesCount++];
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type,
    int heapToAffinitizeTo)
{
    return OBJECTHANDLE();
}

OBJECTHANDLE
uGCHandleStore::CreateHandleWithExtraInfo(Object *object, HandleType type,
    void * pExtraInfo)
{
    return OBJECTHANDLE();
}

OBJECTHANDLE
uGCHandleStore::CreateDependentHandle(Object *primary, Object *secondary)
{
	handles[handlesCount] = (OBJECTHANDLE__*)primary;
	return (OBJECTHANDLE)&handles[handlesCount++];
}
