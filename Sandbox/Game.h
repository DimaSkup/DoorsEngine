#pragma once
#include <Entity/EntityMgr.h>
#include <UI/UserInterface.h>
#include <CRender.h>
#include <Input/inputcodes.h>
#include <Engine/Engine.h>


namespace Game
{

class Game
{
public:
    Game() {}
    ~Game() {}

    bool Init(
        Core::Engine* pEngine,
        ECS::EntityMgr* pEntityMgr,
        Render::CRender* pRender);

    bool Update(const float deltaTime);

    // handle input
    void HandleGameEventMouse(const float deltaTime);
    void HandleGameEventKeyboard();
    void HandlePlayerActions(const eKeyCodes code);
    

    void SwitchFlashLight(ECS::EntityMgr& mgr, Render::CRender& render);

private:
    void InitParticles(ECS::EntityMgr& mgr);

private:
    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_ = nullptr;
    Render::CRender* pRender_    = nullptr;
};

} // namespace
