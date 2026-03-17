// functional for drawing debug shapes
#pragma once
#include <Render/CRender.h>

namespace Core
{

class DbgShapeRender
{
public:
    void Render(Render::CRender* pRender, const bool bGameMode);

private:
    void RenderLines            (Render::CRender* pRender);
    void RenderAABBs            (Render::CRender* pRender);
    void RenderSpheres          (Render::CRender* pRender);
    void RenderTerrainAABBs     (Render::CRender* pRender);
    void RenderFrustum          (Render::CRender* pRender);
    void RenderModelsWireframes (Render::CRender* pRender);
};

}
