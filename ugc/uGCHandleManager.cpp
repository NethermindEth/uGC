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
#include "core/ugc_core.h"

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
    /* Secondary stores (collectible AssemblyLoadContexts) share the global
     * static-array-backed store: uGC never unloads anything, so isolation
     * between stores buys nothing, and reusing the singleton avoids a heap
     * allocation per store. DestroyHandleStore stays a no-op for the same
     * reason. */
    return _handleStore;
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
    if (handle == OBJECTHANDLE())
        return OBJECTHANDLE();
    return _handleStore->CreateHandleOfType(*(Object **)handle,
        _handleStore->uGetHandleType(handle));
}

void
uGCHandleManager::DestroyHandleOfType(OBJECTHANDLE handle, HandleType type)
{
    _handleStore->uDestroyHandle(handle);
}

void
uGCHandleManager::DestroyHandleOfUnknownType(OBJECTHANDLE handle)
{
    _handleStore->uDestroyHandle(handle);
}

void
uGCHandleManager::SetExtraInfoForHandle(OBJECTHANDLE handle, HandleType type,
    void * pExtraInfo)
{
    _handleStore->uSetHandleType(handle, type);
    _handleStore->uSetHandleExtraInfo(handle, pExtraInfo);
}

void *
uGCHandleManager::GetExtraInfoFromHandle(OBJECTHANDLE handle)
{
    return _handleStore->uGetHandleExtraInfo(handle);
}

void
uGCHandleManager::StoreObjectInHandle(OBJECTHANDLE handle, Object *object)
{
    ugc_handle_slot_store((void **)handle, object);
}

bool
uGCHandleManager::StoreObjectInHandleIfNull(OBJECTHANDLE handle, Object *object)
{
    return ugc_handle_slot_store_if_null((void **)handle, object);
}

void
uGCHandleManager::SetDependentHandleSecondary(OBJECTHANDLE handle,
    Object *object)
{
    _handleStore->uSetDependentHandle(handle, object);
}

Object *
uGCHandleManager::GetDependentHandleSecondary(OBJECTHANDLE handle)
{
    Object **handleObj = (Object **)_handleStore->uGetDependentHandle(handle);
    return *handleObj;
}

Object*
uGCHandleManager::InterlockedCompareExchangeObjectInHandle(OBJECTHANDLE handle,
    Object *object, Object *oldObject)
{
    return (Object *)ugc_handle_slot_cas((void **)handle, object, oldObject);
}

HandleType
uGCHandleManager::HandleFetchType(OBJECTHANDLE handle)
{
    return _handleStore->uGetHandleType(handle);
}

void
uGCHandleManager::TraceRefCountedHandles(HANDLESCANPROC callback,
    uintptr_t param1, uintptr_t param2)
{
}
