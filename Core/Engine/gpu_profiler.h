/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: gpu_profiler.h
    Desc:     GPU performance-measurement subsystem

    Created:  14.10.2025 by DimaSkup
\**********************************************************************************/
#pragma once

struct ID3D11Query;

//---------------------------------------------------------
// enum of GPU timestamps to record
//---------------------------------------------------------
enum eGTS
{
    GTS_BeginFrame,
    GTS_ClearFrame,

    GTS_RenderScene_Reset,
    GTS_RenderScene_DepthPrepass,
    GTS_RenderScene_Weapon,
    GTS_RenderScene_Grass,
    GTS_RenderScene_Masked,
    GTS_RenderScene_Opaque,
    GTS_RenderScene_SkinnedModels,
    GTS_RenderScene_Terrain,
    GTS_RenderScene_Sky,
    GTS_RenderScene_SkyPlane,
    GTS_RenderScene_Blended,
    GTS_RenderScene_Transparent,
    GTS_RenderScene_Particles,
    GTS_RenderScene_DbgShapes,
    GTS_RenderScene_PostFX,

    GTS_RenderFullScene,
    GTS_RenderUI,
    GTS_EndFrame,

    GTS_Max
};

//---------------------------------------------------------
// class: GpuProfiler
//---------------------------------------------------------
class GpuProfiler
{
public:
    GpuProfiler();
    ~GpuProfiler();

    void SwitchMetricsCollection(const bool state);

    void BeginFrame();
    void Timestamp(const eGTS gts);
    void EndFrame();

    // wait on GPU for last frame's data (not this frame's) to be available
    void WaitForDataAndUpdate(const float gameTime);
    bool AvgTimingsWasUpdated();

    inline float GetDeltaTime   (const eGTS gts) const { return 1000.0f * deltas_[gts]; }
    inline float GetDeltaTimeAvg(const eGTS gts) const { return 1000.0f * deltaTimesAvg_[gts]; }

    float GetSceneDeltaTime() const;
    float GetSceneDeltaTimeAvg() const;

private:
    bool Init();
    void Shutdown();

private:
    int frameQuery_;                        // which of the two sets of queries are we currently issuing?
    int numFrameCollect_;                   // which of the two did we last collect?
    ID3D11Query* queriesTsDisjoint_[2];     // "Timestamp disjoint" query;  records whether timestamps are valid
    ID3D11Query* queriesTs_[GTS_Max][2];    // individual timestamp queries for each relevant point in the frame

    float   deltas_[GTS_Max];               // last frame's timings (each relative to previous GTS)
    float   deltaTimesAvg_[GTS_Max];        // timings averaged over 0.5 seconds

    float   timingTotalAvg_[GTS_Max];       // total timings thus far within this averaging period
    int     numFramesAvg_;                  // frames rendered in current averaging period
    float   timeBeginAvg_;                  // time at which current averaging period started

    bool    avgTimingsWasUpdated_ = false;
    bool    collectMetrics_ = false;
};

//---------------------------------------------------------
// global instace of the GPU profiler
//---------------------------------------------------------
extern GpuProfiler g_GpuProfiler;
