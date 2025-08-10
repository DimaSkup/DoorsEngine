// =================================================================================
// Description:    parts of code which are used during initialization
// =================================================================================
#include <FileSystemPaths.h>
#include <Material.h>
#include <MaterialMgr.h>

#pragma warning (disable : 4996)
using namespace Core;

namespace Game
{

//---------------------------------------------------------
// Desc:   manually setup of tree spruce model
//---------------------------------------------------------
void SetupTreeSpruce(BasicModel& tree)
{
    // load in textures
    const TexID normalMapID = g_TextureMgr.LoadFromFile(
        g_RelPathExtModelsDir,
        "trees/tree_spruce/tree_spruce_diffuse_NRM.dds");

    // setup the material for a single subset of the tree
    const MaterialID matID = tree.meshes_.subsets_[0].materialID;
    Material& mat          = g_MaterialMgr.GetMatById(matID);

    mat.SetName("tree_spruce_mat");
    mat.SetTexture(TEX_TYPE_NORMALS, normalMapID);
    mat.SetDiffuse(1, 1, 1, 1);
    mat.SetReflection(0, 0, 0, 0);
}

//---------------------------------------------------------
// Desc:   manually setup of tree pine model
//---------------------------------------------------------
void SetupTree(BasicModel& tree)
{

    // prepare path to textures directory
    char pathToTreeModelDir[64]{'\0'};
    sprintf(pathToTreeModelDir, "%s%s", g_RelPathExtModelsDir, "trees/FBX format/");

    
    // load texture from file for tree bark/branches/caps
    const TexID texBarkDiffID      = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "Bark_Color.png");
    const TexID texBarkNormID      = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "Bark_Normal.png");

    const TexID texBranchDiffID    = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "conifer_macedonian_pine_Color.png");
    const TexID texBranchNormID    = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "conifer_macedonian_pine_Normal.png");
    const TexID texBranchSubsurfID = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "conifer_macedonian_pine_Subsurface.png");

    const TexID texCapDiffID       = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "Cap_Color.png");
    const TexID texCapNormID       = g_TextureMgr.LoadFromFile(pathToTreeModelDir, "Cap_Normal.png");


    // setup materials
    Material barkMat;
    barkMat.SetName("tree_pine_bark");
    barkMat.SetTexture(TEX_TYPE_DIFFUSE, texBarkDiffID);
    barkMat.SetTexture(TEX_TYPE_NORMALS, texBarkNormID);
    barkMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    barkMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
    barkMat.SetSpecular(0.1f, 0.1f, 0.1f);
    barkMat.SetSpecularPower(1.0f);

    Material branchMat;
    branchMat.SetName("tree_pine_branch");
    branchMat.SetTexture(TEX_TYPE_DIFFUSE, texBranchDiffID);
    branchMat.SetTexture(TEX_TYPE_NORMALS, texBranchNormID);
    branchMat.SetTexture(TEX_TYPE_OPACITY, texBranchSubsurfID);
    branchMat.SetAmbient(0.17f, 0.30f, 0.28f, 1.0f);
    branchMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
    branchMat.SetSpecular(0.1f, 0.1f, 0.1f);
    branchMat.SetSpecularPower(1.0f);
    branchMat.SetAlphaClip(true);

    Material capMat;
    capMat.SetName("tree_pine_cap");
    capMat.SetTexture(TEX_TYPE_DIFFUSE, texCapDiffID);
    capMat.SetTexture(TEX_TYPE_NORMALS, texCapNormID);
    capMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    capMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);


    // add materials into the material manager
    const MaterialID barkMatID   = g_MaterialMgr.AddMaterial(std::move(barkMat));
    const MaterialID branchMatID = g_MaterialMgr.AddMaterial(std::move(branchMat));
    const MaterialID capMatID    = g_MaterialMgr.AddMaterial(std::move(capMat));

    // setup materials for tree subsets (meshes)
    tree.meshes_.SetMaterialForSubset(0, barkMatID);
    tree.meshes_.SetMaterialForSubset(1, capMatID);
    tree.meshes_.SetMaterialForSubset(2, branchMatID);
}

//---------------------------------------------------------
// Desc:   manually setup a building model
//---------------------------------------------------------
void SetupBuilding9(BasicModel& building)
{
    // load textures
    const TexID texIdHousePart = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/house_part.png");
    const TexID texIdArka      = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/arka.png");
    const TexID texIdBalcony   = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/balcony.png");
    const TexID texIdr5        = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/r5.png");

    // create and setup materials
    Material housePartMat = g_MaterialMgr.AddMaterial("house_part");
    housePartMat.SetTexture(TEX_TYPE_DIFFUSE, texIdHousePart);

    Material arkaMat = g_MaterialMgr.AddMaterial("house_arka");
    arkaMat.SetTexture(TEX_TYPE_DIFFUSE, texIdArka);

    Material balconyMat = g_MaterialMgr.AddMaterial("house_balcony");
    balconyMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBalcony);

    Material r5Mat = g_MaterialMgr.AddMaterial("house_r5");
    r5Mat.SetTexture(TEX_TYPE_DIFFUSE, texIdr5);
   
    // setup model with materials
    building.meshes_.SetMaterialForSubset(0, housePartMat.id);
    building.meshes_.SetMaterialForSubset(1, arkaMat.id);
    building.meshes_.SetMaterialForSubset(2, balconyMat.id);
    building.meshes_.SetMaterialForSubset(3, r5Mat.id);
}

