// =================================================================================
// Description:    parts of code which are used during initialization
// =================================================================================
#include <FileSystemPaths.h>
#include <Material.h>
#include <MaterialMgr.h>
#include <Render/CRender.h>

#pragma warning (disable : 4996)
using namespace Core;

namespace Game
{

//---------------------------------------------------------
// Desc:   manually setup model's materials
//---------------------------------------------------------
#if 0
void SetupDefault(BasicModel& model)
{
    const TexID texIdBlankNorm = g_TextureMgr.GetTexIdByName("blank_NRM");
    MeshGeometry::Subset* subsets = model.meshes_.subsets_;

    if (!subsets)
    {
        LogErr(LOG, "can't setup a model: subsets == nullptr");
        return;
    }

    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        const MaterialID matId = subsets[i].materialId;
        Material& mat = g_MaterialMgr.GetMatById(matId);

        mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
        mat.SetSpecular(0.2f, 0.2f, 0.2f);
        mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    }
}
#endif

//---------------------------------------------------------
// Desc:   manually setup a model's materials
//---------------------------------------------------------
void SetupMilitaryHouse(BasicModel& model, Render::CRender& render)
{
    char texDirPath[128]{'\0'};
    strcat(texDirPath, g_RelPathExtModelsDir);
    strcat(texDirPath, "building/abandoned-military-house/source/Textures/");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    TexID texIdsDiff[4]{0};
    TexID texIdsNormal[4]{0};
    TexID texIdsRough[4]{0};
    TexID texIdsAO[4]{0};

    texIdsDiff[0]   = g_TextureMgr.LoadFromFile(texDirPath, "Metal/BigBuilding_Metal_BaseColor.jpg");
    texIdsNormal[0] = g_TextureMgr.LoadFromFile(texDirPath, "Metal/BigBuilding_Metal_Normal.jpg");
    texIdsRough[0]  = g_TextureMgr.LoadFromFile(texDirPath, "Metal/BigBuilding_Metal_Roughness.jpg");
    texIdsRough[0]  = g_TextureMgr.LoadFromFile(texDirPath, "Metal/BigBuilding_Metal_AO.jpg");

    texIdsDiff[1]   = g_TextureMgr.LoadFromFile(texDirPath, "Concrete/BigBuilding_Concrete_BaseColor.jpg");
    texIdsNormal[1] = g_TextureMgr.LoadFromFile(texDirPath, "Concrete/BigBuilding_Concrete_Normal.jpg");
    texIdsRough[1]  = g_TextureMgr.LoadFromFile(texDirPath, "Concrete/BigBuilding_Concrete_Roughness.jpg");
    texIdsAO[1]     = g_TextureMgr.LoadFromFile(texDirPath, "Concrete/BigBuilding_Concrete_AO.jpg");

    texIdsDiff[2]   = g_TextureMgr.LoadFromFile(texDirPath, "Wood/BigBuilding_Wood_BaseColor.jpg");
    texIdsNormal[2] = g_TextureMgr.LoadFromFile(texDirPath, "Wood/BigBuilding_Wood_Normal.jpg");
    texIdsRough[2]  = g_TextureMgr.LoadFromFile(texDirPath, "Wood/BigBuilding_Wood_Roughness.jpg");
    texIdsAO[2]     = g_TextureMgr.LoadFromFile(texDirPath, "Wood/BigBuilding_Wood_AO.jpg");

    texIdsDiff[3]   = g_TextureMgr.LoadFromFile(texDirPath, "Roof/BigBuilding_Roof_BaseColor.jpg");
    texIdsNormal[3] = g_TextureMgr.LoadFromFile(texDirPath, "Roof/BigBuilding_Roof_Normal.jpg");
    texIdsRough[3]  = g_TextureMgr.LoadFromFile(texDirPath, "Roof/BigBuilding_Roof_Roughness.jpg");
    texIdsAO[3]     = g_TextureMgr.LoadFromFile(texDirPath, "Roof/BigBuilding_Roof_AO.jpg");


    MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    if (!subsets)
    {
        LogErr(LOG, "can't setup a model: subsets == nullptr");
        return;
    }

    // setup textures for each material
    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        Material& mat = g_MaterialMgr.GetMatById(subsets[i].materialId);

        mat.SetShaderId(lightShaderId);

        mat.SetTexture(TEX_TYPE_DIFFUSE, texIdsDiff[i]);
        mat.SetTexture(TEX_TYPE_NORMALS, texIdsNormal[i]);
        mat.SetTexture(TEX_TYPE_SHININESS, texIdsRough[i]);
        mat.SetTexture(TEX_TYPE_LIGHTMAP, texIdsAO[i]);

        mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
        mat.SetSpecular(0.2f, 0.2f, 0.2f);
        mat.SetSpecularPower(1.3f);
    }
}

