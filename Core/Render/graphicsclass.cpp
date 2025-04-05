// =================================================================================
// Filename: graphicsclass.cpp
// Created:  14.10.22
// =================================================================================
#include "graphicsclass.h"


#include <CoreCommon/Assert.h>
#include <CoreCommon/MathHelper.h>
#include <CoreCommon/Utils.h>

#include "../Input/inputcodes.h"
#include "RenderDataPreparator.h"

#include <ImGuizmo.h>
#include <random>
#include <format>

using namespace DirectX;


namespace Core
{

GraphicsClass::GraphicsClass() : prep_(render_, entityMgr_)
{
    Log::Debug();
}

GraphicsClass::~GraphicsClass() 
{
    Log::Debug("start of destroying");
    Shutdown();
    Log::Debug("is destroyed");
}


// =================================================================================
//                             PUBLIC METHODS
// =================================================================================
bool GraphicsClass::Initialize(
    HWND hwnd,
    SystemState& systemState,
    const Settings& settings)
{
    return InitHelper(hwnd, systemState, settings);
}

void GraphicsClass::Update(
    SystemState& sysState,
    const float deltaTime,
    const float gameTime)
{
    UpdateHelper(sysState, deltaTime, gameTime);
}

void GraphicsClass::Render3D()
{
    RenderHelper();
}


///////////////////////////////////////////////////////////

bool GraphicsClass::InitHelper(
    HWND hwnd, 
    SystemState& systemState,
    const Settings& settings)
{
    // Initializes all the main parts of graphics rendering module



    try
    {
        InitializeGraphics initGraphics;
        bool result = false;

        Log::Print();
        Log::Print("------------------------------------------------------------", eConsoleColor::YELLOW);
        Log::Print("              INITIALIZATION: GRAPHICS SYSTEM               ", eConsoleColor::YELLOW);
        Log::Print("------------------------------------------------------------", eConsoleColor::YELLOW);

        pSysState_ = &systemState;

        result = initGraphics.InitializeDirectX(d3d_, hwnd, settings);
        Assert::True(result, "can't initialize D3DClass");

        // after initialization of the DirectX we can use pointers to the device and device context
        d3d_.GetDeviceAndDeviceContext(pDevice_, pDeviceContext_);


        // init all the cameras on the scene
        result = initGraphics.InitializeCameras(
            d3d_,
            gameCamera_,
            editorCamera_,
            baseViewMatrix_,           // init the base view matrix which is used for 2D rendering
            entityMgr_,
            settings);
        Assert::True(result, "can't initialize cameras / view matrices");

        // choose the editor camera as current by default
        pCurrCamera_ = &editorCamera_;

        // initializer the textures global manager (container)
        g_TextureMgr.Initialize(pDevice_);

#if 0
        // create a texture which can be used as a render target
        FrameBufferSpecification fbSpec;

        fbSpec.width = 480;
        fbSpec.height = 320;
        fbSpec.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        fbSpec.screenNear = d3d_.GetScreenNear();
        fbSpec.screenDepth = d3d_.GetScreenDepth();

        result = frameBuffer_.Initialize(pDevice_, fbSpec);
        Assert::True(result, "can't initialize the render to texture object");
#endif

        // initialize scene objects: cubes, spheres, trees, etc.
        result = initGraphics.InitializeScene(settings, d3d_, entityMgr_);
        Assert::True(result, "can't initialize the scene elements (models, etc.)");

    
        // create frustums for frustum culling
        frustums_.push_back(DirectX::BoundingFrustum());

        // setup loggers of the modules to make possible writing into the log file
        entityMgr_.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());
        render_.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());

        // matrix for 2D rendering
        WVO_ = worldMatrix_ * baseViewMatrix_ * d3d_.GetOrthoMatrix();

    
        // setup render initial params
        Render::InitParams renderParams;

        renderParams.worldViewOrtho = DirectX::XMMatrixTranspose(WVO_); 

        // zaporizha sky box horizon (darker by 0.1f)
        renderParams.fogColor = 
        { 
            settings.GetFloat("FOG_RED"),
            settings.GetFloat("FOG_GREEN"),
            settings.GetFloat("FOG_BLUE"),
        };
        renderParams.fogStart = settings.GetFloat("FOG_START");
        renderParams.fogRange = settings.GetFloat("FOG_RANGE");

        const DirectX::XMFLOAT3 skyColorCenter = g_ModelMgr.GetSky().GetColorCenter();
        const DirectX::XMFLOAT3 skyColorApex   = g_ModelMgr.GetSky().GetColorApex();

        renderParams.fogColor.x *= skyColorCenter.x;
        renderParams.fogColor.y *= skyColorCenter.y;
        renderParams.fogColor.z *= skyColorCenter.z;

        fullFogDistance_    = (int)(renderParams.fogStart + renderParams.fogRange);
        fullFogDistanceSqr_ = (int)(fullFogDistance_ * fullFogDistance_);

        result = render_.Initialize(
            pDevice_,
            pDeviceContext_,
            renderParams);
        Assert::True(result, "can't init the render module");


        render_.SetSkyGradient(pDeviceContext_, skyColorCenter, skyColorApex);

