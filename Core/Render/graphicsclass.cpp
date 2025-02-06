// =================================================================================
// Filename: graphicsclass.cpp
// Created:  14.10.22
// =================================================================================
#include "graphicsclass.h"


#include "../Common/Assert.h"
#include "../Common/MathHelper.h"
#include "../Common/Utils.h"

#include "../Input/inputcodes.h"
#include "RenderDataPreparator.h"

#include <ImGuizmo.h>
#include <random>


GraphicsClass::GraphicsClass() : prep_(render_, entityMgr_)
{
	Log::Debug();
}

// the class destructor
GraphicsClass::~GraphicsClass() 
{
	Log::Debug("start of destroying");
	Shutdown();
	Log::Debug("is destroyed");
}


// =================================================================================
//
//                             PUBLIC METHODS
//
// =================================================================================


bool GraphicsClass::Initialize(
	HWND hwnd, 
	SystemState& systemState,
	const Settings& settings)
{
	// Initializes all the main parts of graphics rendering module

	try
	{
		InitializeGraphics initGraphics;
		bool result = false;

		Log::Print();
		Log::Print("------------------------------------------------------------", ConsoleColor::YELLOW);
		Log::Print("              INITIALIZATION: GRAPHICS SYSTEM               ", ConsoleColor::YELLOW);
		Log::Print("------------------------------------------------------------", ConsoleColor::YELLOW);

		pSysState_ = &systemState;

		result = initGraphics.InitializeDirectX(d3d_, hwnd, settings);
		Assert::True(result, "can't initialize D3DClass");

		// after initialization of the DirectX we can use pointers to the device and device context
		d3d_.GetDeviceAndDeviceContext(pDevice_, pDeviceContext_);


		// init all the cameras on the scene
		result = initGraphics.InitializeCameras(
			d3d_,
			gameCamera_,
			editorCamera_,
			baseViewMatrix_,           // init the base view matrix which is used for 2D rendering
			entityMgr_,
			settings);
		Assert::True(result, "can't initialize cameras / view matrices");

		// choose the editor camera as current by default
		pCurrCamera_ = &editorCamera_;


		// initializer the textures container
		texMgr_.Initialize(pDevice_);

#if 0
		// create a texture which can be used as a render target
		FrameBufferSpecification fbSpec;

		fbSpec.width = 480;
		fbSpec.height = 320;
		fbSpec.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		fbSpec.screenNear = d3d_.GetScreenNear();
		fbSpec.screenDepth = d3d_.GetScreenDepth();

		result = frameBuffer_.Initialize(pDevice_, fbSpec);
		Assert::True(result, "can't initialize the render to texture object");
#endif

		// initialize scene objects: cubes, spheres, trees, etc.
		result = initGraphics.InitializeScene(settings, d3d_, entityMgr_);
		Assert::True(result, "can't initialize the scene elements (models, etc.)");

	
		// create frustums for frustum culling
		frustums_.push_back(DirectX::BoundingFrustum());

		// setup loggers of the modules to make possible writing into the log file
		entityMgr_.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());
		render_.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());

		// matrix for 2D rendering
		WVO_ = worldMatrix_ * baseViewMatrix_ * d3d_.GetOrthoMatrix();

	
		// setup render initial params
		Render::InitParams renderParams;

		renderParams.worldViewOrtho = DirectX::XMMatrixTranspose(WVO_); 

		// zaporizha sky box horizon (darker by 0.1f)
		renderParams.fogColor = 
		{ 
			settings.GetFloat("FOG_RED"),
			settings.GetFloat("FOG_GREEN"),
			settings.GetFloat("FOG_BLUE"),
		};
		renderParams.fogStart = settings.GetFloat("FOG_START");
		renderParams.fogRange = settings.GetFloat("FOG_RANGE");


		result = render_.Initialize(
			pDevice_,
			pDeviceContext_,
			renderParams);
		Assert::True(result, "can't init the render module");


		render_.SetSkyGradient(
			pDeviceContext_,
			modelStorage_.GetSky().GetColorCenter(),
			modelStorage_.GetSky().GetColorApex());
	}
	catch (EngineException & e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the graphics class");
		this->Shutdown();
		return false;
	}

	Log::Print(" is successfully initialized");
	return true;
}