//---------------------------------------------------------
// Desc:   setup some params of the stalker_freedom model
//---------------------------------------------------------
void SetupStalkerFreedom(BasicModel& stalkerFreedom)
{
    const TexID texIdBodyNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "stalker_freedom_1/texture/act_stalker_freedom_1_NRM.dds");
    const TexID texIdHeadNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "stalker_freedom_1/texture/act_stalker_head_mask_NRM.dds");
    
    MeshGeometry::Subset* subsets = stalkerFreedom.meshes_.subsets_;

    // get a material by ID from the manager and setup it
    Material& mat0 = g_MaterialMgr.GetMatById(subsets[0].materialID);

    mat0.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdHeadNorm);
    mat0.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat0.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat0.specular = { 0,0,0,1 };


    Material& mat1 = g_MaterialMgr.GetMatById(subsets[1].materialID);

    mat1.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdBodyNorm);
    mat1.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat1.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat1.specular = { 0,0,0,1 };
}

//---------------------------------------------------------
// Desc:   manually setup a model of ak-47
//---------------------------------------------------------
void SetupAk47(BasicModel& ak47)
{
    ak47.SetName("ak_47");

    MeshGeometry::Subset* subsets = ak47.meshes_.subsets_;

    for (int i = 0; i < ak47.numSubsets_; ++i)
    {
        // get a material by ID from the manager and setup it
        Material& mat = g_MaterialMgr.GetMatById(subsets[i].materialID);

        mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
        mat.specular = { 0,0,0,1 };
    }
}

//---------------------------------------------------------
// Desc:   manually setup a model of ak-74
//---------------------------------------------------------
void SetupAk74(BasicModel& ak74)
{

    const MaterialID matID = ak74.meshes_.subsets_[0].materialID;
    Material& mat          = g_MaterialMgr.GetMatById(matID);

    const TexID texDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "ak_74u/texture/wpn_aksu.png");
    const TexID texNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "ak_74u/texture/wpn_aksu_NRM.dds");

    mat.SetTexture(eTexType::TEX_TYPE_DIFFUSE, texDiff);
    mat.SetTexture(eTexType::TEX_TYPE_NORMALS, texNorm);

    mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat.specular = { 0.1f, 0.1f, 0.1f, 32.0f };
}

//---------------------------------------------------------
// Desc:   manually setup a traktor model
//---------------------------------------------------------
void SetupTraktor(BasicModel& traktor)
{
    const int numSubsets = (int)traktor.meshes_.numSubsets_;
    for (int i = 0; i < numSubsets; ++i)
    {
        const MaterialID matID = traktor.meshes_.subsets_[i].materialID;
        Material& mat = g_MaterialMgr.GetMatById(matID);

        mat.ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
        mat.specular = { 0.0f, 0.0f, 0.0f, 32.0f };
    }
}

///////////////////////////////////////////////////////////

void SetupStalkerSmallHouse(BasicModel& house)
{
    // manually setup materials for the model

    MeshGeometry::Subset* subsets = house.meshes_.subsets_;

    for (int i = 0; i < house.numSubsets_; ++i)
    {
        // get a material by ID from the manager and setup it
        Material& mat = g_MaterialMgr.GetMatById(subsets[i].materialID);

        mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
        mat.specular = { 0,0,0,2 };
    }
}

///////////////////////////////////////////////////////////

