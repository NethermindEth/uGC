/**
 * @file
 * @brief uGC - handle manager
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#pragma once
#include "uGCBase.h"
#include "gcenv.base.h"
#include "gcinterface.h"
#include "uGCHandleStore.h"

class uGCHandleManager : public IGCHandleManager
{
public:
    virtual bool Initialize() override;

    virtual void Shutdown() override;

    virtual IGCHandleStore * GetGlobalHandleStore() override;

    virtual IGCHandleStore * CreateHandleStore() override;

    virtual void DestroyHandleStore(IGCHandleStore * store) override;

    virtual OBJECTHANDLE CreateGlobalHandleOfType(Object * object,
                                                  HandleType type) override;

    virtual OBJECTHANDLE CreateDuplicateHandle(OBJECTHANDLE handle) override;

    virtual void DestroyHandleOfType(OBJECTHANDLE handle,
                                     HandleType type) override;
    virtual void DestroyHandleOfUnknownType(OBJECTHANDLE handle) override;

    virtual void SetExtraInfoForHandle(OBJECTHANDLE handle, HandleType type,
                                       void *pExtraInfo) override;

    virtual void * GetExtraInfoFromHandle(OBJECTHANDLE handle) override;

    virtual void StoreObjectInHandle(OBJECTHANDLE handle,
                                     Object *object) override;
    virtual bool StoreObjectInHandleIfNull(OBJECTHANDLE handle,
                                           Object *object) override;

    virtual void SetDependentHandleSecondary(OBJECTHANDLE handle,
                                             Object *object) override;

    virtual Object * GetDependentHandleSecondary(OBJECTHANDLE handle) override;
    virtual Object * InterlockedCompareExchangeObjectInHandle(
        OBJECTHANDLE handle, Object *object, Object *comparandObject) override;

    virtual HandleType HandleFetchType(OBJECTHANDLE handle) override;
    virtual void TraceRefCountedHandles(HANDLESCANPROC callback,
        uintptr_t param1, uintptr_t param2) override;

private:
    uGCHandleStore *_handleStore;
};