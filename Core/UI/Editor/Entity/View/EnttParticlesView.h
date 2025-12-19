#pragma once

#include "../Model/EnttParticlesModel.h"


namespace UI
{
// forward declaration
class IEditorController;


class EnttParticlesView
{
private:
    IEditorController* pController_ = nullptr;

public:
    EnttParticlesView(IEditorController* pController);

    void Render(const EnttParticlesModel& model);
};

} // namespace
