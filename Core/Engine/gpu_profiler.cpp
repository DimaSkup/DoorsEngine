/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: gpu_profiler.cpp
    Desc:     GPU performance-measurement subsystem implementation

    Created:  14.10.2025 by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "gpu_profiler.h"
#include <Render/d3dclass.h>


//---------------------------------------------------------
// global instance of the GPU profiler
//---------------------------------------------------------
GpuProfiler g_GpuProfiler;


//---------------------------------------------------------
// Desc:   constructor 
//---------------------------------------------------------
GpuProfiler::GpuProfiler() :
    frameQuery_(0),
    numFrameCollect_(-1),
    numFramesAvg_(0),
    timeBeginAvg_(0.0f),
    avgTimingsWasUpdated_(false)
{
    memset(queriesTsDisjoint_, 0, sizeof(queriesTsDisjoint_));
    memset(queriesTs_,         0, sizeof(queriesTs_));
    memset(deltas_,            0, sizeof(deltas_));
    memset(deltaTimesAvg_,     0, sizeof(deltaTimesAvg_));
    memset(timingTotalAvg_,    0, sizeof(timingTotalAvg_));
}

//---------------------------------------------------------
// Desc:   destructor 
//---------------------------------------------------------
GpuProfiler::~GpuProfiler()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:  when turn on we initialize the GPU profiler and set metrics gathering;
//        when turn off we shutdown the GPU profiler, release memory, don't do metrics gathering
//---------------------------------------------------------
void GpuProfiler::SwitchMetricsCollection(const bool state)
{
    collectMetrics_ = state;

    if (state)
    {
        if (!Init())
        {
            LogErr(LOG, "can't init a GPU profiler");
            LogErr(LOG, "can't turn on GPU metrics collection");
        }
    }
    else
    {
        Shutdown();
    }
}

//---------------------------------------------------------
// Desc:   create all the queries we'll need
//---------------------------------------------------------
bool GpuProfiler::Init()
{
    ID3D11Device* pDevice = Render::g_pDevice;
    assert(pDevice != nullptr);

    HRESULT hr = S_OK;
    D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };

    // create disjoint timestamp queries
    hr = pDevice->CreateQuery(&queryDesc, &(queriesTsDisjoint_[0]));
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create timestamp disjoint query for frame 0!");
        return false;
    }

    hr = pDevice->CreateQuery(&queryDesc, &queriesTsDisjoint_[1]);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create timestamp disjoint query for frame 1!");
        return false;
    }

    queryDesc.Query = D3D11_QUERY_TIMESTAMP;

    // create timestamp queries
    for (eGTS gts = GTS_BeginFrame; gts < GTS_Max; gts = eGTS(gts + 1))
    {
        hr = pDevice->CreateQuery(&queryDesc, &queriesTs_[gts][0]);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create start-frame timestamp query for GTS %d, frame 0!", gts);
            return false;
        }

        hr = pDevice->CreateQuery(&queryDesc, &queriesTs_[gts][1]);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create start-frame timestamp query for GTS %d, frame 1!", gts);
            return false;
        }
    }

    LogMsg(LOG, "is initialized successfully");
    return true;
}

//---------------------------------------------------------
// Desc:   release memory
//---------------------------------------------------------
void GpuProfiler::Shutdown()
{
    SafeRelease(&queriesTsDisjoint_[0]);
    SafeRelease(&queriesTsDisjoint_[1]);

    for (eGTS gts = GTS_BeginFrame; gts < GTS_Max; gts = eGTS(gts + 1))
    {
        SafeRelease(&queriesTs_[gts][0]);
        SafeRelease(&queriesTs_[gts][1]);
    }

    frameQuery_ = 0;
    numFrameCollect_ = -1;
    numFramesAvg_ = 0;
    timeBeginAvg_ = 0.0f;
    avgTimingsWasUpdated_ = false;

    memset(queriesTsDisjoint_, 0, sizeof(queriesTsDisjoint_));
    memset(queriesTs_,         0, sizeof(queriesTs_));
    memset(deltas_,            0, sizeof(deltas_));
    memset(deltaTimesAvg_,     0, sizeof(deltaTimesAvg_));
    memset(timingTotalAvg_,    0, sizeof(timingTotalAvg_));

    LogDbg(LOG, "is shutted down");
}

//---------------------------------------------------------
// Desc:   begin measurements for the current frame
//---------------------------------------------------------
void GpuProfiler::BeginFrame()
{
    if (!collectMetrics_)
        return;

    Render::g_pContext->Begin(queriesTsDisjoint_[frameQuery_]);
    Timestamp(GTS_BeginFrame);
}

