// =================================================================================
// Filename:       EnttEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttEditorController.h"

#include <UICommon/gui_states.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/events_history.h>

#include <UICommon/icommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include <imgui.h>
#include <ImGuizmo.h>
#include <stdexcept>

#pragma warning (disable : 4996)


namespace UI
{

EnttEditorController::EnttEditorController() :
    viewEnttParticles_(this)
{
}

EnttEditorController::~EnttEditorController()
{
    SafeDelete(pAnimationController_);
}

// =================================================================================
// public API
// =================================================================================

void EnttEditorController::Init(IFacadeEngineToUI* pFacade)
{
    try
    {
        // the facade interface is used to contact with the rest of the engine
        CAssert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
        pFacade_ = pFacade;

        particlesController_.Initialize(pFacade);

        pAnimationController_ = NEW EnttAnimationController();
        CAssert::NotNullptr(pAnimationController_, "can't alloc memory from entity animation controller");
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't init entities editor controller");
        exit(0);
    }
    
}

//---------------------------------------------------------
// Desc:  set that we have selected (or not) some entity and load its data
//---------------------------------------------------------
void EnttEditorController::SetSelectedEntt(const EntityID enttId)
{
    // if we deselected the chosen entt so we won't render any control panel
    // until we won't select any other
    if (enttId == 0)
    {
        selectedEnttData_.id   = 0;
        memset(selectedEnttData_.name, 0, MAX_LEN_ENTT_NAME);
        return;
    }
    // we have chosen some entt
    else
    {
        // get selected entity ID and name
        selectedEnttData_.id = enttId;
        const char* enttName = pFacade_->GetEnttNameById(enttId);
        if (enttName)
        {
            strncpy(selectedEnttData_.name, enttName, MAX_LEN_ENTT_NAME);
        }

        // get names of added components
        pFacade_->GetEnttAddedComponentsTypes(enttId, selectedEnttData_.componentsTypes);


        // load data for each component which is added to the selected entity
        for (const eEnttComponentType type : selectedEnttData_.componentsTypes)
        {
            switch (type)
            {
                case NameComponent:
                {
                    break;
                }
                case TransformComponent:
                {
                    transformController_.LoadEnttData(pFacade_, enttId);
                    break;
                }
                case MoveComponent:
                {
                    break;
                }
                case RenderedComponent:
                {
                    break;
                }
                case ModelComponent:
                {
                    break;
                }
                case CameraComponent:
                {
                    break;
                }
                case MaterialComponent:
                {
                    break;
                }
                case TextureTransformComponent:
                {
                    break;
                }
                case ParticlesComponent:
                {
                    particlesController_.LoadEnttData(enttId);
                    break;
                }
                case LightComponent:
                {
                    int lightType;

                    if (pFacade_->GetEnttLightType(enttId, lightType))
                    {
                        if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_DIRECTED)
                            dirLightController_.LoadEnttData(pFacade_, enttId);

                        else if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_POINT)
                            pointLightController_.LoadEnttData(pFacade_, enttId);

                        else if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_SPOT)
                            spotLightController_.LoadEnttData(pFacade_, enttId);

                        else
                        {
                            LogErr(LOG, "unknown light type (%d) for entity (id: %" PRIu32 ", name: % s)", lightType, enttId, selectedEnttData_.name);
                            return;
                        }

                        // store the subtype of the light for this entity
                        selectedEnttData_.lightType = eEnttLightType(lightType);
                    }
                    
                    break;
                }
                case BoundingComponent:
                {
                    break;
                }
                case AnimationComponent:
                {
                    pAnimationController_->LoadEnttData(pFacade_, enttId);
                    break;
                }
            } // switch
        } // for
    }
}

//---------------------------------------------------------
// Desc:  render a panel for controlling properties of the chosen entity
//---------------------------------------------------------
void EnttEditorController::Render()
{
    // if we have no entity as selected...
    if (selectedEnttData_.id == 0)
        return;

    const EntityID enttId   = selectedEnttData_.id;
    const char*    enttName = selectedEnttData_.name;

    // we want the next editor panel to be visible
    static bool isOpen = true;
    ImGui::SetNextItemOpen(isOpen);

    // render a list of components which are added to the entity (for editing these components)
    ImGui::Separator();
    ImGui::Text("Entity ID:   %d", enttId);
    ImGui::Text("Entity name: %s", enttName);

    
    // render view (editor control fields) for each added component
    const ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_SpanFullWidth;

    for (const eEnttComponentType type : selectedEnttData_.componentsTypes)
    {
        switch (type)
        {
            case NameComponent:
            {
                if (ImGui::CollapsingHeader("Name", flags))
                    ImGui::Text("Entity name: %s", enttName);
                break;
            }
            case TransformComponent:
            {
                if (ImGui::CollapsingHeader("Transformation", flags))
                    viewEnttTransform_.Render(this, transformController_.GetData());
                break;
            }
            case MoveComponent:
            {
                break;
            }
            case RenderedComponent:
            {
                break;
            }
            case ModelComponent:
            {
                break;
            }
            case CameraComponent:
            {
                break;
            }
            case MaterialComponent:
            {
                break;
            }
            case TextureTransformComponent:
            {
                break;
            }
            case ParticlesComponent:
            {
                if (ImGui::CollapsingHeader("Particle Emitter", flags))
                    viewEnttParticles_.Render(particlesController_.GetModel());

                break;
            }
            case LightComponent:
            {
                // according to the light type which is added to entity we render responsible view (set of editor fields)
                if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_DIRECTED)
                {
                    if (ImGui::CollapsingHeader("Directed light", flags))
                        viewEnttLight_.Render(this, dirLightController_.GetData());
                }
                else if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_POINT)
                {
                    if (ImGui::CollapsingHeader("Point light", flags))
                        viewEnttLight_.Render(this, pointLightController_.GetData());
                }
                else if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_SPOT)
                {
                    if (ImGui::CollapsingHeader("Spotlight", flags))
                        viewEnttLight_.Render(this, spotLightController_.GetData());
                }
                break;
            }
            case BoundingComponent:
            {
                break;
            }
            case AnimationComponent:
            {
                // for animations we have to actively update some info
                pAnimationController_->LoadEnttData(pFacade_, enttId);

                if (ImGui::CollapsingHeader("Animations", flags))
                    viewEnttAnimation_.Render(this, pAnimationController_->GetData());
                break;
            }
        } // switch
    } // for 
}