        BuildGeometryBuffers();
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        Log::Error("can't initialize the graphics class");
        this->Shutdown();
        return false;
    }

    Log::Print(" is successfully initialized");
    return true;
}

///////////////////////////////////////////////////////////

void GraphicsClass::Shutdown()
{
    // Shutdowns all the graphics rendering parts, releases the memory
    Log::Debug();
    d3d_.Shutdown();
}


// =================================================================================
// Update / prepare scene
// =================================================================================

void GraphicsClass::UpdateHelper(
    SystemState& sysState,
    const float deltaTime,
    const float totalGameTime)
{
    // update all the graphics related stuff for this frame


    pCurrCamera_->UpdateViewMatrix();

    // DIRTY HACK: update the camera height according to the terrain height function
    DirectX::XMFLOAT3 prevCamPos = pCurrCamera_->GetPosition();

    const float strideByY = 0.01f * (prevCamPos.z * sinf(0.1f * prevCamPos.x) +
                           prevCamPos.x * cosf(0.1f * prevCamPos.z)) + 1.5f;

    //pCurrCamera_->SetStrideByY(strideByY);

    // ---------------------------------------------

    const DirectX::XMMATRIX& viewMatrix = pCurrCamera_->View();
    const DirectX::XMMATRIX& projMatrix = pCurrCamera_->Proj();
    viewProj_ = viewMatrix * projMatrix;

    // update the cameras states
    sysState.cameraPos = pCurrCamera_->GetPosition();
    sysState.cameraDir = pCurrCamera_->GetLook();
    sysState.cameraView = viewMatrix;
    sysState.cameraProj = projMatrix;
    
    // update the entities and related data
    entityMgr_.Update(totalGameTime, deltaTime);
    //entityMgr_.lightSystem_.UpdateSpotLights(cameraPos, cameraDir);
    
    // build the frustum from the projection matrix in view space.
    DirectX::BoundingFrustum::CreateFromMatrix(frustums_[0], projMatrix);

    // perform frustum culling on all of our currently loaded entities
    ComputeFrustumCulling(sysState);
    ComputeFrustumCullingOfLightSources(sysState);

    // Update shaders common data for this frame
    UpdateShadersDataPerFrame();

    // prepare all the visible entities data for rendering
    const ECS::cvector<EntityID>& visibleEntts = entityMgr_.renderSystem_.GetAllVisibleEntts();

    // separate entts into opaque, entts with alpha clipping, blended, etc.
    entityMgr_.renderStatesSystem_.SeparateEnttsByRenderStates(visibleEntts, rsDataToRender_);

    pSysState_->visibleVerticesCount = 0;


    //
    // separate entts by distance
    //

    // render as lit entts only that entts which are closer than some distance
    // all entts that are farther will be rendered as fully fogged with a single fog color
    ECS::cvector<EntityID>& alphaClippedEntts = rsDataToRender_.enttsAlphaClipping_.ids_;
    const size numVisEntts = alphaClippedEntts.size();

    using namespace DirectX;

    if (numVisEntts > 0)
    {
        size count = 0;
        std::vector<index> idxs(numVisEntts);
        ECS::cvector<XMFLOAT3> positions;


        entityMgr_.transformSystem_.GetPositionsByIDs(alphaClippedEntts.data(), numVisEntts, positions);
        const XMVECTOR camPos = pCurrCamera_->GetPositionVec();

        // check if entity by idx is farther than fog range if so we store its idx
        for (index i = 0; i < numVisEntts; ++i)
        {
            const XMVECTOR enttPos      = XMLoadFloat3(&positions[i]);
            const XMVECTOR camToEnttVec = XMVectorSubtract(enttPos, camPos);
            const int distSqr           = (int)XMVectorGetX(XMVector3Dot(camToEnttVec, camToEnttVec));

            if (distSqr > fullFogDistanceSqr_)
            {
                idxs[count] = i;
                ++count;
            }
        }

        idxs.resize(count);
        ECS::cvector<EntityID>& foggedEntts = rsDataToRender_.enttsFogged_.ids_;
        foggedEntts.resize(count);

        // store IDs of entts which are farther than fog range
        for (index i = 0; const index enttIdx : idxs)
        {
            foggedEntts[i++] = alphaClippedEntts[enttIdx];
        }

        // erase IDs of entts which are farther than fog range from the origin IDs array
        for (index i = 0; const index idx : idxs)
        {
            alphaClippedEntts.erase(idx - i);
            i++;
        }

        idxs.clear();
    }


    // ----------------------------------------------------
    // prepare data for each entts set

    
    const EntityID* basicEntts = rsDataToRender_.enttsDefault_.ids_.data();
    const size numBasicEntts = rsDataToRender_.enttsDefault_.ids_.size();
    PrepBasicInstancesForRender(basicEntts, numBasicEntts);
    
    const EntityID* alphaClippedEnttsIDs = rsDataToRender_.enttsAlphaClipping_.ids_.data();
    const size numAlphaClippedEntts = rsDataToRender_.enttsAlphaClipping_.ids_.size();
    PrepAlphaClippedInstancesForRender(alphaClippedEnttsIDs, numAlphaClippedEntts);

    
    /*
    const EntityID* blendedEntts = rsDataToRender_.enttsBlended_.ids_.data();
    const size numBlendedEntts = rsDataToRender_.enttsBlended_.ids_.size();
    PrepBlendedInstancesForRender(blendedEntts, numBlendedEntts);
    */
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepBasicInstancesForRender(
    const EntityID* ids,
    const size numEntts)
{
    // prepare rendering data of entts which have default render states

    if (numEntts == 0)
        return;

    Assert::True(ids != nullptr, "input ptr to entts IDs arr == nullptr");

    Render::RenderDataStorage& storage = render_.dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        storage.modelInstBuffer,
        storage.modelInstances);

    // compute how many vertices will we render
    for (const Render::Instance& inst : storage.modelInstances)
        pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepAlphaClippedInstancesForRender(
    const EntityID* ids,
    const size numEntts)
{
    // prepare rendering data of entts which have alpha clip + cull none

    if (numEntts == 0)
        return;

    Assert::True(ids != nullptr, "input ptr to entts IDs arr == nullptr");


    Render::RenderDataStorage& storage = render_.dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        storage.alphaClippedModelInstBuffer,
        storage.alphaClippedModelInstances);

    // compute how many vertices will we render
    for (const Render::Instance& inst : storage.alphaClippedModelInstances)
        pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepBlendedInstancesForRender(
    const EntityID* ids,
    const size numEntts)
{
    // prepare rendering data of entts which have alpha clip + cull none

    if (numEntts == 0)
        return;

    Assert::True(ids != nullptr, "input ptr to entts IDs arr == nullptr");


    Render::RenderDataStorage& storage = render_.dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        storage.blendedModelInstBuffer,
        storage.blendedModelInstances);
}

