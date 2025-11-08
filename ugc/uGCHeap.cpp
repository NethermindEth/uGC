/**
 * @file
 * @brief uGC - heap implementation
 *
 * Copyright (C) 2025 Demerzel Solutions Limited (Nethermind)
 *
 * @author Maxim Menshikov <maksim.menshikov@nethermind.io>
 */
#include "uGCHeap.h"
#include <stdlib.h>

class ObjHeader
{
private:
#if defined(HOST_64BIT)
    uint32_t m_uAlignpad;
#endif
    uint32_t m_uSyncBlockValue;
};

bool
uGCHeap::IsValidSegmentSize(size_t size)
{
    return false;
}

bool
uGCHeap::IsValidGen0MaxSize(size_t size)
{
    return false;
}

size_t
uGCHeap::GetValidSegmentSize(bool large_seg)
{
    return size_t();
}

void
uGCHeap::SetReservedVMLimit(size_t vmlimit)
{
}

void
uGCHeap::WaitUntilConcurrentGCComplete()
{
}

bool
uGCHeap::IsConcurrentGCInProgress()
{
    return false;
}

void
uGCHeap::TemporaryEnableConcurrentGC()
{
}

void
uGCHeap::TemporaryDisableConcurrentGC()
{
}

bool
uGCHeap::IsConcurrentGCEnabled()
{
    return false;
}

HRESULT
uGCHeap::WaitUntilConcurrentGCCompleteAsync(int millisecondsTimeout)
{
    return E_NOTIMPL;
}

size_t
uGCHeap::GetNumberOfFinalizable()
{
    return size_t();
}

Object *
uGCHeap::GetNextFinalizable()
{
    return nullptr;
}

void
uGCHeap::GetMemoryInfo(uint64_t* highMemLoadThresholdBytes,
                       uint64_t* totalAvailableMemoryBytes,
                       uint64_t* lastRecordedMemLoadBytes,
                       uint64_t* lastRecordedHeapSizeBytes,
                       uint64_t* lastRecordedFragmentationBytes,
                       uint64_t* totalCommittedBytes,
                       uint64_t* promotedBytes,
                       uint64_t* pinnedObjectCount,
                       uint64_t* finalizationPendingCount,
                       uint64_t* index,
                       uint32_t* generation,
                       uint32_t* pauseTimePct,
                       bool* isCompaction,
                       bool* isConcurrent,
                       uint64_t* genInfoRaw,
                       uint64_t* pauseInfoRaw,
                       int kind)
{
    *highMemLoadThresholdBytes = 0;
    *totalAvailableMemoryBytes = 0;
    *lastRecordedMemLoadBytes = 0;
    *lastRecordedHeapSizeBytes = 0;
    *lastRecordedFragmentationBytes = 0;
    *totalCommittedBytes = 0;
    *promotedBytes = 0;
    *pinnedObjectCount = 0;
    *finalizationPendingCount = 0;
    *index = 0;
    *generation = 0;
    *pauseTimePct = 0;
    *isCompaction = false;
    *isConcurrent = false;
    *genInfoRaw = 0;
    *pauseInfoRaw = 0;
}

uint32_t
uGCHeap::GetMemoryLoad()
{
    return 0;
}

int
uGCHeap::GetGcLatencyMode()
{
    return 0;
}

int
uGCHeap::SetGcLatencyMode(int newLatencyMode)
{
    return 0;
}

int
uGCHeap::GetLOHCompactionMode()
{
    return 0;
}

void
uGCHeap::SetLOHCompactionMode(int newLOHCompactionMode)
{
}

bool
uGCHeap::RegisterForFullGCNotification(uint32_t gen2Percentage,
    uint32_t lohPercentage)
{
    return false;
}

bool
uGCHeap::CancelFullGCNotification()
{
    return false;
}

int
uGCHeap::WaitForFullGCApproach(int millisecondsTimeout)
{
    return 0;
}

int
uGCHeap::WaitForFullGCComplete(int millisecondsTimeout)
{
    return 0;
}

unsigned
uGCHeap::WhichGeneration(Object * obj)
{
    return 0;
}

int
uGCHeap::CollectionCount(int generation, int get_bgc_fgc_coutn)
{
    return 0;
}

int
uGCHeap::StartNoGCRegion(uint64_t totalSize, bool lohSizeKnown,
    uint64_t lohSize, bool disallowFullBlockingGC)
{
    return 0;
}

int
uGCHeap::EndNoGCRegion()
{
    return 0;
}

size_t
uGCHeap::GetTotalBytesInUse()
{
    return size_t();
}

uint64_t
uGCHeap::GetTotalAllocatedBytes()
{
    return 0;
}

HRESULT
uGCHeap::GarbageCollect(int generation, bool low_memory_p, int mode)
{
    return NOERROR;
}

unsigned
uGCHeap::GetMaxGeneration()
{
    return 1;
}

void
uGCHeap::SetFinalizationRun(Object * obj)
{
}

bool
uGCHeap::RegisterForFinalization(int gen, Object * obj)
{
    return false;
}

int
uGCHeap::GetLastGCPercentTimeInGC()
{
    return 1;
}

size_t
uGCHeap::GetLastGCGenerationSize(int gen)
{
    return 1;
}

HRESULT
uGCHeap::Initialize()
{
    return NOERROR;
}

bool
uGCHeap::IsPromoted(Object * object)
{
    return false;
}

bool
uGCHeap::IsHeapPointer(void *object, bool small_heap_only)
{
    return object != 0;
}

unsigned
uGCHeap::GetCondemnedGeneration()
{
    return 0;
}

bool
uGCHeap::IsGCInProgressHelper(bool bConsiderGCStart)
{
    return false;
}

unsigned
uGCHeap::GetGcCount()
{
    return 0;
}

