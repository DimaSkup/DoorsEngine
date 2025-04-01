// =================================================================================
// Description:    parts of code which are used during initialization
// =================================================================================

namespace Core
{

void SetupTerrain(BasicModel& terrain)
{
    TextureMgr& texMgr       = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    // load and set a texture for the terrain model
    const TexID texTerrainDiff = texMgr.LoadFromFile(g_RelPathTexDir + "terrain/detail_grnd_grass.dds");
    const TexID texTerrainNorm = texMgr.LoadFromFile(g_RelPathTexDir + "terrain/detail_grnd_grass_bump.dds");

    Material terrainMat;
    terrainMat.SetTexture(TEX_TYPE_DIFFUSE, texTerrainDiff);
    terrainMat.SetTexture(TEX_TYPE_NORMALS, texTerrainNorm);
    const MaterialID terrainMatID = materialMgr.AddMaterial(std::move(terrainMat));

    terrain.meshes_.SetMaterialForSubset(0, terrainMatID);
}

///////////////////////////////////////////////////////////

void SetupTree(BasicModel& tree)
{
    // manually setup of tree pine model

    TextureMgr& texMgr       = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    const std::string pathToModelDir = g_RelPathExtModelsDir + "trees/FBX format/";

    // load texture from file for tree bark/branches/caps
    const TexID texBarkDiffID      = texMgr.LoadFromFile(pathToModelDir + "Bark_Color.png");
    const TexID texBarkNormID      = texMgr.LoadFromFile(pathToModelDir + "Bark_Normal.png");

    const TexID texBranchDiffID    = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Color.png");
    const TexID texBranchNormID    = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Normal.png");
    const TexID texBranchSubsurfID = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Subsurface.png");

    const TexID texCapDiffID       = texMgr.LoadFromFile(pathToModelDir + "Cap_Color.png");
    const TexID texCapNormID       = texMgr.LoadFromFile(pathToModelDir + "Cap_Normal.png");


    // setup materials
    Material barkMat;
    barkMat.SetName("tree_pine_bark");
    barkMat.SetTexture(TEX_TYPE_DIFFUSE, texBarkDiffID);
    barkMat.SetTexture(TEX_TYPE_NORMALS, texBarkNormID);
    barkMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    barkMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);

    Material branchMat;
    branchMat.SetName("tree_pine_branch");
    branchMat.SetTexture(TEX_TYPE_DIFFUSE, texBranchDiffID);
    branchMat.SetTexture(TEX_TYPE_NORMALS, texBranchNormID);
    branchMat.SetTexture(TEX_TYPE_OPACITY, texBranchSubsurfID);
    branchMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    branchMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
    branchMat.SetAlphaClip(true);

    Material capMat;
    capMat.SetName("tree_pine_cap");
    capMat.SetTexture(TEX_TYPE_DIFFUSE, texCapDiffID);
    capMat.SetTexture(TEX_TYPE_NORMALS, texCapNormID);
    capMat.SetAmbient(0.3f, 0.3f, 0.3f, 1.0f);
    capMat.SetDiffuse(0.6f, 0.6f, 0.6f, 1.0f);


    // add materials into the material manager
    const MaterialID barkMatID   = materialMgr.AddMaterial(std::move(barkMat));
    const MaterialID branchMatID = materialMgr.AddMaterial(std::move(branchMat));
    const MaterialID capMatID    = materialMgr.AddMaterial(std::move(capMat));

    // setup materials for tree subsets (meshes)
    tree.meshes_.SetMaterialForSubset(0, barkMatID);
    tree.meshes_.SetMaterialForSubset(1, capMatID);
    tree.meshes_.SetMaterialForSubset(2, branchMatID);
}

///////////////////////////////////////////////////////////

void SetupPowerLine(BasicModel& powerLine)
{
    // manually setup of power line model

    TextureMgr& texMgr       = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    // load textures from files
    const std::string texDirPath = g_RelPathExtModelsDir + "power_line/";
    const TexID texDiffID        = texMgr.LoadFromFile(texDirPath + "bigpoleiron_co.png");
    const TexID texNormID        = texMgr.LoadFromFile(texDirPath + "bigpoleiron_nohq.png");

    // setup and create a material
    Material powerLineMat;
    powerLineMat.SetName("power_line");
    powerLineMat.SetTexture(TEX_TYPE_DIFFUSE, texDiffID);
    powerLineMat.SetTexture(TEX_TYPE_NORMALS, texNormID);
    const MaterialID powerLineMatID = materialMgr.AddMaterial(std::move(powerLineMat));

    powerLine.SetMaterialForSubset(0, powerLineMatID);
}

///////////////////////////////////////////////////////////