///////////////////////////////////////////////////////////

void GraphicsClass::ComputeFrustumCulling(SystemState& sysState)
{
    // reset render counters (do it before frustum culling)
    sysState.visibleObjectsCount = 0;

    ECS::EntityMgr& mgr = entityMgr_;
    ECS::RenderSystem& renderSys = entityMgr_.renderSystem_;
    renderSys.ClearVisibleEntts();

    const ECS::cvector<EntityID>& enttsRenderable = renderSys.GetAllEnttsIDs();
    const size numRenderableEntts = enttsRenderable.size();
    size numVisEntts = 0;                                     // the number of currently visible entts

    ECS::cvector<size>     numBoxesPerEntt;
    ECS::cvector<DirectX::BoundingOrientedBox> OBBs;       // bounding box of each mesh of each renderable entt
    cvector<XMMATRIX> invWorlds(numRenderableEntts);      // inverse world matrix of each renderable entt
    cvector<XMMATRIX> enttsLocal(numRenderableEntts);     // local space of each renderable entt
    cvector<index>    idxsToVisEntts(numRenderableEntts);

    // get inverse world matrix of each renderable entt
    mgr.transformSystem_.GetInverseWorldMatricesOfEntts(
        enttsRenderable.data(),
        invWorlds.data(),
        (int)numRenderableEntts);

    // compute local space matrices for frustum transformations
    for (index i = 0; i < numRenderableEntts; ++i)
    {
        enttsLocal[i] = DirectX::XMMatrixMultiply(pCurrCamera_->InverseView(), invWorlds[i]);
    }
        
    // clear some arrs since we don't need them already
    invWorlds.clear();

    // get arr of AABB / bounding spheres for each renderable entt
    mgr.boundingSystem_.GetOBBs(
        enttsRenderable.data(),
        numRenderableEntts,
        numBoxesPerEntt,
        OBBs);

    // go through each entity and define if it is visible
    for (index idx = 0, obbIdx = 0; idx < numRenderableEntts; ++idx)
    {
        // decompose the matrix into its individual parts
        XMVECTOR scale;
        XMVECTOR dirQuat;
        XMVECTOR translation;
        DirectX::XMMatrixDecompose(&scale, &dirQuat, &translation, enttsLocal[idx]);

        // transform the camera frustum from view space to the object's local space
        DirectX::BoundingFrustum LSpaceFrustum; 
        frustums_[0].Transform(LSpaceFrustum, DirectX::XMVectorGetX(scale), dirQuat, translation);

        // if we have any mesh OBB of the entt in view -- we set this entt as visible
        for (index i = 0; i < numBoxesPerEntt[idx]; ++i)
        {
            if (LSpaceFrustum.Intersects(OBBs[obbIdx + i]))
            {
                idxsToVisEntts[numVisEntts++] = idx;
                i = numBoxesPerEntt[idx];              // go out from the for-loop
            }
        }

        obbIdx += numBoxesPerEntt[idx];
    }

    // ------------------------------------------

    // store ids of visible entts
    ECS::cvector<EntityID> visibleEntts(numVisEntts);

    for (index i = 0; i < numVisEntts; ++i)
        visibleEntts[i] = enttsRenderable[idxsToVisEntts[i]];

    renderSys.SetVisibleEntts(visibleEntts);
    sysState.visibleObjectsCount = (u32)numVisEntts;
}

///////////////////////////////////////////////////////////