//---------------------------------------------------------
// Desc:   make a timestamp according to type 
//---------------------------------------------------------
void GpuProfiler::Timestamp(const eGTS gts)
{
    if (!collectMetrics_)
        return;

    Render::g_pContext->End(queriesTs_[gts][frameQuery_]);
}

//---------------------------------------------------------
// Desc:   end measurements for the current frame
//---------------------------------------------------------
void GpuProfiler::EndFrame()
{
    if (!collectMetrics_)
        return;

    Timestamp(GTS_EndFrame);
    Render::g_pContext->End(queriesTsDisjoint_[frameQuery_]);

    // always be 0 or 1
    ++frameQuery_ &= 1;
}

//---------------------------------------------------------
// Desc:
//---------------------------------------------------------
void GpuProfiler::WaitForDataAndUpdate(const float gameTime)
{
    if (numFrameCollect_ < 0)
    {
        // haven't run enough frames yet to have data
        numFrameCollect_ = 0;
        return;
    }

    // wait for data
    ID3D11DeviceContext* pContext = Render::g_pContext;

    while (pContext->GetData(queriesTsDisjoint_[numFrameCollect_], NULL, 0, 0) == S_FALSE)
    {
        //Sleep(1);
    }

    int frame = numFrameCollect_;
    ++numFrameCollect_ &= 1;          // always be 0 or 1

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
    if (pContext->GetData(queriesTsDisjoint_[frame], &tsDisjoint, sizeof(tsDisjoint), 0) != S_OK)
    {
        //LogDbg(LOG, "can't retrieve timestamp disjoint query data");
        return;
    }

    if (tsDisjoint.Disjoint)
    {
        // throw out this frame's data
        LogDbg(LOG, "Timestamps disjoint");
        return;
    }

    // get timestamps for each stage
    UINT64 tsPrev = 0;
    if (pContext->GetData(queriesTs_[GTS_BeginFrame][frame], &tsPrev, sizeof(tsPrev), 0) != S_OK)
    {
        LogDbg(LOG, "can't retrieve timestamp query data for GTS %d", GTS_BeginFrame);
        return;
    }

    const float invFrequency = 1.0f / float(tsDisjoint.Frequency);

    // compute delta time btw each timestamp
    for (eGTS gts = eGTS(GTS_BeginFrame + 1); gts < GTS_Max; gts = eGTS(gts + 1))
    {
        UINT64 ts = 0;
        if (pContext->GetData(queriesTs_[gts][frame], &ts, sizeof(UINT64), 0) != S_OK)
        {
            LogDbg(LOG, "can't retrieve timestamp query data for GTS %d", gts);
            return;
        }

        deltas_[gts] = float(ts - tsPrev) * invFrequency;
        tsPrev = ts;

        timingTotalAvg_[gts] += deltas_[gts];
    }


    // compute timings averaged over 0.5 seconds
    ++numFramesAvg_;

    if (gameTime > timeBeginAvg_ + 0.5f)
    {
        const float invNumFramesAvg = 1.0f / numFramesAvg_;

        for (eGTS gts = GTS_BeginFrame; gts < GTS_Max; gts = eGTS(gts + 1))
        {
            deltaTimesAvg_[gts] = timingTotalAvg_[gts] * invNumFramesAvg;
            timingTotalAvg_[gts] = 0.0f;
        }

        numFramesAvg_ = 0;
        timeBeginAvg_ = gameTime;
        avgTimingsWasUpdated_ = true;
    }
}

//---------------------------------------------------------
// Desc:   if 0.5 seconds passed our averaged timings was
//         recomputed so we check if it was done
//---------------------------------------------------------
bool GpuProfiler::AvgTimingsWasUpdated()
{
    bool ret = avgTimingsWasUpdated_;
    avgTimingsWasUpdated_ = false;
    return ret;
}

//---------------------------------------------------------
// Desc:   get timings for the whole scene rendering process
//---------------------------------------------------------
float GpuProfiler::GetSceneDeltaTime() const
{
    float ts = 0;

    for (eGTS gts = GTS_RenderScene_Reset; gts <= GTS_RenderFullScene; gts = eGTS(gts + 1))
        ts += GetDeltaTime(gts);

    return ts;
}

//---------------------------------------------------------
// Desc:   get timings for the whole scene rendering process for last 0.5 sec
//---------------------------------------------------------
float GpuProfiler::GetSceneDeltaTimeAvg() const
{
    float tsAvg = 0;

    for (eGTS gts = GTS_RenderScene_Reset; gts <= GTS_RenderFullScene; gts = eGTS(gts + 1))
        tsAvg += GetDeltaTimeAvg(gts);

    return tsAvg;
}