void SetupBuilding9(BasicModel& building)
{
    // manually setup building model

    TextureMgr& texMgr = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    // load textures from files
    const TexID texIdHousePart = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/house_part.png");
    const TexID texIdArka      = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/arka.png");
    const TexID texIdBalcony   = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/balcony.png");
    const TexID texIdr5        = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/r5.png");

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
    const MaterialID housePartMatID = materialMgr.AddMaterial(std::move(housePartMat));
    const MaterialID arkaMatID      = materialMgr.AddMaterial(std::move(arkaMat));
    const MaterialID balconyMatID   = materialMgr.AddMaterial(std::move(balconyMat));
    const MaterialID r5MatID        = materialMgr.AddMaterial(std::move(r5Mat));
    
    // setup model with materials
    building.meshes_.SetMaterialForSubset(0, housePartMatID);
    building.meshes_.SetMaterialForSubset(1, arkaMatID);
    building.meshes_.SetMaterialForSubset(2, balconyMatID);
    building.meshes_.SetMaterialForSubset(3, r5MatID);
}

///////////////////////////////////////////////////////////

void SetupAk47(BasicModel& ak47)
{
    ak47.SetName("ak_47");
}

///////////////////////////////////////////////////////////

void SetupStalkerSmallHouse(BasicModel& house)
{
    // manually setup materials for the model

    MaterialMgr& materialMgr = *MaterialMgr::Get();
    MeshGeometry::Subset* subsets = house.meshes_.subsets_;

    for (int i = 0; i < house.numSubsets_; ++i)
    {
        // get a material by ID from the manager and setup it
        Material& mat = materialMgr.GetMaterialByID(subsets[i].materialID);

        mat.ambient  = { 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse  = { 0.8f, 0.8f, 0.8f, 1.0f };
        mat.specular = { 0,0,0,1 };
    }
}

///////////////////////////////////////////////////////////

void SetupStalkerAbandonedHouse(BasicModel& house)
{
    // manually setup materials for the model

    TextureMgr& texMgr = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    const std::string dirPath     = g_RelPathExtModelsDir + "/stalker/abandoned-house-20/";

    // load textures for the model
    const TexID texBrickDiff      = texMgr.LoadFromFile(dirPath + "textures/Brick_BaseColor.jpg");
    const TexID texConcreteDiff   = texMgr.LoadFromFile(dirPath + "textures/Concrete_BaseColor.jpg");
    const TexID texConcrete2Diff  = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_BaseColor.jpg");
    const TexID texRoofDiff       = texMgr.LoadFromFile(dirPath + "textures/Roof_BaseColor.jpg");
    const TexID texDoorDiff       = texMgr.LoadFromFile(dirPath + "textures/Door_BaseColor.jpg");
    const TexID texMetalDiff      = texMgr.LoadFromFile(dirPath + "textures/Metal_BaseColor.jpg");
    const TexID texWindowDiff     = texMgr.LoadFromFile(dirPath + "textures/Windows_BaseColor.jpg");
    const TexID texGroundDiff     = texMgr.LoadFromFile(dirPath + "textures/Ground_BaseColor.jpg");

    const TexID texBrickNorm      = texMgr.LoadFromFile(dirPath + "textures/Brick_Normal.jpg");
    const TexID texConcreteNorm   = texMgr.LoadFromFile(dirPath + "textures/Concrete_Normal.jpg");
    const TexID texConcrete2Norm  = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_Normal.jpg");
    const TexID texRoofNorm       = texMgr.LoadFromFile(dirPath + "textures/Roof_Normal.jpg");
    const TexID texDoorNorm       = texMgr.LoadFromFile(dirPath + "textures/Door_Normal.jpg");
    const TexID texMetalNorm      = texMgr.LoadFromFile(dirPath + "textures/Metal_Normal.jpg");
    const TexID texWindowNorm     = texMgr.LoadFromFile(dirPath + "textures/Windows_Normal.jpg");
    const TexID texGroundNorm     = texMgr.LoadFromFile(dirPath + "textures/Ground_Normal.jpg");

    const TexID texBrickRough     = texMgr.LoadFromFile(dirPath + "textures/Brick_Roughness.jpg");
    const TexID texConcreteRough  = texMgr.LoadFromFile(dirPath + "textures/Concrete_Roughness.jpg");
    const TexID texConcrete2Rough = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_Roughness.jpg");
    const TexID texRoofRough      = texMgr.LoadFromFile(dirPath + "textures/Roof_Roughness.jpg");
    const TexID texDoorRough      = texMgr.LoadFromFile(dirPath + "textures/Door_Roughness.jpg");
    const TexID texMetalRough     = texMgr.LoadFromFile(dirPath + "textures/Metal_Roughness.jpg");
    const TexID texWindowRough    = texMgr.LoadFromFile(dirPath + "textures/Windows_Roughness.jpg");
    const TexID texGroundRough    = texMgr.LoadFromFile(dirPath + "textures/Ground_Roughness.jpg");



    // setup materials of model's meshes
    MeshGeometry::Subset* subsets = house.meshes_.subsets_;

    Material& concreteMat = materialMgr.GetMaterialByID(subsets[0].materialID);
    concreteMat.SetName("abandoned_house_concrete");
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE, texConcreteDiff);
    concreteMat.SetTexture(TEX_TYPE_NORMALS, texConcreteNorm);
    concreteMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texConcreteRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& brickMat = materialMgr.GetMaterialByID(subsets[1].materialID);
    brickMat.SetName("abandoned_house_brick");
    brickMat.SetTexture(TEX_TYPE_DIFFUSE, texBrickDiff);
    brickMat.SetTexture(TEX_TYPE_NORMALS, texBrickNorm);
    brickMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texBrickRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& roofMat = materialMgr.GetMaterialByID(subsets[2].materialID);
    roofMat.SetName("abandoned_house_roof");
    roofMat.SetTexture(TEX_TYPE_DIFFUSE, texRoofDiff);
    roofMat.SetTexture(TEX_TYPE_NORMALS, texRoofNorm);
    roofMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texRoofRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& metalMat = materialMgr.GetMaterialByID(subsets[3].materialID);
    metalMat.SetName("abandoned_house_metal");
    metalMat.SetTexture(TEX_TYPE_DIFFUSE, texMetalDiff);
    metalMat.SetTexture(TEX_TYPE_NORMALS, texMetalNorm);
    metalMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texMetalRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& groundMat = materialMgr.GetMaterialByID(subsets[4].materialID);
    groundMat.SetName("abandoned_house_ground");
    groundMat.SetTexture(TEX_TYPE_DIFFUSE, texGroundDiff);
    groundMat.SetTexture(TEX_TYPE_NORMALS, texGroundNorm);
    groundMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texGroundNorm);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& doorMat = materialMgr.GetMaterialByID(subsets[5].materialID);
    doorMat.SetName("abandoned_house_door");
    doorMat.SetTexture(TEX_TYPE_DIFFUSE, texDoorDiff);
    doorMat.SetTexture(TEX_TYPE_NORMALS, texDoorNorm);
    doorMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texDoorRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

    Material& windowMat = materialMgr.GetMaterialByID(subsets[6].materialID);
    windowMat.SetName("abandoned_house_window");
    windowMat.SetTexture(TEX_TYPE_DIFFUSE, texWindowDiff);
    windowMat.SetTexture(TEX_TYPE_NORMALS, texWindowNorm);
    windowMat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, texWindowRough);
    concreteMat.SetAmbient(0.2f, 0.2f, 0.2f, 1.0f);
    concreteMat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);

