// =================================================================================
// Description:    an old parts of code which was used during initialization
// 
// =================================================================================

namespace Core
{

void SetupTerrain(BasicModel& terrain)
{
    TextureMgr& texMgr = *TextureMgr::Get();

    // load and set a texture for the terrain model
    const TexID texTerrainDiff = texMgr.LoadFromFile(g_RelPathTexDir + "terrain/detail_grnd_grass.dds");
    const TexID texTerrainNorm = texMgr.LoadFromFile(g_RelPathTexDir + "terrain/detail_grnd_grass_bump.dds");

    terrain.SetTexture(0, TexType::DIFFUSE, texTerrainDiff);
    terrain.SetTexture(0, TexType::NORMALS, texTerrainNorm);
}

///////////////////////////////////////////////////////////

void SetupTree(BasicModel& tree)
{
    TextureMgr& texMgr = *TextureMgr::Get();

    // import model from file
    const std::string pathToModelDir = g_RelPathExtModelsDir + "trees/FBX format/";

    // manually setup the tree model's textures
    using enum TexType;
    TexType types[2] = { DIFFUSE, NORMALS };
    TexType typesForLeaves[3] = { DIFFUSE, NORMALS, OPACITY };

    TexID texturesBark[2];
    TexID texturesLeaves[3];
    TexID texturesCap[2];

    texturesBark[0] = texMgr.LoadFromFile(pathToModelDir + "Bark_Color.png");
    texturesBark[1] = texMgr.LoadFromFile(pathToModelDir + "Bark_Normal.png");

    texturesLeaves[0] = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Color.png");
    texturesLeaves[1] = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Normal.png");
    texturesLeaves[2] = texMgr.LoadFromFile(pathToModelDir + "conifer_macedonian_pine_Subsurface.png");

    texturesCap[0] = texMgr.LoadFromFile(pathToModelDir + "Cap_Color.png");
    texturesCap[1] = texMgr.LoadFromFile(pathToModelDir + "Cap_Normal.png");

    tree.SetTextures(0, types, texturesBark, 2);
    tree.SetTextures(1, types, texturesCap, 2);
    tree.SetTextures(2, typesForLeaves, texturesLeaves, 3);


    // setup materials for the tree
    for (int i = 0; i < tree.numSubsets_; ++i)
    {
        MeshMaterial& mat = tree.materials_[i];
        mat.ambient_ = { 0.1f,0.1f,0.1f,1.0f };
        mat.specular_ = { 0,0,0,1 };
    }
}

///////////////////////////////////////////////////////////

void SetupPowerLine(BasicModel& powerLine)
{
    TexType texTypes[2] = { DIFFUSE, NORMALS };
    TexID texIds[2]{ 0 };

    TextureMgr& texMgr = *TextureMgr::Get();
    const std::string texDirPath = g_RelPathExtModelsDir + "power_line/";

    texIds[0] = texMgr.LoadFromFile(texDirPath + "bigpoleiron_co.png");
    texIds[1] = texMgr.LoadFromFile(texDirPath + "bigpoleiron_nohq.png");

    powerLine.SetTextures(0, texTypes, texIds, 2);
}

///////////////////////////////////////////////////////////

void SetupBuilding9(BasicModel& building)
{
    // setup building
    TextureMgr& texMgr = *TextureMgr::Get();

    TexID texIdHousePart = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/house_part.png");
    TexID texIdArka = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/arka.png");
    TexID texIdBalcony = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/balcony.png");
    TexID texIdr5 = texMgr.LoadFromFile(g_RelPathExtModelsDir + "building9/r5.png");

    building.SetTexture(0, TexType::DIFFUSE, texIdHousePart);
    building.SetTexture(1, TexType::DIFFUSE, texIdArka);
    building.SetTexture(2, TexType::DIFFUSE, texIdBalcony);
    building.SetTexture(3, TexType::DIFFUSE, texIdr5);
}

///////////////////////////////////////////////////////////

void SetupAk47(BasicModel& ak47)
{
    ak47.SetName("ak_47");
}

///////////////////////////////////////////////////////////

void SetupStalkerSmallHouse(BasicModel& house)
{
    // setup materials
    for (int i = 0; i < house.numSubsets_; ++i)
    {
        MeshMaterial& mat = house.materials_[i];

        //mat.diffuse_ = MathHelper::RandColorRGBA();
        mat.ambient_ = { 0.1f,0.1f,0.1f,1.0f };
        mat.specular_ = { 0,0,0,1 };
    }
}

///////////////////////////////////////////////////////////

void SetupStalkerAbandonedHouse(BasicModel& house)
{
    TextureMgr& texMgr = *TextureMgr::Get();

    const std::string dirPath = g_RelPathExtModelsDir + "/stalker/abandoned-house-20/";
    const std::string pathToModel = dirPath + "source/LittleHouse.fbx";

    TexID texBrickDiff      = texMgr.LoadFromFile(dirPath + "textures/Brick_BaseColor.jpg");
    TexID texConcreteDiff   = texMgr.LoadFromFile(dirPath + "textures/Concrete_BaseColor.jpg");
    TexID texConcrete2Diff  = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_BaseColor.jpg");
    TexID texRoofDiff       = texMgr.LoadFromFile(dirPath + "textures/Roof_BaseColor.jpg");
    TexID texDoorDiff       = texMgr.LoadFromFile(dirPath + "textures/Door_BaseColor.jpg");
    TexID texMetalDiff      = texMgr.LoadFromFile(dirPath + "textures/Metal_BaseColor.jpg");
    TexID texWindowDiff     = texMgr.LoadFromFile(dirPath + "textures/Windows_BaseColor.jpg");
    TexID texGroundDiff     = texMgr.LoadFromFile(dirPath + "textures/Ground_BaseColor.jpg");

    TexID texBrickNorm      = texMgr.LoadFromFile(dirPath + "textures/Brick_Normal.jpg");
    TexID texConcreteNorm   = texMgr.LoadFromFile(dirPath + "textures/Concrete_Normal.jpg");
    TexID texConcrete2Norm  = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_Normal.jpg");
    TexID texRoofNorm       = texMgr.LoadFromFile(dirPath + "textures/Roof_Normal.jpg");
    TexID texDoorNorm       = texMgr.LoadFromFile(dirPath + "textures/Door_Normal.jpg");
    TexID texMetalNorm      = texMgr.LoadFromFile(dirPath + "textures/Metal_Normal.jpg");
    TexID texWindowNorm     = texMgr.LoadFromFile(dirPath + "textures/Windows_Normal.jpg");
    TexID texGroundNorm     = texMgr.LoadFromFile(dirPath + "textures/Ground_Normal.jpg");

    TexID texBrickRough     = texMgr.LoadFromFile(dirPath + "textures/Brick_Roughness.jpg");
    TexID texConcreteRough  = texMgr.LoadFromFile(dirPath + "textures/Concrete_Roughness.jpg");
    TexID texConcrete2Rough = texMgr.LoadFromFile(dirPath + "textures/Concrete_2_Roughness.jpg");
    TexID texRoofRough      = texMgr.LoadFromFile(dirPath + "textures/Roof_Roughness.jpg");
    TexID texDoorRough      = texMgr.LoadFromFile(dirPath + "textures/Door_Roughness.jpg");
    TexID texMetalRough     = texMgr.LoadFromFile(dirPath + "textures/Metal_Roughness.jpg");
    TexID texWindowRough    = texMgr.LoadFromFile(dirPath + "textures/Windows_Roughness.jpg");
    TexID texGroundRough    = texMgr.LoadFromFile(dirPath + "textures/Ground_Roughness.jpg");


    house.SetTexture(0, TexType::DIFFUSE, texConcreteDiff);
    house.SetTexture(1, TexType::DIFFUSE, texBrickDiff);
    house.SetTexture(2, TexType::DIFFUSE, texRoofDiff);
    house.SetTexture(3, TexType::DIFFUSE, texMetalDiff);
    house.SetTexture(4, TexType::DIFFUSE, texGroundDiff);
    house.SetTexture(5, TexType::DIFFUSE, texDoorDiff);
    house.SetTexture(6, TexType::DIFFUSE, texWindowDiff);
    house.SetTexture(7, TexType::DIFFUSE, texMetalDiff);

    house.SetTexture(0, TexType::HEIGHT, texConcreteNorm);
    house.SetTexture(1, TexType::HEIGHT, texBrickNorm);
    house.SetTexture(2, TexType::HEIGHT, texRoofNorm);
    house.SetTexture(3, TexType::HEIGHT, texMetalNorm);
    house.SetTexture(4, TexType::HEIGHT, texGroundNorm);
    house.SetTexture(5, TexType::HEIGHT, texDoorNorm);
    house.SetTexture(6, TexType::HEIGHT, texWindowNorm);
    house.SetTexture(7, TexType::HEIGHT, texMetalNorm);

    house.SetTexture(0, TexType::DIFFUSE_ROUGHNESS, texConcreteRough);
    house.SetTexture(1, TexType::DIFFUSE_ROUGHNESS, texBrickRough);
    house.SetTexture(2, TexType::DIFFUSE_ROUGHNESS, texRoofRough);
    house.SetTexture(3, TexType::DIFFUSE_ROUGHNESS, texMetalRough);
    house.SetTexture(4, TexType::DIFFUSE_ROUGHNESS, texGroundRough);
    house.SetTexture(5, TexType::DIFFUSE_ROUGHNESS, texDoorRough);
    house.SetTexture(6, TexType::DIFFUSE_ROUGHNESS, texWindowRough);
    house.SetTexture(7, TexType::DIFFUSE_ROUGHNESS, texMetalRough);

    // setup materials
    for (int i = 0; i < house.numSubsets_; ++i)
    {
        MeshMaterial& mat = house.materials_[i];

        mat.diffuse_ = { 0.8f,0.8f,0.8f,1.0f };
        mat.ambient_ = { 0.2f,0.2f,0.2f,1.0f };
        mat.specular_.w = 1.0f;
    }
}

///////////////////////////////////////////////////////////

void SetupSphere(BasicModel& sphere)
{
    TextureMgr& texMgr = *TextureMgr::Get();
    TexID texID = texMgr.LoadFromFile(g_RelPathTexDir + "gigachad.dds");

    sphere.SetTexture(0, TexType::DIFFUSE, texID);
}

} // namespace Core
