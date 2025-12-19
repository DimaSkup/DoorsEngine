// =================================================================================
// Filename:       EnttEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttEditorController.h"

#include <UICommon/gui_states.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/events_history.h>

#include <imgui.h>
#include <ImGuizmo.h>

#pragma warning (disable : 4996)


namespace UI
{

EnttEditorController::EnttEditorController() :
    viewEnttTransform_(this),
    viewEnttLight_(this),
    viewEnttParticles_(this)
{
}


// =================================================================================
// public API
// =================================================================================

void EnttEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    CAssert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
    pFacade_ = pFacade;

    transformController_.Initialize(pFacade);
    dirLightController_.Initialize(pFacade);
    pointLightController_.Initialize(pFacade);
    spotLightController_.Initialize(pFacade);
    particlesController_.Initialize(pFacade);
}

///////////////////////////////////////////////////////////

void EnttEditorController::SetSelectedEntt(const EntityID enttID)
{
    // set that we have selected some entity and load its data

    // if we deselected the chosen entt so we won't render any control panel
    // until we won't select any other
    if (enttID == 0)
    {
        selectedEnttData_.id   = 0;
        selectedEnttData_.name = "";
        return;
    }
    // we have chosen some entt
    else
    {
        // get selected entity ID and name
        selectedEnttData_.id = enttID;
        pFacade_->GetEnttNameById(enttID, selectedEnttData_.name);

        // get names of added components
        pFacade_->GetEntityAddedComponentsTypes(enttID, selectedEnttData_.componentsTypes);


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
                    transformController_.LoadEnttData(enttID);
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
                    particlesController_.LoadEnttData(enttID);
                    break;
                }
                case LightComponent:
                {
                    int lightType;

                    if (pFacade_->GetEnttLightType(enttID, lightType))
                    {
                        if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_DIRECTED)
                            dirLightController_.LoadEnttData(enttID);

                        else if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_POINT)
                            pointLightController_.LoadEnttData(enttID);

                        else if (lightType == eEnttLightType::ENTT_LIGHT_TYPE_SPOT)
                            spotLightController_.LoadEnttData(enttID);

                        else
                        {
                            const char* enttName = selectedEnttData_.name.c_str();
                            LogErr(LOG, "unknown light type (%d) for entity (id: %" PRIu32 ", name: % s)", lightType, enttID, enttName);
                            return;
                        }

                        // store the subtype of the light for this entity
                        selectedEnttData_.lightType = eEnttLightType(lightType);
                    }
                    
                    break;
                }
                case RenderStatesComponent:
                {
                    break;
                }
                case BoundingComponent:
                {
                    break;
                }
            } // switch
        } // for
    }
}

///////////////////////////////////////////////////////////

void RenderEntityIdAndName(const EntityID id, const std::string& enttName)
{
    ImGui::Text("Entity ID:   %d", id);
    ImGui::Text("Entity name: %s", enttName.c_str());
}

///////////////////////////////////////////////////////////

void EnttEditorController::Render()
{
    //  render a panel for controlling properties of the chosen entity

    const EntityID enttID = selectedEnttData_.id;

    // we have no entity as selected
    if (enttID == 0)
        return;

    // we want the next editor panel to be visible
    static bool isOpen = true;
    ImGui::SetNextItemOpen(isOpen);


    // render a list of components which are added to the entity (for editing these components)
    ImGui::Separator();
    RenderEntityIdAndName(enttID, selectedEnttData_.name);


    // render view (editor control fields) for each added component
    for (const eEnttComponentType type : selectedEnttData_.componentsTypes)
    {
        switch (type)
        {
            case NameComponent:
            {
                if (ImGui::CollapsingHeader("Name", ImGuiTreeNodeFlags_SpanFullWidth))
                    ImGui::Text("Entity name: %s", selectedEnttData_.name.c_str());
                break;
            }
            case TransformComponent:
            {
                if (ImGui::CollapsingHeader("Transformation", ImGuiTreeNodeFlags_SpanFullWidth))
                    viewEnttTransform_.Render(transformController_.GetModel());
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
                if (ImGui::CollapsingHeader("Particle Emitter", ImGuiTreeNodeFlags_SpanFullWidth))
                    viewEnttParticles_.Render(particlesController_.GetModel());

                break;
            }
            case LightComponent:
            {
                // according to the light type which is added to entity we render responsible view (set of editor fields)
                if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_DIRECTED)
                {
                    if (ImGui::CollapsingHeader("Directed light properties", ImGuiTreeNodeFlags_SpanFullWidth))
                        viewEnttLight_.Render(dirLightController_.GetModel());
                }
                else if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_POINT)
                {
                    if (ImGui::CollapsingHeader("Point Light Properties", ImGuiTreeNodeFlags_SpanFullWidth))
                        viewEnttLight_.Render(pointLightController_.GetModel());
                }
                else if (selectedEnttData_.lightType == eEnttLightType::ENTT_LIGHT_TYPE_SPOT)
                {
                    if (ImGui::CollapsingHeader("Spotlight Properties", ImGuiTreeNodeFlags_SpanFullWidth))
                        viewEnttLight_.Render(spotLightController_.GetModel());
                }
                break;
            }
            case RenderStatesComponent:
            {
                break;
            }
            case BoundingComponent:
            {
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
            transformController_.ExecuteCommand(pCmd, enttId);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            dirLightController_.ExecuteCommand(pCmd, enttId);
            break;
        }
        case CHANGE_POINT_LIGHT_ACTIVATION:
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.ExecuteCommand(pCmd, enttId);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.ExecuteCommand(pCmd, enttId);
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
        default:
        {
            sprintf(g_String, "unknown type of command: %d", pCmd->type_);
            LogErr(g_String);
            return;
        }
    }
}

//---------------------------------------------------------
// Desc:  undo/alt_undo an event from the events history for entity by ID
//---------------------------------------------------------
void EnttEditorController::UndoCmd(const ICommand* pCmd, const EntityID enttID)
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
    if (enttID == INVALID_ENTITY_ID)
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
            transformController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            dirLightController_.UndoCommand(pCmd, enttID);
            break;
        }
        //case CHANGE_POINT_LIGHT_ACTIVATION:
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.UndoCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.UndoCommand(pCmd, enttID);
            break;
        }
        default:
        {
            LogErr(LOG, "unknown type of undo command (%d)", pCmd->type_);
        }
    }
}

} // namespace UI
