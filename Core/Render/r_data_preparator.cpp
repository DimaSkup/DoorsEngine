// ********************************************************************************
// Filename:     r_data_preparator.cpp
// 
// Created:      17.10.24
// ********************************************************************************
#include "r_data_preparator.h"
#include "../Texture/enum_texture_types.h"
#include "../Model/model_mgr.h"
#include "../Mesh/material_mgr.h"
#include "../Texture/texture_mgr.h"
#include <Render/CRender.h>

#define PRINT_DBG_DATA 0

using namespace DirectX;


namespace Core
{

//---------------------------------------------------------

struct EntityDataAndPos
{
    EntityDataAndPos() {}

    EntityDataAndPos(
        const EntityID inEnttId,
        const MaterialID inMatId,
        const ModelID inModelId,
        const SubmeshID inSubsetId)
        :
        enttId(inEnttId),
        matId(inMatId),
        modelId(inModelId),
        subsetId(inSubsetId) {}


    EntityID   enttId = INVALID_ENTITY_ID;
    MaterialID matId = INVALID_MATERIAL_ID;
    ModelID    modelId = INVALID_MODEL_ID;
    SubmeshID  subsetId = 0;
    float      sqrDistToCamera = 0;
};

//---------------------------------------------------------

// static pointer to ECS entity manager (for internal purposes)
ECS::EntityMgr* s_pEnttMgr = nullptr;


// static arrays for internal purposes
static cvector<ECS::MaterialData> s_MaterialsDataPerEntt;
static cvector<DirectX::XMMATRIX> s_Worlds;
static cvector<ModelID>           s_ModelsIds;      
static cvector<bool>              s_IsLod;          // flags to define if model by responsible index is some kind of lod or it is an original model

static cvector<EntityModelMesh>   s_Data;
static cvector<EntityModelMesh>   s_MaskedGroup;
static cvector<EntityModelMesh>   s_OpaqueGroup;
static cvector<EntityModelMesh>   s_BlendGroup;
static cvector<EntityModelMesh>   s_BlendTransparentGroup;

// for sorting by distance
static cvector<EntityID>          s_TempEnttsIds;
static cvector<EntityDataAndPos>  s_TempData;
static cvector<XMFLOAT3>          s_Positions;

static cvector<EntityID>          s_VisEntts;
static cvector<EntityID>          s_EnttsIdPerInstance;

static cvector<float>             s_SqrDistances;   // array of squared distanced from camera to entity


//----------------------------------------------------------------------------------
// forward declaration of private helpers
//----------------------------------------------------------------------------------

void LodsStuff(const XMFLOAT3& camPos, cvector<EntityID>& enttsIds, cvector<ModelID>& modelsIds);

vsize PrepareCommonIds(const EntityID* enttsIds, const vsize numEntts);

void SortEnttsByMaterials (cvector<EntityModelMesh>& data);
void GroupEnttsByGeomTypes(const vsize numRenderItems);
void SortByDistance       (const XMFLOAT3& camPos, cvector<EntityModelMesh>& data);

void PrepareMaterials(
    int& instanceMatIdx,
    const cvector<EntityModelMesh>& renderGroup,
    Render::InstancesBuf& instancesBuf,
    cvector<Render::InstanceBatch>& instanceBatches);

void PrepareBuffers(cvector<Render::InstanceBatch>& instanceBatches);


//----------------------------------------------------------------------------------
// Desc:   prepare instances data and instances buffer for rendering
//         entities by input IDs
//
// Args:   - visibleEntts:     currently visible entities
//         - cameraPos:        camera's current position
//         - pEnttMgr:         a ptr to ECS entity manager
//         - storage:          container with prepared data for rendering
//----------------------------------------------------------------------------------
void RenderDataPreparator::PrepareEnttsDataForRendering(
    cvector<EntityID>& visibleEntts,
    const XMFLOAT3& cameraPos,
    ECS::EntityMgr* pEnttMgr,
    Render::RenderDataStorage& storage)
{
    assert(pEnttMgr);
    s_pEnttMgr = pEnttMgr;

    //------------------------------------------------

    // clear the render data storage before filling it with data
    storage.Clear();

    s_VisEntts.reserve(visibleEntts.size());
    s_VisEntts.clear();


    // separate visible entities with animation component from others
    // since we will render such entities in a separate way
    const cvector<EntityID>& animatedEntts = pEnttMgr->animationSys_.GetEnttsIds();

    size numNotAnimEntts = 0;

    for (const EntityID enttId : visibleEntts)
    {
        s_VisEntts[numNotAnimEntts] = enttId;
        numNotAnimEntts += (!animatedEntts.binary_search(enttId));
    }
    s_VisEntts.resize(numNotAnimEntts);

    // if we have no non-animated visible entities
    if (s_VisEntts.empty())
        return;

    //------------------------------------------------

    pEnttMgr->modelSys_.GetModelsIdsPerEntts(
        s_VisEntts.data(),
        s_VisEntts.size(),
        s_ModelsIds);

    // change lod of model if necessary
    LodsStuff(cameraPos, s_VisEntts, s_ModelsIds);


    const vsize numRenderItems = PrepareCommonIds(s_VisEntts.data(), s_VisEntts.size());
    storage.instancesBuf.Resize((int)numRenderItems);

    //------------------------------------------------

    // sort entities by materials (later we will split them into batches by materials)
    SortEnttsByMaterials(s_Data);

    //------------------------------------------------

    // group entities by geometry type (masked, opaque, blended, etc.)
    GroupEnttsByGeomTypes(numRenderItems);
    
    // sort both blending groups elements by distance from the camera
    SortByDistance(cameraPos, s_BlendGroup);
    SortByDistance(cameraPos, s_BlendTransparentGroup);

    //------------------------------------------------

    // prepare materials for the instances buffer and each instances batch
    int instanceMatIdx = 0;
    PrepareMaterials(instanceMatIdx, s_MaskedGroup,           storage.instancesBuf, storage.masked);
    PrepareMaterials(instanceMatIdx, s_OpaqueGroup,           storage.instancesBuf, storage.opaque);
    PrepareMaterials(instanceMatIdx, s_BlendGroup,            storage.instancesBuf, storage.blended);
    PrepareMaterials(instanceMatIdx, s_BlendTransparentGroup, storage.instancesBuf, storage.blendedTransparent);

    //------------------------------------------------

    // prepare vertex/index buffers data for each instance batch
    PrepareBuffers(storage.masked);
    PrepareBuffers(storage.opaque);
    PrepareBuffers(storage.blended);
    PrepareBuffers(storage.blendedTransparent);

    //------------------------------------------------

    // gather entts ids from each render group into a single array
    s_EnttsIdPerInstance.resize(numRenderItems);
    int instanceIdx = 0;

    for (const EntityModelMesh& data : s_MaskedGroup)
        s_EnttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh & data : s_OpaqueGroup)
        s_EnttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh& data : s_BlendGroup)
        s_EnttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh& data : s_BlendTransparentGroup)
        s_EnttsIdPerInstance[instanceIdx++] = data.enttId;


    // prepare world matrix for each instance
    PrepareInstancesWorldMatrices(s_EnttsIdPerInstance, storage);
}

