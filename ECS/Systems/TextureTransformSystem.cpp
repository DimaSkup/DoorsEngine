// ********************************************************************************
// Filename:      TextureTransformSystem.cpp
// Description:   implementation of the TextureTransformSystem functional
// 
// Created:       29.06.24
// ********************************************************************************
#include "../Common/pch.h"
#include "TextureTransformSystem.h"


namespace ECS
{

TextureTransformSystem::TextureTransformSystem(TextureTransform* pTexTransformComp)
{
    Assert::NotNullptr(pTexTransformComp, "input ptr to the texture transform component == nullptr");
    pTexTransformComponent_ = pTexTransformComp;
}


// *********************************************************************************
//                               PUBLIC METHODS
// *********************************************************************************
void TextureTransformSystem::AddTexTransformation(
    const EntityID* ids,
    const size numEntts,
    const TexTransformType type,
    const TexTransformInitParams& params)
{
    Assert::True(ids != nullptr, "input ptr to entities arr == nullptr");
    Assert::True(numEntts > 0, "input number of entts must be > 0");

    if (type == STATIC)
    {
        AddStaticTexTransform(ids, numEntts, params);
    }
    else if (type == ATLAS_ANIMATION)
    {
        AddAtlasTextureAnimation(ids, numEntts, params);
    }
    else if (type == ROTATION_AROUND_TEX_COORD)
    {
        AddRotationAroundTexCoord(ids, numEntts, params);
    }
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::GetTexTransformsForEntts(
    const EntityID* ids,
    const size numEntts,
    cvector<XMMATRIX>& outTexTransforms)
{
    // what we do here:
    // go through each input entity and define if it has some tex transformation 
    // if so we store this transformation or in another case we store an identity matrix;
    //
    // in:    arr of entities IDs
    // out:   arr of texture transformations for these entities

    Assert::True((ids != nullptr) && (numEntts > 0), "invalid input args");

    const TextureTransform& comp = *pTexTransformComponent_;
    cvector<bool> exist(numEntts);
    cvector<index> idxs(numEntts);

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get bool flags to define if such entities exist in the component
    for (index i = 0; i < numEntts; ++i)
        exist[i] = (comp.ids[idxs[i]] == ids[i]);

    // fill in the output arr with texture transformation matrices
    outTexTransforms.resize(numEntts);

    for (index i = 0; i < numEntts; ++i)
        outTexTransforms[i] = (exist[i]) ? comp.texTransforms[idxs[i]] : DirectX::XMMatrixIdentity();
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::UpdateAllTextrureAnimations(
    const float totalGameTime,
    const float deltaTime)
{
    UpdateTextureStaticTransformation(deltaTime);
    UpdateTextureAtlasAnimations(deltaTime);
    UpdateTextureRotations(totalGameTime);
}


// =================================================================================
//                               PRIVATE METHODS
// =================================================================================

void TextureTransformSystem::AddStaticTexTransform(
    const EntityID* ids,
    const size numEntts,
    const TexTransformInitParams& inParams)
{
    // add a STATIC (it won't move) texture transformation to each input entity

    Assert::True(CheckCanAddRecords(ids, numEntts), "there is already a record with some input entity ID");

    TextureTransform& comp = *pTexTransformComponent_;
    TexStaticTransformations& staticTransf = comp.texStaticTrans;
    const StaticTexTransInitParams& params = static_cast<const StaticTexTransInitParams&>(inParams);

    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of new records into the data arrays
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.transformTypes.insert_before(idxs[i], TexTransformType::STATIC);

    for (index i = 0; i < numEntts; ++i)
        comp.texTransforms.insert_before(idxs[i], params.initTransform[i]);

    // --------------------------------------------------------

    // setup specific data for this kind of texture transformation
    staticTransf.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of values
    for (index i = 0; i < numEntts; ++i)
        staticTransf.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        staticTransf.transformations.insert_before(idxs[i], params.texTransforms[i]);
}

///////////////////////////////////////////////////////////

void PrepareAtlasAnimationInitData(
    const size numEntts,
    const AtlasAnimInitParams& params,
    cvector<TexAtlasAnimationData>& outData)
{
    outData.resize(numEntts);

    for (index i = 0; i < numEntts; ++i)
        outData[i].texRows = params.texRows[i];

    for (index i = 0; i < numEntts; ++i)
        outData[i].texColumns = params.texColumns[i];

    for (index i = 0; i < numEntts; ++i)
        outData[i].animDuration = params.animDurations[i];
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::AddAtlasTextureAnimation(
    const EntityID* ids,
    const size numEntts,
    const TexTransformInitParams& inParams)
{
    // make a texture animation for entity by ID;
    //
    // if texRows or texColumns > 0 (or both) we separate a texture into frames 
    // (it's supposed that we use an atlas texture getting it from the Textured component);
    // and go through these frames during the time so we make a texture animation;

    Assert::True(CheckCanAddRecords(ids, numEntts), "there is already a record with some input entity ID");


    TextureTransform& comp = *pTexTransformComponent_;
    const AtlasAnimInitParams& params = static_cast<const AtlasAnimInitParams&>(inParams);

    // prepare animation data for storing
    cvector<TexAtlasAnimationData> animData(numEntts);
    PrepareAtlasAnimationInitData(numEntts, params, animData);


    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;


    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.transformTypes.insert_before(idxs[i], TexTransformType::ATLAS_ANIMATION);

    // initially we set texture transformation matrix as scaling matrix
    for (index i = 0; i < numEntts; ++i)
        comp.texTransforms.insert_before(idxs[i], DirectX::XMMatrixScaling(animData[i].texCellWidth, animData[i].texCellHeight, 0));
        
    // store texture transformation (animation) data
    for (index i = 0; i < numEntts; ++i)
       AddAtlasAnimationData(ids[i], animData[i]);
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::AddRotationAroundTexCoord(
    const EntityID* ids,
    const size numEntts,
    const TexTransformInitParams& inParams)
{
    // add rotation around particular texture coordinate for input entity
    // (for instance: p(0.5, 0.5) - rotation arount its center)
    // 
    // input: inParams.center         -- texture coords
    //        rotationSpeed -- how fast the texture will rotate

    Assert::True(ids != nullptr, "input entts arr == nullptr");
    Assert::True(numEntts > 0, "input number of entts must be > 0");
    Assert::True(CheckCanAddRecords(ids, numEntts), "there is already a record with some input entity ID");


    TextureTransform& comp = *pTexTransformComponent_;
    TexRotationsAroundCoords& rotations = comp.texRotations;
    const RotationAroundCoordInitParams& rotationParams = static_cast<const RotationAroundCoordInitParams&>(inParams);

    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.transformTypes.insert_before(idxs[i], TexTransformType::ROTATION_AROUND_TEX_COORD);

    for (index i = 0; i < numEntts; ++i)
        comp.texTransforms.insert_before(idxs[i], DirectX::XMMatrixIdentity());   // initial texture transformation which is modified during the time

    // ------------------------------------------------------------
    // setup specific data for this kind of texture transformation

    rotations.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        rotations.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        rotations.texCoords.insert_before(idxs[i], rotationParams.center[i]);

    for (index i = 0; i < numEntts; ++i)
        rotations.rotationsSpeed.insert_before(idxs[i], rotationParams.speed[i]);
}

///////////////////////////////////////////////////////////

const index TextureTransformSystem::AddAtlasAnimationData(
    const EntityID id,
    const TexAtlasAnimationData& data)
{
    // add new texture atlas animation for entity by ID;
    // return:  idx into array of animations in the TextureTransform component

    Assert::True(id != INVALID_ENTITY_ID, "invalid input entity ID");
    Assert::True(data.texRows > 0, "the number of atlas texture rows must be > 0");
    Assert::True(data.texColumns > 0, "the number of atlas texture columns must be > 0");
    Assert::True(data.animDuration > 0, " the duration of atlas animation must be > 0");

    TexAtlasAnimations& anim = pTexTransformComponent_->texAtlasAnim;
    const index animIdx = anim.ids.get_insert_idx(id);

    anim.ids.insert_before(animIdx, id);

    // frame_duration = full_anim_duration / frames_count
    anim.timeSteps.insert_before(animIdx, data.animDuration / (data.texRows * data.texColumns));
    anim.currAnimTime.insert_before(animIdx, 0.0f);

    // compute and store animation frames data
    anim.data.insert_before(animIdx, data);

    return animIdx;
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::UpdateTextureStaticTransformation(const float deltaTime)
{
    // update all the static texture transformations

    using namespace DirectX;
    TextureTransform& comp = *pTexTransformComponent_;
    cvector<index> idxs;

    comp.ids.get_idxs(comp.texStaticTrans.ids, idxs);
   
    // currently we can only translate the texture over the surface
    for (int i = 0; const index idx : idxs)
    {
        const XMVECTOR& translationVec = comp.texStaticTrans.transformations[i++].r[3];

        comp.texTransforms[idx].r[3] += DirectX::XMVectorScale(translationVec, deltaTime);
    }
}


// =================================================================================
// Private functions: helpers for updating of atlas animations
// =================================================================================
void UpdateAtlasAnimationTime(
    const float deltaTime,
    TexAtlasAnimations& anim,
    cvector<index>& idxs)
{
    // update animation time of each animation and define if its frame must be changed
    for (index idx = 0; idx < anim.ids.size(); ++idx)
    {
        anim.currAnimTime[idx] += deltaTime;

        if (anim.currAnimTime[idx] > anim.timeSteps[idx])
        {
            anim.currAnimTime[idx] = 0;           // reset the current animation time
            idxs.push_back(idx);                  // and store its index
        }
    }
}

///////////////////////////////////////////////////////////

void ComputeAtlasAnimationTransformation(
    TexAtlasAnimations& anim,
    const cvector<index>& idxs,
    cvector<XMMATRIX>& outTransformations)
{
    // update each atlas animation by index (change frame position over the texture)

    outTransformations.resize(idxs.size());

    for (int i = 0; const index animIdx : idxs)
    {
        TexAtlasAnimationData& data = anim.data[animIdx];

        // change animation frame
        ++data.currTexFrameIdx;

        // do we need to restart animation?
        if (data.currTexFrameIdx >= (data.texFramesCount))
            data.currTexFrameIdx = 0;

        // compute frame row and column at the texture (atlas texture)
        const index col_idx = data.currTexFrameIdx % data.texColumns;
        const index row_idx = (index)(data.currTexFrameIdx * data.texCellWidth);

        // scale params
        const float m00 = data.texCellWidth;
        const float m11 = data.texCellHeight;

        // translation params
        const float m30 = m00 * col_idx;
        const float m31 = m11 * row_idx;

        // store transformation matrix to use it later for update
        outTransformations[i++] = DirectX::XMMATRIX(
            m00,  0.0f, 0.0f, 0.0f,                    // m00, m01, m02, m03
            0.0f, m11,  0.0f, 0.0f,                    // m10, m11, m12, m13
            0.0f, 0.0f, 0.0f, 0.0f,                    // m20, m21, m22, m23
            m30,  m31,  0.0f, 0.0f);                   // m30, m31, m32, m33
    }
}

///////////////////////////////////////////////////////////

void ApplyAtlasAnimationDataByIdxs(
    TextureTransform& comp,
    TexAtlasAnimations& anim,
    const cvector<index>& idxsOfAnimToUpdate,
    const cvector<XMMATRIX>& newTransformations)
{
    cvector<EntityID> enttsToUpdate;                // entts IDs whose texture animation was updated
    cvector<index>    transformsIdxs;               // idxs of transformations which will be updated

    // get IDs of entities which animations will be updated
    enttsToUpdate.resize(idxsOfAnimToUpdate.size());

    for (int i = 0; const index animIdx : idxsOfAnimToUpdate)
        enttsToUpdate[i++] = anim.ids[animIdx];

    // get data idxs of transformations to update and apply new values by these idxs
    comp.ids.get_idxs(enttsToUpdate, transformsIdxs);

    // apply texture transformations by idxs
    for (index i = 0; const index idx : transformsIdxs)
        comp.texTransforms[idx] = newTransformations[i++];
}


// =================================================================================
// Private methods: update texture transformation for each kind of transformation
// =================================================================================
void TextureTransformSystem::UpdateTextureAtlasAnimations(const float deltaTime)
{
    // here we update animation time for each texture atlas animation;
    // if some animation frame must be changed for particular animation we compute
    // a new texture transformation matrix for it

    cvector<index>    idxsOfAnimToUpdate;           // idxs of animations where a frame must be changed
    cvector<XMMATRIX> texTransformsToUpdate;        // computed tex transformations for new animation frames

    TextureTransform& comp = *pTexTransformComponent_;
    TexAtlasAnimations& anim = pTexTransformComponent_->texAtlasAnim;

    UpdateAtlasAnimationTime(deltaTime, anim, idxsOfAnimToUpdate);
    ComputeAtlasAnimationTransformation(anim, idxsOfAnimToUpdate, texTransformsToUpdate);
    ApplyAtlasAnimationDataByIdxs(comp, anim, idxsOfAnimToUpdate, texTransformsToUpdate);
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::UpdateTextureRotations(const float totalGameTime)
{
    // update all the texture rotations around responsible texture coords 
    // according to the time

    TextureTransform& comp = *pTexTransformComponent_;
    TexRotationsAroundCoords& rotations = comp.texRotations;
    cvector<index> idxs;                                      // apply new data by these idxs
    cvector<XMMATRIX> texTransToUpdate;                       // new texture transformations

    const size texRotCount = rotations.ids.size();
    texTransToUpdate.resize(texRotCount);

    // compute and store new rotation transformations
    for (index i = 0; i < texRotCount; ++i)
    {
        const XMFLOAT2& texCoord = rotations.texCoords[i];

        texTransToUpdate[i] =
            DirectX::XMMatrixTranslation(-texCoord.x, -texCoord.y, 0) *                   // translate to the rotation center
            DirectX::XMMatrixRotationZ(rotations.rotationsSpeed[i] * totalGameTime) *     // rotate by this angle
            DirectX::XMMatrixTranslation(texCoord.x, texCoord.y, 0);                      // translate back to the origin pos
    }

    // get data idxs of transformations to update and apply new values by these idxs
    comp.ids.get_idxs(rotations.ids, idxs);
    ApplyTexTransformsByIdxs(idxs, texTransToUpdate);
}

///////////////////////////////////////////////////////////

void TextureTransformSystem::ApplyTexTransformsByIdxs(
    const cvector<index>& idxs,
    const cvector<XMMATRIX>& newTexTransforms)
{
    // here apply new texture transformations by input data indices

    TextureTransform& comp = *pTexTransformComponent_;

    for (index i = 0; const index idx : idxs)
        comp.texTransforms[idx] = newTexTransforms[i++];
}

} // namespace ECS