//---------------------------------------------------------
// Desc:   manually setup a model's materials
//---------------------------------------------------------
void SetupPaz3201(BasicModel& model, Render::CRender& render)
{
    const TexID texIdDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/paz_3201/car_paz_3201.dds");
    const TexID texIdSpec = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/paz_3201/car_paz_3201_mask.dds");
    const TexID texIdNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/paz_3201/car_paz_3201_norm.dds");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    if (!subsets)
    {
        LogErr(LOG, "can't setup a model: subsets == nullptr");
        return;
    }

    const MaterialID matId = subsets[0].materialId;
    Material& mat = g_MaterialMgr.GetMatById(matId);

    mat.SetShaderId(lightShaderId);
    mat.SetName("paz_3201");
    mat.SetAmbient(0.4f, 0.4f, 0.4f, 1.0f);
    mat.SetDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetSpecular(0.1f, 0.1f, 0.1f);
    mat.SetSpecularPower(5.0f);
    mat.SetReflection(0,0,0,1);
    mat.SetTexture(TEX_TYPE_DIFFUSE, texIdDiff);
    mat.SetTexture(TEX_TYPE_SPECULAR, texIdSpec);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdNorm);
}


//---------------------------------------------------------
// Desc:   manually setup a model's materials
//---------------------------------------------------------
void SetupZil131(BasicModel& model, Render::CRender& render)
{
    const TexID texIdDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zil_131/zil_131.dds");
    const TexID texIdSpec = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zil_131/zil_131_spec.dds");
    const TexID texIdNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zil_131/zil_131_norm.dds");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    if (!subsets)
    {
        LogErr(LOG, "can't setup a model: subsets == nullptr");
        return;
    }

    const MaterialID matId = subsets[0].materialId;
    Material& mat = g_MaterialMgr.GetMatById(matId);

    mat.SetShaderId(lightShaderId);
    mat.SetName("zil_131");
    mat.SetAmbient(0.4f, 0.4f, 0.4f, 1.0f);
    mat.SetDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetSpecular(0.1f, 0.1f, 0.1f);
    mat.SetSpecularPower(5.0f);
    mat.SetReflection(0,0,0,1);
    mat.SetTexture(TEX_TYPE_DIFFUSE, texIdDiff);
    mat.SetTexture(TEX_TYPE_SPECULAR, texIdSpec);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdNorm);
}

//---------------------------------------------------------
// Desc:   manually setup a model's materials
//---------------------------------------------------------
void SetupBtr(BasicModel& model, Render::CRender& render)
{
    // setup materials
    const size numSubsets = model.GetNumSubsets();
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    const TexID texIdBlankNorm = g_TextureMgr.GetTexIdByName("blank_NRM");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    for (index i = 0; i < numSubsets; ++i)
    {
        const MaterialID matId = subsets[i].materialId;
        Material& mat = g_MaterialMgr.GetMatById(matId);

        mat.SetShaderId(lightShaderId);
        mat.ambient  = { 0.5f, 0.5f, 0.5f, 1.0f };
        mat.specular = { 0.1f, 0.1f, 0.1f, 25.0f };
        mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    }
}

//---------------------------------------------------------
// Desc:   manually setup a model's materials
//---------------------------------------------------------
void SetupZaz968(BasicModel& model, Render::CRender& render)
{
    const TexID texIdDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zaz_968m/texture/cars_02.dds");
    const TexID texIdSpec = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zaz_968m/texture/cars_02_mask.dds");
    const TexID texIdNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "vehicle/zaz_968m/texture/cars_02_norm.dds");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    if (!subsets)
    {
        LogErr(LOG, "can't setup a model: subsets == nullptr");
        return;
    }

    const MaterialID matId = subsets[0].materialId;
    Material& mat = g_MaterialMgr.GetMatById(matId);

    mat.SetShaderId(lightShaderId);
    mat.SetName("zaz_968_rust");
    mat.SetAmbient(0.4f, 0.4f, 0.4f, 1.0f);
    mat.SetSpecular(0.1f, 0.1f, 0.1f);
    mat.SetSpecularPower(5.0f);
    mat.SetReflection(0.05f, 0.05f, 0.05f, 1.0f);
    mat.SetTexture(TEX_TYPE_DIFFUSE, texIdDiff);
    mat.SetTexture(TEX_TYPE_SPECULAR, texIdSpec);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdNorm);
}