//----------------------------------------------------------------------------------
// Desc:  execute LODs "switching" here, we have 3 possible variants:
//        1. model is too close so don't use any its LOD
//        2. see both model and its LOD1 (for smooth switching, prevent popping)
//        3. see only model's some LOD, and remove model by itself from render list
//----------------------------------------------------------------------------------
void LodsStuff(
    const XMFLOAT3& camPos,
    cvector<EntityID>& enttsIds,
    cvector<ModelID>& modelsIds)
{
    if (enttsIds.empty())
        return;

    assert(s_pEnttMgr);
    assert(enttsIds.size() == modelsIds.size());

#if PRINT_DBG_DATA
    PrintEnttModelData(enttsIds, numEntts, s_ModelsIds, "BEFORE SORTING BY LODS");
#endif

    const vsize numEntts = enttsIds.size();

    cvector<EntityID> enttsIdsTmp;
    cvector<ModelID>  modelsIdsTmp;

    enttsIdsTmp.reserve(numEntts + 100);
    modelsIdsTmp.reserve(numEntts + 100);

    // reset flags to define if we need to render
    // entity using model with lower detail level (higher LOD)
    s_IsLod.clear();

    // compute squared distances from camera to entities
    s_SqrDistances.resize(numEntts, FLT_MAX);
    s_pEnttMgr->transformSys_.GetPositions(enttsIds.data(), numEntts, s_Positions);

    for (index i = 0; const XMFLOAT3& p : s_Positions)
    {
        s_SqrDistances[i++] = (SQR(p.x-camPos.x) + SQR(p.y-camPos.y) + SQR(p.z-camPos.z));
    }

    ModelID modelId = -1;
    Model*  pModel = nullptr;
    constexpr uint16 lodSwitchRange = 15;

    ModelID lod1 = 0;
    ModelID lod2 = 0;
    uint16  lod1Dist = 0;
    uint16  lod2SqrDist = 0;
    float   sqrDistLodAppear = 0;
    float   sqrDistModelRemove = 0;
    bool    bModelHasLods = false;

    // for each entity: check if we need to switch models lods
    for (index i = 0; i < numEntts; ++i)
    {
        // if need to switch model
        if (modelId != modelsIds[i])
        {
            modelId = modelsIds[i];
            pModel  = &g_ModelMgr.GetModelById(modelId);
            bModelHasLods = pModel->HasLods();

            // precompute some values for LODs
            if (bModelHasLods)
            {
                lod1 = pModel->GetLod(LOD_1);
                lod2 = pModel->GetLod(LOD_2);

                lod1Dist    = pModel->GetLodDistance(LOD_1);
                lod2SqrDist = SQR(pModel->GetLodDistance(LOD_2));

                sqrDistLodAppear   = (float)SQR(lod1Dist - lodSwitchRange);
                sqrDistModelRemove = (float)SQR(lod1Dist + lodSwitchRange);
            }
        }

        if (!bModelHasLods)
        {
            enttsIdsTmp.push_back(enttsIds[i]);
            modelsIdsTmp.push_back(modelsIds[i]);
            s_IsLod.push_back(false);
            continue;
        }
    

        // sqr distance to entity
        const float sqrDist = s_SqrDistances[i];

        // if currently don't need to use any LOD
        if (sqrDist < sqrDistLodAppear)
        {
            enttsIdsTmp.push_back(enttsIds[i]);
            modelsIdsTmp.push_back(modelsIds[i]);
            s_IsLod.push_back(false);
            continue;
        }
            

        // remove model from render list if it is farther than its fade out range
        if (sqrDist < sqrDistModelRemove)
        {
            enttsIdsTmp.push_back(enttsIds[i]);
            modelsIdsTmp.push_back(modelsIds[i]);
            s_IsLod.push_back(false);
        }

        // if we need to use LOD2...
        if (lod2 && (sqrDist > lod2SqrDist))
        {
            enttsIdsTmp.push_back(enttsIds[i]);
            modelsIdsTmp.push_back(lod2);
            s_IsLod.push_back(true);
        }

        // use LOD1...
        else
        {
            enttsIdsTmp.push_back(enttsIds[i]);
            modelsIdsTmp.push_back(lod1);
            s_IsLod.push_back(true);
        }
    }

    enttsIds = enttsIdsTmp;
    modelsIds = modelsIdsTmp;


#if PRINT_DBG_DATA
    PrintEnttModelData(enttsIds, numEntts, s_ModelsIds, "AFTER SORTING BY LODS");
#endif
}

