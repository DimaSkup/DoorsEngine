// ********************************************************************************
// Filename:     RenderDataPreparator.cpp
// 
// Created:      17.10.24
// ********************************************************************************
#include "rnd_data_preparator.h"
#include "../Texture/enum_texture_types.h"
#include "../Model/model_mgr.h"
#include "../Mesh/material_mgr.h"
#include "../Texture/texture_mgr.h"

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

// static arrays for internal purposes
static cvector<ECS::MaterialData> s_MaterialsDataPerEntt;
static cvector<DirectX::XMMATRIX> s_Worlds;
static cvector<ModelID>           s_ModelsIds;
static cvector<bool>              s_LodChangeFlags;

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

static cvector<float>             s_SqrDistances;


// static pointer to ECS entity manager (for internal purposes)
ECS::EntityMgr* s_pEnttMgr = nullptr;


//----------------------------------------------------------------------------------
// default constructor
//----------------------------------------------------------------------------------
RenderDataPreparator::RenderDataPreparator()
{
}

//----------------------------------------------------------------------------------
// Desc:   sort input data by material id
// Args:   - data: in/out container of data to sort
//----------------------------------------------------------------------------------
void SortEnttsByMaterials(cvector<EntityModelMesh>& data)
{
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
}

//----------------------------------------------------------------------------------
void SortByLods(
    ECS::EntityMgr& mgr,
    const XMFLOAT3& camPos,
    const EntityID* enttsIds,
    const vsize numEntts,
    cvector<ModelID>& modelsIds)
{
    if (numEntts == 0)
        return;

    s_TempEnttsIds.resize(numEntts, INVALID_ENTITY_ID);
    s_TempData.resize(numEntts);
    s_Positions.resize(numEntts);
    modelsIds.resize(numEntts);
    s_SqrDistances.resize(numEntts, FLT_MAX);


    // gather entities positions
    mgr.transformSys_.GetPositions(enttsIds, numEntts, s_Positions);

    for (index i = 0; i < numEntts; ++i)
    {
        s_SqrDistances[i] = (
            SQR(s_Positions[i].x - camPos.x) +
            SQR(s_Positions[i].y - camPos.y) +
            SQR(s_Positions[i].z - camPos.z));
    }

    ModelID    modelId = -1;
    BasicModel* pModel = nullptr;

    for (index i = 0; i < numEntts; ++i)
    {
        // if need to switch model
        if (modelId != modelsIds[i])
        {
            pModel  = &g_ModelMgr.GetModelById(modelsIds[i]);
            modelId = pModel->id_;
        }

        if (!pModel->HasLods())
            continue;

        // this model has lods so handle it
        for (int lodIdx = 0; lodIdx < pModel->GetNumLods(); ++lodIdx)
        {
            const eModelLodLevel lod = eModelLodLevel(lodIdx);
            const float sqrDistToLod = (float)SQR(pModel->GetLodDistance(lod));

            // if distance to instance is bigger than LOD's distance we change its model to another LOD
            if (s_SqrDistances[i] >= sqrDistToLod)
            {
                modelsIds[i]        = pModel->GetLod(lod);
                s_LodChangeFlags[i] = true;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   sort input data elements by distance from the camera
// Args:   - camPos:  camera's current position
//         - data:    in/out container of data to sort
//---------------------------------------------------------
void SortByDistance(
    ECS::EntityMgr& mgr,
    const XMFLOAT3& camPos,
    cvector<EntityModelMesh>& data)
{
    const vsize numElems = data.size();

    s_TempEnttsIds.resize(numElems, INVALID_ENTITY_ID);
    s_TempData.resize(numElems);
    s_Positions.resize(numElems);

    // gather entities ids
    for (index i = 0; i < numElems; ++i)
        s_TempEnttsIds[i] = data[i].enttId;

    // gather entities positions
    mgr.transformSys_.GetPositions(s_TempEnttsIds.data(), numElems, s_Positions);

    // convert to transient data
    for (index i = 0; i < numElems; ++i)
    {
        s_TempData[i] = {
            data[i].enttId,
            data[i].matId,
            data[i].modelId,
            data[i].subsetId };
    }

    for (index i = 0; i < numElems; ++i)
    {
        s_TempData[i].sqrDistToCamera =
            SQR(s_Positions[i].x - camPos.x) +
            SQR(s_Positions[i].y - camPos.y) +
            SQR(s_Positions[i].z - camPos.z);
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
        printf("%2d ", (int)s_LodChangeFlags[i]);
    }
    printf("\n");

    printf("%slod_1:       ", MAGENTA);
    for (index i = 0; i < numEntts; ++i)
    {
        BasicModel& model = g_ModelMgr.GetModelById(modelsIds[i]);
        printf("%2d ", (int)model.lods_[LOD_1]);
    }
    printf("\n");

    printf("%slod_2:       ", WHITE);
    for (index i = 0; i < numEntts; ++i)
    {
        BasicModel& model = g_ModelMgr.GetModelById(modelsIds[i]);
        printf("%2d ", (int)model.lods_[LOD_2]);
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
        if ((data.matId != matId) || (data.modelId != modelId) || (data.subsetId != subsetId))
        {
            pMat = &g_MaterialMgr.GetMatById(data.matId);
            matId = data.matId;

            // add new empty instance batch and setup it
            instanceBatches.push_back(Render::InstanceBatch());

            pInstances               = &instanceBatches.back();
            pInstances->modelId      = data.modelId;
            pInstances->subsetId     = data.subsetId;
            pInstances->shaderId     = pMat->shaderId;
            modelId                  = data.modelId;
            subsetId                 = data.subsetId;
            pInstances->numInstances = 1;
            pInstances->renderStates = pMat->renderStates;       

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
// Desc:   prepare vertex/index buffers data for each input instance batch
// Args: 
//----------------------------------------------------------------------------------
void PrepareBuffers(cvector<Render::InstanceBatch>& instanceBatches)
{
    if (instanceBatches.empty())
        return;

    ModelID modelId = INVALID_MODEL_ID;
    BasicModel* pModel = &g_ModelMgr.GetModelById(INVALID_MODEL_ID);

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
        
        const MeshGeometry& meshes = pModel->meshes_;
        const SubmeshID   subsetId = batch.subsetId;
        const Subset&       subset = meshes.subsets_[subsetId];

        // instances batch name
        strncpy(batch.name, pModel->name_, MAX_LEN_MODEL_NAME);

        // vertex/index buffers data
        batch.vertexStride = meshes.vb_.GetStride();
        batch.pVB = meshes.vb_.Get();
        batch.pIB = meshes.ib_.Get();

        // subset metadata
        strncpy(batch.subset.name, subset.name, MAX_LEN_MESH_NAME);
        batch.subset.vertexStart = subset.vertexStart;
        batch.subset.vertexCount = subset.vertexCount;
        batch.subset.indexStart  = subset.indexStart;
        batch.subset.indexCount  = subset.indexCount;
    }
}

//----------------------------------------------------------------------------------
// Desc:   prepare instances data and instances buffer for rendering
//         entities by input IDs
//
// Args:   - visibleEntts:     currently visible entities
//         - cameraPos:        camera's current position
//         - pEnttMgr:         a ptr to ECS entity manager
//         - instanceBufData:  data for the instances buffer
//         - instances:        models data for rendering
//----------------------------------------------------------------------------------
void RenderDataPreparator::PrepareEnttsDataForRendering(
    cvector<EntityID>& visibleEntts,
    const XMFLOAT3& cameraPos,
    ECS::EntityMgr* pEnttMgr,
    Render::RenderDataStorage& storage)
{
    s_pEnttMgr = pEnttMgr;

    //------------------------------------------------

    // clear the render data storage before filling it with data
    storage.Clear();

    s_VisEntts.resize(visibleEntts.size());
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

    const EntityID* enttsIds = s_VisEntts.data();
    const size      numEntts = s_VisEntts.size();

    if (numEntts == 0)
        return;

    // get materials ids for each subset of each input entity
    pEnttMgr->materialSys_.GetDataByEnttsIds(enttsIds, numEntts, s_MaterialsDataPerEntt);


    // compute the number of instances
    vsize numAllInstances = 0;

    for (const ECS::MaterialData& matData : s_MaterialsDataPerEntt)
        numAllInstances += matData.materialsIds.size();

    //------------------------------------------------

    s_Data.resize(numAllInstances);

    s_MaskedGroup.clear();
    s_OpaqueGroup.clear();
    s_BlendGroup.clear();
    s_BlendTransparentGroup.clear();

    s_MaskedGroup.reserve(numAllInstances);
    s_OpaqueGroup.reserve(numAllInstances);
    s_BlendGroup.reserve(numAllInstances);
    s_BlendTransparentGroup.reserve(numAllInstances);

    Render::InstancesBuf& instancesBuf = storage.instancesBuf;
    instancesBuf.Resize((int)numAllInstances);

    //------------------------------------------------

    pEnttMgr->modelSys_.GetModelsIdsPerEntts(enttsIds, numEntts, s_ModelsIds);

    // flags to define if we need to render
    // entity using model with lower detail level (higher LOD)
    s_LodChangeFlags.resize(numEntts);
    memset(s_LodChangeFlags.data(), 0, s_LodChangeFlags.size());


#if PRINT_DBG_DATA
    PrintEnttModelData(enttsIds, numEntts, s_ModelsIds, "BEFORE SORTING BY LODS");
#endif

    // change lod of model if necessary
    SortByLods(*pEnttMgr, cameraPos, enttsIds, numEntts, s_ModelsIds);

#if PRINT_DBG_DATA
    PrintEnttModelData(enttsIds, numEntts, s_ModelsIds, "AFTER SORTING BY LODS");
#endif


    BasicModel* pModel = nullptr;
    ModelID modelId = -1;
    int instanceIdx = 0;

    // prepare common data (only ids)
    for (int i = 0; i < numEntts; ++i)
    {
        // if lod was changed (from LOD_0) the entity instead of
        // multiple meshes will have only one mesh (from LOD_1, or LOD_2, etc.)
        if (s_LodChangeFlags[i] == true)
        {
            if (modelId != s_ModelsIds[i])
            {
                modelId = s_ModelsIds[i];
                pModel = &g_ModelMgr.GetModelById(modelId);
            }

            const MaterialID matId = pModel->meshes_.subsets_[0].materialId;
            s_Data[instanceIdx].enttId   = enttsIds[i];
            s_Data[instanceIdx].matId    = matId;
            s_Data[instanceIdx].subsetId = 0;
            s_Data[instanceIdx].modelId  = s_ModelsIds[i];

            instanceIdx++;
            continue;
        }


        // LOD wan't changed so handle entity as usual
        const ECS::MaterialData& matData = s_MaterialsDataPerEntt[i];

        for (int matIdx = 0; const MaterialID& matId : matData.materialsIds)
        {
            s_Data[instanceIdx].enttId   = enttsIds[i];
            s_Data[instanceIdx].matId    = matId;
            s_Data[instanceIdx].subsetId = matIdx;
            s_Data[instanceIdx].modelId  = s_ModelsIds[i];

            matIdx++;
            instanceIdx++;
        }
    }

    // if any entity's LOD was changed the actual number of instance was reduced
    // so handle it here
    const int numActualInstances = instanceIdx;
    s_Data.resize(numActualInstances);

    //------------------------------------------------

    // sort entities by materials (later we will put them into batches by materials)
#if PRINT_DBG_DATA
    PrintData(s_Data, "BEFORE SORTING BY MATERIALS");
#endif

    SortEnttsByMaterials(s_Data);

#if PRINT_DBG_DATA
    PrintData(s_Data, "AFTER SORTING BY MATERIALS AND BEFORE LODS");
#endif

    //------------------------------------------------

    MaterialID matId = INVALID_MATERIAL_ID;
    const Material* pMat = &g_MaterialMgr.GetMatById(matId);

    // group entities geometry type (masked, opaque, blended, etc.)
    for (vsize i = 0; i < numActualInstances; ++i)
    {
        // if current material differs from the previous one we get another material
        if (s_Data[i].matId != matId)
        {
            pMat  = &g_MaterialMgr.GetMatById(s_Data[i].matId);
            matId = s_Data[i].matId;
        }

        // define to which rendering group does this instance belongs to:

        // 1. blending group (add / sub / mul)
        if (pMat->renderStates & MAT_BLEND_NOT_TRANSPARENT)
            s_BlendGroup.push_back(s_Data[i]);

        // 2. blending group (transparency)
        else if (pMat->renderStates & MAT_PROP_BS_TRANSPARENCY)
            s_BlendTransparentGroup.push_back(s_Data[i]);

        // 3. masked / alpha clipping group
        else if (pMat->renderStates & MAT_PROP_ALPHA_CLIPPING)
            s_MaskedGroup.push_back(s_Data[i]);

        // 4. opaque group
        else
            s_OpaqueGroup.push_back(s_Data[i]);
    }

#if PRINT_DBG_DATA
    PrintData(s_MaskedGroup,           "MASKED");
    PrintData(s_OpaqueGroup,           "OPAQUE");
    PrintData(s_BlendGroup,            "BLEND");
    PrintData(s_BlendTransparentGroup, "BLEND (TRANSPARENT)");
#endif

    // sort both blending groups elements by distance from the camera
    SortByDistance(*pEnttMgr, cameraPos, s_BlendGroup);
    SortByDistance(*pEnttMgr, cameraPos, s_BlendTransparentGroup);

    //------------------------------------------------

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

    // gather entts ids from each render group
    s_EnttsIdPerInstance.resize(numActualInstances);
    instanceIdx = 0;


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

} // namespace Core
