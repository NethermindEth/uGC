/**
 * @file
 * @brief uGC - handle store
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#pragma once
#include "uGCBase.h"
#include "gcenv.base.h"
#include "gcinterface.h"

class uGCHandleStore : public IGCHandleStore
{
public:
    virtual void Uproot() override;
    virtual bool ContainsHandle(OBJECTHANDLE handle) override;
    virtual OBJECTHANDLE CreateHandleOfType(Object *object,
                                            HandleType type) override;
    virtual OBJECTHANDLE CreateHandleOfType(Object *object, HandleType type,
        int heapToAffinitizeTo) override;
    virtual OBJECTHANDLE CreateHandleWithExtraInfo(Object *object,
        HandleType type, void * pExtraInfo) override;
    virtual OBJECTHANDLE CreateDependentHandle(Object *primary,
        Object *secondary) override;

    virtual ~uGCHandleStore() {};

    OBJECTHANDLE uGetDependentHandle(OBJECTHANDLE hndl);
    void uSetDependentHandle(OBJECTHANDLE hndl, Object *secondary);
    HandleType uGetHandleType(OBJECTHANDLE hndl);
    void uSetHandleType(OBJECTHANDLE hndl, HandleType type);
    void *uGetHandleExtraInfo(OBJECTHANDLE hndl);
    void  uSetHandleExtraInfo(OBJECTHANDLE hndl, void *extraInfo);
};
