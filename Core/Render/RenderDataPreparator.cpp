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


namespace Core
{

cvector<DirectX::XMMATRIX>          s_Worlds;
cvector<DirectX::XMMATRIX>          s_EnttsTexTransforms;
cvector<EntityID>                   s_EnttsWithOrigMat;
cvector<EntityID>                   s_EnttsWithUniqueMat;
cvector<bool>                       s_MaterialsFlags;
cvector<ModelID>                    s_ModelsIDs;
cvector<EntityID>                   s_EnttsSortedByModels;
cvector<size>                       s_NumEnttsPerModel;
cvector<const BasicModel*>          s_Models;
cvector<ECS::MaterialData>          s_MaterialsDataPerEntt;
cvector<ID3D11ShaderResourceView*>  textureSRVs_(1024, nullptr);

// static pointer to ECS entity manager (for internal purposes)
ECS::EntityMgr* s_pEnttMgr = nullptr;


RenderDataPreparator::RenderDataPreparator()
{}

//----------------------------------------------------------------------------------
// Desc:   prepare instances data and instances buffer for rendering
//         entities by input IDs
//
// Args:   - enttsIds:         entities by these ids will be rendered onto the screen
//         - numEntts:         how many entts we want to render
//         - pEnttMgr:         a ptr to ECS entity manager
//         - instanceBufData:  data for the instances buffer
//         - instances:        models data for rendering
//----------------------------------------------------------------------------------
void RenderDataPreparator::PrepareEnttsDataForRendering(
    const EntityID* enttsIds,
    const size numEntts,
    ECS::EntityMgr* pEnttMgr,
    Render::RenderDataStorage& storage)
{
    CAssert::True(enttsIds != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,        "input number of entities must be > 0");

    s_pEnttMgr = pEnttMgr;


    //--------------------------------------------------------------------

    vsize numEntities = 2;
    vsize numInstances = 0;

    // get materials ids for each subset of each input entity
    pEnttMgr->materialSystem_.GetDataByEnttsIds(enttsIds, numEntities, s_MaterialsDataPerEntt);


    // compute the number of instances
    for (const ECS::MaterialData& matData : s_MaterialsDataPerEntt)
    {
        numInstances += matData.materialsIds.size();
    }

    cvector<EntityModelMesh> data(numInstances);

    for (int i = 0; i < numEntities; ++i)
    {
        data[i].enttId   = enttsIds[i];
        data[i].matId    = s_MaterialsDataPerEntt[i].materialsIds[0];        // get material for subset 0
        data[i].subsetId = 0;
        data[i].modelId  = pEnttMgr->modelSystem_.GetModelIdRelatedToEntt(data[i].enttId);
    }


    //--------------------------------------------------------------------

    // prepare instance batch

    cvector<Render::InstanceBatch>& instancesBatch = storage.opaque;
    instancesBatch.reserve(128);

    // prepare geometry data for each instance batch
    for (int i = 0; i < numEntities; ++i)
    {
        const BasicModel&           model  = g_ModelMgr.GetModelById(data[i].modelId);
        const MeshGeometry&         meshes = model.meshes_;
        const MeshGeometry::Subset& subset = meshes.subsets_[0];


        instancesBatch.push_back(Render::InstanceBatch());

        Render::InstanceBatch& instances = instancesBatch.back();

        strcpy(instances.name, model.name_);
        instances.vertexStride = meshes.vb_.GetStride();
        instances.pVB = meshes.vb_.Get();
        instances.pIB = meshes.ib_.Get();

        instances.subset.vertexStart = subset.vertexStart;
        instances.subset.vertexCount = subset.vertexCount;
        instances.subset.indexStart = subset.indexStart;
        instances.subset.indexCount = subset.indexCount;
    }

    //--------------------------------------------------------------------


    // prepare material for instance batch
    for (vsize i = 0; i < numEntities; ++i)
    {
        Material& mat = g_MaterialMgr.GetMatById(data[i].matId);
        instancesBatch[i].renderStates = mat.renderStates;

        g_TextureMgr.GetTexViewsByIds(mat.texIds, NUM_TEXTURE_TYPES, instancesBatch[i].textures);

    }

  
    // prepare world matrix for each instance
    PrepareInstancesWorldMatrices(
        &data[0].enttId,
        1,
        storage.instancesBuf,
        storage.opaque);

    // prepare material for each instance
}

//---------------------------------------------------------
// Desc:   prepare world matrix for each subset (mesh) of each entity
//---------------------------------------------------------
void RenderDataPreparator::PrepareInstancesWorldMatrices(
    const EntityID* enttsIds,
    const size numEntts,
    Render::InstBuffData& instancesBuf,
    const cvector<Render::InstanceBatch>& instances)
{

    s_Worlds.clear();

    s_pEnttMgr->transformSystem_.GetWorlds(enttsIds, numEntts, s_Worlds);

    for (int idx1 = 0, idx2 = 0; const Render::InstanceBatch& instances : instances)
    {
        memcpy(
            &(instancesBuf.worlds_[idx1]),
            &(s_Worlds[idx2]),
            sizeof(DirectX::XMMATRIX) * instances.numInstances);

        idx1 += instances.numInstances;
        idx2 += instances.numInstances;
    }
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstancesMaterials(
    Render::InstBuffData& instanceBuffData,
    const cvector<EntityModelMesh>& instances,
    const cvector<Render::Material>& materialsSortedByInstances)
{
}


} // namespace Core