#if 0
    // add materials into the material manager
    const MaterialID concreteMatID = materialMgr.AddMaterial(std::move(concreteMat));
    const MaterialID brickMatID    = materialMgr.AddMaterial(std::move(brickMat));
    const MaterialID roofMatID     = materialMgr.AddMaterial(std::move(roofMat));
    const MaterialID metalMatID    = materialMgr.AddMaterial(std::move(metalMat));
    const MaterialID groundMatID   = materialMgr.AddMaterial(std::move(groundMat));
    const MaterialID doorMatID     = materialMgr.AddMaterial(std::move(doorMat));
    const MaterialID windowMatID   = materialMgr.AddMaterial(std::move(windowMat));


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

    TextureMgr& texMgr = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    const TexID texID = texMgr.LoadFromFile(g_RelPathTexDir + "box01d.dds");

    // setup material and add it into the material manager
    Material cubeBox01Mat;
    cubeBox01Mat.SetName("cube_box01");
    cubeBox01Mat.SetTexture(TEX_TYPE_DIFFUSE, texID);
    const MaterialID cubeBox01MatID = materialMgr.AddMaterial(std::move(cubeBox01Mat));

    // cube has only one subset (mesh) by ID == 0
    cube.SetMaterialForSubset(0, cubeBox01MatID);
}

///////////////////////////////////////////////////////////

void SetupSphere(BasicModel& sphere)
{
    // manually setup the sphere model

    TextureMgr& texMgr = *TextureMgr::Get();
    MaterialMgr& materialMgr = *MaterialMgr::Get();

    const TexID texID = texMgr.LoadFromFile(g_RelPathTexDir + "gigachad.dds");

    // setup material and add it into the material manager
    Material sphereMat;
    sphereMat.SetName("gigachad");
    sphereMat.SetTexture(TEX_TYPE_DIFFUSE, texID);
    const MaterialID sphereMatID = materialMgr.AddMaterial(std::move(sphereMat));

    // setup material for a single mesh of the sphere model
    sphere.SetMaterialForSubset(0, sphereMatID);
}

} // namespace Core