//---------------------------------------------------------
// Desc:   manually setup of tree spruce model
//---------------------------------------------------------
void SetupTreeSpruce(BasicModel& tree, Render::CRender& render)
{
    // load in textures
    const TexID normalMapID = g_TextureMgr.LoadFromFile(
        g_RelPathExtModelsDir,
        "trees/tree_spruce/tree_spruce_diffuse_NRM.dds");

    // setup the material for a single subset of the tree
    const MaterialID matID = tree.meshes_.subsets_[0].materialId;
    Material& mat          = g_MaterialMgr.GetMatById(matID);

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    mat.SetShaderId(lightShaderId);
    mat.SetName("tree_spruce_mat");
    mat.SetTexture(TEX_TYPE_NORMALS, normalMapID);
    mat.SetDiffuse(1, 1, 1, 1);
    mat.SetReflection(0, 0, 0, 0);
    mat.SetAlphaClip(true);
}

//---------------------------------------------------------
// Desc:   manually setup of tree pine model
//---------------------------------------------------------
void SetupTreePine(BasicModel& tree, Render::CRender& render)
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

    const ShaderID lightShaderId   = render.shaderMgr_.GetShaderIdByName("LightShader");

    // setup materials
    Material barkMat;
    barkMat.SetShaderId(lightShaderId);
    barkMat.SetName("tree_pine_bark");
    barkMat.SetTexture(TEX_TYPE_DIFFUSE, texBarkDiffID);
    barkMat.SetTexture(TEX_TYPE_NORMALS, texBarkNormID);
    barkMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    barkMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
    barkMat.SetSpecular(0.1f, 0.1f, 0.1f);
    barkMat.SetSpecularPower(1.0f);

    Material branchMat;
    barkMat.SetShaderId(lightShaderId);
    branchMat.SetName("tree_pine_branch");
    branchMat.SetTexture(TEX_TYPE_DIFFUSE, texBranchDiffID);
    branchMat.SetTexture(TEX_TYPE_NORMALS, texBranchNormID);
    branchMat.SetTexture(TEX_TYPE_OPACITY, texBranchSubsurfID);
    branchMat.SetAmbient(0.17f, 0.22f, 0.20f, 1.0f);
    branchMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
    branchMat.SetSpecular(0.1f, 0.1f, 0.1f);
    branchMat.SetSpecularPower(1.0f);
    branchMat.SetAlphaClip(true);
    branchMat.SetCull(MAT_PROP_CULL_NONE);

    Material capMat;
    barkMat.SetShaderId(lightShaderId);
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
void SetupBuilding9(BasicModel& building, Render::CRender& render)
{
    // load textures
    const TexID texIdHousePart = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/house_part.png");
    const TexID texIdArka      = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/arka.png");
    const TexID texIdBalcony   = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/balcony.png");
    const TexID texIdr5        = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "building9/r5.png");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    // create and setup materials
    Material housePartMat = g_MaterialMgr.AddMaterial("house_part");
    housePartMat.SetShaderId(lightShaderId);
    housePartMat.SetTexture(TEX_TYPE_DIFFUSE, texIdHousePart);

    Material arkaMat = g_MaterialMgr.AddMaterial("house_arka");
    arkaMat.SetShaderId(lightShaderId);
    arkaMat.SetTexture(TEX_TYPE_DIFFUSE, texIdArka);

    Material balconyMat = g_MaterialMgr.AddMaterial("house_balcony");
    balconyMat.SetShaderId(lightShaderId);
    balconyMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBalcony);

    Material r5Mat = g_MaterialMgr.AddMaterial("house_r5");
    r5Mat.SetShaderId(lightShaderId);
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
void SetupStalkerFreedom(BasicModel& model, Render::CRender& render)
{
    const TexID texIdBodyNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "stalker_freedom_1/texture/act_stalker_freedom_1_NRM.dds");
    const TexID texIdHeadNorm = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "blank_NRM.dds");

    model.meshes_.SetSubsetName(0, "head");
    model.meshes_.SetSubsetName(1, "body");
    MeshGeometry::Subset* subsets = model.meshes_.subsets_;

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    // get a material by ID from the manager and setup it
    Material& mat0 = g_MaterialMgr.GetMatById(subsets[0].materialId);

    mat0.SetShaderId(lightShaderId);
    mat0.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdHeadNorm);
    mat0.ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat0.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat0.specular = { 0,0,0,1 };
    mat0.reflect = { 0,0,0,0 };


    Material& mat1 = g_MaterialMgr.GetMatById(subsets[1].materialId);
    mat1.SetShaderId(lightShaderId);
    mat1.SetTexture(eTexType::TEX_TYPE_NORMALS, texIdBodyNorm);
    mat1.ambient = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat1.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat1.specular = { 0,0,0,1 };
    mat1.reflect = { 0,0,0,0 };
}