//---------------------------------------------------------
// Desc:  prepare common data (only ids: entity, material, mesh, model)
//        for our render items (each separate instance of geometry)
// Ret:   number of render items
//---------------------------------------------------------
vsize PrepareCommonIds(const EntityID* enttsIds, const vsize numEntts)
{
    assert(enttsIds);
    assert(numEntts > 0);

    Model*  pModel = nullptr;
    ModelID modelId = -1;
    vsize   instanceIdx = 0;

    ECS::MaterialSystem& matSys = s_pEnttMgr->materialSys_;

    s_Data.clear();

    // get materials ids for each subset of each input entity
    s_pEnttMgr->materialSys_.GetDataByEnttsIds(enttsIds, numEntts, s_MaterialsDataPerEntt);
    
    for (int i = 0; i < numEntts; ++i)
    {
        // is this entity a LOD (lod1, lod2, etc.) ?
        if (s_IsLod[i])
        {
            if (modelId != s_ModelsIds[i])
            {
                modelId = s_ModelsIds[i];
                pModel = &g_ModelMgr.GetModelById(modelId);
            }

            s_Data.push_back(EntityModelMesh());
            EntityModelMesh& data = s_Data.back();

            data.enttId     = enttsIds[i];
            data.matId      = pModel->GetSubsets()[0].materialId;
            data.subsetId   = 0;
            data.modelId    = s_ModelsIds[i];

            instanceIdx++;
            continue;
        }


        // this entity is a usual geometry (not a LOD)
        //const ECS::MaterialData& matData = s_MaterialsDataPerEntt[i];
        const ECS::MaterialData& matData = matSys.GetDataByEnttId(enttsIds[i]);

        for (int matIdx = 0; const MaterialID matId : matData.materialsIds)
        {
            s_Data.push_back(EntityModelMesh());
            EntityModelMesh& data = s_Data.back();

            data.enttId     = enttsIds[i];
            data.matId      = matId;
            data.subsetId   = matIdx;
            data.modelId    = s_ModelsIds[i];

            matIdx++;
            instanceIdx++;
        }
    }

    return instanceIdx;
}