bool
uGCHeap::IsThreadUsingAllocationContextHeap(gc_alloc_context * acontext,
    int thread_number)
{
    return false;
}

bool
uGCHeap::IsEphemeral(Object * object)
{
    return false;
}

uint32_t
uGCHeap::WaitUntilGCComplete(bool bConsiderGCStart)
{
    return uint32_t();
}

void
uGCHeap::FixAllocContext(gc_alloc_context* acontext, void* arg, void* heap)
{
}

size_t
uGCHeap::GetCurrentObjSize()
{
    return size_t();
}

void
uGCHeap::SetGCInProgress(bool fInProgress)
{
}

bool
uGCHeap::RuntimeStructuresValid()
{
    return true;
}

void
uGCHeap::SetSuspensionPending(bool fSuspensionPending)
{
}

__attribute__((noinline,optimize("O2")))
void
uGCHeap::SetYieldProcessorScalingFactor(float yieldProcessorScalingFactor)
{
    asm volatile("" ::: "memory");
}

void
uGCHeap::Shutdown()
{
}

size_t
uGCHeap::GetLastGCStartTime(int generation)
{
    return size_t();
}

size_t
uGCHeap::GetLastGCDuration(int generation)
{
    return size_t();
}

size_t
uGCHeap::GetNow()
{
    return size_t();
}

Object *
uGCHeap::Alloc(gc_alloc_context * acontext, size_t size, uint32_t flags)
{
    int sizeWithHeader = size + sizeof(ObjHeader);
    ObjHeader* address = (ObjHeader*)calloc(sizeWithHeader, sizeof(char));
	return (Object*)(address + 1);
}

void
uGCHeap::PublishObject(uint8_t * obj)
{
}

void
uGCHeap::SetWaitForGCEvent()
{
}

void
uGCHeap::ResetWaitForGCEvent()
{
}

bool
uGCHeap::IsLargeObject(Object *pObj)
{
    return false;
}

void
uGCHeap::ValidateObjectMember(Object * obj)
{
}

Object *
uGCHeap::NextObj(Object * object)
{
    return nullptr;
}

Object *
uGCHeap::GetContainingObject(void *pInteriorPtr, bool fCollectedGenOnly)
{
    return nullptr;
}

void
uGCHeap::DiagWalkObject(Object * obj, walk_fn fn, void *context)
{
}

void
uGCHeap::DiagWalkObject2(Object * obj, walk_fn2 fn, void *context)
{
}

void
uGCHeap::DiagWalkHeap(walk_fn fn, void *context, int gen_number,
    bool walk_large_object_heap_p)
{
}

void
uGCHeap::DiagWalkSurvivorsWithType(void *gc_context, record_surv_fn fn,
    void *diag_context, walk_surv_type type, int gen_number)
{
}

void
uGCHeap::DiagWalkFinalizeQueue(void *gc_context, fq_walk_fn fn)
{
}

void
uGCHeap::DiagScanFinalizeQueue(fq_scan_fn fn, ScanContext *context)
{
}

void
uGCHeap::DiagScanHandles(handle_scan_fn fn, int gen_number,
    ScanContext *context)
{
}

void
uGCHeap::DiagScanDependentHandles(handle_scan_fn fn, int gen_number,
    ScanContext *context)
{
}

void
uGCHeap::DiagDescrGenerations(gen_walk_fn fn, void *context)
{
}

void
uGCHeap::DiagTraceGCSegments()
{
}

void
uGCHeap::DiagGetGCSettings(EtwGCSettingsInfo* settings)
{
}

bool
uGCHeap::StressHeap(gc_alloc_context * acontext)
{
    return false;
}

segment_handle
uGCHeap::RegisterFrozenSegment(segment_info * pseginfo)
{
    registeredSegments++;
    return (segment_handle)registeredSegments;
}

void
uGCHeap::UnregisterFrozenSegment(segment_handle seg)
{
}

bool
uGCHeap::IsInFrozenSegment(Object *object)
{
    return false;
}

void
uGCHeap::ControlEvents(GCEventKeyword keyword, GCEventLevel level)
{
}

void
uGCHeap::ControlPrivateEvents(GCEventKeyword keyword, GCEventLevel level)
{
}

unsigned int
uGCHeap::GetGenerationWithRange(Object *object, uint8_t **ppStart,
    uint8_t **ppAllocated, uint8_t **ppReserved)
{
    return 0;
}

// constructor

int64_t
uGCHeap::GetTotalPauseDuration()
{
    return 0;
}

void
uGCHeap::EnumerateConfigurationValues(void* context,
    ConfigurationValueFunc configurationValueFunc)
{
}

void
uGCHeap::UpdateFrozenSegment(segment_handle seg, uint8_t *allocated,
    uint8_t *committed)
{
}

int
uGCHeap::RefreshMemoryLimit()
{
    return 0;
}

enable_no_gc_region_callback_status
uGCHeap::EnableNoGCRegionCallback(NoGCRegionCallbackFinalizerWorkItem* callback,
    uint64_t callback_threshold)
{
    return succeed;
}

FinalizerWorkItem*
uGCHeap::GetExtraWorkForFinalization()
{
    return nullptr;
}

uint64_t
uGCHeap::GetGenerationBudget(int generation)
{
    return 0;
}

size_t
uGCHeap::GetLOHThreshold()
{
    return 0;
}

void
uGCHeap::DiagWalkHeapWithACHandling(walk_fn fn, void* context, int gen_number,
    bool walk_large_object_heap_p)
{
}

void
uGCHeap::NullBridgeObjectsWeakRefs(size_t length,
    void* unreachableObjectHandles)
{
}

bool
uGCHeap::IsPromoted(Object* object, bool bVerifyNextHeader)
{
    return false;
}