void SetupStalkerAbandonedHouse(BasicModel& house)
{
    // manually setup materials for the model

    char dirPath[64]{ '\0' };
    sprintf(dirPath, "%s%s", g_RelPathExtModelsDir, "/stalker/abandoned-house-20/");

    // load textures for the model
    const TexID texBrickDiff      = g_TextureMgr.LoadFromFile(dirPath, "textures/Brick_BaseColor.jpg");
    const TexID texConcreteDiff   = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_BaseColor.jpg");
    const TexID texConcrete2Diff  = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_2_BaseColor.jpg");
    const TexID texRoofDiff       = g_TextureMgr.LoadFromFile(dirPath, "textures/Roof_BaseColor.jpg");
    const TexID texDoorDiff       = g_TextureMgr.LoadFromFile(dirPath, "textures/Door_BaseColor.jpg");
    const TexID texMetalDiff      = g_TextureMgr.LoadFromFile(dirPath, "textures/Metal_BaseColor.jpg");
    const TexID texWindowDiff     = g_TextureMgr.LoadFromFile(dirPath, "textures/Windows_BaseColor.jpg");
    const TexID texGroundDiff     = g_TextureMgr.LoadFromFile(dirPath, "textures/Ground_BaseColor.jpg");

    const TexID texBrickNorm      = g_TextureMgr.LoadFromFile(dirPath, "textures/Brick_Normal.jpg");
    const TexID texConcreteNorm   = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_Normal.jpg");
    const TexID texConcrete2Norm  = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_2_Normal.jpg");
    const TexID texRoofNorm       = g_TextureMgr.LoadFromFile(dirPath, "textures/Roof_Normal.jpg");
    const TexID texDoorNorm       = g_TextureMgr.LoadFromFile(dirPath, "textures/Door_Normal.jpg");
    const TexID texMetalNorm      = g_TextureMgr.LoadFromFile(dirPath, "textures/Metal_Normal.jpg");
    const TexID texWindowNorm     = g_TextureMgr.LoadFromFile(dirPath, "textures/Windows_Normal.jpg");
    const TexID texGroundNorm     = g_TextureMgr.LoadFromFile(dirPath, "textures/Ground_Normal.jpg");

    const TexID texBrickRough     = g_TextureMgr.LoadFromFile(dirPath, "textures/Brick_Roughness.jpg");
    const TexID texConcreteRough  = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_Roughness.jpg");
    const TexID texConcrete2Rough = g_TextureMgr.LoadFromFile(dirPath, "textures/Concrete_2_Roughness.jpg");
    const TexID texRoofRough      = g_TextureMgr.LoadFromFile(dirPath, "textures/Roof_Roughness.jpg");
    const TexID texDoorRough      = g_TextureMgr.LoadFromFile(dirPath, "textures/Door_Roughness.jpg");
    const TexID texMetalRough     = g_TextureMgr.LoadFromFile(dirPath, "textures/Metal_Roughness.jpg");
    const TexID texWindowRough    = g_TextureMgr.LoadFromFile(dirPath, "textures/Windows_Roughness.jpg");
    const TexID texGroundRough    = g_TextureMgr.LoadFromFile(dirPath, "textures/Ground_Roughness.jpg");


    // setup materials of model's meshes
    MeshGeometry::Subset* subsets = house.meshes_.subsets_;

    Material& concreteMat = g_MaterialMgr.GetMatById(subsets[0].materialID);
    concreteMat.SetName("abandoned_house_concrete");
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE, texConcreteDiff);
    concreteMat.SetTexture(TEX_TYPE_NORMALS, texConcreteNorm);
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texConcreteRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& brickMat = g_MaterialMgr.GetMatById(subsets[1].materialID);
    brickMat.SetName("abandoned_house_brick");
    brickMat.SetTexture(TEX_TYPE_DIFFUSE, texBrickDiff);
    brickMat.SetTexture(TEX_TYPE_NORMALS, texBrickNorm);
    brickMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texBrickRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& roofMat = g_MaterialMgr.GetMatById(subsets[2].materialID);
    roofMat.SetName("abandoned_house_roof");
    roofMat.SetTexture(TEX_TYPE_DIFFUSE, texRoofDiff);
    roofMat.SetTexture(TEX_TYPE_NORMALS, texRoofNorm);
    roofMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texRoofRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& metalMat = g_MaterialMgr.GetMatById(subsets[3].materialID);
    metalMat.SetName("abandoned_house_metal");
    metalMat.SetTexture(TEX_TYPE_DIFFUSE, texMetalDiff);
    metalMat.SetTexture(TEX_TYPE_NORMALS, texMetalNorm);
    metalMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texMetalRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& groundMat = g_MaterialMgr.GetMatById(subsets[4].materialID);
    groundMat.SetName("abandoned_house_ground");
    groundMat.SetTexture(TEX_TYPE_DIFFUSE, texGroundDiff);
    groundMat.SetTexture(TEX_TYPE_NORMALS, texGroundNorm);
    groundMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texGroundNorm);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& doorMat = g_MaterialMgr.GetMatById(subsets[5].materialID);
    doorMat.SetName("abandoned_house_door");
    doorMat.SetTexture(TEX_TYPE_DIFFUSE, texDoorDiff);
    doorMat.SetTexture(TEX_TYPE_NORMALS, texDoorNorm);
    doorMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texDoorRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& windowMat = g_MaterialMgr.GetMatById(subsets[6].materialID);
    windowMat.SetName("abandoned_house_window");
    windowMat.SetTexture(TEX_TYPE_DIFFUSE, texWindowDiff);
    windowMat.SetTexture(TEX_TYPE_NORMALS, texWindowNorm);
    windowMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texWindowRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
}

} // namespace Game
