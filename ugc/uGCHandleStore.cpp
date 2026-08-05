/**
 * @file
 * @brief uGC - handle store implementation
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGCHandleStore.h"
#include "core/ugc_core.h"

void
uGCHandleStore::Uproot()
{
}

bool
uGCHandleStore::ContainsHandle(OBJECTHANDLE hndl)
{
    return ugc_handle_store_contains((const void *)hndl);
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type)
{
    return (OBJECTHANDLE)ugc_handle_create(object, (int)type);
}

OBJECTHANDLE
uGCHandleStore::CreateHandleOfType(Object *object, HandleType type,
    int heapToAffinitizeTo)
{
    return (OBJECTHANDLE)ugc_handle_create(object, (int)type);
}

OBJECTHANDLE
uGCHandleStore::CreateHandleWithExtraInfo(Object *object, HandleType type,
    void * pExtraInfo)
{
    return (OBJECTHANDLE)ugc_handle_create_with_extra(object, (int)type,
        pExtraInfo);
}

OBJECTHANDLE
uGCHandleStore::CreateDependentHandle(Object *primary, Object *secondary)
{
    return (OBJECTHANDLE)ugc_handle_create_dependent(primary, secondary,
        (int)HNDTYPE_DEPENDENT);
}

void
uGCHandleStore::uDestroyHandle(OBJECTHANDLE hndl)
{
    if (!ugc_handle_store_contains((const void *)hndl))
        return;
    ugc_handle_destroy_at(ugc_handle_index((void **)hndl));
}

OBJECTHANDLE
uGCHandleStore::uGetDependentHandle(OBJECTHANDLE hndl)
{
    return (OBJECTHANDLE)ugc_handle_dependent_slot_at(
        ugc_handle_index((void **)hndl));
}

void
uGCHandleStore::uSetDependentHandle(OBJECTHANDLE hndl, Object *secondary)
{
    ugc_handle_set_dependent_at(ugc_handle_index((void **)hndl), secondary);
}

HandleType
uGCHandleStore::uGetHandleType(OBJECTHANDLE hndl)
{
    return (HandleType)ugc_handle_get_type_at(ugc_handle_index((void **)hndl));
}

void
uGCHandleStore::uSetHandleType(OBJECTHANDLE hndl, HandleType type)
{
    ugc_handle_set_type_at(ugc_handle_index((void **)hndl), (int)type);
}

void *
uGCHandleStore::uGetHandleExtraInfo(OBJECTHANDLE hndl)
{
    return ugc_handle_get_extra_at(ugc_handle_index((void **)hndl));
}

void
uGCHandleStore::uSetHandleExtraInfo(OBJECTHANDLE hndl, void *extraInfo)
{
    ugc_handle_set_extra_at(ugc_handle_index((void **)hndl), extraInfo);
}
