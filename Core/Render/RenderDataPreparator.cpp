// ********************************************************************************
// Filename:     RenderDataPreparator.cpp
// 
// Created:      17.10.24
// ********************************************************************************
#include "RenderDataPreparator.h"
#include "../Texture/TextureTypes.h"
#include "../Model/ModelMgr.h"
#include "../Mesh/MaterialMgr.h"
#include "../Texture/TextureMgr.h"

#define PRINT_DBG_DATA 0

using namespace DirectX;


namespace Core
{

// static arrays for internal purposes
cvector<ECS::MaterialData>          s_MaterialsDataPerEntt;
cvector<DirectX::XMMATRIX>          s_Worlds;

// static pointer to ECS entity manager (for internal purposes)
ECS::EntityMgr* s_pEnttMgr = nullptr;


//----------------------------------------------------------------------------------
// default constructor
//----------------------------------------------------------------------------------
RenderDataPreparator::RenderDataPreparator() {}

//----------------------------------------------------------------------------------
// Desc:   sort input data by material id
// Args:   - data: in/out container of data to sort
//----------------------------------------------------------------------------------
void SortData(cvector<EntityModelMesh>& data)
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


    EntityID   enttId   = INVALID_ENTITY_ID;
    MaterialID matId    = INVALID_MATERIAL_ID;
    ModelID    modelId  = INVALID_MODEL_ID;
    SubmeshID  subsetId = 0;
    float      sqrDistToCamera = 0;
};

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
    const vsize               numElems = data.size();
    cvector<EntityID>         tempEnttsIds(numElems, INVALID_ENTITY_ID);
    cvector<EntityDataAndPos> tempData(numElems);
    cvector<XMFLOAT3>         positions(numElems);

    // gather entities ids
    for (index i = 0; i < numElems; ++i)
        tempEnttsIds[i] = data[i].enttId;

    // gather entities positions
    mgr.transformSystem_.GetPositions(tempEnttsIds.data(), numElems, positions);

    // convert to transient data
    for (int i = 0; const EntityModelMesh& elem : data)
    {
        tempData[i] = { elem.enttId, elem.matId, elem.modelId, elem.subsetId };
        tempData[i].sqrDistToCamera =
            SQR(positions[i].x - camPos.x) +
            SQR(positions[i].y - camPos.y) +
            SQR(positions[i].z - camPos.z);

        i++;
    }

    // sort by square of distance to camera
    std::qsort(
        tempData.data(),
        tempData.size(),
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
    for (int i = 0; EntityDataAndPos& elem : tempData)
    {
        data[i++] = { elem.enttId, elem.matId, elem.modelId, elem.subsetId };
    }
}