//---------------------------------------------------------
// Desc:   manually setup a model of sword
//---------------------------------------------------------
void SetupSword(BasicModel& model, Render::CRender& render)
{
    const MaterialID matId       = model.meshes_.subsets_[0].materialId;
    Material& mat                = g_MaterialMgr.GetMatById(matId);
    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    mat.SetShaderId(lightShaderId);
    mat.SetName("sword_mat");
    mat.SetSpecular(0.5f, 0.5f, 0.5f);
    mat.SetReflection(0, 0, 0, 0);
}

//---------------------------------------------------------
// Desc:   manually setup a model of obrez bm-16
//---------------------------------------------------------
void SetupObrez(BasicModel& model, Render::CRender& render)
{
    const MaterialID matId       = model.meshes_.subsets_[0].materialId;
    Material& mat                = g_MaterialMgr.GetMatById(matId);
    const TexID texIdBlankNorm   = g_TextureMgr.GetTexIdByName("blank_NRM");
    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    mat.SetShaderId(lightShaderId);
    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetSpecular(0.2f, 0.2f, 0.2f);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
}

//---------------------------------------------------------
void SetupAk74Stalker(BasicModel& model, Render::CRender& render)
{
    const MaterialID matId      = model.meshes_.subsets_[0].materialId;
    Material& mat               = g_MaterialMgr.GetMatById(matId);
    const TexID texIdBlankNorm  = g_TextureMgr.GetTexIdByName("blank_NRM");
    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    mat.SetShaderId(lightShaderId);
    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetSpecular(0.2f, 0.2f, 0.2f);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
}

//---------------------------------------------------------
void SetupGroza(BasicModel& model, Render::CRender& render)
{
    const TexID texIdBlankNorm    = g_TextureMgr.GetTexIdByName("blank_NRM");
    MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    const ShaderID lightShaderId  = render.shaderMgr_.GetShaderIdByName("LightShader");

    if (!subsets)
    {
        LogErr(LOG, "can't setup groza: subsets == nullptr");
        return;
    }

    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        const MaterialID matId = subsets[i].materialId;
        Material& mat          = g_MaterialMgr.GetMatById(matId);

        mat.SetShaderId(lightShaderId);
        mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
        mat.SetSpecular(0.2f, 0.2f, 0.2f);
        mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    }
}

//---------------------------------------------------------
void SetupHpsa(BasicModel& model, Render::CRender& render)
{
    const MaterialID matId       = model.meshes_.subsets_[0].materialId;
    Material& mat                = g_MaterialMgr.GetMatById(matId);
    const TexID texIdBlankNorm   = g_TextureMgr.GetTexIdByName("blank_NRM");
    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    mat.SetShaderId(lightShaderId);
    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetSpecular(0.2f, 0.2f, 0.2f);
    mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
}