///////////////////////////////////////////////////////////

void GraphicsClass::Shutdown()
{
	// Shutdowns all the graphics rendering parts, releases the memory
	Log::Debug();
	d3d_.Shutdown();
}

///////////////////////////////////////////////////////////

void GraphicsClass::Update(
	SystemState& sysState,
	const float deltaTime,
	const float totalGameTime)
{
	// update all the graphics related stuff for this frame


	pCurrCamera_->UpdateViewMatrix();

	// DIRTY HACK: update the camera height according to the terrain height function
	DirectX::XMFLOAT3 prevCamPos = pCurrCamera_->GetPosition();

	const float strideByY = 0.01f * (prevCamPos.z * sinf(0.1f * prevCamPos.x) +
		                   prevCamPos.x * cosf(0.1f * prevCamPos.z)) + 1.5f;

	//pCurrCamera_->SetStrideByY(strideByY);

	// ---------------------------------------------

          

	const DirectX::XMMATRIX& viewMatrix = pCurrCamera_->View();  // update the view matrix for this frame
	const DirectX::XMMATRIX& projMatrix = pCurrCamera_->Proj(); // update the projection matrix
	viewProj_ = viewMatrix * projMatrix;

	// update the cameras states
	sysState.cameraPos = pCurrCamera_->GetPosition();
	//sysState.cameraDir = pCurrCamera_->GetDirection();
	sysState.cameraView = viewMatrix;
	sysState.cameraProj = projMatrix;

	const XMFLOAT3& cameraPos = sysState.cameraPos;
	const XMFLOAT3& cameraDir = sysState.cameraDir;

	
	// update the entities and related data
	entityMgr_.Update(totalGameTime, deltaTime);
	entityMgr_.lightSystem_.UpdateSpotLights(cameraPos, cameraDir);
	
	// build the frustum from the projection matrix in view space.
	DirectX::BoundingFrustum::CreateFromMatrix(frustums_[0], projMatrix);

	// perform frustum culling on all of our currently loaded entities
	ComputeFrustumCulling(sysState);

	// Update shaders common data for this frame
	UpdateShadersDataPerFrame();

	// prepare all the visible entities data for rendering
	const std::vector<EntityID>& visibleEntts = entityMgr_.renderSystem_.GetAllVisibleEntts();

	// separate entts into opaque, entts with alpha clipping, blended, etc.
	entityMgr_.renderStatesSystem_.SeparateEnttsByRenderStates(visibleEntts, rsDataToRender_);


	//pSysState_->visibleObjectsCount = 0;
	pSysState_->visibleVerticesCount = 0;

	// prepare data for each entts set
	PrepBasicInstancesForRender(rsDataToRender_.enttsDefault_.ids_);
	PrepAlphaClippedInstancesForRender(rsDataToRender_.enttsAlphaClipping_.ids_);
	PrepBlendedInstancesForRender(rsDataToRender_.enttsBlended_.ids_);
}

///////////////////////////////////////////////////////////