//---------------------------------------------------------
// Desc:   sort input data elements by distance from the camera
// Args:   - camPos:  camera's current position
//         - data:    in/out container of data to sort
//---------------------------------------------------------
void SortByDistance(const XMFLOAT3& camPos, cvector<EntityModelMesh>& data)
{
    const vsize numElems = data.size();

    s_TempEnttsIds.resize(numElems, INVALID_ENTITY_ID);
    s_TempData.resize(numElems);
    s_Positions.resize(numElems);

    // gather entities ids
    for (index i = 0; i < numElems; ++i)
        s_TempEnttsIds[i] = data[i].enttId;

    // gather entities positions
    s_pEnttMgr->transformSys_.GetPositions(s_TempEnttsIds.data(), numElems, s_Positions);

    // convert to transient data
    for (index i = 0; i < numElems; ++i)
    {
        s_TempData[i] = {
            data[i].enttId,
            data[i].matId,
            data[i].modelId,
            data[i].subsetId };
    }

    for (index i = 0; const XMFLOAT3& p : s_Positions)
    {
        s_TempData[i++].sqrDistToCamera =
            SQR(p.x-camPos.x) +
            SQR(p.y-camPos.y) +
            SQR(p.z-camPos.z);
    }


    // sort by square of distance to camera
    std::qsort(
        s_TempData.data(),
        s_TempData.size(),
        sizeof(EntityDataAndPos),
        [](const void* x, const void* y)
        {
            const EntityDataAndPos& arg1 = *(EntityDataAndPos*)(x);
            const EntityDataAndPos& arg2 = *(EntityDataAndPos*)(y);

            if (arg1.sqrDistToCamera > arg2.sqrDistToCamera)
                return -1;

            if (arg1.sqrDistToCamera < arg2.sqrDistToCamera)
                return 1;

            return 0;
        });

    // store sorted data into the output array
    for (int i = 0; EntityDataAndPos& elem : s_TempData)
    {
        data[i++] = { elem.enttId, elem.matId, elem.modelId, elem.subsetId };
    }
}

