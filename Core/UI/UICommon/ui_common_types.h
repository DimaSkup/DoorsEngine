//=================================================================================
// Filename:  ui_common_types.h
// Desc:      some types which are used in the engine's editor / game UI
//=================================================================================
#pragma once
#include <types.h>

//---------------------------------------------------------
// flags for turning on/off rendering of particular debug shapes
//---------------------------------------------------------
enum eRenderDbgShape
{
    R_DBG_SHAPES,           // turn on/off rendering of debug shapes at all

    R_DBG_SHAPES_LINE,
    R_DBG_SHAPES_CROSS,
    R_DBG_SHAPES_SPHERE,

    R_DBG_SHAPES_CIRCLE,
    R_DBG_SHAPES_AXIS,
    R_DBG_SHAPES_TRIANGLE,

    R_DBG_SHAPES_AABB,
    R_DBG_SHAPES_OBB,
    R_DBG_SHAPES_FRUSTUM,
    R_DBG_SHAPES_TERRAIN_AABB,
    R_DBG_SHAPES_MODELS_WIRE,
};

//---------------------------------------------------------
//---------------------------------------------------------
enum eEnttLightType
{
    ENTT_LIGHT_TYPE_DIRECTED,
    ENTT_LIGHT_TYPE_POINT,
    ENTT_LIGHT_TYPE_SPOT,
    NUM_LIGHT_TYPES
};

//---------------------------------------------------------
// list of ECS components
//---------------------------------------------------------
enum eEnttComponentType
{
    NameComponent,              // REQUIRED: attach some name for the entity
    TransformComponent,         // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
    MoveComponent,              // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,          // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,             // attach to entity a 2D/3D model by ID

    CameraComponent,            // attach to entity a camera
    MaterialComponent,          // set each mesh of entity (model) has some particular material
    TextureTransformComponent,  // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,             // attach to entity some type of light source (directed, point, spotlight, etc.)
    BoundingComponent,          // for using AABB, OBB, bounding spheres

    PlayerComponent,            // to hold First-Person-Shooter (FPS) player's data
    ParticlesComponent,
    InventoryComponent,         // inventory of entity
    AnimationComponent,         // for model skinning
    SpriteComponent,            // 2D sprite

    // NOT IMPLEMENTED YET
    AIComponent,
    HealthComponent,
    DamageComponent,
    EnemyComponent,
    ColliderComponent,

    PhysicsTypeComponent,
    CollisionComponent,

    NUM_COMPONENTS
};

///////////////////////////////////////////////////////////

enum eRndStatesGroup
{
    RND_STATES_RASTER,
    RND_STATES_BLEND,
    RND_STATES_DEPTH_STENCIL,

    NUM_GROUPS_RND_STATES
};

enum eRasterProp
{
    UI_RS_FILL,
    UI_RS_CULL,
    UI_RS_FRONT_CCW,
    UI_RS_DEPTH_BIAS,
    UI_RS_DEPTH_BIAS_CLAMP,
    UI_RS_SLOPE_SCALED_DEPTH_BIAS,
    UI_RS_DEPTH_CLIP_ENABLE,
    UI_RS_SCISSOR_ENABLE,
    UI_RS_MULTISAMPLE_ENABLE,
    UI_RS_ANTIALIASED_LINE_ENABLE,
};

enum eBlendProp
{
    UI_BLEND_OPERATION,
    UI_BLEND_FACTOR,

    //-----------------------

    UI_BLEND_RND_TARGET_WRITE_MASK,

    UI_BLEND_SRC_BLEND,
    UI_BLEND_DST_BLEND,
    UI_BLEND_OP,

    UI_BLEND_SRC_BLEND_ALPHA,
    UI_BLEND_DST_BLEND_ALPHA,
    UI_BLEND_OP_ALPHA,

    UI_BLEND_IS_ALPHA_TO_COVERAGE,
    UI_BLEND_IS_INDEPENDENT,
    UI_BLEND_IS_ENABLED,
};

enum eDepthStencilProp
{
    UI_DSS_STENCIL_OP,
    UI_DSS_COMPARISON_FUNC,

    //-----------------------

    UI_DSS_DEPTH_ENABLED,
    UI_DSS_DEPTH_WRITE_MASK,
    UI_DSS_DEPTH_FUNC,

    UI_DSS_STENCIL_ENABLED,
    UI_DSS_STENCIL_READ_MASK,
    UI_DSS_STENCIL_WRITE_MASK,

    UI_DSS_FRONT_FACE_STENCIL_FAIL_OP,
    UI_DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP,
    UI_DSS_FRONT_FACE_STENCIL_PASS_OP,
    UI_DSS_FRONT_FACE_STENCIL_FUNC,

    UI_DSS_BACK_FACE_STENCIL_FAIL_OP,
    UI_DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP,
    UI_DSS_BACK_FACE_STENCIL_PASS_OP,
    UI_DSS_BACK_FACE_STENCIL_FUNC,
};

//---------------------------------------------------------
// model preview parameters (for model editor, or model screenshot tool)
//---------------------------------------------------------
enum eModelPreviewParams
{
    MODEL_ID,                    // which model to render
    MODEL_POS_X,
    MODEL_POS_Y,
    MODEL_POS_Z,
    MODEL_ROT_X,
    MODEL_ROT_Y,
    MODEL_ROT_Z,
    MODEL_SCALE,                 // uniform scale

    CAMERA_POS_X,
    CAMERA_POS_Y,
    CAMERA_POS_Z,
    CAMERA_ROT_X,
    CAMERA_ROT_Y,
    CAMERA_ROT_Z,

    FRAME_BUF_WIDTH,
    FRAME_BUF_HEIGHT,

    ORTHO_MATRIX_VIEW_HEIGHT,
    USE_ORTHO_MATRIX,            // flag: use ortho matrix or not

    BG_COLOR_R,
    BG_COLOR_G,
    BG_COLOR_B,

    NUM_MODEL_PREVIEW_PARAMS
};

//---------------------------------------------------------
// data container for setup a custom render state or switch to some predefined state 
//---------------------------------------------------------
struct RenderStateSetup
{
    MaterialID matId;
    eRndStatesGroup rndState;
    int rndStateId = 0;
    char propName[32];
    char propValue[32];
};