void GraphicsClass::ComputeFrustumCulling(SystemState& sysState)
{
	// reset render counters (do it before frustum culling)
	sysState.visibleObjectsCount = 0;

	ECS::EntityMgr& mgr = entityMgr_;
	mgr.renderSystem_.ClearVisibleEntts();

	const std::vector<EntityID>& enttsRenderable = mgr.renderSystem_.GetAllEnttsIDs();

	const size numRenderEntts = std::ssize(enttsRenderable);
	size numVisEntts = 0;                                     // the number of currently visible entts
	std::vector<EntityID> visibleEntts;

	std::vector<size> numBoxesPerEntt;
	std::vector<DirectX::BoundingOrientedBox> OBBs;       // bounding box of each mesh of each renderable entt
	std::vector<XMMATRIX> invWorlds(numRenderEntts);      // inverse world matrix of each renderable entt
	std::vector<XMMATRIX> enttsLocal(numRenderEntts);     // local space of each renderable entt
	std::vector<index> idxsToVisEntts(numRenderEntts);

	// get inverse world matrix of each renderable entt
	mgr.transformSystem_.GetInverseWorldMatricesOfEntts(enttsRenderable, invWorlds);

	// compute local space matrices for each renderable entt
	for (index i = 0; i < numRenderEntts; ++i)
		enttsLocal[i] = DirectX::XMMatrixMultiply(pCurrCamera_->InverseView(), invWorlds[i]);

	// clear some arrs since we don't need already
	invWorlds.clear();

	// get arr of AABB / bounding spheres for each renderable entt
	mgr.boundingSystem_.GetOBBs(enttsRenderable, numBoxesPerEntt, OBBs);

	// go through each entity and define if it is visible
	for (index idx = 0, obbIdx = 0; idx < numRenderEntts; ++idx)
	{
		// decompose the matrix into its individual parts
		XMVECTOR scale;
		XMVECTOR dirQuat;
		XMVECTOR translation;
		XMMatrixDecompose(&scale, &dirQuat, &translation, enttsLocal[idx]);

		// transform the camera frustum from view space to the object's local space
		DirectX::BoundingFrustum LSpaceFrustum; 
		frustums_[0].Transform(LSpaceFrustum, DirectX::XMVectorGetX(scale), dirQuat, translation);

		// if we have any mesh OBB of the entt in view -- we set this entt as visible
		for (index i = 0; i < numBoxesPerEntt[idx]; ++i)
		{
			if (LSpaceFrustum.Intersects(OBBs[obbIdx + i]))
			{
				idxsToVisEntts[numVisEntts++] = idx;
				i = numBoxesPerEntt[idx];              // go out from the for-loop
			}
		}

		obbIdx += numBoxesPerEntt[idx];
	}

	// ------------------------------------------

	// store ids of visible entts
	visibleEntts.resize(numVisEntts);

	for (index i = 0; i < numVisEntts; ++i)
		visibleEntts[i] = enttsRenderable[idxsToVisEntts[i]];

	mgr.renderSystem_.SetVisibleEntts(visibleEntts);
	sysState.visibleObjectsCount = (u32)numVisEntts;
}


///////////////////////////////////////////////////////////

void GraphicsClass::ChangeModelFillMode()
{
	// toggling on / toggling off the fill mode for the models

	using enum RenderStates::STATES;

	isWireframeMode_ = !isWireframeMode_;
	RenderStates::STATES fillParam = (isWireframeMode_) ? FILL_WIREFRAME : FILL_SOLID;

	d3d_.SetRS(fillParam);
};

///////////////////////////////////////////////////////////

void GraphicsClass::ChangeCullMode()
{
	// toggling on and toggling off the cull mode for the models

	using enum RenderStates::STATES;

	isCullBackMode_ = !isCullBackMode_;
	d3d_.SetRS((isCullBackMode_) ? CULL_BACK : CULL_FRONT);
}

///////////////////////////////////////////////////////////

void GraphicsClass::SwitchGameMode(bool enableGameMode)
{
	// switch btw the game and editor modes and do some other related changes

	isGameMode_ = enableGameMode;
	pCurrCamera_ = (enableGameMode) ? &gameCamera_ : &editorCamera_;
}

///////////////////////////////////////////////////////////

// memory allocation and releasing
void* GraphicsClass::operator new(size_t i)
{
	if (void* ptr = _aligned_malloc(i, 16))
		return ptr;

	Log::Error("can't allocate memory for this object");
	throw std::bad_alloc{};
}

///////////////////////////////////////////////////////////

void GraphicsClass::operator delete(void* ptr)
{
	_aligned_free(ptr);
}




// =================================================================================
//                               private helpers
// =================================================================================

void GraphicsClass::UpdateShadersDataPerFrame()
{
	// Update shaders common data for this frame: 
	// viewProj matrix, camera position, light sources data, etc.

	Render::PerFrameData& perFrameData = render_.perFrameData_;

	perFrameData.viewProj = DirectX::XMMatrixTranspose(viewProj_);
	perFrameData.cameraPos = pCurrCamera_->GetPosition();

	SetupLightsForFrame(entityMgr_.lightSystem_, perFrameData);

	// update lighting data, camera pos, etc. for this frame
	render_.UpdatePerFrame(pDeviceContext_, perFrameData);
}

///////////////////////////////////////////////////////////

