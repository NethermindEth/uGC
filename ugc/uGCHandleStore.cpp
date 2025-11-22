/**
 * @file
 * @brief uGC - handle store implementation
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGCHandleStore.h"

#define HANDLE_COUNT (65535)

static int handlesCount = 0;
static Object *handles[HANDLE_COUNT] = { 0 };
static HandleType handleTypes[HANDLE_COUNT] = { HandleType() };
static void *extraInfos[HANDLE_COUNT] = { 0 };
static Object *dependentHandles[HANDLE_COUNT] = { 0 };

void
uGCHandleStore::Uproot()
{
}

bool
uGCHandleStore::ContainsHandle(OBJECTHANDLE hndl)
{
    uintptr_t handle = (uintptr_t) hndl;
    uintptr_t handleStart = (uintptr_t)&handles;
    uintptr_t handleEnd = (uintptr_t)&handles + sizeof(handles);
    if (handle >= handleStart && handle < handleEnd)
        return true;
    return false;
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type)
{
    if (handlesCount == HANDLE_COUNT)
        return 0;
    handles[handlesCount] = object;
    handleTypes[handlesCount] = type;
    return (OBJECTHANDLE)&handles[handlesCount++];
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type,
    int heapToAffinitizeTo)
{
    if (handlesCount == HANDLE_COUNT)
        return 0;

    handles[handlesCount] = object;
    handleTypes[handlesCount] = type;
    return (OBJECTHANDLE)&handles[handlesCount++];
}

OBJECTHANDLE
uGCHandleStore::CreateHandleWithExtraInfo(Object *object, HandleType type,
    void * pExtraInfo)
{
    if (handlesCount == HANDLE_COUNT)
        return 0;

    handles[handlesCount] = object;
    handleTypes[handlesCount] = type;
    extraInfos[handlesCount] = pExtraInfo;
    return (OBJECTHANDLE)&handles[handlesCount++];
}

OBJECTHANDLE
uGCHandleStore::CreateDependentHandle(Object *primary, Object *secondary)
{
    if (handlesCount == HANDLE_COUNT)
        return 0;

    handles[handlesCount] = primary;
    dependentHandles[handlesCount] = secondary;
    return (OBJECTHANDLE)&handles[handlesCount++];
}

OBJECTHANDLE
uGCHandleStore::uGetDependentHandle(OBJECTHANDLE hndl)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    return (OBJECTHANDLE)&dependentHandles[handleNumber];
}

void
uGCHandleStore::uSetDependentHandle(OBJECTHANDLE hndl, Object *secondary)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    dependentHandles[handleNumber] = secondary;
}

HandleType
uGCHandleStore::uGetHandleType(OBJECTHANDLE hndl)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    return handleTypes[handleNumber];
}

void
uGCHandleStore::uSetHandleType(OBJECTHANDLE hndl, HandleType type)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    handleTypes[handleNumber] = type;
}


void *
uGCHandleStore::uGetHandleExtraInfo(OBJECTHANDLE hndl)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    return extraInfos[handleNumber];
}

void
uGCHandleStore::uSetHandleExtraInfo(OBJECTHANDLE hndl, void *extraInfo)
{
    int handleNumber = ((uintptr_t)hndl - (uintptr_t)&handles[0]) /
        sizeof(handles[0]);

    extraInfos[handleNumber] = extraInfo;
}