void GraphicsClass::ComputeFrustumCullingOfLightSources(SystemState& sysState)
{
    // store IDs of light sources which are currently visible by camera frustum

    using namespace DirectX;
    ECS::EntityMgr& mgr = entityMgr_;

    mgr.renderSystem_.ClearVisibleLightSources();
    ECS::cvector<EntityID>& visPointLights = mgr.renderSystem_.GetVisiblePointLights();

    // get all the point light sources which are in the visibility range
    const int numPointLights = (int)mgr.lightSystem_.GetNumPointLights();
    const EntityID* pointLightsIDs = mgr.lightSystem_.GetPointLights().ids.data();

    std::vector<XMMATRIX> invWorlds(numPointLights);
    std::vector<XMMATRIX> localSpaces(numPointLights);

    // get inverse world matrix of each point light source
    mgr.transformSystem_.GetInverseWorldMatricesOfEntts(pointLightsIDs, invWorlds.data(), numPointLights);

    // compute local space matrices for frustum transformations
    for (int i = 0; i < numPointLights; ++i)
        localSpaces[i] = DirectX::XMMatrixMultiply(pCurrCamera_->InverseView(), invWorlds[i]);

    invWorlds.clear();
    visPointLights.reserve(numPointLights);

    // go through each point light source and define if it is visible
    for (int idx = 0; idx < numPointLights; ++idx)
    {
        // decompose the matrix into its individual parts
        XMVECTOR scale;
        XMVECTOR dirQuat;
        XMVECTOR translation;
        DirectX::XMMatrixDecompose(&scale, &dirQuat, &translation, localSpaces[idx]);

        // transform the camera frustum from view space to the point light local space
        BoundingFrustum LSpaceFrustum;
        frustums_[0].Transform(LSpaceFrustum, XMVectorGetX(scale), dirQuat, translation);

        // if we see any part of bound sphere we store an index to related light source
        if (LSpaceFrustum.Intersects(BoundingSphere({ 0,0,0 }, 1)))
        {
            visPointLights.push_back(pointLightsIDs[idx]);
        }
    }

    visPointLights.shrink_to_fit();

    // we'll use these values to render counts onto the screen
    sysState.numVisiblePointLights = (u32)std::ssize(visPointLights);
}

///////////////////////////////////////////////////////////

void GraphicsClass::UpdateShadersDataPerFrame()
{
    // Update shaders common data for this frame: 
    // viewProj matrix, camera position, light sources data, etc.

    Render::PerFrameData& perFrameData = render_.perFrameData_;

    perFrameData.viewProj = DirectX::XMMatrixTranspose(viewProj_);
    perFrameData.cameraPos = pCurrCamera_->GetPosition();

    SetupLightsForFrame(entityMgr_.lightSystem_, perFrameData);

    // update lighting data, camera pos, etc. for this frame
    render_.UpdatePerFrame(pDeviceContext_, perFrameData);
}

///////////////////////////////////////////////////////////

void GraphicsClass::ClearRenderingDataBeforeFrame()
{
    // clear rendering data from the previous frame / instances set

    render_.dataStorage_.Clear();
    rsDataToRender_.Clear();
}


// =================================================================================
// Render state control
// =================================================================================

void GraphicsClass::ChangeModelFillMode()
{
    // toggling on / toggling off the fill mode for the models

    isWireframeMode_ = !isWireframeMode_;
    const eRenderState fillParam = (isWireframeMode_) ? FILL_WIREFRAME : FILL_SOLID;

    d3d_.SetRS(fillParam);
};

///////////////////////////////////////////////////////////

void GraphicsClass::ChangeCullMode()
{
    // toggling on and toggling off the cull mode for the models

    isCullBackMode_ = !isCullBackMode_;
    d3d_.SetRS((isCullBackMode_) ? CULL_BACK : CULL_FRONT);
}

///////////////////////////////////////////////////////////

void GraphicsClass::SwitchGameMode(bool enableGameMode)
{
    // switch btw the game and editor modes and do some other related changes

    isGameMode_ = enableGameMode;
    pCurrCamera_ = (enableGameMode) ? &gameCamera_ : &editorCamera_;
}

///////////////////////////////////////////////////////////

// memory allocation and releasing
void* GraphicsClass::operator new(size_t i)
{
    if (void* ptr = _aligned_malloc(i, 16))
        return ptr;

    Log::Error("can't allocate memory for this object");
    throw std::bad_alloc{};
}

///////////////////////////////////////////////////////////

void GraphicsClass::operator delete(void* ptr)
{
    _aligned_free(ptr);
}


// =================================================================================
// Rendering methods
// =================================================================================