//---------------------------------------------------------
// Desc:   manually setup a model of aks-74
//---------------------------------------------------------
void SetupAk74(BasicModel& model, Render::CRender& render)
{
    model.SetName("ak_74");

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    // setup material of each subset (mesh) of the model
    MeshGeometry::Subset* subsets = model.meshes_.subsets_;

    // wooden furniture
    Material& mat0 = g_MaterialMgr.GetMatById(subsets[0].materialId);
    mat0.SetShaderId(lightShaderId);
    mat0.ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    mat0.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat0.specular = { 0.9f, 0.9f, 0.9f, 35.0f };
    mat0.reflect = { 0.05f, 0.05f, 0.05f, 0 };

    // metalic magazine
    Material& mat1 = g_MaterialMgr.GetMatById(subsets[1].materialId);
    mat1.SetShaderId(lightShaderId);
    mat1.ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    mat1.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat1.specular = { 0.8f, 0.8f, 0.8f,256 };
    mat1.reflect = { 0.1f, 0.1f, 0.1f, 1 };
    mat1.SetCull(MAT_PROP_CULL_BACK);

    // metalic body
    Material& mat2 = g_MaterialMgr.GetMatById(subsets[2].materialId);
    mat2.SetShaderId(lightShaderId);
    mat2.ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    mat2.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat2.specular = { 0.8f, 0.8f, 0.8f,256 };
    mat2.reflect = { 0.1f, 0.1f, 0.1f, 1 };
    mat2.SetCull(MAT_PROP_CULL_BACK);

    // wooden furniture
    Material& mat3 = g_MaterialMgr.GetMatById(subsets[3].materialId);
    mat3.SetShaderId(lightShaderId);
    mat3.ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
    mat3.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    mat3.specular = { 0.7f, 0.7f, 0.7f, 35.0f };
    mat3.reflect = { 0.05f, 0.05f, 0.05f, 0 };
}

//---------------------------------------------------------
// Desc:   manually setup a model of aks-74u
//---------------------------------------------------------
void SetupAks74u(BasicModel& model, Render::CRender& render)
{
    model.SetName("aks_74u");

    const MaterialID matID       = model.meshes_.subsets_[0].materialId;
    Material& mat                = g_MaterialMgr.GetMatById(matID);
    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    const TexID texDiff = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "weapon/ak_74u/texture/wpn_aksu.png");
    const TexID texNorm = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "weapon/ak_74u/texture/wpn_aksu_NRM.dds");

    mat.SetShaderId(lightShaderId);
    mat.SetTexture(eTexType::TEX_TYPE_DIFFUSE, texDiff);
    mat.SetTexture(eTexType::TEX_TYPE_NORMALS, texNorm);

    mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
    mat.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
    mat.specular = { 0.1f, 0.1f, 0.1f, 7.0f };
    mat.reflect = { 0.05f, 0.05f, 0.05f, 1.0f };
}

//---------------------------------------------------------
// Desc:   manually setup a traktor model
//---------------------------------------------------------
void SetupTraktor(BasicModel& traktor, Render::CRender& render)
{
    const MeshGeometry::Subset& subset0  = traktor.meshes_.subsets_[0];
    const MeshGeometry::Subset& subset1  = traktor.meshes_.subsets_[1];
    const MeshGeometry::Subset& subset2  = traktor.meshes_.subsets_[2];
    const MeshGeometry::Subset& subset3  = traktor.meshes_.subsets_[3];
    const MeshGeometry::Subset& subset4  = traktor.meshes_.subsets_[4];

    const MaterialID matIdVehicle        = subset0.materialId;
    const MaterialID matIdSpotlightTrace = subset1.materialId;
    const MaterialID matIdSpotlightCone  = subset2.materialId;
    const MaterialID matIdBacksideLight  = subset3.materialId;
    const MaterialID matIdGlass          = subset4.materialId;

    Material& matVehicle                 = g_MaterialMgr.GetMatById(matIdVehicle);
    Material& matSpotlightTrace          = g_MaterialMgr.GetMatById(matIdSpotlightTrace);
    Material& matSpotlightCone           = g_MaterialMgr.GetMatById(matIdSpotlightCone);
    Material& matBacksideLight           = g_MaterialMgr.GetMatById(matIdBacksideLight);
    Material& matGlass                   = g_MaterialMgr.GetMatById(matIdGlass);

    const TexID texIdBlankNorm           = g_TextureMgr.GetTexIdByName("blank_NRM");
    const ShaderID textureShaderId       = render.shaderMgr_.GetShaderIdByName("TextureShader");
    const ShaderID lightShaderId         = render.shaderMgr_.GetShaderIdByName("LightShader");

    // setup material for subset0 (vehicle)
    matVehicle.shaderId = lightShaderId;
    matVehicle.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);

    matVehicle.ambient  = { 0.8f, 0.8f, 0.8f, 1.0f };
    matVehicle.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
    matVehicle.specular = { 0,0,0,1 };
    matVehicle.reflect  = { 0,0,0,0 };
    
    // setup material for subset1 (spotlight trace)
   // matSpotlightTrace.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    matSpotlightTrace.ambient   = { 0.170f, 0.170f, 0.170f, 1 };
    matSpotlightTrace.diffuse   = { 0,0,0,1 };
    matSpotlightTrace.specular  = { 0,0,0,1 };
    matSpotlightTrace.reflect   = { 0,0,0,0 };
    matSpotlightTrace.SetCull(MAT_PROP_CULL_NONE);
    matSpotlightTrace.SetBlending(MAT_PROP_BS_ADD);
    matSpotlightTrace.SetDepthStencil(MAT_PROP_DSS_MARK_MIRROR);
    matSpotlightTrace.shaderId = textureShaderId;

    // setup material for subset2 (spotlight cone)
   // matSpotlightCone.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    matSpotlightCone.ambient  = { 1,1,1,1 };
    matSpotlightCone.diffuse  = { 1,1,1,1 };
    matSpotlightCone.specular = { 0,0,0,1 };
    matSpotlightCone.reflect  = { 0,0,0,0 };
    matSpotlightCone.SetCull(MAT_PROP_CULL_NONE);
    matSpotlightCone.SetBlending(MAT_PROP_BS_ADD);
    matSpotlightCone.SetDepthStencil(MAT_PROP_DSS_MARK_MIRROR);
    matSpotlightCone.shaderId = textureShaderId;

    // setup material for subset3 (backside light)
    matBacksideLight.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    matBacksideLight.ambient  = { 0.8f, 0.8f, 0.8f, 1.0f };
    matBacksideLight.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
    matBacksideLight.specular = { 0,0,0,1 };
    matBacksideLight.reflect  = { 0,0,0,0 };
    matBacksideLight.shaderId = lightShaderId;

    // setup material for subset4 (glass)
    matGlass.shaderId   = lightShaderId;
    matGlass.ambient    = { 1,1,1,1 };
    matGlass.specular   = { 1,1,1,64 };
    matGlass.reflect    = { 0.5f, 0.5f, 0.5f, 0 };
    matGlass.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    matGlass.SetCull(MAT_PROP_CULL_NONE);
    matGlass.SetBlending(MAT_PROP_BS_TRANSPARENCY);
    matGlass.SetDepthStencil(MAT_PROP_DSS_NO_DOUBLE_BLEND);
}