//----------------------------------------------------------------------------------
// for debug
//----------------------------------------------------------------------------------
void PrintEnttModelData(
    const EntityID* enttsIds,
    const vsize numEntts,
    const cvector<ModelID>& modelsIds,
    const char* msg)
{
    assert(msg != nullptr);
    printf("%s:\n", msg);

    printf("%sentt id:     ", YELLOW);
    for (index i = 0; i < numEntts; ++i)
    {
        printf("%2d ", (int)enttsIds[i]);
    }
    printf("\n");

    printf("%smodel id:    ", CYAN);
    for (index i = 0; i < numEntts; ++i)
    {
        printf("%2d ", (int)modelsIds[i]);
    }
    printf("\n");

    printf("%slod changed: ", GREEN);
    for (index i = 0; i < numEntts; ++i)
    {
        printf("%2d ", (int)s_IsLod[i]);
    }
    printf("\n");

    printf("%slod_1:       ", MAGENTA);
    for (index i = 0; i < numEntts; ++i)
    {
        Model& model = g_ModelMgr.GetModelById(modelsIds[i]);
        printf("%2d ", (int)model.GetLod(LOD_1));
    }
    printf("\n");

    printf("%slod_2:       ", WHITE);
    for (index i = 0; i < numEntts; ++i)
    {
        Model& model = g_ModelMgr.GetModelById(modelsIds[i]);
        printf("%2d ", (int)model.GetLod(LOD_2));
    }

    printf("\n\n");
    printf("%s", RESET);
}

//---------------------------------------------------------

void PrintData(const cvector<EntityModelMesh>& data, const char* msg)
{
    assert(msg != nullptr);

    printf("%s:\n", msg);

    printf("%sentt id:  ", YELLOW);
    for (const EntityModelMesh d : data)
    {
        printf("%2ld ", d.enttId);
    }
    printf("\n");

    printf("%smat id:   ", GREEN);
    for (const EntityModelMesh d : data)
    {
        printf("%2ld ", d.matId);
    }
    printf("\n");

    printf("%smodel id: ", CYAN);
    for (const EntityModelMesh d : data)
    {
        printf("%2ld ", d.modelId);
    }
    printf("\n");

    printf("%smesh id:  ", MAGENTA);
    for (const EntityModelMesh d : data)
    {
        printf("%2ld ", d.subsetId);
    }

    printf("\n\n");
    printf("%s", RESET);
}

//----------------------------------------------------------------------------------
// Desc:  prepare materials for the instances buffer and instances batches
// Args:  - renderGroup:    container for ids of particular rendering group
//                          (masked, opaque, blended, etc.)
//        - instancesBuf:   buffer for instances data
//        - instaceBatches: output a container for instance batches
//----------------------------------------------------------------------------------
void PrepareMaterials(
    int& instanceMatIdx,
    const cvector<EntityModelMesh>& renderGroup,
    Render::InstancesBuf& instancesBuf,
    cvector<Render::InstanceBatch>& instanceBatches)
{
    MaterialID matId                  = -1;
    ModelID    modelId                = INVALID_MODEL_ID;
    SubmeshID  subsetId               = -1;
    Material*  pMat                   = nullptr;
    Render::InstanceBatch* pInstances = nullptr;


    for (const EntityModelMesh& data : renderGroup)
    {
        // if current material differs from the previous one we get another material
        // if model OR submesh was changed to another one
        const bool sameMaterial = data.matId == matId;
        const bool sameModel    = data.modelId == modelId;
        const bool sameMesh     = data.subsetId == subsetId;

        if (!sameMaterial || !sameModel || !sameMesh)
        {
            pMat = &g_MaterialMgr.GetMatById(data.matId);
            matId = data.matId;

            // add new empty instance batch and setup it
            instanceBatches.push_back(Render::InstanceBatch());

            pInstances               = &instanceBatches.back();
            pInstances->shaderId     = pMat->shaderId;
            pInstances->modelId      = data.modelId;
            pInstances->subsetId     = data.subsetId;
            pInstances->numInstances = 1;
            pInstances->rsId         = pMat->rsId;      // raster state id
            pInstances->bsId         = pMat->bsId;      // blend state id
            pInstances->dssId        = pMat->dssId;     // depth-stencil state id
            pInstances->alphaClip    = pMat->alphaClip; // use alpha clip

            modelId                  = data.modelId;
            subsetId                 = data.subsetId;

            // get textures for the current instance batch
            g_TextureMgr.GetTexViewsByIds(pMat->texIds, NUM_TEXTURE_TYPES, pInstances->textures);
        }

        // we have the same material/model/submesh (but another instance of another entt)
        else
        {
            pInstances->numInstances++;
        }


        // push material colors of this instance into the instances buffer
        instancesBuf.materials_[instanceMatIdx] =
        {
            DirectX::XMFLOAT4(&pMat->ambient.x),
            DirectX::XMFLOAT4(&pMat->diffuse.x),
            DirectX::XMFLOAT4(&pMat->specular.x),
            DirectX::XMFLOAT4(&pMat->reflect.x),
        };

        ++instanceMatIdx;
    }
}