void GraphicsClass::ClearRenderingDataBeforeFrame()
{
	// clear rendering data from the previous frame / instances set

	render_.dataStorage_.Clear();
	rsDataToRender_.Clear();
}

///////////////////////////////////////////////////////////

void GraphicsClass::Render3D()
{
	try
	{
		pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		RenderEnttsDefault();
		RenderEnttsAlphaClipCullNone();
		RenderEnttsBlended();

		pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		RenderBoundingLineBoxes();

		RenderSkyDome();
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		Log::Error("there is no such a key to data");
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("can't render 3D entts onto the scene");
	}
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepBasicInstancesForRender(
	const std::vector<EntityID>& enttsIds)           

{
	// prepare rendering data of entts which have default render states
	
	if (enttsIds.empty()) return;

	Render::RenderDataStorage& storage = render_.dataStorage_;

	prep_.PrepareEnttsDataForRendering(
		enttsIds,
		storage.modelInstBuffer_,
		storage.modelInstances_);

	// compute how many vertices will we render
	for (const Render::Instance& inst : storage.modelInstances_)
		pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepAlphaClippedInstancesForRender(
	const std::vector<EntityID>& enttsIds)              
{
	// prepare rendering data of entts which have alpha clip + cull none

	if (enttsIds.empty()) return;

	Render::RenderDataStorage& storage = render_.dataStorage_;

	prep_.PrepareEnttsDataForRendering(
		enttsIds,
		storage.alphaClippedModelInstBuffer_,
		storage.alphaClippedModelInstances_);

	// compute how many vertices will we render
	for (const Render::Instance& inst : storage.alphaClippedModelInstances_)
		pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void GraphicsClass::PrepBlendedInstancesForRender(
	const std::vector<EntityID>& enttsIds)
{
	// prepare rendering data of entts which have alpha clip + cull none

	if (enttsIds.empty()) return;

	prep_.PrepareEnttsDataForRendering(
		enttsIds,
		render_.dataStorage_.blendedModelInstBuffer_,
		render_.dataStorage_.blendedModelInstances_);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderModel(BasicModel& model, const DirectX::XMMATRIX& world)
{
	// for specific purposes:
	// just render a single asset/model at the center of the world

	Render::Instance instance;
	Render::InstBuffData instanceBuffData;
	int numSubsets = model.GetNumSubsets();

	prep_.PrepareInstanceFromModel(model, instance);

	// each subset (mesh) will have its own world/texTransform/material
	instanceBuffData.Resize(numSubsets);

	for (int i = 0; i < numSubsets; ++i)
	{
		instanceBuffData.worlds_[i]        = world;
		instanceBuffData.texTransforms_[i] = DirectX::XMMatrixIdentity();
		instanceBuffData.materials_[i]     = instance.materials[i];
	}

	render_.UpdateInstancedBuffer(pDeviceContext_, instanceBuffData);

	// render prepared instances using shaders
	render_.RenderInstances(
		pDeviceContext_, 
		Render::ShaderTypes::LIGHT, 
		&instance, 1);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsDefault()
{
	const Render::RenderDataStorage& storage = render_.dataStorage_;

	// check if we have any instances to render
	if (storage.modelInstances_.empty())
		return;

	// setup states before rendering
	d3d_.GetRenderStates().ResetRS(pDeviceContext_);
	d3d_.GetRenderStates().ResetBS(pDeviceContext_);

	UpdateInstanceBuffAndRenderInstances(
		pDeviceContext_,
		Render::ShaderTypes::LIGHT,
		storage.modelInstBuffer_,
		storage.modelInstances_);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsAlphaClipCullNone()
{
	// render all the visible entts with cull_none and alpha clipping;
	// (entts for instance: wire fence, bushes, leaves, etc.)

	const Render::RenderDataStorage& storage = render_.dataStorage_;

	// check if we have any instances to render
	if (storage.alphaClippedModelInstances_.empty())
		return;


	// setup rendering pipeline
	RenderStates& renderStates = d3d_.GetRenderStates();
	renderStates.SetRS(pDeviceContext_, { RenderStates::STATES::CULL_NONE });
	render_.SwitchAlphaClipping(pDeviceContext_, true);

	// render
	UpdateInstanceBuffAndRenderInstances(
		pDeviceContext_,
		Render::ShaderTypes::LIGHT,
		storage.alphaClippedModelInstBuffer_,
		storage.alphaClippedModelInstances_);

	// reset rendering pipeline
	renderStates.ResetRS(pDeviceContext_);
	render_.SwitchAlphaClipping(pDeviceContext_, false);
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderEnttsBlended()
{

	// render all the visible blended entts

	const Render::RenderDataStorage& storage = render_.dataStorage_;

	// check if we have any instances to render
	if (storage.blendedModelInstances_.empty())
		return;


	const ECS::EnttsBlended& blendData = rsDataToRender_.enttsBlended_;
	const std::vector<ECS::RSTypes>& blendStates = blendData.states_;
	const std::vector<u32>& instPerBS = blendData.instanceCountPerBS_;
	const Render::InstBuffData& instBuffer = storage.blendedModelInstBuffer_;

	// push data into the instanced buffer
	render_.UpdateInstancedBuffer(pDeviceContext_, instBuffer);

	int instanceOffset = 0;

	// go through each blending state, turn it on and render blended entts with this state
	for (int bsIdx = 0; bsIdx < (int)std::ssize(blendStates); ++bsIdx)
	{
		d3d_.TurnOnBlending(RenderStates::STATES(blendStates[bsIdx]));

		for (u32 instCount = 0; instCount < instPerBS[bsIdx]; ++instCount)
		{
			const Render::Instance* instance = &(storage.blendedModelInstances_[instanceOffset]);
			render_.RenderInstances(pDeviceContext_, Render::ShaderTypes::LIGHT, instance, 1);
			++instanceOffset;
		}
		
		//storage.blendedModelInstances_
	}

	// turn off blending after rendering of all the visible blended entities
	//d3d_.TurnOffBlending();
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderBoundingLineBoxes()
{
	// if we don't want to show bound boxes just go out
	if (aabbShowMode_ == NONE)
		return;
	
	const std::vector<EntityID>& visEntts = entityMgr_.renderSystem_.GetAllVisibleEntts();

	// check if we have any visible entts
	if (visEntts.empty())
		return;                          

	Render::RenderDataStorage& storage       = render_.dataStorage_;
	Render::InstBuffData& instancesBuffer    = storage.boundingLineBoxBuffer_;
	std::vector<Render::Instance>& instances = storage.boundingLineBoxInstances_;
	

	// prepare the line box instance
	const int numInstances = 1;                // how many different line box models we have
	const ModelID lineBoxId = 1;
	BasicModel& lineBox = modelStorage_.GetModelByID(lineBoxId);
	
	
	// we will use only one type of model -- line box
	instances.resize(1);                         
	Render::Instance& instance = instances[0];
	prep_.PrepareInstanceData(lineBox, instance, *TextureMgr::Get());


	// choose the bounding box show mode
	// (1: box around the while model, 2: box around each model's mesh)
	if (aabbShowMode_ == MODEL)
		prep_.PrepareEnttsBoundingLineBox(visEntts, instance, instancesBuffer);
	else if (aabbShowMode_ == MESH)
		prep_.PrepareEnttsMeshesBoundingLineBox(visEntts, instance, instancesBuffer);


	pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// render
	render_.UpdateInstancedBuffer(pDeviceContext_, instancesBuffer);
	render_.RenderBoundingLineBoxes(pDeviceContext_, &instance, numInstances);

	/*
	UpdateInstanceBuffAndRenderInstances(
		pDeviceContext_,
		Render::ShaderTypes::COLOR,
		storage.boundingLineBoxBuffer_,
		storage.boundingLineBoxInstances_);
		*/
}

///////////////////////////////////////////////////////////

void GraphicsClass::RenderSkyDome()
{
	const EntityID skyEnttID  = entityMgr_.nameSystem_.GetIdByName("sky");

	// if there is no sky entity
	if (skyEnttID == INVALID_ENTITY_ID)
		return;

	const XMMATRIX skyWorld   = entityMgr_.transformSystem_.GetWorldMatrixOfEntt(skyEnttID);
	const SkyModel& sky       = modelStorage_.GetSky();
	Render::SkyInstance instance;


	//
	// prepare the sky instance
	//
	const TexID* skyTexIDs = sky.GetTexIDs();
	const int skyTexMaxNum = sky.GetMaxTexturesNum();

	// get shader resource views for the sky
	texMgr_.GetSRVsByTexIDs(skyTexIDs, skyTexMaxNum, instance.texSRVs);

	instance.vertexStride = sky.GetVertexStride();
	instance.pVB          = sky.GetVertexBuffer();
	instance.pIB          = sky.GetIndexBuffer();
	instance.indexCount   = sky.GetNumIndices();

	instance.colorCenter  = sky.GetColorCenter();
	instance.colorApex    = sky.GetColorApex();


	// setup rendering pipeline before rendering of the sky dome
	using enum RenderStates::STATES;
	RenderStates& renderStates = d3d_.GetRenderStates();
	renderStates.ResetRS(pDeviceContext_);
	renderStates.SetRS(pDeviceContext_, { RenderStates::STATES::CULL_NONE });
	renderStates.ResetBS(pDeviceContext_);
	renderStates.SetDSS(pDeviceContext_, SKY_DOME, 1);

	// compute a worldViewProj matrix for the sky instance
	const DirectX::XMFLOAT3& eyePos = pCurrCamera_->GetPosition();
	const XMMATRIX camOffset        = DirectX::XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	const XMMATRIX worldViewProj    = DirectX::XMMatrixTranspose(camOffset * skyWorld * viewProj_);

	render_.RenderSkyDome(pDeviceContext_, instance, worldViewProj);
}

///////////////////////////////////////////////////////////

void GraphicsClass::UpdateInstanceBuffAndRenderInstances(
	ID3D11DeviceContext* pDeviceContext,
	const Render::ShaderTypes type,
	const Render::InstBuffData& instanceBuffData,
	const std::vector<Render::Instance>& instances)
{

	render_.UpdateInstancedBuffer(pDeviceContext_, instanceBuffData);

	// render prepared instances using shaders
	render_.RenderInstances(pDeviceContext_, type, instances.data(), (int)instances.size());
}

///////////////////////////////////////////////////////////

void GraphicsClass::SetupLightsForFrame(
	const ECS::LightSystem& lightSys,
	Render::PerFrameData& outData)
{
	// convert light source data from the ECS into Render format
	// (they are the same so we simply need to copy data)

	const ECS::DirLights&   dirLights   = lightSys.GetDirLights();
	const ECS::PointLights& pointLights = lightSys.GetPointLights();
	const ECS::SpotLights&  spotLights  = lightSys.GetSpotLights();

	const int numDirLights   = static_cast<int>(dirLights.GetCount());
	const int numPointLights = static_cast<int>(pointLights.GetCount());
	const int numSpotLights  = static_cast<int>(spotLights.GetCount());

	outData.ResizeLightData(numDirLights, numPointLights, numSpotLights);

	// --------------------------------

	size dirLightSize   = sizeof(ECS::DirLight);
	size pointLightSize = sizeof(ECS::PointLight);
	size spotLightSize  = sizeof(ECS::SpotLight);

	// copy data of directional/point/spot light sources
	for (int idx = 0; idx < numDirLights; ++idx)
		memcpy(&outData.dirLights[idx], &dirLights.data_[idx], dirLightSize);

	for (int idx = 0; idx < numPointLights; ++idx)
		memcpy(&outData.pointLights[idx], &pointLights.data_[idx], pointLightSize);

	for (int idx = 0; idx < numSpotLights; ++idx)
		memcpy(&outData.spotLights[idx], &spotLights.data_[idx], spotLightSize);
}

///////////////////////////////////////////////////////////

int GraphicsClass::TestEnttSelection(const int sx, const int sy)
{
	// check if we have any entity by input screen coords;
	// 
	// in:   screen pixel coords
	// out:  0  - there is no entity, we didn't select any
	//       ID - we selected some entity so return its ID

	using namespace DirectX;

	const XMMATRIX P = pCurrCamera_->Proj();
	const XMMATRIX& invView = pCurrCamera_->InverseView();

	const float xndc = (+2.0f * sx / d3d_.GetWindowWidth() - 1.0f);
	const float yndc = (-2.0f * sy / d3d_.GetWindowHeight() + 1.0f);

	// compute picking ray in view space
	const float vx = xndc / P.r[0].m128_f32[0];
	const float vy = yndc / P.r[1].m128_f32[1];

	// ray definition in view space
	XMVECTOR rayOrigin_ = { 0,0,0,1 };
	XMVECTOR rayDir_    = { vx, vy, 1, 0 };

	// assume we have not picked anything yet, 
	// so init the ID to 0 and triangle idx to -1
	uint32_t selectedEnttID = 0;
	int selectedTriangleIdx = -1;
	float tmin = MathHelper::Infinity;  // the distance along the ray where the intersection occurs
	float dist = 0;                     // the length of the ray from origin to the intersection point with the AABB

	//const std::vector<EntityID>& visEntts = ;
	//const size numVisEntts = std::ssize(visEntts);

	// go through each visible entt and check if we have an intersection with it
	for (const EntityID enttID : entityMgr_.renderSystem_.GetAllVisibleEntts())
	{
		//const EntityID enttID = visEntts[i];
		const ModelID modelID = entityMgr_.modelSystem_.GetModelIdRelatedToEntt(enttID);
		const BasicModel& model = modelStorage_.GetModelByID(modelID);

		if (model.type_ == ModelType::Terrain)
		{
			continue;
		}
	
		// get an inverse world matrix of the current entt
		const XMMATRIX invWorld = entityMgr_.transformSystem_.GetInverseWorldMatrixOfEntt(enttID);
		const XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

		XMVECTOR rayOrigin = XMVector3TransformCoord(rayOrigin_, toLocal);   // supposed to take a point (w == 1)
		XMVECTOR rayDir    = XMVector3TransformNormal(rayDir_, toLocal);  // supposed to take a vec (w == 0)

		// make the ray direction unit length for the intersection tests
		rayDir = DirectX::XMVector3Normalize(rayDir);


		// if we hit the bounding box of the model, then we might have picked
		// a model triangle, so do the ray/triangle tests;
		//
		// if we didn't hit the bounding box, then it is impossible that we
		// hit the model, so do not waste efford doing ray/triangle tests
		if (model.GetModelAABB().Intersects(rayOrigin, rayDir, dist))
		{
			// execute ray/triangle tests
			for (int i = 0; i < model.GetNumIndices() / 3; ++i)
			{
				// indices for this triangle
				UINT i0 = model.indices_[i * 3 + 0];
				UINT i1 = model.indices_[i * 3 + 1];
				UINT i2 = model.indices_[i * 3 + 2];

				// vertices for this triangle
				XMVECTOR v0 = XMLoadFloat3(&model.vertices_[i0].position);
				XMVECTOR v1 = XMLoadFloat3(&model.vertices_[i1].position);
				XMVECTOR v2 = XMLoadFloat3(&model.vertices_[i2].position);

				// we have to iterate over all the triangle in order 
				// to find the nearest intersection
				float t = 0.0f;

				if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
				{
					if (t <= tmin)
					{
						// this is the new nearest picked entt and its triangle
						tmin = t;
						selectedTriangleIdx = i;
						selectedEnttID = enttID;
					}
				}
			}
		}
	}

	// print a msg about selection of the entity
	if (selectedEnttID)
	{
		const EntityName& name = entityMgr_.nameSystem_.GetNameById(selectedEnttID);
		const std::string msg = std::format("picked entt (id, name): {} {}", selectedEnttID, name);

		Log::Print(msg, ConsoleColor::YELLOW);
	}

	// return ID of the selected entt, or 0 if we didn't pick any
	return selectedEnttID;
}

///////////////////////////////////////////////////////////

void GraphicsClass::UpdateCameraEntity(
	const std::string& cameraEnttName,
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& proj)
{
	// load updated camera data into ECS
	
	if (cameraEnttName.empty())
	{
		Log::Error("input name is empty");
		return;
	}

	const EntityID cameraID = entityMgr_.nameSystem_.GetIdByName("editor_camera");

	// if we found any entt by such name
	if (cameraID != 0)
	{
		entityMgr_.cameraSystem_.Update(cameraID, view, proj);
	}
	
}