//----------------------------------------------------------------------------------
// for debug
//----------------------------------------------------------------------------------
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
    cvector<Render::InstanceBatch>& instaceBatches)
{
    MaterialID matId                  = INVALID_MATERIAL_ID;
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
            instaceBatches.push_back(Render::InstanceBatch());

            pInstances               = &instaceBatches.back();
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
    ModelID modelId = INVALID_MODEL_ID;
    BasicModel* pModel = nullptr;

     // prepare geometry data for each instance batch
    for (Render::InstanceBatch& batch : instanceBatches)
    {
        // if we switched to another model
        if (batch.modelId != modelId)
        {
            pModel = &g_ModelMgr.GetModelById(batch.modelId);
            modelId = batch.modelId;
        }
        
        const MeshGeometry&         meshes   = pModel->meshes_;
        const SubmeshID             subsetId = batch.subsetId;
        const MeshGeometry::Subset& subset   = meshes.subsets_[subsetId];

        // instances batch name
        strncpy(batch.name, pModel->name_, MAX_LENGTH_MODEL_NAME);

        // vertex/index buffers data
        batch.vertexStride = meshes.vb_.GetStride();
        batch.pVB = meshes.vb_.Get();
        batch.pIB = meshes.ib_.Get();

        // subset metadata
        strncpy(batch.subset.name, subset.name, SUBSET_NAME_LENGTH_LIMIT);
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
// Args:   - enttsIds:         entities by these ids will be rendered onto the screen
//         - numEntts:         how many entts we want to render
//         - cameraPos:        camera's current position
//         - pEnttMgr:         a ptr to ECS entity manager
//         - instanceBufData:  data for the instances buffer
//         - instances:        models data for rendering
//----------------------------------------------------------------------------------
void RenderDataPreparator::PrepareEnttsDataForRendering(
    const EntityID* enttsIds,
    const size numEntts,
    const XMFLOAT3& cameraPos,
    ECS::EntityMgr* pEnttMgr,
    Render::RenderDataStorage& storage)
{
    CAssert::True(enttsIds != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,        "input number of entities must be > 0");

    s_pEnttMgr = pEnttMgr;

    //------------------------------------------------

    // clear the render data storage before filling it with data
    storage.Clear();

    // get materials ids for each subset of each input entity
    pEnttMgr->materialSystem_.GetDataByEnttsIds(enttsIds, numEntts, s_MaterialsDataPerEntt);


    // compute the number of instances
    vsize numAllInstances = 0;

    for (const ECS::MaterialData& matData : s_MaterialsDataPerEntt)
        numAllInstances += matData.materialsIds.size();

    //------------------------------------------------

    cvector<EntityModelMesh>        data(numAllInstances);
    cvector<EntityModelMesh>        maskedGroup;
    cvector<EntityModelMesh>        opaqueGroup;
    cvector<EntityModelMesh>        blendGroup;
    cvector<EntityModelMesh>        blendTransparentGroup;

    maskedGroup.reserve(numAllInstances);
    opaqueGroup.reserve(numAllInstances);
    blendGroup.reserve(numAllInstances);
    blendTransparentGroup.reserve(numAllInstances);

    Render::InstancesBuf& instancesBuf = storage.instancesBuf;
    instancesBuf.Resize((int)numAllInstances);

    //------------------------------------------------

    // prepare common data (only ids)
    for (int i = 0, instanceIdx = 0; i < numEntts; ++i)
    {
         const EntityID enttId = enttsIds[i];
         const ModelID modelId = pEnttMgr->modelSystem_.GetModelIdRelatedToEntt(enttId);
         const ECS::MaterialData& matData = s_MaterialsDataPerEntt[i];

        for (int matIdx = 0; const MaterialID& matId : matData.materialsIds)
        {
            data[instanceIdx].enttId   = enttId;
            data[instanceIdx].matId    = matId;
            data[instanceIdx].subsetId = matIdx;
            data[instanceIdx].modelId  = modelId;

            matIdx++;
            instanceIdx++;
        }
    }

    //------------------------------------------------

#if PRINT_DBG_DATA
    PrintData(data, "BEFORE SORTING");
#endif

    SortData(data);

#if PRINT_DBG_DATA
    PrintData(data, "AFTER SORTING");
#endif
 
    //------------------------------------------------

    MaterialID matId = INVALID_MATERIAL_ID;
    const Material* pMat = &g_MaterialMgr.GetMatById(matId);

    // group entities by materials
    for (vsize i = 0; i < numAllInstances; ++i)
    {
        // if current material differs from the previous one we get another material
        if (data[i].matId != matId)
        {
            pMat  = &g_MaterialMgr.GetMatById(data[i].matId);
            matId = data[i].matId;
        }

        // define to which rendering group does this instance belongs to:

        // 1. blending group (add / sub / mul)
        if (pMat->renderStates & MAT_BLEND_NOT_TRANSPARENT)
            blendGroup.push_back(data[i]);

        // 2. blending group (transparency)
        else if (pMat->renderStates & MAT_PROP_BS_TRANSPARENCY)
            blendTransparentGroup.push_back(data[i]);

        // 3. masked / alpha clipping group
        else if (pMat->renderStates & MAT_PROP_ALPHA_CLIPPING)
            maskedGroup.push_back(data[i]);

        // 4. opaque group
        else
            opaqueGroup.push_back(data[i]);
    }

#if PRINT_DBG_DATA
    PrintData(maskedGroup,           "MASKED");
    PrintData(opaqueGroup,           "OPAQUE");
    PrintData(blendGroup,            "BLEND");
    PrintData(blendTransparentGroup, "BLEND (TRANSPARENT)");
#endif

    // sort both blending groups elements by distance from the camera
    SortByDistance(*pEnttMgr, cameraPos, blendGroup);
    SortByDistance(*pEnttMgr, cameraPos, blendTransparentGroup);

    //------------------------------------------------

    int instanceMatIdx = 0;
    PrepareMaterials(instanceMatIdx, maskedGroup,           storage.instancesBuf, storage.masked);
    PrepareMaterials(instanceMatIdx, opaqueGroup,           storage.instancesBuf, storage.opaque);
    PrepareMaterials(instanceMatIdx, blendGroup,            storage.instancesBuf, storage.blended);
    PrepareMaterials(instanceMatIdx, blendTransparentGroup, storage.instancesBuf, storage.blendedTransparent);

    //------------------------------------------------


    // prepare vertex/index buffers data for each instance batch
    PrepareBuffers(storage.masked);
    PrepareBuffers(storage.opaque);
    PrepareBuffers(storage.blended);
    PrepareBuffers(storage.blendedTransparent);

    //------------------------------------------------

    // gather entts ids from each render group
    cvector<EntityID> enttsIdPerInstance(numAllInstances, INVALID_ENTITY_ID);
    int instanceIdx = 0;


    for (const EntityModelMesh& data : maskedGroup)
        enttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh & data : opaqueGroup)
        enttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh& data : blendGroup)
        enttsIdPerInstance[instanceIdx++] = data.enttId;

    for (const EntityModelMesh& data : blendTransparentGroup)
        enttsIdPerInstance[instanceIdx++] = data.enttId;


    // prepare world matrix for each instance
    PrepareInstancesWorldMatrices(
        enttsIdPerInstance,
        storage);
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
    s_pEnttMgr->transformSystem_.GetWorlds(
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