// =================================================================================
// For manipulation with gizmos
// =================================================================================

//---------------------------------------------------------
// Desc:   when we used a gizmo to modify a world of the selected entity
//         we call this method to actual update of the world properties
// Args:   - world:  new world matrix for the currently selected entity
//---------------------------------------------------------
void EnttEditorController::UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world)
{
    DirectX::XMVECTOR S, R, T;
    XMMatrixDecompose(&S, &R, &T, world);

    // execute transformation of the selected entity according to the transformation type
    switch (g_GuiStates.gizmoOperation)
    {
        case ImGuizmo::OPERATION::TRANSLATE:
        {
            DirectX::XMFLOAT3 t;
            DirectX::XMStoreFloat3(&t, T);
            CmdChangeVec3 cmd(CHANGE_ENTITY_POSITION, Vec3(t.x, t.y, t.z));
            ExecCmd(&cmd);
            break;
        }
        case ImGuizmo::OPERATION::ROTATE:
        {
            DirectX::XMFLOAT4 q;
            DirectX::XMStoreFloat4(&q, R);
            CmdChangeVec4 cmd(CHANGE_ENTITY_ROTATION, Vec4(q.x, q.y, q.z, q.w));
            ExecCmd(&cmd);
            break;
        }
        case ImGuizmo::OPERATION::SCALE:
        {
            const float uniformScale = DirectX::XMVectorGetX(S);
            CmdChangeFloat cmd(CHANGE_ENTITY_SCALE, uniformScale);
            ExecCmd(&cmd);
            break;
        }
        default:
        {
            LogErr(LOG, "Unknown ImGuizmo::OPERATION: %d", g_GuiStates.gizmoOperation);
        }
    }
}


// =================================================================================
// private API: commands executors
// =================================================================================

//---------------------------------------------------------
// execute a command according to its type for the currently selected entity
//---------------------------------------------------------
void EnttEditorController::ExecCmd(const ICommand* pCmd)
{
    // check input args
    if (!pFacade_)
    {
        LogErr(LOG, "your ptr to facade interface == nullptr (init it first!)");
        return;
    }
    if (!pCmd)
    {
        LogErr(LOG, "input ptr to command == nullptr");
        return;
    }
    if (!selectedEnttData_.IsSelectedAnyEntt())
    {
        LogErr(LOG, "you try to modify entity but you didn't select any yet! (cmd type: %d)", pCmd->type_);
        return;
    }


    const EntityID enttId = selectedEnttData_.id;

    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        case CHANGE_ENTITY_ROTATION:
        case CHANGE_ENTITY_SCALE:
        {
            transformController_.ExecCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            dirLightController_.ExecCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_POINT_LIGHT_ACTIVATION:
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.ExecCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.ExecCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_PARTICLES_COLOR:
        case CHANGE_PARTICLES_EXTERNAL_FORCE:
        case CHANGE_PARTICLES_SPAWN_NUM_PER_SECOND:
        case CHANGE_PARTICLES_LIFESPAN_MS:
        case CHANGE_PARTICLES_MASS:
        case CHANGE_PARTICLES_SIZE:
        case CHANGE_PARTICLES_FRICTION:   // air resistance
        {
            particlesController_.ExecCmd(pCmd, enttId);
            break;
        }
        case CHANGE_ENTITY_ANIMATION_SKELETON:
        case CHANGE_ENTITY_ANIMATION_TYPE:
        {
            pAnimationController_->ExecCmd(pFacade_, pCmd, enttId);
            break;
        }
        default:
        {
            const char* enttName = pFacade_->GetEnttNameById(enttId);
            LogErr(LOG, "unknown type of command (%d) for entt '%s' (%" PRIu32 ")", pCmd->type_, enttName, enttId);
            return;
        }
    }
}

//---------------------------------------------------------
// Desc:  undo/alt_undo an event from the events history for entity by ID
//---------------------------------------------------------
void EnttEditorController::UndoCmd(const ICommand* pCmd, const EntityID enttId)
{
    // check input args
    if (!pFacade_)
    {
        LogErr(LOG, "UI facade ptr == nullptr (init it first!)");
        return;
    }
    if (!pCmd)
    {
        LogErr(LOG, "command ptr == nullptr");
        return;
    }
    if (enttId == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "you try to modify invalid entity (id: 0, cmd type: %d)", pCmd->type_);
        return;
    }


    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        case CHANGE_ENTITY_ROTATION:
        case CHANGE_ENTITY_SCALE:
        {
            transformController_.UndoCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            dirLightController_.UndoCmd(pFacade_, pCmd, enttId);
            break;
        }
        //case CHANGE_POINT_LIGHT_ACTIVATION:
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.UndoCmd(pFacade_, pCmd, enttId);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.UndoCmd(pFacade_, pCmd, enttId);
            break;
        }
        default:
        {
            const char* enttName = pFacade_->GetEnttNameById(enttId);
            LogErr(LOG, "unknown type of undo command (%d) for entt '%s' (%" PRIu32 ")", pCmd->type_, enttName, enttId);
        }
    }
}

} // namespace UI
