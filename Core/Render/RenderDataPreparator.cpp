// ********************************************************************************
// Filename:     RenderDataPreparator.cpp
// 
// Created:      17.10.24
// ********************************************************************************
#include "RenderDataPreparator.h"


namespace Core
{

RenderDataPreparator::RenderDataPreparator() {}

///////////////////////////////////////////////////////////

void PrepareInstancesWorldMatrices(
    ECS::EntityMgr* pEnttMgr,
    const EntityID* enttsSortedByModels,
    const size numEntts,
    Render::InstBuffData& instanceBuffData,         // data for the instances buffer
    const Render::cvector<Render::Instance>& instances)       // instances (models) data for rendering
{
    // prepare world matrix for each subset (mesh) of each entity

    ECS::cvector<DirectX::XMMATRIX> worlds;
    pEnttMgr->transformSystem_.GetWorldMatricesOfEntts(enttsSortedByModels, numEntts, worlds);

    for (int worldIdx = 0, i = 0; const Render::Instance & instance : instances)
    {
        // currently: set the same world matrix for each subset (mesh) of the entity
        for (int subsetIdx = 0; subsetIdx < instance.subsets.size(); ++subsetIdx)
        {
            memcpy(&(instanceBuffData.worlds_[i]), &(worlds[worldIdx]), sizeof(DirectX::XMMATRIX) * instance.numInstances);
            i += instance.numInstances;
        }
        worldIdx += instance.numInstances;
    }
}

///////////////////////////////////////////////////////////

void PrepareInstancesTextureTransformations(
    ECS::EntityMgr* pEnttMgr,
    const EntityID* enttsSortedByModels,
    const size numEntts,
    Render::InstBuffData& instanceBuffData,         // data for the instances buffer
    const Render::cvector<Render::Instance>& instances)       // instances (models) data for rendering
{
    // prepare texture transformation matrix for each subset (mesh) of each entity

    cvector<DirectX::XMMATRIX> enttsTexTransforms(numEntts);

    pEnttMgr->texTransformSystem_.GetTexTransformsForEntts(
        enttsSortedByModels,
        enttsTexTransforms.data(),
        numEntts);

    for (int transformIdx = 0, i = 0; const Render::Instance & instance : instances)
    {
        for (int subsetIdx = 0; subsetIdx < instance.subsets.size(); ++subsetIdx)
        {
            for (int j = 0; j < instance.numInstances; ++j)
            {
                instanceBuffData.texTransforms_[i++] = enttsTexTransforms[transformIdx + j];
            }
        }

        transformIdx += instance.numInstances;
    }
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsDataForRendering(
    const EntityID* enttsIds,
    const size numEntts,
    ECS::EntityMgr* pEnttMgr,
    Render::InstBuffData& instanceBuffData,         // data for the instances buffer
    Render::cvector<Render::Instance>& instances)       // instances (models) data for rendering
{
    // prepare instances data and instances buffer for rendering entities by input IDs;

    Assert::True(enttsIds != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,        "input number of entities must be > 0");

    // get data of models which are related to the input entts
    cvector<EntityID> enttsSortedByInstances;
    PrepareInstancesData(enttsIds, numEntts, pEnttMgr, instances, enttsSortedByInstances);

    // compute num of instances for the instanced buffer
    int numElems = 0;

    for (const Render::Instance& instance : instances)
        numElems += (instance.numInstances * (int)std::ssize(instance.subsets));

    // prepare memory for instances transient data buffer
    instanceBuffData.Resize(numElems);

    // prepare world matrix for each mesh of each instance
    PrepareInstancesWorldMatrices(
        pEnttMgr,
        enttsSortedByInstances.data(),
        numEntts,
        instanceBuffData,
        instances);

    // prepate texture transformation for each mesh of each instance
    PrepareInstancesTextureTransformations(
        pEnttMgr,
        enttsSortedByInstances.data(),
        numEntts,
        instanceBuffData,
        instances);
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsBoundingLineBox(
    ECS::EntityMgr* pEnttMgr,
    Render::Instance& instance,
    Render::InstBuffData& instanceBuffer)
{
    // prepare data to render bounding box of each visible entity
    // (BB of the whole model)

    assert(0 && "FIXME");

#if 0
    ECS::EntityMgr&                 mgr = *pEnttMgr;
    ECS::cvector<ModelID>           modelsIDs;
    ECS::cvector<EntityID>          enttsSortByModels(numEntts);
    ECS::cvector<size>              numInstancesPerModel;
    ECS::cvector<DirectX::XMMATRIX> boxLocals;         // local space matrix of the AABB of the whole model
    ECS::cvector<DirectX::XMMATRIX> worlds;            // world matrix of each visible entity


    mgr.modelSystem_.GetModelsIdsRelatedToEntts(
        visibleEntts,
        numEntts,
        modelsIDs,
        enttsSortByModels,
        numInstancesPerModel);
    
    // get AABB of each visible model
    ECS::cvector<DirectX::BoundingBox> modelsAABBs(std::ssize(modelsIDs));

    for (int i = 0; const ModelID id : modelsIDs)
        modelsAABBs[i++] = g_ModelMgr.GetModelByID(id).GetModelAABB();

    // get world matrix for each entts and local space matrix by each model's AABB
    mgr.transformSystem_.GetWorldMatricesOfEntts(enttsSortByModels.data(), numEntts, worlds);
    mgr.boundingSystem_.GetBoxesLocalSpaceMatrices(modelsAABBs.data(), modelsAABBs.size(), boxLocals);

    // prepare instances buffer data
    instance.numInstances = (int)numEntts;
    instanceBuffer.Resize(instance.numInstances);

    // prepare world matrix for each bounding box to render
    for (index i = 0, instIdx = 0; i < std::ssize(boxLocals); ++i)
    {
        for (index j = 0; j < numInstancesPerModel[i]; ++j, ++instIdx)
            instanceBuffer.worlds_[instIdx] = boxLocals[i] * worlds[instIdx];
    }
#endif
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsMeshesBoundingLineBox(
    ECS::EntityMgr* pEnttMgr,
    Render::Instance& instance,
    Render::InstBuffData& instanceBuffer)
{
    // prepare data to render bounding box of each mesh of each visible entity

    assert(0 && "FIXME");

#if 0
    Assert::True(visibleEntts != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,            "input number of entts must be > 0");
    
    ECS::EntityMgr&    mgr = *pEnttMgr_;
    ECS::cvector<size> numBoxesPerEntt;                     // how many boxes each entt has
    ECS::cvector<DirectX::XMMATRIX> locals;                 // local space of each mesh of each entity
    ECS::cvector<DirectX::XMMATRIX> worlds;                 // world matrix of each visible entity

    mgr.boundingSystem_.GetBoxLocalSpaceMatricesByIDs(visibleEntts, numEntts, numBoxesPerEntt, locals);
    mgr.transformSystem_.GetWorldMatricesOfEntts(visibleEntts, numEntts, worlds);

    // prepare instances buffer data
    instance.numInstances = (int)std::ssize(locals);
    instanceBuffer.Resize(instance.numInstances);

    for (index instanceIdx = 0, i = 0; i < worlds.size(); ++i)
    {
        for (index idx = 0; idx < numBoxesPerEntt[i]; ++idx)
        {
            instanceBuffer.worlds_[instanceIdx] = locals[instanceIdx] * worlds[i];
            ++instanceIdx;
        }
    }
#endif
}


// =================================================================================
// GROUP ENTITIES; PREPARE INSTANCES
// =================================================================================
void RenderDataPreparator::PrepareInstancesData(
    const EntityID* ids,
    const size numEntts,
    ECS::EntityMgr* pEnttMgr,
    Render::cvector<Render::Instance>& instances,
    cvector<EntityID>& outEnttsSortedByInstances)
{
    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");


    // separate entities by entts with mesh based materials and unique materials
    cvector<EntityID> enttsWithOrigMat;
    cvector<EntityID> enttsWithUniqueMat;

    SeparateEnttsByMaterialGroups(
        *pEnttMgr,
        ids,
        numEntts,
        enttsWithOrigMat,
        enttsWithUniqueMat);

    // prepare instances for entts which materials are based on model
    PrepareInstancesForEntts(
        *pEnttMgr,
        enttsWithOrigMat.data(),
        enttsWithOrigMat.size(),
        instances,
        outEnttsSortedByInstances);

    // prepare instances for entts which materials are unique per each entt
    PrepareInstancesForEnttsWithUniqueMaterials(
        *pEnttMgr,
        enttsWithUniqueMat.data(),
        enttsWithUniqueMat.size(),
        instances,
        outEnttsSortedByInstances);

    // prepare textures shader resource view (SRV) for each instance
    for (Render::Instance& instance : instances)
        PrepareTexturesForInstance(instance);
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::SeparateEnttsByMaterialGroups(
    const ECS::EntityMgr& mgr,
    const EntityID* ids,
    const size numEntts,
    cvector<EntityID>& outEnttsWithOrigMat,
    cvector<EntityID>& outEnttsWithUniqueMat)
{
    // separate entities by entts with mesh based materials and unique materials

    ECS::cvector<bool> materialsFlags;
    mgr.materialSystem_.GetMaterialsFlagsByEntts(ids, numEntts, materialsFlags);

    // we expect that maybe all the entts has materials which are the same as its model and just a bit of entitites may have unique materials
    outEnttsWithOrigMat.reserve(numEntts);
    outEnttsWithUniqueMat.reserve(8);

#if 0
    LogMsg("list of entts BEFORE grouping: ", eConsoleColor::YELLOW);
    for (index i = 0; i < numEntts; ++i)
        LogMsgf("entity [%s] [%d]", mgr.nameSystem_.GetNameById(ids[i]).c_str(), ids[i]);
#endif

    // extract entts with materials based on model
    for (index i = 0; i < numEntts; ++i)
    {
        if (materialsFlags[i])
            outEnttsWithOrigMat.push_back(ids[i]);
    }

    // extract entts with unique materials
    for (index i = 0; i < numEntts; ++i)
    {
        if (!materialsFlags[i])
            outEnttsWithUniqueMat.push_back(ids[i]);
    }

#if 0
    LogMsg("list of entts AFTER grouping: ", eConsoleColor::YELLOW);

    LogErr("orig mat: ");
    for (const EntityID id : outEnttsWithOrigMat)
        LogMsgf("entity [%s] [%d]", mgr.nameSystem_.GetNameById(id).c_str(), id);

    LogErr("unique mat: ");
    for (const EntityID id : outEnttsWithUniqueMat)
        LogMsgf("entity [%s] [%d]", mgr.nameSystem_.GetNameById(id).c_str(), id);
#endif
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstancesForEntts(
    ECS::EntityMgr& mgr,
    const EntityID* ids,
    const size numEntts,
    Render::cvector<Render::Instance>& instances,
    cvector<EntityID>& outEnttsSortedByInstances)
{
    // prepare instances data for input entities by IDs

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");

    // maybe we have no entities to render -- it's OK, just go out
    if (numEntts == 0)
        return;

    ECS::cvector<ModelID>  modelsIDs;
    ECS::cvector<EntityID> enttsSortedByModels;
    ECS::cvector<size>     numEnttsPerModel;

    mgr.modelSystem_.GetModelsIdsRelatedToEntts(
        ids,
        numEntts,
        modelsIDs,
        enttsSortedByModels,
        numEnttsPerModel);

    const index startInstanceIdx = instances.size();
    const size  numInstances = modelsIDs.size();

    // store entities IDs which are sorted by models (by instances)
    outEnttsSortedByInstances.append_vector(std::move(enttsSortedByModels));

    // update how many instances will we have at all
    instances.resize(instances.size() + numInstances);

    // get pointers to models by its IDs
    cvector<const BasicModel*> models;
    g_ModelMgr.GetModelsByIDs(modelsIDs.data(), modelsIDs.size(), models);

    // fill in particular instance with data of model
    for (index i = startInstanceIdx, j = 0; j < numInstances; ++i, ++j)
        PrepareInstanceData(*models[j], instances[i]);

    // how many instances of this model will be rendered
    for (index i = startInstanceIdx, j = 0; j < numInstances; ++i, ++j)
        instances[i].numInstances = (int)numEnttsPerModel[j];
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstancesForEnttsWithUniqueMaterials(
    ECS::EntityMgr& mgr,
    const EntityID* ids,
    const size numEntts,
    Render::cvector<Render::Instance>& instances,
    cvector<EntityID>& outEnttsSortedByInstances)
{
    // prepare instances data for input entities by IDs

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");

    // maybe we have no entities to render -- it's OK, just go out
    if (numEntts == 0)
        return;

    ECS::cvector<ModelID>           modelsIDs;
    ECS::cvector<EntityID>          enttsSortedByModels;
    ECS::cvector<size>              numEnttsPerInstance;
    ECS::cvector<ECS::MaterialData> materialsDataPerEntt;

    mgr.modelSystem_.GetModelsIdsRelatedToEntts(
        ids,
        numEntts,
        modelsIDs,
        enttsSortedByModels,
        numEnttsPerInstance);

    const index startInstanceIdx = instances.size();
    const size  numNewInstances = enttsSortedByModels.size();  // one instance per entity

    mgr.materialSystem_.GetDataByEnttsIDs(
        enttsSortedByModels.data(),
        enttsSortedByModels.size(),
        materialsDataPerEntt);

    // store entities IDs which are sorted by models (by instances)
    outEnttsSortedByInstances.append_vector(std::move(enttsSortedByModels));

    // update how many instances will we have at all
    instances.resize(instances.size() + numNewInstances);

    // get pointers to models by its IDs
    cvector<const BasicModel*> models;
    g_ModelMgr.GetModelsByIDs(modelsIDs.data(), modelsIDs.size(), models);

    // fill in particular instance with data of model
    for (index i = startInstanceIdx, modelIdx = 0; const size num : numEnttsPerInstance)
    {
        for (index j = 0; j < num; ++j, ++i)
        {
            const BasicModel& model = *(models[modelIdx]);
            PrepareInstanceData(model, instances[i]);
        }
        ++modelIdx;
    }

    // each instance will be rendered only once (since one instance per entity)
    for (index i = startInstanceIdx, j = 0; j < numNewInstances; ++i, ++j)
        instances[i].numInstances = 1;

    // fill instances with material IDs of related entity (so later we will load textures using these IDs)
    for (index i = startInstanceIdx; ECS::MaterialData& data : materialsDataPerEntt)
    {
        const MaterialID* matsIDs = data.materialsIDs.data();
        const size        numIDs = data.materialsIDs.size();

        std::move(matsIDs, matsIDs + numIDs, instances[i++].materialIDs.data());
    }
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstanceData(
    const BasicModel& model,
    Render::Instance& instance)
{
    // fill in the rendering instance with input model data;
    // 
    // NOTE: we don't write textures shader resource view (!!!) into the instance
    //       we do it in the PrepareTexturesForInstance() method;

    strcpy(instance.name, model.name_);

    // copy buffers data
    const MeshGeometry& meshes = model.meshes_;
    instance.vertexStride = meshes.vb_.GetStride();
    instance.pVB          = meshes.vb_.Get();
    instance.pIB          = meshes.ib_.Get();

    // prepare data of each subset (mesh)
    const size numSubsets = model.GetNumSubsets();
    instance.subsets.resize(numSubsets);
    
    for (index i = 0; i < numSubsets; ++i)
    {
        const MeshGeometry::Subset& srcSubset = meshes.subsets_[i];
        Render::Subset& dstSubset = instance.subsets[i];

        // copy 4 ints: vertex start/count, index start/count
        //memcpy(&subset.vertexStart, &subsetData.vertexStart_, sizeof(int) * 4);

        // copy name of the subset
        strcpy(dstSubset.name, srcSubset.name);

        // copy subset's vertex/index info
        dstSubset.vertexStart = srcSubset.vertexStart;
        dstSubset.vertexCount = srcSubset.vertexCount;
        dstSubset.indexStart  = srcSubset.indexStart;
        dstSubset.indexCount  = srcSubset.indexCount;
    }

    // prepare material IDs
    instance.materialIDs.resize(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        instance.materialIDs[i] = meshes.subsets_[i].materialID;
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareTexturesForInstance(Render::Instance& instance)
{
    // prepare textures for the instance which are based on original related model

    using SRV = ID3D11ShaderResourceView;

    const size numSubsets = (int)instance.subsets.size();

    // prepare valid textures (SRVs) for this instance
    MaterialID        materialsIDs[256]{ INVALID_MATERIAL_ID };
    TexID             texturesIDs[256]{ INVALID_TEXTURE_ID };
    cvector<Material> materials(numSubsets);
    cvector<SRV*>     textureSRVs(256, nullptr);

    
    // get materials data by materials IDs (one material per one subset)
    g_MaterialMgr.GetMaterialsByIDs(instance.materialIDs.data(), numSubsets, materials);

    // go through each material and store related textures IDs
    for (index i = 0, texIdx = 0; i < numSubsets; ++i)
    {
        for (index j = 0; j < NUM_TEXTURE_TYPES; ++j)
            texturesIDs[texIdx++] = materials[i].textureIDs[j];
    }

    g_TextureMgr.GetSRVsByTexIDs(texturesIDs, NUM_TEXTURE_TYPES * numSubsets, textureSRVs);

    // store texture SRVs into the instance
    instance.texSRVs.resize(numSubsets * NUM_TEXTURE_TYPES);

    for (index i = 0; i < NUM_TEXTURE_TYPES * numSubsets; ++i)
        instance.texSRVs[i] = textureSRVs[i];
}

///////////////////////////////////////////////////////////

#if 0
void RenderDataPreparator::PrepareInstanceFromModel(
    BasicModel& model,
    Render::Instance& instance)
{
    // prepare a rendering instance data by a single input model

    const MeshGeometry& meshes = model.meshes_;

    instance.name = model.name_;
    instance.subsets.resize(model.GetNumSubsets());
    instance.materials.resize(model.GetNumSubsets());

    instance.vertexStride = meshes.vb_.GetStride();
    instance.pVB = meshes.vb_.Get();
    instance.pIB = meshes.ib_.Get();

    instance.numInstances = 1;

    // copy subsets data into instance
    PrepareSubsetsData(meshes.subsets_, instance.subsets);

    // copy materials data from model into instance
    PrepareSubsetsMaterials(
        model.materials_,
        model.numSubsets_,            // each subset has one material so num materials == num subsets
        instance.materials.data());

    // prepare textures (SRVs) for this instance
    g_TextureMgr->GetSRVsByTexIDs(model.texIDs_, model.numTextures_, instance.texSRVs);
}
#endif

///////////////////////////////////////////////////////////

} // namespace Core
