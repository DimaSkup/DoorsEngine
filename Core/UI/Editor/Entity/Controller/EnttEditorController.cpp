// =================================================================================
// Filename:       EnttEditorController.cpp
// Created:        01.01.25
// =================================================================================
#include "EnttEditorController.h"

#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <CoreCommon/MathHelper.h>
#include <UICommon/EditorCommands.h>
#include <UICommon/EventsHistory.h>

#include <imgui.h>
#include <ImGuizmo.h>
#include <format>


using namespace Core;

namespace UI
{

EnttEditorController::EnttEditorController(StatesGUI* pStatesGUI) :
    viewEnttTransform_(this),
    viewEnttLight_(this),
    pStatesGUI_(pStatesGUI)
{
    Assert::NotNullptr(pStatesGUI, "input ptr to the GUI states container == nullptr");
}


// =================================================================================
// public API
// =================================================================================

void EnttEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
    pFacade_ = pFacade;

    transformController_.Initialize(pFacade);
    dirLightController_.Initialize(pFacade);
    pointLightController_.Initialize(pFacade);
    spotLightController_.Initialize(pFacade);
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
        pFacade_->GetEnttNameByID(enttID, selectedEnttData_.name);

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
                            Log::Error(std::format("unknown light type ({}) for entity (id: {}, name: {})", enttID, lightType, selectedEnttData_.name));
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
void EnttEditorController::UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world)
{
    // when we used a gizmo to modify a world of the selected entity
    // we call this method to actual update of the world properties

    int transformType = pStatesGUI_->gizmoOperation_;
    DirectX::XMVECTOR scale, rotQuat, translation;
    XMMatrixDecompose(&scale, &rotQuat, &translation, world);

    // execute transformation of the selected entity according to the transformation type
    switch (transformType)
    {
        case ImGuizmo::OPERATION::TRANSLATE:
        {
            CmdChangeVec3 cmd(CHANGE_ENTITY_POSITION, translation);
            Execute(&cmd);
            break;
        }
        case ImGuizmo::OPERATION::ROTATE:
        {
            CmdChangeVec4 cmd(CHANGE_ENTITY_DIRECTION, Vec4(rotQuat));
            Execute(&cmd);
            break;
        }
        case ImGuizmo::OPERATION::SCALE:
        {
            const float uniformScale = DirectX::XMVectorGetX(scale);
            CmdChangeFloat cmd(CHANGE_ENTITY_SCALE, uniformScale);
            Execute(&cmd);
            break;
        }
        default:
        {
            Log::Error("Unknown ImGuizmo::OPERATION: " + std::to_string(transformType));
        }
    }
}


// =================================================================================
// private API: commands executors
// =================================================================================

void EnttEditorController::Execute(const ICommand* pCmd)
{
    // execute a command according to its type for the currently selected entity

    if ((pCmd == nullptr) || (pFacade_ == nullptr) || (!selectedEnttData_.IsSelectedAnyEntt()))
    {
        Log::Error("can't execute a command of type: " + std::to_string(pCmd->type_));
        return;
    }

    const EntityID enttID = selectedEnttData_.id;

    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        case CHANGE_ENTITY_DIRECTION:
        case CHANGE_ENTITY_SCALE:
        {
#if 0
            const std::string enttIdStr = std::to_string(enttID);
            std::string enttName;
            pFacade_->GetEnttNameByID(enttID, enttName);
            Core::Log::Debug("TRANSFORM ENTITY: (id: " + enttIdStr + ", name: " + enttName + ")");
#endif

            transformController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_DIR_LIGHT_AMBIENT:
        case CHANGE_DIR_LIGHT_DIFFUSE:
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            dirLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_POINT_LIGHT_AMBIENT:
        case CHANGE_POINT_LIGHT_DIFFUSE:
        case CHANGE_POINT_LIGHT_SPECULAR:
        case CHANGE_POINT_LIGHT_RANGE:
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            pointLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        case CHANGE_SPOT_LIGHT_AMBIENT:
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        case CHANGE_SPOT_LIGHT_SPECULAR:
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            spotLightController_.ExecuteCommand(pCmd, enttID);
            break;
        }
        default:
        {
            Log::Error("unknown type of command: " + std::to_string(pCmd->type_));
            return;
        }
    }
}

///////////////////////////////////////////////////////////

void EnttEditorController::Undo(const ICommand* pCmd, const EntityID enttID)
{
    // undo/alt_undo an event from the events history for entity by ID

    if ((pCmd == nullptr) || (pFacade_ == nullptr) || (enttID == 0))
    {
        Log::Error("can't execute a command of type: " + std::to_string(pCmd->type_));
        return;
    }

    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        case CHANGE_ENTITY_DIRECTION:
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

    }
}

} // namespace UI
