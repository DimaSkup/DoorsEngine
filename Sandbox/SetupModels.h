// =================================================================================
// Description:    parts of code which are used during initialization
// =================================================================================
#include "Common/FileSystemPaths.h"
#include <Material.h>
#include <MaterialMgr.h>

#pragma warning (disable : 4996)
using namespace Core;

namespace Game
{

void SetupTerrain(BasicModel& terrain)
{
    // load and set a texture for the terrain model

#if 0
    // use stone textures
    const TexID texTerrainDiff = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "stone02d.jpg");
    const TexID texTerrainNorm = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "stone02n.png");
#else
    // use ground textures
    const TexID texTerrainDiff = g_TextureMgr.LoadFromFile(g_RelPathDataDir, "terrain/detail_grnd_grass.dds");
    const TexID texTerrainNorm = g_TextureMgr.LoadFromFile(g_RelPathDataDir, "terrain/detail_grnd_grass_bump.dds");
#endif

  
    // create and setup material
    Material terrainMat;
    terrainMat.SetTexture(TEX_TYPE_DIFFUSE, texTerrainDiff);
    terrainMat.SetTexture(TEX_TYPE_NORMALS, texTerrainNorm);
    strcpy(terrainMat.name, "terrain_mat_1");
    const MaterialID terrainMatID = g_MaterialMgr.AddMaterial(std::move(terrainMat));

    terrain.meshes_.SetMaterialForSubset(0, terrainMatID);
}

///////////////////////////////////////////////////////////

void SetupTree(BasicModel& tree)
{
    // manually setup of tree pine model

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
    branchMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
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

///////////////////////////////////////////////////////////

void SetupPowerLine(BasicModel& powerLine)
{
    // manually setup of power line model

    // prepare path to textures directory
    char texDirPath[64]{'\0'};
    sprintf(texDirPath, "%s%s", g_RelPathExtModelsDir, "power_line/");

    // load textures from files
    const TexID texDiffID = g_TextureMgr.LoadFromFile(texDirPath, "bigpoleiron_co.png");
    const TexID texNormID = g_TextureMgr.LoadFromFile(texDirPath, "bigpoleiron_nohq.png");

    // setup and create a material
    Material mat;
    mat.SetName("power_line");
    mat.SetTexture(TEX_TYPE_DIFFUSE, texDiffID);
    mat.SetTexture(TEX_TYPE_NORMALS, texNormID);
    mat.ambient  = { 0.4f, 0.4f, 0.4f, 1.0f };
    mat.specular = { 0.3f, 0.3f, 0.3f, 1.0f };

    const MaterialID powerLineMatID = g_MaterialMgr.AddMaterial(std::move(mat));
    powerLine.SetMaterialForSubset(0, powerLineMatID);
}

///////////////////////////////////////////////////////////

void SetupBuilding9(BasicModel& building)
{
    // manually setup building model

    // load textures
    const TexID texIdHousePart = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/house_part.png");
    const TexID texIdArka      = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/arka.png");
    const TexID texIdBalcony   = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/balcony.png");
    const TexID texIdr5        = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/r5.png");

    // setup materials
    Material housePartMat;
    housePartMat.SetName("house_part");
    housePartMat.SetTexture(TEX_TYPE_DIFFUSE, texIdHousePart);

    Material arkaMat;
    arkaMat.SetName("house_arka");
    arkaMat.SetTexture(TEX_TYPE_DIFFUSE, texIdArka);

    Material balconyMat;
    balconyMat.SetName("house_balcony");
    balconyMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBalcony);

    Material r5Mat;
    r5Mat.SetName("house_r5");
    r5Mat.SetTexture(TEX_TYPE_DIFFUSE, texIdr5);
   

    // store materials into the manager
    const MaterialID housePartMatID = g_MaterialMgr.AddMaterial(std::move(housePartMat));
    const MaterialID arkaMatID      = g_MaterialMgr.AddMaterial(std::move(arkaMat));
    const MaterialID balconyMatID   = g_MaterialMgr.AddMaterial(std::move(balconyMat));
    const MaterialID r5MatID        = g_MaterialMgr.AddMaterial(std::move(r5Mat));
    
    // setup model with materials
    building.meshes_.SetMaterialForSubset(0, housePartMatID);
    building.meshes_.SetMaterialForSubset(1, arkaMatID);
    building.meshes_.SetMaterialForSubset(2, balconyMatID);
    building.meshes_.SetMaterialForSubset(3, r5MatID);
}

