/**
 * @file
 * @brief uGC - handle manager implementation
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGCHandleManager.h"
#include "uGCHandleStore.h"

bool
uGCHandleManager::Initialize()
{
    _handleStore = new uGCHandleStore();
    return true;
}

void
uGCHandleManager::Shutdown()
{
}

IGCHandleStore *
uGCHandleManager::GetGlobalHandleStore()
{
    return _handleStore;
}

IGCHandleStore *
uGCHandleManager::CreateHandleStore()
{
    return nullptr;
}

void
uGCHandleManager::DestroyHandleStore(IGCHandleStore * store)
{
}

OBJECTHANDLE
uGCHandleManager::CreateGlobalHandleOfType(Object *object, HandleType type)
{
    return _handleStore->CreateHandleOfType(object, type);
}

OBJECTHANDLE
uGCHandleManager::CreateDuplicateHandle(OBJECTHANDLE handle)
{
    return OBJECTHANDLE();
}

void
uGCHandleManager::DestroyHandleOfType(OBJECTHANDLE handle, HandleType type)
{
}

void
uGCHandleManager::DestroyHandleOfUnknownType(OBJECTHANDLE handle)
{
}

void
uGCHandleManager::SetExtraInfoForHandle(OBJECTHANDLE handle, HandleType type,
    void * pExtraInfo)
{
}

void *
uGCHandleManager::GetExtraInfoFromHandle(OBJECTHANDLE handle)
{
    return nullptr;
}

void
uGCHandleManager::StoreObjectInHandle(OBJECTHANDLE handle, Object *object)
{
    Object **handleObj = (Object **)handle;
    *handleObj = object;
}

bool
uGCHandleManager::StoreObjectInHandleIfNull(OBJECTHANDLE handle, Object *object)
{
    Object **handleObj = (Object **)handle;

    if (*handleObj == NULL)
    {
        *handleObj = object;
        return true;
    }
    return false;
}

void
uGCHandleManager::SetDependentHandleSecondary(OBJECTHANDLE handle,
    Object *object)
{
}

Object *
uGCHandleManager::GetDependentHandleSecondary(OBJECTHANDLE handle)
{
    return nullptr;
}

Object*
uGCHandleManager::InterlockedCompareExchangeObjectInHandle(OBJECTHANDLE handle,
    Object *object, Object *oldObject)
{
    Object **handleObject = (Object **)handle;

    if (*handleObject == oldObject)
    {
        *handleObject = object;
    }
    return *handleObject;
}

HandleType
uGCHandleManager::HandleFetchType(OBJECTHANDLE handle)
{
    return HandleType();
}

void
uGCHandleManager::TraceRefCountedHandles(HANDLESCANPROC callback,
    uintptr_t param1, uintptr_t param2)
{
}
