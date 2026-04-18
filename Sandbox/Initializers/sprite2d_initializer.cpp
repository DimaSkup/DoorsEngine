#include "../Common/pch.h"
#include "sprite2d_initializer.h"
#include <Texture/texture_mgr.h>

#pragma warning (disable : 4996)


namespace Game
{

using Core::g_TextureMgr;



//---------------------------------------------------------
// internal helper structures
//---------------------------------------------------------
struct SpriteInitParams
{
    // name for this sprite (for entity creation)
    char name[MAX_LEN_ENTT_NAME];

    // texture for sprite
    char texture[MAX_LEN_TEX_NAME];

    // position
    int top, left;

    // size
    int width, height;
};

//---------------------------------------------------------
// private helpers
//---------------------------------------------------------
void ReadSpriteParams(FILE* pFile, SpriteInitParams& params);

void CreateSprite(
    SpriteInitParams& params,
    ECS::EntityMgr& enttMgr,
    const int wndWidth,
    const int wndHeight);

//---------------------------------------------------------
// Desc:  load sprites declarations from the file
//        and create sprite entities
//---------------------------------------------------------
bool Sprite2dInitializer::Init(
    const char* cfgFilepath,
    ECS::EntityMgr& enttMgr,
    const int wndWidth,
    const int wndHeight)
{
    // check input args
    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogErr(LOG, "empty config filepath");
        return false;
    }
    if (wndWidth <= 0 || wndHeight <= 0)
    {
        LogErr(LOG, "invalid window's dimensions: (width: %d,  height: %d)", wndWidth, wndHeight);
        return false;
    }


    int count = 0;
    FILE* pFile = nullptr;
    char buf[128];

    // open config file for reading
    pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return false;
    }

    // skip comments section
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');


    // read in all the sprites declarations and create these 2D sprites
    while (fgets(buf, sizeof(buf), pFile))
    {
        if (strncmp(buf, "sprite", 6) == 0)
        {
            SpriteInitParams params;
            memset(&params, 0, sizeof(params));

            // get sprite name
            count = sscanf(buf, "sprite \"%s", params.name);
            if (count != 1)
            {
                LogErr(LOG, "can't read a sprite name from buffer: %s", buf);
                continue;
            }

            // skip last quote symbol (") at the end of the read name
            params.name[strlen(params.name) - 1] = '\0';

            LogMsg("create 2D sprite: %s", params.name);
            ReadSpriteParams(pFile, params);
            CreateSprite(params, enttMgr, wndWidth, wndHeight);
        }
    }

    // great success!
    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  read in from file paraterers for a sprite initialization
//---------------------------------------------------------
void ReadSpriteParams(FILE* pFile, SpriteInitParams& params)
{
    ReadFileStr(pFile, "texture",  params.texture);
    ReadFileInt(pFile, "top",     &params.top);
    ReadFileInt(pFile, "left",    &params.left);

    ReadFileInt(pFile, "width",   &params.width);
    ReadFileInt(pFile, "height",  &params.height);
}

//---------------------------------------------------------
// Desc:  create a new 2D sprite entity by input params
//---------------------------------------------------------
void CreateSprite(
    SpriteInitParams& params,
    ECS::EntityMgr& enttMgr,
    const int wndWidth,
    const int wndHeight)
{
    const TexID texId = g_TextureMgr.GetTexIdByName(params.texture);
    const EntityID spriteId = enttMgr.CreateEntity();

    constexpr int baseWndWidth = 1600;
    constexpr int baseWndHeight = 900;

    const float widthRatio  = (float)wndWidth  / (float)baseWndWidth;
    const float heightRatio = (float)wndHeight / (float)baseWndHeight;

    // adjust a position of sprite according to the current window's size
    params.left = (int)floorf(params.left * widthRatio);
    params.top  = (int)floorf(params.top * heightRatio);

    enttMgr.AddNameComponent(spriteId, params.name);
    enttMgr.AddSpriteComponent(spriteId, texId, params.left, params.top, params.width, params.height);
}

}
