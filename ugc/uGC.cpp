/**
 * @file
 * @brief uGC - main entrypoints
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGC.h"
#include "uGCHeap.h"
#include "uGCHandleManager.h"

/** Initialize GC */
extern "C" HRESULT
GC_Initialize(
    /* In  */ IGCToCLR* clrToGC,
    /* Out */ IGCHeap** gcHeap,
    /* Out */ IGCHandleManager** gcHandleManager,
    /* Out */ GcDacVars* gcDacVars
)
{
    IGCHeap          *heap;
    IGCHandleManager *handleManager;

    heap = new uGCHeap(clrToGC);
    handleManager = new uGCHandleManager();

    *gcHeap = heap;
    *gcHandleManager = handleManager;

    return S_OK;
}

/* Provide information about GC version */
extern "C" void
GC_VersionInfo(VersionInfo* result)
{
    result->MajorVersion = GC_INTERFACE_MINOR_VERSION;
    result->MinorVersion = GC_INTERFACE_MAJOR_VERSION;
    result->BuildVersion = 0;
    result->Name = "uGC";
}