//----------------------------------------------------------------------------------
// Desc:   prepare VB/IB data for each input instance batch
// Args: 
//----------------------------------------------------------------------------------
void PrepareBuffers(cvector<Render::InstanceBatch>& instanceBatches)
{
    if (instanceBatches.empty())
        return;

    ModelID modelId = INVALID_MODEL_ID;
    Model* pModel = &g_ModelMgr.GetModelById(INVALID_MODEL_ID);


     // prepare geometry data for each instance batch
    for (Render::InstanceBatch& batch : instanceBatches)
    {
        // if we switched to another model
        if (batch.modelId != modelId)
        {
            pModel = &g_ModelMgr.GetModelById(batch.modelId);
            modelId = batch.modelId;
        }

        assert(pModel != nullptr);
        
        const MeshGeometry& meshes = pModel->GetMeshes();
        const SubmeshID   subsetId = batch.subsetId;
        const Subset&       subset = meshes.subsets_[subsetId];

        batch.primTopology = pModel->GetPrimTopology();

        // vertex/index buffers data
        batch.vertexStride = meshes.vb_.GetStride();
        batch.pVB          = meshes.vb_.Get();
        batch.pIB          = meshes.ib_.Get();

        // subset metadata
        strncpy(batch.subset.name, subset.name, MAX_LEN_MESH_NAME);
        batch.subset.vertexStart = subset.vertexStart;
        batch.subset.vertexCount = subset.vertexCount;
        batch.subset.indexStart  = subset.indexStart;
        batch.subset.indexCount  = subset.indexCount;

        // instances batch name
        strncpy(batch.name, pModel->GetName(), MAX_LEN_MODEL_NAME);
    }
}


//----------------------------------------------------------------------------------
// Desc:   push world matrices into instances buffer for input instance batches
//----------------------------------------------------------------------------------
void PushWorldsIntoInstanceBuf(
    int& instanceIdx,
    const cvector<Render::InstanceBatch>& instancesBatches,
    const cvector<DirectX::XMMATRIX>& inWorlds,
    DirectX::XMMATRIX* outWorlds)
{
    for (const Render::InstanceBatch& instances : instancesBatches)
    {
        memcpy(
            &(outWorlds[instanceIdx]),
            &(s_Worlds[instanceIdx]),
            sizeof(DirectX::XMMATRIX) * instances.numInstances);

        instanceIdx += instances.numInstances;
    }
}