//---------------------------------------------------------
// Desc:   manually setup a stalker's small house model
//---------------------------------------------------------
void SetupStalkerSmallHouse(BasicModel& house, Render::CRender& render)
{
    MeshGeometry::Subset* subsets = house.meshes_.subsets_;

    char dirPath[128]{ '\0' };
    sprintf(dirPath, "%s%s", g_RelPathExtModelsDir, "building/stalker-house/source/");
    const TexID texIdBlankNorm = g_TextureMgr.GetTexIdByName("blank_NRM");

    char texPaths[][32] = {
        "wood_board_01.dds",
        "wall_red_01.dds",
        "crete_dirt_2.dds",
        "wood_plank6.dds",
        "briks_br2.dds",
        "wood_old.dds",
        "wood_plank6.dds",
        "mtl_rust_dark.dds",
        "wood_old.dds",
        "wood_old.dds",
        "wood_old.dds",
        "crete_dirt_2.dds",
        "wall_red_01.dds",
        "briks_dirt_01.dds",
        "wall_wallpaper_02.dds",
        "tile_walls_red_01.dds",
        "wall_stucco_05.dds",
        "briks_br2.dds",
        "wood_old.dds",
        "wood_board_01.dds",
        "wood_wire.dds",
        "wood_wire.dds",
    };

    const ShaderID lightShaderId = render.shaderMgr_.GetShaderIdByName("LightShader");

    SetConsoleColor(MAGENTA);

    for (int i = 0; i < house.numSubsets_; ++i)
    {
        // get a material by ID from the manager and setup it
        Material& mat     = g_MaterialMgr.GetMatById(subsets[i].materialId);
        const TexID texId = g_TextureMgr.LoadFromFile(dirPath, texPaths[i]);


        mat.SetShaderId(lightShaderId);
        mat.SetTexture(TEX_TYPE_DIFFUSE, texId);
        mat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);

        mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
        mat.specular = { 0,0,0,2 };
        mat.reflect  = { 0,0,0,0 };
    }

    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:   manually setup materials for the model
//---------------------------------------------------------
void SetupBuildingMilitaryBlockpost(BasicModel& model)
{

}


} // namespace Game