///////////////////////////////////////////////////////////

void SetupStalkerFreedom(BasicModel& stalkerFreedom)
{
    // setup some params of the stalker_freedom model

    const TexID texIdBodyNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "stalker_freedom_1/texture/act_stalker_freedom_1_NRM.dds");
    const TexID texIdHeadNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "stalker_freedom_1/texture/act_stalker_head_mask_NRM.dds");
    
    MeshGeometry::Subset* subsets = stalkerFreedom.meshes_.subsets_;

    // get a material by ID from the manager and setup it
    Material& mat0 = g_MaterialMgr.GetMaterialByID(subsets[0].materialID);

    mat0.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdHeadNorm);
    mat0.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat0.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat0.specular = { 0,0,0,1 };


    Material& mat1 = g_MaterialMgr.GetMaterialByID(subsets[1].materialID);

    mat1.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdBodyNorm);
    mat1.ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat1.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat1.specular = { 0,0,0,1 };
}

///////////////////////////////////////////////////////////

void SetupAk47(BasicModel& ak47)
{
    // setup some params of the ak-47 model
    ak47.SetName("ak_47");

    MeshGeometry::Subset* subsets = ak47.meshes_.subsets_;

    for (int i = 0; i < ak47.numSubsets_; ++i)
    {
        // get a material by ID from the manager and setup it
        Material& mat = g_MaterialMgr.GetMaterialByID(subsets[i].materialID);

        mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
        mat.specular = { 0,0,0,1 };
    }
}

///////////////////////////////////////////////////////////

void SetupAk74(BasicModel& ak74)
{
    // manually setup a model of ak-74

    const MaterialID matID = ak74.meshes_.subsets_[0].materialID;
    Material& mat          = g_MaterialMgr.GetMaterialByID(matID);

    const TexID texDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "ak_74u/texture/wpn_aksu.png");
    const TexID texNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "ak_74u/texture/wpn_aksu_NRM.dds");

    mat.SetTexture(eTexType::TEX_TYPE_DIFFUSE, texDiff);
    mat.SetTexture(eTexType::TEX_TYPE_NORMALS, texNorm);

    mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat.specular = { 0.1f, 0.1f, 0.1f, 32.0f };
}

///////////////////////////////////////////////////////////