void GraphicsClass::RenderHelper()
{
    try
    {
        pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        RenderEnttsDefault();

        RenderEnttsAlphaClipCullNone();
        RenderSkyDome();
#if 0
        pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        RenderBoundingLineBoxes();
        RenderBoundingLineSpheres();
        RenderBillboards();
        
        RenderEnttsBlended();
#endif
    }
    catch (const std::out_of_range& e)
    {
        Log::Error(e.what());
        Log::Error("there is no such a key to data");
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error("can't render 3D entts onto the scene");
    }
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderModel(BasicModel& model, const DirectX::XMMATRIX& world)
{
    // for specific purposes:
    // just render a single asset/model at the center of the world

#if 0
    Render::Instance instance;
    Render::InstBuffData instanceBuffData;
    int numSubsets = model.GetNumSubsets();

    prep_.PrepareInstanceFromModel(model, instance);

    // each subset (mesh) will have its own world/texTransform/material
    instanceBuffData.Resize(numSubsets);

    for (int i = 0; i < numSubsets; ++i)
    {
        instanceBuffData.worlds_[i]        = world;
        instanceBuffData.texTransforms_[i] = DirectX::XMMatrixIdentity();
        instanceBuffData.materials_[i]     = Render::Material();//instance.materials[i];
    }

    render_.UpdateInstancedBuffer(pDeviceContext_, instanceBuffData);

    // render prepared instances using shaders
    render_.RenderInstances(
        pDeviceContext_, 
        Render::ShaderTypes::LIGHT,
        &instance, 1);
#endif
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsDefault()
{
    const Render::RenderDataStorage& storage = render_.dataStorage_;

    // check if we have any instances to render
    if (storage.modelInstances.empty())
        return;

    // setup states before rendering
    d3d_.GetRenderStates().ResetRS(pDeviceContext_);
    d3d_.GetRenderStates().ResetBS(pDeviceContext_);

    // load instances data and render them
    UpdateInstanceBuffAndRenderInstances(
        pDeviceContext_,
        Render::ShaderTypes::LIGHT,
        storage.modelInstBuffer,
        storage.modelInstances);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsAlphaClipCullNone()
{
    // render all the visible entts with cull_none and alpha clipping;
    // (entts for instance: wire fence, bushes, leaves, etc.)

    const Render::RenderDataStorage& storage = render_.dataStorage_;

    // check if we have any instances to render
    if (storage.alphaClippedModelInstances.empty())
        return;


    // setup rendering pipeline
    RenderStates& renderStates = d3d_.GetRenderStates();
    renderStates.SetRS(pDeviceContext_, CULL_NONE);
    render_.SwitchAlphaClipping(pDeviceContext_, true);

    // load instances data and render them
    UpdateInstanceBuffAndRenderInstances(
        pDeviceContext_,
        Render::ShaderTypes::LIGHT,
        storage.alphaClippedModelInstBuffer,
        storage.alphaClippedModelInstances);

    // reset rendering pipeline
    renderStates.ResetRS(pDeviceContext_);
    render_.SwitchAlphaClipping(pDeviceContext_, false);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsBlended()
{
    // render all the visible blended entts

    const Render::RenderDataStorage& storage = render_.dataStorage_;

    // check if we have any instances to render
    if (storage.blendedModelInstances.empty())
        return;

    const ECS::EnttsBlended& blendData = rsDataToRender_.enttsBlended_;
    const ECS::eRenderState* blendStates = blendData.states_.data();
    const size numBlendStates = blendData.states_.size();
    const size* numInstancesPerBlendState = blendData.instanceCountPerBS_.data();
    const Render::InstBuffData& instBuffer = storage.blendedModelInstBuffer;

    // push data into the instanced buffer
    render_.UpdateInstancedBuffer(pDeviceContext_, instBuffer);

    int instanceOffset = 0;

    // go through each blending state, turn it on and render blended entts with this state
    for (index bsIdx = 0; bsIdx < numBlendStates; ++bsIdx)
    {
        d3d_.TurnOnBlending(eRenderState(blendStates[bsIdx]));

        for (u32 instCount = 0; instCount < numInstancesPerBlendState[bsIdx]; ++instCount)
        {
            const Render::Instance* instance = &(storage.blendedModelInstances[instanceOffset]);
            render_.RenderInstances(pDeviceContext_, Render::ShaderTypes::LIGHT, instance, 1);
            ++instanceOffset;
        }
    }
            
    // turn off blending after rendering of all the visible blended entities
    //d3d_.TurnOffBlending();
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderBoundingLineBoxes()
{
    // if we don't want to show bound boxes just go out
    if (aabbShowMode_ == NONE)
        return;
    

    const EntityID* visEntts = entityMgr_.renderSystem_.GetAllVisibleEntts().data();
    const size numVisEntts = entityMgr_.renderSystem_.GetVisibleEnttsCount();

    // check if we have any visible entts
    if (numVisEntts == 0)
        return;                          

    Render::RenderDataStorage& storage       = render_.dataStorage_;
    Render::InstBuffData& instancesBuffer    = storage.boundingLineBoxBuffer;
    std::vector<Render::Instance>& instances = storage.boundingLineBoxInstances;
    

    // prepare the line box instance
    const int numInstances = 1;                // how many different line box models we have
    const ModelID lineBoxId = 1;
    BasicModel& lineBox = g_ModelMgr.GetModelByID(lineBoxId);
    
    // we will use only one type of model -- line box
    instances.resize(1);                         
    Render::Instance& instance = instances[0];
    prep_.PrepareInstanceData(lineBox, instance);


    // choose the bounding box show mode
    // (1: box around the while model, 2: box around each model's mesh)
    if (aabbShowMode_ == MODEL)
        prep_.PrepareEnttsBoundingLineBox(visEntts, numVisEntts, instance, instancesBuffer);
    else if (aabbShowMode_ == MESH)
        prep_.PrepareEnttsMeshesBoundingLineBox(visEntts, numVisEntts, instance, instancesBuffer);


    pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // render
    render_.UpdateInstancedBuffer(pDeviceContext_, instancesBuffer);
    render_.RenderBoundingLineBoxes(pDeviceContext_, &instance, numInstances);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderBoundingLineSpheres()
{
    // render line bound sphere around each visible point light source

#if 0
    const std::vector<EntityID>& visPointLights = entityMgr_.renderSystem_.GetArrVisibleLightSources();
    const size numVisLightSources = std::ssize(visPointLights);

    if (numVisLightSources > 0)
    {
        Render::RenderDataStorage& storage = render_.dataStorage_;
        Render::InstBuffData& instancesBuffer = storage.boundingLineBoxBuffer_;
        std::vector<Render::Instance>& instances = storage.boundingLineBoxInstances_;

        // prepare instance data
        const int numInstances = 1;
        BasicModel& boundSphere = modelStorage_.GetModelByName("bound_sphere");

        instances.resize(1);
        Render::Instance& instance = instances[0];
        prep_.PrepareInstanceData(boundSphere, instance);


        // prepare instances buffer data
        instance.numInstances = numVisLightSources;
        instancesBuffer.Resize(instance.numInstances);

        // generate world matrix for each instance of bounding sphere 
        for (int i = 0; i < (int)numVisLightSources; ++i)
        {
            const int pointLightIdx = idxsToVisLightSources[i];
            const float scale = posAndRange[pointLightIdx].range;
            const XMFLOAT3& pos = posAndRange[pointLightIdx].position;
            instancesBuffer.worlds_[i] = DirectX::XMMatrixScaling(scale, scale, scale) * DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        }

        pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

        // render
        render_.UpdateInstancedBuffer(pDeviceContext_, instancesBuffer);
        render_.RenderBoundingLineBoxes(pDeviceContext_, &instance, numInstances);
    }
#endif
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderBillboards()
{
    const EntityID* foggedEntts = rsDataToRender_.enttsFogged_.ids_.data();
    const size numFoggedEntts = rsDataToRender_.enttsFogged_.ids_.size();

    if (numFoggedEntts > 0)
    {
        d3d_.TurnOnBlending(ALPHA_TO_COVERAGE);

        RenderStates& renderStates = d3d_.GetRenderStates();
        renderStates.SetRS(pDeviceContext_, CULL_NONE);
        render_.SwitchAlphaClipping(pDeviceContext_, true);

        //
        // prepare billboards data for rendering
        //
        std::vector<Render::Material> materials(numFoggedEntts);
        ECS::cvector<DirectX::XMFLOAT3> foggedPositions;
        std::vector<DirectX::XMFLOAT2> sizes(numFoggedEntts, { 30, 30 });

        entityMgr_.transformSystem_.GetPositionsByIDs(foggedEntts, numFoggedEntts, foggedPositions);

        render_.shadersContainer_.billboardShader_.UpdateInstancedBuffer(
            pDeviceContext_,
            materials.data(),
            foggedPositions.data(),
            sizes.data(),
            (int)numFoggedEntts);


        //
        // render billboards instances
        //
        Render::Instance instance;
        instance.pVB = pGeomVB_;
        instance.pIB = pGeomIB_;
        instance.texSRVs = { g_TextureMgr.GetTexPtrByName("texture_array")->GetTextureResourceView() };
        instance.vertexStride = sizeof(TreePointSprite);

        render_.shadersContainer_.billboardShader_.Render(pDeviceContext_, instance);

        // reset rendering pipeline
        renderStates.ResetRS(pDeviceContext_);
        render_.SwitchAlphaClipping(pDeviceContext_, false);
}
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderSkyDome()
{
    const EntityID skyEnttID  = entityMgr_.nameSystem_.GetIdByName("sky");

    // if there is no sky entity
    if (skyEnttID == 0)
        return;

    const SkyModel& sky = g_ModelMgr.GetSky();
    Render::SkyInstance instance;

    //
    // prepare the sky instance
    //
    const TexID* skyTexIDs = sky.GetTexIDs();
    const int skyTexMaxNum = sky.GetMaxTexturesNum();

    // get shader resource views for the sky
    cvector<ID3D11ShaderResourceView*> instanceTexSRVs;
    g_TextureMgr.GetSRVsByTexIDs(skyTexIDs, skyTexMaxNum, instanceTexSRVs);
    instance.texSRVs = { instanceTexSRVs.begin(), instanceTexSRVs.end() };

    instance.vertexStride = sky.GetVertexStride();
    instance.pVB          = sky.GetVertexBuffer();
    instance.pIB          = sky.GetIndexBuffer();
    instance.indexCount   = sky.GetNumIndices();

    instance.colorCenter  = sky.GetColorCenter();
    instance.colorApex    = sky.GetColorApex();


    // setup rendering pipeline before rendering of the sky dome
    RenderStates& renderStates = d3d_.GetRenderStates();
    renderStates.ResetRS(pDeviceContext_);
    renderStates.SetRS(pDeviceContext_, CULL_NONE);
    renderStates.ResetBS(pDeviceContext_);
    renderStates.SetDSS(pDeviceContext_, SKY_DOME, 1);

    // compute a worldViewProj matrix for the sky instance
    const XMFLOAT3& eyePos = pCurrCamera_->GetPosition();
    const XMMATRIX camOffset        = DirectX::XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
    const XMMATRIX skyWorld         = entityMgr_.transformSystem_.GetWorldMatrixOfEntt(skyEnttID);
    const XMMATRIX worldViewProj    = DirectX::XMMatrixTranspose(camOffset * skyWorld * viewProj_);

    render_.RenderSkyDome(pDeviceContext_, instance, worldViewProj);
}

///////////////////////////////////////////////////////////

void GraphicsClass::UpdateInstanceBuffAndRenderInstances(
    ID3D11DeviceContext* pDeviceContext,
    const Render::ShaderTypes type,
    const Render::InstBuffData& instanceBuffData,
    const std::vector<Render::Instance>& instances)
{
    render_.UpdateInstancedBuffer(pDeviceContext_, instanceBuffData);

    // render prepared instances using shaders
    render_.RenderInstances(pDeviceContext_, type, instances.data(), (int)instances.size());
}

///////////////////////////////////////////////////////////

void GraphicsClass::SetupLightsForFrame(
    const ECS::LightSystem& lightSys,
    Render::PerFrameData& outData)
{
    // convert light source data from the ECS into Render format
    // (they are the same so we simply need to copy data)

    const size dirLightSize   = sizeof(ECS::DirLight);
    const size pointLightSize = sizeof(ECS::PointLight);
    const size spotLightSize  = sizeof(ECS::SpotLight);

    const ECS::DirLights&  dirLights  = lightSys.GetDirLights();
    const ECS::SpotLights& spotLights = lightSys.GetSpotLights();

    const size numDirLights   = dirLights.data.size();
    const size numSpotLights  = spotLights.data.size();

    const ECS::RenderSystem& renderSys           = entityMgr_.renderSystem_;
    const ECS::cvector<EntityID>& visPointLights = renderSys.GetVisiblePointLights();
    const size numVisPointLightSources           = visPointLights.size();


    outData.ResizeLightData(numDirLights, (int)numVisPointLightSources, numSpotLights);

    if (numVisPointLightSources > 0)
    {
        ECS::cvector<ECS::PointLight>   pointLightsData;
        ECS::cvector<DirectX::XMFLOAT3> pointLightsPositions;

        lightSys.GetPointLightsData(
            visPointLights.data(),
            numVisPointLightSources,
            pointLightsData,
            pointLightsPositions);

        // store light properties, range, and attenuation
        for (index i = 0; i < numVisPointLightSources; ++i)
        {
            outData.pointLights[i].ambient  = pointLightsData[i].ambient;
            outData.pointLights[i].diffuse  = pointLightsData[i].diffuse;
            outData.pointLights[i].specular = pointLightsData[i].specular;
            outData.pointLights[i].att      = pointLightsData[i].att;
            outData.pointLights[i].range    = pointLightsData[i].range;
        }

        // store positions
        for (index i = 0; i < numVisPointLightSources; ++i)
            outData.pointLights[i].position = pointLightsPositions[i];
    }

    // ----------------------------------------------------
    // prepare data of directed lights

    for (index i = 0; i < numDirLights; ++i)
    {
        outData.dirLights[i].ambient  = dirLights.data[i].ambient;
        outData.dirLights[i].diffuse  = dirLights.data[i].diffuse;
        outData.dirLights[i].specular = dirLights.data[i].specular;
    }

    ECS::cvector<XMVECTOR> dirLightsDirections;
    entityMgr_.transformSystem_.GetDirectionsQuatsByIDs(dirLights.ids.data(), numDirLights, dirLightsDirections);

    for (int i = 0; const XMVECTOR& dirQuat : dirLightsDirections)
    {
        DirectX::XMStoreFloat3(&outData.dirLights[i].direction, dirQuat);
        ++i;
    }

    // ----------------------------------------------------
    // prepare data of spot lights

    ECS::cvector<ECS::SpotLight> spotLightsData;
    ECS::cvector<XMFLOAT3>       spotLightsPositions;
    ECS::cvector<XMFLOAT3>       spotLightsDirections;

    entityMgr_.lightSystem_.GetSpotLightsData(
        spotLights.ids.data(),
        numSpotLights,
        spotLightsData,
        spotLightsPositions,
        spotLightsDirections);

    for (index i = 0; i < numSpotLights; ++i)
    {
        outData.spotLights[i].ambient  = spotLightsData[i].ambient;
        outData.spotLights[i].diffuse  = spotLightsData[i].diffuse;
        outData.spotLights[i].specular = spotLightsData[i].specular;
        outData.spotLights[i].range    = spotLightsData[i].range;
        outData.spotLights[i].spot     = spotLightsData[i].spot;
        outData.spotLights[i].att      = spotLightsData[i].att;
    }

    for (int i = 0; const XMFLOAT3& pos : spotLightsPositions)
        outData.spotLights[i++].position = pos;

    for (int i = 0; const XMFLOAT3& dir : spotLightsDirections)
        outData.spotLights[i++].direction = dir;
}

///////////////////////////////////////////////////////////

int GraphicsClass::TestEnttSelection(const int sx, const int sy)
{
    // check if we have any entity by input screen coords;
    // 
    // in:   screen pixel coords
    // out:  0  - there is no entity, we didn't select any
    //       ID - we selected some entity so return its ID

    using namespace DirectX;

    const XMMATRIX& P = pCurrCamera_->Proj();
    const XMMATRIX& invView = pCurrCamera_->InverseView();

    const float xndc = (+2.0f * sx / d3d_.GetWindowWidth() - 1.0f);
    const float yndc = (-2.0f * sy / d3d_.GetWindowHeight() + 1.0f);

    // compute picking ray in view space
    const float vx = xndc / P.r[0].m128_f32[0];
    const float vy = yndc / P.r[1].m128_f32[1];

    // ray definition in view space
    XMVECTOR rayOrigin_ = { 0,0,0,1 };
    XMVECTOR rayDir_    = { vx, vy, 1, 0 };

    // assume we have not picked anything yet, 
    // so init the ID to 0 and triangle idx to -1
    uint32_t selectedEnttID = 0;
    int selectedTriangleIdx = -1;
    float tmin = MathHelper::Infinity;  // the distance along the ray where the intersection occurs
    float dist = 0;                     // the length of the ray from origin to the intersection point with the AABB


    // go through each visible entt and check if we have an intersection with it
    for (const EntityID enttID : entityMgr_.renderSystem_.GetAllVisibleEntts())
    {
        //const EntityID enttID = visEntts[i];
        const ModelID modelID = entityMgr_.modelSystem_.GetModelIdRelatedToEntt(enttID);
        const BasicModel& model = g_ModelMgr.GetModelByID(modelID);

        if (model.type_ == eModelType::Terrain)
        {
            continue;
        }
    
        // get an inverse world matrix of the current entt
        const XMMATRIX invWorld = entityMgr_.transformSystem_.GetInverseWorldMatrixOfEntt(enttID);
        const XMMATRIX toLocal = DirectX::XMMatrixMultiply(invView, invWorld);

        XMVECTOR rayOrigin = XMVector3TransformCoord(rayOrigin_, toLocal);   // supposed to take a point (w == 1)
        XMVECTOR rayDir    = XMVector3TransformNormal(rayDir_, toLocal);  // supposed to take a vec (w == 0)

        // make the ray direction unit length for the intersection tests
        rayDir = XMVector3Normalize(rayDir);


        // if we hit the bounding box of the model, then we might have picked
        // a model triangle, so do the ray/triangle tests;
        //
        // if we didn't hit the bounding box, then it is impossible that we
        // hit the model, so do not waste efford doing ray/triangle tests
        if (model.GetModelAABB().Intersects(rayOrigin, rayDir, dist))
        {
            // execute ray/triangle tests
            for (int i = 0; i < model.GetNumIndices() / 3; ++i)
            {
                // indices for this triangle
                UINT i0 = model.indices_[i * 3 + 0];
                UINT i1 = model.indices_[i * 3 + 1];
                UINT i2 = model.indices_[i * 3 + 2];

                // vertices for this triangle
                XMVECTOR v0 = XMLoadFloat3(&model.vertices_[i0].position);
                XMVECTOR v1 = XMLoadFloat3(&model.vertices_[i1].position);
                XMVECTOR v2 = XMLoadFloat3(&model.vertices_[i2].position);

                // we have to iterate over all the triangle in order 
                // to find the nearest intersection
                float t = 0.0f;

                if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
                {
                    if (t <= tmin)
                    {
                        // this is the new nearest picked entt and its triangle
                        tmin = t;
                        selectedTriangleIdx = i;
                        selectedEnttID = enttID;
                    }
                }
            }
        }
    }

    // print a msg about selection of the entity
    if (selectedEnttID)
    {
        const EntityName& name = entityMgr_.nameSystem_.GetNameById(selectedEnttID);
        const std::string msg = std::format("picked entt (id, name): {} {}", selectedEnttID, name);

        Log::Print(msg, eConsoleColor::YELLOW);
    }

    // return ID of the selected entt, or 0 if we didn't pick any
    return selectedEnttID;
}

///////////////////////////////////////////////////////////

void GraphicsClass::UpdateCameraEntity(
    const std::string& cameraEnttName,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& proj)
{
    // load updated camera data into ECS
    
    if (cameraEnttName.empty())
    {
        Log::Error("input name is empty");
        return;
    }

    const EntityID cameraID = entityMgr_.nameSystem_.GetIdByName("editor_camera");

    // if we found any entt by such name
    if (cameraID != 0)
    {
        entityMgr_.cameraSystem_.Update(cameraID, view, proj);
    }	
}

///////////////////////////////////////////////////////////

void GraphicsClass::BuildGeometryBuffers()
{
    HRESULT hr = S_OK;
    TreePointSprite spriteVertex;

    spriteVertex.pos = { 0,0,0 };
    spriteVertex.size = { 30, 30 };

    //
    // Create VB
    //
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(TreePointSprite);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vbInitData;
    vbInitData.pSysMem = &spriteVertex;

    hr = pDevice_->CreateBuffer(&vbd, &vbInitData, &pGeomVB_);
    Assert::NotFailed(hr, "can't create a vertex buffer for tree sprite");


    //
    // Create IB
    //
    UINT indices = 0;

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA ibInitData;
    ibInitData.pSysMem = &indices;

    hr = pDevice_->CreateBuffer(&ibd, &ibInitData, &pGeomIB_);
    Assert::NotFailed(hr, "can't create an index buffer for tree sprite");
}

} // namespace Core
