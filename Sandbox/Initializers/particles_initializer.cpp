/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: particles_initializer.cpp
    Desc:     implementation of the ParticlesInitializer class

    Created:  21.01.2025  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include <Timers/game_timer.h>
#include <parse_helpers.h>              // helpers for parsing string buffers, or reading data from file
#include "particles_initializer.h"
#include <Render/debug_draw_manager.h>

#pragma warning (disable : 4996)

using namespace ECS;


namespace Game
{

//---------------------------------------------------------
// forward declarations of some private helpers
//---------------------------------------------------------
eEmitterPropType GetEmitterPropType(const char* prop);

void HandleEmitterProperty(
    ParticleEmitter& emitter,
    const eEmitterPropType prop,
    const char* buf,
    DirectX::BoundingBox& aabb);

void PushEmitterForDbgRender(
    const DirectX::XMFLOAT3& emitterPos,
    const DirectX::XMFLOAT3& aabbCenter,
    const DirectX::XMFLOAT3& aabbExtents);

//---------------------------------------------------------
// Desc:  load particle emitters data from file and create them
// Args:  - filepath:  a path to the particles config file
//                       (relatively to the working directory)
//        - enttMgr:   a ref to ECS entity manager
// Ret:   true if everything is ok
//---------------------------------------------------------
bool ParticlesInitializer::Init(const char* configFilepath, EntityMgr& enttMgr)
{
    using namespace Core;

    if (StrHelper::IsEmpty(configFilepath))
    {
        LogErr(LOG, "empty filepath");
        return false;
    }


    FILE* pFile = nullptr;
    char buf[256];
    char emitterName[64];
    int  emitterIdx = 0;
    int  count = 0;
    const TimePoint start = GameTimer::GetTimePoint();

    SetConsoleColor(YELLOW);
    LogMsg("---------------------------------------------------------");
    LogMsg("            INITIALIZATION: PARTICLE EMITTERS            ");
    LogMsg("---------------------------------------------------------");
    LogMsg(LOG, "initialize emitters from file: %s", configFilepath);


    pFile = fopen(configFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a particles file: %s", configFilepath);
        return false;
    }

    // skip comments section
    do
    {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');


    // read in definition of each particle emitter
    while (!feof(pFile))
    {
        fgets(buf, sizeof(buf), pFile);

        if (strncmp(buf, "emitter", 7) != 0)
            continue;

        count = sscanf(buf, "emitter \"%s", emitterName);
        assert(count == 1);

        // skip the last quote (") symbol in the name
        emitterName[strlen(emitterName) - 1] = '\0';

        ReadAndCreateEmitter(enttMgr, pFile, emitterName, emitterIdx);
        emitterIdx++;
    }


    const TimePoint      end = GameTimer::GetTimePoint();
    const TimeDurationMs dur = end - start;

    LogMsg(LOG, "all the PARTICLE emitters are initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("--------------------------------------");
    LogMsg("Init of particles took: %.3f ms", dur.count());
    LogMsg("--------------------------------------\n");
    SetConsoleColor(RESET);


    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:    load data from a file and create particle emitters using this data
// Args:    - pFile:   particles config file descriptor
//---------------------------------------------------------
void ParticlesInitializer::ReadAndCreateEmitter(
    EntityMgr& enttMgr,
    FILE* pFile,
    const char* emitterName,
    const int emitterIdx)
{
    assert(pFile);
    assert(emitterName && emitterName[0] != '\0');
    assert(emitterIdx >= 0);

    char buf[128];
    char propName[32]{'\0'};
    int count = 0;
    bool readProps = true;

    // create a new entity and bind a particles emitter
    const EntityID enttId = enttMgr.CreateEntity();
    enttMgr.AddParticleEmitterComponent(enttId);

    ParticleEmitter& emitter = enttMgr.particleSys_.GetEmitter(enttId);
    DirectX::BoundingBox aabb;


    // read in common params
    LogMsg("\t[%d] create particle emitter: %s", emitterIdx, emitterName);

    while (readProps && fgets(buf, sizeof(buf), pFile))
    {
        if (buf[0] == '}')
        {
            readProps = false;
            continue;
        }

        count = sscanf(buf, " %s", propName);
        assert(count == 1);

        const eEmitterPropType propType = GetEmitterPropType(propName);

        // something went wrong so just move to the next property
        if (propType == EMITTER_PROP_TYPE_NONE)
            continue;

        HandleEmitterProperty(emitter, propType, buf, aabb);
    }


    DirectX::XMFLOAT3 pos;
    DirectX::XMStoreFloat3(&pos, emitter.position);
    PushEmitterForDbgRender(pos, aabb.Center, aabb.Extents);

    // setup the entity
    enttMgr.AddTransformComponent(enttId, pos);
    enttMgr.AddNameComponent(enttId, emitterName);
    enttMgr.AddBoundingComponent(enttId, aabb);
}

//---------------------------------------------------------
// Desc:  define what generation type the emitter will have
//        (what will be a source of particles?)
//---------------------------------------------------------
eEmitterSrcType GetEmitterSrcType(const char* type)
{
    if (StrHelper::IsEmpty(type))
    {
        LogErr(LOG, "empty input str");
        return EMITTER_SRC_TYPE_POINT;
    }

    if (strcmp(type, "point") == 0)
        return EMITTER_SRC_TYPE_POINT;

    if (strcmp(type, "plane") == 0)
        return EMITTER_SRC_TYPE_PLANE;

    if (strcmp(type, "volume") == 0)
        return EMITTER_SRC_TYPE_VOLUME;

    if (strcmp(type, "splash") == 0)
        return EMITTER_SRC_TYPE_SPLASH;

    LogErr(LOG, "unknown emitter's source type: %s", type);
    return EMITTER_SRC_TYPE_POINT;
}

//---------------------------------------------------------
// Desc:  define a generation type for velocity direction
//---------------------------------------------------------
eVelocityDirInitType GetVelocityDirectionGenType(const char* type)
{
    if (StrHelper::IsEmpty(type))
    {
        LogErr(LOG, "empty input str");
        return PARTICLE_VELOCITY_DIR_RANDOM;
    }

    if (strcmp(type, "rand") == 0)
        return PARTICLE_VELOCITY_DIR_RANDOM;

    if (strcmp(type, "defined") == 0)
        return PARTICLE_VELOCITY_DIR_DEFINED;

    LogErr(LOG, "uknown velocity direction generation type: %s", type);
    return PARTICLE_VELOCITY_DIR_RANDOM;
}

//---------------------------------------------------------
// Desc:  define what to do with a particle when it hit its emitter's AABB
//---------------------------------------------------------
eEventParticleHitBox GetEventParticleHitAABB(const char* eventType)
{
    if (StrHelper::IsEmpty(eventType))
    {
        LogErr(LOG, "empty input str");
        return EVENT_PARTICLE_HIT_BOX_DIE;
    }

    if (strcmp(eventType, "die") == 0)
        return EVENT_PARTICLE_HIT_BOX_DIE;

    if (strcmp(eventType, "reflect") == 0)
        return EVENT_PARTICLE_HIT_BOX_REFLECT;

    LogErr(LOG, "unknown particle's hit event type: %s", eventType);
    return EVENT_PARTICLE_HIT_BOX_DIE;
}

//---------------------------------------------------------
// Desc:  add a debug AABB of this emitter for rendering (when we turn on dbg rendering)
//---------------------------------------------------------
void PushEmitterForDbgRender(
    const DirectX::XMFLOAT3& emitterPos,
    const DirectX::XMFLOAT3& aabbCenter,
    const DirectX::XMFLOAT3& aabbExtents)
{
    const DirectX::XMFLOAT3 minP(aabbCenter - aabbExtents + emitterPos);   // AABB's min point in world
    const DirectX::XMFLOAT3 maxP(aabbCenter + aabbExtents + emitterPos);   // AABB's max point in world
    const Vec3 yellow = Vec3(0, 1, 1);


    Core::g_DebugDrawMgr.AddAABB(
        Vec3(minP.x, minP.y, minP.z),
        Vec3(maxP.x, maxP.y, maxP.z),
        yellow);
}

//---------------------------------------------------------
// Desc:  get a type of emitter's property by input string
//---------------------------------------------------------
eEmitterPropType GetEmitterPropType(const char* prop)
{
    assert(prop && prop[0] != '\0');

    switch (prop[0])
    {
        // AABB params
        case 'a':
            if (strcmp(prop, "aabb_center") == 0)   return EMITTER_AABB_CENTER;
            if (strcmp(prop, "aabb_extents") == 0)  return EMITTER_AABB_EXTENTS;
            break;

        case 'c':
            if (strcmp(prop, "color_after_reflect") == 0)
                return EMITTER_PARTICLE_COLOR_AFTER_REFLECT;
            break;

   
        case 'e':
            if (strcmp(prop, "end_color") == 0)      return EMITTER_PARTICLE_END_COLOR;
            if (strcmp(prop, "end_size") == 0)       return EMITTER_PARTICLE_END_SIZE;
            if (strcmp(prop, "end_alpha") == 0)      return EMITTER_PARTICLE_END_ALPHA;
            if (strcmp(prop, "ext_forces") == 0)     return EMITTER_EXTERNAL_FORCES;
            if (strcmp(prop, "event_hit_aabb") == 0) return EMITTER_EVENT_PARTICLE_HIT_AABB;
            break;

        // friction (air resistance)
        case 'f':
            if (strcmp(prop, "friction") == 0)      return EMITTER_PARTICLE_FRICTION;
            break;

        case 'l':
            if (strcmp(prop, "lifetime_sec") == 0)  return EMITTER_PARTICLE_LIFETIME_SEC;
            break;

        // material / mass
        case 'm':
            if (strcmp(prop, "material") == 0)      return EMITTER_MATERIAL;
            if (strcmp(prop, "mass") == 0)          return EMITTER_PARTICLE_MASS;
            break;

          
        // position
        case 'p':
            if (strcmp(prop, "pos") == 0)           return EMITTER_POSITION;
            break;

        // src_type / spawn_rate / etc.
        case 's':
            if (strcmp(prop, "src_type") == 0)      return EMITTER_SRC_TYPE;
            if (strcmp(prop, "spawn_rate") == 0)    return EMITTER_SPAWN_RATE;
            if (strcmp(prop, "start_color") == 0)   return EMITTER_PARTICLE_START_COLOR;
            if (strcmp(prop, "start_size") == 0)    return EMITTER_PARTICLE_START_SIZE;
            if (strcmp(prop, "start_alpha") == 0)   return EMITTER_PARTICLE_START_ALPHA;
            if (strcmp(prop, "src_plane_offset_y") == 0) return EMITTER_SRC_PLANE_OFFSET_Y;
            break;

        // texture params
        case 't':
            if (strcmp(prop, "tex_anim_enable") == 0)
                return EMITTER_TEX_ANIM_ENABLE;

            if (strcmp(prop, "tex_anim_duration_sec") == 0)
                return EMITTER_TEX_ANIM_DURATION_SEC;

            if (strcmp(prop, "tex_num_frames_by_x") == 0)
                return EMITTER_TEX_NUM_FRAMES_BY_X;

            if (strcmp(prop, "tex_num_frames_by_y") == 0)
                return EMITTER_TEX_NUM_FRAMES_BY_Y;

            break;

        // velocity params
        case 'v':
            if (strcmp(prop, "vel_init_type") == 0) return EMITTER_VEL_INIT_TYPE;
            if (strcmp(prop, "vel_init_dir") == 0)  return EMITTER_VEL_INIT_DIR;
            if (strcmp(prop, "vel_init_mag") == 0)  return EMITTER_VEL_INIT_MAG;
            break;
    }

    LogErr(LOG, "invalid emitter's property: %s", prop);
    return EMITTER_PROP_TYPE_NONE;
}

//---------------------------------------------------------
// Desc:  parse emitter's property values from the input str buffer
//        according to the property type
//---------------------------------------------------------
void HandleEmitterProperty(
    ParticleEmitter& emitter,
    const eEmitterPropType prop,
    const char* buf,
    DirectX::BoundingBox& aabb)
{
    switch (prop)
    {
        case EMITTER_MATERIAL:
        {
            char materialName[64]{ '\0' };
            ReadStr(buf, " material %s", materialName);
            emitter.materialId = Core::g_MaterialMgr.GetMatIdByName(materialName);
            break;
        }

        case EMITTER_SRC_TYPE:
        {
            char srcTypeStr[32]{ '\0' };
            ReadStr(buf, " src_type %s", srcTypeStr);
            emitter.srcType = GetEmitterSrcType(srcTypeStr);
            break;
        }

        case EMITTER_POSITION:
        {
            Vec3 pos;
            ReadFloat3(buf, " pos %f %f %f", &pos.x);
            emitter.position = DirectX::XMVECTOR{ pos.x, pos.y, pos.z };
            break;
        }

        case EMITTER_SRC_PLANE_OFFSET_Y:
        {
            ReadFloat(buf, " src_plane_offset_y %f", &emitter.srcPlaneHeight);
            break;
        }

        case EMITTER_VEL_INIT_TYPE:
        {
            char velInitTypeStr[16]{ '\0' };
            ReadStr(buf, " vel_init_type %s", velInitTypeStr);
            emitter.velDirInitType = GetVelocityDirectionGenType(velInitTypeStr);
            break;
        }

        case EMITTER_VEL_INIT_DIR:
        {
            ReadFloat3(buf, " vel_init_dir %f %f %f", &emitter.velInitDir.x);
            break;
        }
        
        case EMITTER_VEL_INIT_MAG:
        {
            ReadFloat(buf, " vel_init_mag %f", &emitter.velInitMag);
            break;
        }

        case EMITTER_SPAWN_RATE:
        {
            ReadInt(buf, " spawn_rate %d", &emitter.spawnRate);
            break;
        }

        case EMITTER_PARTICLE_LIFETIME_SEC:
        {
            ReadFloat(buf, " lifetime_sec %f", &emitter.life);
            break;
        }

        case EMITTER_PARTICLE_START_COLOR:
        {
            ReadFloat3(buf, " start_color %f %f %f", &emitter.startColor.x);
            break;
        }

        case EMITTER_PARTICLE_END_COLOR:
        {
            ReadFloat3(buf, " end_color %f %f %f", &emitter.endColor.x);
            break;
        }

        case EMITTER_PARTICLE_COLOR_AFTER_REFLECT:
        {
            ReadFloat3(buf, " color_after_reflect %f %f %f", &emitter.colorAfterReflect.x);
            break;
        }

        case EMITTER_PARTICLE_START_SIZE:
        {
            ReadFloat2(buf, " start_size %f %f", &emitter.startSize.x);
            break;
        }

        case EMITTER_PARTICLE_END_SIZE:
        {
            ReadFloat2(buf, " end_size %f %f", &emitter.endSize.x);
            break;
        }

        case EMITTER_PARTICLE_START_ALPHA:
        {
            ReadFloat(buf, " start_alpha %f", &emitter.startAlpha);
            break;
        }

        case EMITTER_PARTICLE_END_ALPHA:
        {
            ReadFloat(buf, " end_alpha %f", &emitter.endAlpha);
            break;
        }

        case EMITTER_PARTICLE_MASS:
        {
            ReadFloat(buf, " mass %f", &emitter.mass);
            break;
        }

        case EMITTER_PARTICLE_FRICTION:
        {
            ReadFloat(buf, " friction %f", &emitter.friction);
            break;
        }

        case EMITTER_EXTERNAL_FORCES:
        {
            Vec3 forces = { 0,0,0 };
            ReadFloat3(buf, " ext_forces %f %f %f", &forces.x);
            emitter.forces = DirectX::XMVECTOR{ forces.x, forces.y, forces.z };
            break;
        }

        case EMITTER_AABB_CENTER:
        {
            ReadFloat3(buf, " aabb_center %f %f %f", &aabb.Center.x);
            break;
        }

        case EMITTER_AABB_EXTENTS:
        {
            ReadFloat3(buf, " aabb_extents %f %f %f", &aabb.Extents.x);
            break;
        }

        case EMITTER_EVENT_PARTICLE_HIT_AABB:
        {
            char hitEventStr[16]{ '\0' };
            ReadStr(buf, " event_hit_aabb %s", hitEventStr);
            emitter.hitEvent = GetEventParticleHitAABB(hitEventStr);
            break;
        }

        case EMITTER_TEX_ANIM_ENABLE:
        {
            char cond[8];
            ReadStr(buf, " tex_anim_enable %s", cond);

            if (strcmp(cond, "true") == 0)
                emitter.hasTexAnimations = true;
            break;
        }

        case EMITTER_TEX_ANIM_DURATION_SEC:
        {
            ReadFloat(buf, " tex_anim_duration_sec %f", &emitter.texAnimDurationSec);
            break;
        }

        case EMITTER_TEX_NUM_FRAMES_BY_X:
        {
            int numFrames = 1;
            ReadInt(buf, " tex_num_frames_by_x %d", &numFrames);
            emitter.numTexFramesByX = numFrames;
            break;
        }

        case EMITTER_TEX_NUM_FRAMES_BY_Y:
        {
            int numFrames = 1;
            ReadInt(buf, " tex_num_frames_by_y %d", &numFrames);
            emitter.numTexFramesByY = numFrames;
            break;
        }
    }
}



} // namespace