void SetupTraktor(BasicModel& traktor)
{
    // manually setup a traktor model

    const int numSubsets = (int)traktor.meshes_.numSubsets_;
    for (int i = 0; i < numSubsets; ++i)
    {
        const MaterialID matID = traktor.meshes_.subsets_[i].materialID;
        Material& mat = g_MaterialMgr.GetMaterialByID(matID);

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
        Material& mat = g_MaterialMgr.GetMaterialByID(subsets[i].materialID);

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

    Material& concreteMat = g_MaterialMgr.GetMaterialByID(subsets[0].materialID);
    concreteMat.SetName("abandoned_house_concrete");
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE, texConcreteDiff);
    concreteMat.SetTexture(TEX_TYPE_NORMALS, texConcreteNorm);
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texConcreteRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& brickMat = g_MaterialMgr.GetMaterialByID(subsets[1].materialID);
    brickMat.SetName("abandoned_house_brick");
    brickMat.SetTexture(TEX_TYPE_DIFFUSE, texBrickDiff);
    brickMat.SetTexture(TEX_TYPE_NORMALS, texBrickNorm);
    brickMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texBrickRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& roofMat = g_MaterialMgr.GetMaterialByID(subsets[2].materialID);
    roofMat.SetName("abandoned_house_roof");
    roofMat.SetTexture(TEX_TYPE_DIFFUSE, texRoofDiff);
    roofMat.SetTexture(TEX_TYPE_NORMALS, texRoofNorm);
    roofMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texRoofRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& metalMat = g_MaterialMgr.GetMaterialByID(subsets[3].materialID);
    metalMat.SetName("abandoned_house_metal");
    metalMat.SetTexture(TEX_TYPE_DIFFUSE, texMetalDiff);
    metalMat.SetTexture(TEX_TYPE_NORMALS, texMetalNorm);
    metalMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texMetalRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& groundMat = g_MaterialMgr.GetMaterialByID(subsets[4].materialID);
    groundMat.SetName("abandoned_house_ground");
    groundMat.SetTexture(TEX_TYPE_DIFFUSE, texGroundDiff);
    groundMat.SetTexture(TEX_TYPE_NORMALS, texGroundNorm);
    groundMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texGroundNorm);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& doorMat = g_MaterialMgr.GetMaterialByID(subsets[5].materialID);
    doorMat.SetName("abandoned_house_door");
    doorMat.SetTexture(TEX_TYPE_DIFFUSE, texDoorDiff);
    doorMat.SetTexture(TEX_TYPE_NORMALS, texDoorNorm);
    doorMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texDoorRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& windowMat = g_MaterialMgr.GetMaterialByID(subsets[6].materialID);
    windowMat.SetName("abandoned_house_window");
    windowMat.SetTexture(TEX_TYPE_DIFFUSE, texWindowDiff);
    windowMat.SetTexture(TEX_TYPE_NORMALS, texWindowNorm);
    windowMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texWindowRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

#if 0
    // add materials into the material manager
    const MaterialID concreteMatID = g_MaterialMgr.AddMaterial(std::move(concreteMat));
    const MaterialID brickMatID    = g_MaterialMgr.AddMaterial(std::move(brickMat));
    const MaterialID roofMatID     = g_MaterialMgr.AddMaterial(std::move(roofMat));
    const MaterialID metalMatID    = g_MaterialMgr.AddMaterial(std::move(metalMat));
    const MaterialID groundMatID   = g_MaterialMgr.AddMaterial(std::move(groundMat));
    const MaterialID doorMatID     = g_MaterialMgr.AddMaterial(std::move(doorMat));
    const MaterialID windowMatID   = g_MaterialMgr.AddMaterial(std::move(windowMat));


    // setup materials for the model
    house.SetMaterialForSubset(0, concreteMatID);
    house.SetMaterialForSubset(1, brickMatID);
    house.SetMaterialForSubset(2, roofMatID);
    house.SetMaterialForSubset(3, metalMatID);
    house.SetMaterialForSubset(4, groundMatID);
    house.SetMaterialForSubset(5, doorMatID);
    house.SetMaterialForSubset(6, windowMatID);
    house.SetMaterialForSubset(7, metalMatID);   // yes, the same material as for 3rd subset
#endif
}

///////////////////////////////////////////////////////////

void SetupCube(BasicModel& cube)
{
    // manually setup the cube model

    const TexID texIdDiff = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "box01d.dds");
    const TexID texIdNorm = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "box01n.dds");

    // create and setup a material and add it into the material manager
    Material mat;
    mat.SetName("cube_box01");
    mat.SetTexture(TEX_TYPE_DIFFUSE, texIdDiff);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdNorm);
    const MaterialID matID = g_MaterialMgr.AddMaterial(std::move(mat));

    // cube has only one subset (mesh) by ID == 0
    cube.SetMaterialForSubset(0, matID);
}

///////////////////////////////////////////////////////////

void SetupSphere(BasicModel& sphere)
{
    // manually setup the sphere model

    const TexID texID = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "gigachad.dds");

    // setup material and add it into the material manager
    Material mat;
    mat.SetName("gigachad");
    mat.SetTexture(TEX_TYPE_DIFFUSE, texID);
    const MaterialID matID = g_MaterialMgr.AddMaterial(std::move(mat));

    // setup material for a single mesh of the sphere model
    sphere.SetMaterialForSubset(0, matID);
}

} // namespace Game