//---------------------------------------------------------
// Desc:   prepare world matrix for each instance
//---------------------------------------------------------
void RenderDataPreparator::PrepareInstancesWorldMatrices(
    const cvector<EntityID>& enttIdPerInstance,
    Render::RenderDataStorage& storage)
{
    // if we have no entities to render
    if (enttIdPerInstance.empty())
        return;

    s_Worlds.clear();

    // get world matrices
    s_pEnttMgr->transformSys_.GetWorlds(
        enttIdPerInstance.data(),
        enttIdPerInstance.size(),
        s_Worlds);

    int instanceIdx = 0;
    DirectX::XMMATRIX* outWorlds = storage.instancesBuf.worlds_;

    // prepare world matrices for each rendering group (masked, opaque, blended, etc.)
    PushWorldsIntoInstanceBuf(instanceIdx, storage.masked, s_Worlds, outWorlds);
    PushWorldsIntoInstanceBuf(instanceIdx, storage.opaque, s_Worlds, outWorlds);
    PushWorldsIntoInstanceBuf(instanceIdx, storage.blended, s_Worlds, outWorlds);
    PushWorldsIntoInstanceBuf(instanceIdx, storage.blendedTransparent, s_Worlds, outWorlds);
}

//---------------------------------------------------------
// Desc:  group entities by geometry type (masked, opaque, blended, etc.)
//---------------------------------------------------------
void RenderDataPreparator::GroupEnttsByGeomTypes(const vsize numRenderItems)
{
    MaterialID                      matId = INVALID_MATERIAL_ID;
    const Material*                  pMat = &g_MaterialMgr.GetMatById(matId);
    const Render::RenderStates& rndStates = Render::g_Render.GetRenderStates();

    s_MaskedGroup.clear();
    s_OpaqueGroup.clear();
    s_BlendGroup.clear();
    s_BlendTransparentGroup.clear();


    for (index i = 0; i < numRenderItems; ++i)
    {
        // if current material differs from the previous one we get another material
        if (s_Data[i].matId != matId)
        {
            pMat = &g_MaterialMgr.GetMatById(s_Data[i].matId);
            matId = s_Data[i].matId;
        }

        //
        // define to which rendering group does this instance belongs to:
        //
        bool isBlended = false;
        bool isTransparent = false;

        rndStates.IsBlendEnabled(pMat->bsId, isTransparent, isBlended);

        // 1. blending
        if (isBlended)
        {
            if (isTransparent)
                s_BlendTransparentGroup.push_back(s_Data[i]);
            else
                s_BlendGroup.push_back(s_Data[i]);
        }

        // 2. masked / alpha clipping group
        else if (pMat->HasAlphaClip())
            s_MaskedGroup.push_back(s_Data[i]);

        // 3. opaque group
        else
            s_OpaqueGroup.push_back(s_Data[i]);
    }

#if PRINT_DBG_DATA
    PrintData(s_MaskedGroup,           "MASKED");
    PrintData(s_OpaqueGroup,           "OPAQUE");
    PrintData(s_BlendGroup,            "BLEND");
    PrintData(s_BlendTransparentGroup, "BLEND (TRANSPARENT)");
#endif
}

//----------------------------------------------------------------------------------
// Desc:   sort input data by material id
// Args:   - data: in/out container of data to sort
//----------------------------------------------------------------------------------
void SortEnttsByMaterials(cvector<EntityModelMesh>& data)
{

#if PRINT_DBG_DATA
    PrintData(s_Data, "BEFORE SORTING BY MATERIALS");
#endif

    std::qsort(
        data.data(),
        data.size(),
        sizeof(EntityModelMesh),
        [](const void* x, const void* y)
        {
            const EntityModelMesh& arg1 = *(EntityModelMesh*)(x);
            const EntityModelMesh& arg2 = *(EntityModelMesh*)(y);

            if (arg1.matId < arg2.matId)
                return -1;

            if (arg1.matId > arg2.matId)
                return 1;

            return 0;
        });


#if PRINT_DBG_DATA
    PrintData(s_Data, "AFTER SORTING BY MATERIALS");
#endif
}


} // namespace Core
