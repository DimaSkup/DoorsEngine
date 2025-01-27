// =================================================================================
// Filename:     UserInterface.cpp
// 
// Created:      25.05.23
// =================================================================================
#include "UserInterface.h"

#include "../Common/FileSystemPaths.h"
#include "UI_Windows/EnttCreationWnd.h"
#include "../Texture/TextureMgr.h"
#include "Render.h"                     // from the Render module

#include <format>
#include <string>
#include <imgui.h>
#include <ImGuizmo.h>

#pragma warning(disable : 4996)


UserInterface::UserInterface()
{
}

UserInterface::~UserInterface()
{
	pFacadeEngineToUI_ = nullptr;
}



// =================================================================================
//                             PUBLIC METHODS
// =================================================================================

// initialize the graphics user interface (GUI)
void UserInterface::Initialize(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	IFacadeEngineToUI* pFacadeEngineToUI,
	const std::string & fontDataFilePath,      // a path to file with data about this type of font
	const std::string & fontTextureFilePath,   // a path to texture file for this font
	const int wndWidth,
	const int wndHeight,
	const UINT videoCardMemory,
	const std::string& videoCardName) 
{
	Log::Debug();

	try
	{
		Assert::NotNullptr(pFacadeEngineToUI, "a ptr to the facade interface == nullptr");
		Assert::True((wndWidth > 0) && (wndHeight > 0), "wrong window dimensions");
		Assert::True((!videoCardName.empty()) && (videoCardMemory > 0), "wrong video card data");
		Assert::True((!fontDataFilePath.empty()), "wrong path to font data file");
		Assert::True((!fontTextureFilePath.empty()), "wrong path to font texture file");

		// initialize the window dimensions members for internal using
		windowWidth_  = wndWidth;
		windowHeight_ = wndHeight;

		// --------------------------------------------
		
		// initialize the first font object
		font1_.Initialize(pDevice, fontDataFilePath, fontTextureFilePath);

		// initialize the editor parts and interfaces
		pFacadeEngineToUI_ = pFacadeEngineToUI;
		editorPanels_.Initialize(pFacadeEngineToUI_);
		
		
		// create text strings to show debug info onto the screen
		CreateDebugInfoStrings(pDevice, videoCardName, videoCardMemory);

		
		Log::Debug("USER INTERFACE is initialized");
	}
	catch (EngineException & e)
	{
		Log::Error(e, false);
		Log::Error("can't initialize the UserInterface");
	}
}


// =================================================================================
//                           PUBLIC MODIFICATION API
// =================================================================================

void UserInterface::Update(
	ID3D11DeviceContext* pContext, 
	const SystemState& systemState)
{
	// each frame we call this function for updating the UI
	
	try
	{
		textStorage_.Update(pContext, font1_, systemState);
	}
	catch (EngineException & e)
	{
		Log::Error(e);
		Log::Error("can't update the GUI");
	}
}

///////////////////////////////////////////////////////////

void UserInterface::Render(
	ID3D11DeviceContext* pContext,
	Render::FontShaderClass& fontShader,
	SystemState& systemState)
{
	//
	// this functions renders all the UI elements onto the screen
	//
	// ATTENTION: do 2D rendering only when all 3D rendering is finished;
	// this function renders the engine/game GUI


	// in the game mode: print onto the screen some debug info
	if (systemState.isShowDbgInfo)
		RenderDebugInfo(pContext, fontShader, systemState);


	if (systemState.isEditorMode)
	{
		editorMainMenuBar_.RenderBar(guiStates_);

		// show window to control engine options
		if (guiStates_.showWndEngineOptions_)
			editorMainMenuBar_.RenderWndEngineOptions(&guiStates_.showWndEngineOptions_);

		// show modal window for entity creation
		if (guiStates_.showWndForEnttCreation_)
		{
			static EnttCreationWnd wnd(pContext);
			//wnd.ShowWndToCreateEntt(&guiStates_.showWndForEnttCreation_, entityMgr);
		}

		RenderEditor(systemState);
	}
}

///////////////////////////////////////////////////////////

SentenceID UserInterface::CreateConstStr(
	ID3D11Device* pDevice,
	const std::string& content,                         
	const POINT& drawAt)                            // upper left position of the text in the window
{
	// create a new GUI string by input data;
	// the content of this string won't be changed;
	// you can only change its position on the screen;

	try
	{
		Assert::True(!content.empty(), "wrong input data: str is empty");

		return textStorage_.CreateConstSentence(
			pDevice,
			font1_,                             // a font which is used for this sentence
			content,
			ComputePosOnScreen(drawAt));
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("can't create the sentence: " + content);
		return 0;
	}
}

///////////////////////////////////////////////////////////

SentenceID UserInterface::CreateDynamicStr(
	ID3D11Device* pDevice,
	const std::string& content,
	const POINT& drawAt,
	const int maxStrSize)                           // max possible length for this string
{
	// create a new GUI string by input data;
	// the content of this string is supposed to be changed from frame to frame;
	// you can also change its position on the screen;

	try
	{
		Assert::True((!content.empty()) && (maxStrSize > 0), "wrong input data");

		// max possible length for this dynamic string
		int maxSize = (maxStrSize >= content.length()) ? maxStrSize : (int)std::ssize(content);

		return textStorage_.CreateSentence(
			pDevice,
			font1_,
			content,
			maxSize,
			ComputePosOnScreen(drawAt),
			true);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("can't create a sentence: " + content);
		return 0;
	}
}




// =================================================================================
// 
//                              PRIVATE METHODS
// 
// =================================================================================

void UserInterface::CreateDebugInfoStrings(
	ID3D11Device* pDevice,
	const std::string& videoCardName,
	const int videoCardMemory)
{
	// load debug info string params from the file and create these strings;
	// and create some strings manually

	const std::string filepath = g_DataDir + "ui_debug_info_strings.txt";

	FILE* pFile = fopen(filepath.c_str(), "r");
	if (!pFile)
	{
		Log::Error("can't open a file: " + filepath);
		return;
	}

	char buffer[64] = { 0 };
	char str[64] = { 0 };
	const int bufsize = 64;
	int posX, posY, maxStrSize;

	while (fgets(buffer, bufsize, pFile))
	{
		// we read and create a const string (won't be changed during the runtime)
		if (strncmp(buffer, "const_str", 9) == 0)
		{
			sscanf(buffer, "%*s %s %d %d", str, &posX, &posY);
			CreateConstStr(pDevice, std::string(str), { posX, posY });
		}
		// we read and create a dynamic string (will be changed during the runtime)
		else if (strncmp(buffer, "dynamic_str", 11) == 0)
		{
			sscanf(buffer, "%*s %s %d %d %d", str, &posX, &posY, &maxStrSize);
			SentenceID id = CreateDynamicStr(pDevice, "0", { posX, posY }, maxStrSize);
			textStorage_.SetKeyByID(str, id);
		}
	}

	// ------------------------------------------

	// create some strings about the video card
	CreateConstStr(pDevice, videoCardName, { 150, 10 });
	CreateConstStr(pDevice, std::format("{} MB", videoCardMemory), { 150, 30 });
}


// ====================================================================================
//                                 editor stuff
// ====================================================================================

void UserInterface::RenderEditor(SystemState& systemState)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	

	static bool isEditorOpen = true;

	static ImGuiChildFlags childFlags = 0;
	static ImGuiWindowFlags wndFlags = 0;

	childFlags |= ImGuiChildFlags_Border;
	childFlags |= ImGuiChildFlags_ResizeX;
	childFlags |= ImGuiChildFlags_ResizeY;
	
	editorPanels_.Render(systemState, childFlags, wndFlags);




#if 0 // IMGUIZMO

	static ImGuizmo::OPERATION currGizmoOp;
	static ImGuizmo::MODE currGizmoMode = ImGuizmo::WORLD;


	const float* cameraView = systemState.CameraView.r->m128_f32;
	const float* cameraProj = systemState.CameraProj.r->m128_f32;
	const float* gridWorld = DirectX::XMMatrixIdentity().r->m128_f32;

	static DirectX::XMMATRIX cubeWorldMat = DirectX::XMMatrixIdentity();
	static float* cubeWorld = cubeWorldMat.r->m128_f32;

#if 0
	ImGuizmo::DrawGrid(
		cameraView,
		cameraProj,
		gridWorld,
		100.f);

	ImGuizmo::DrawCubes(
		cameraView, cameraProj, cubeWorld, 1);
#endif


	if (ImGui::IsKeyPressed(ImGuiKey_T))
	{
		currGizmoOp = ImGuizmo::TRANSLATE;
		ImGui::Text("T is pressed");
	}
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		currGizmoOp = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
		currGizmoOp = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", currGizmoOp == ImGuizmo::TRANSLATE))
		currGizmoOp = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", currGizmoOp == ImGuizmo::ROTATE))
		currGizmoOp = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", currGizmoOp == ImGuizmo::SCALE))
		currGizmoOp = ImGuizmo::SCALE;
	//float matrixTranslation[3], matrixRotation[3], matrixScale[3];


	ImGuizmo::PushID("Guizmo##manipulate");

	ImGuizmo::Manipulate(
		cameraView,
		cameraProj,
		currGizmoOp,
		currGizmoMode,
		cubeWorld, NULL, NULL);

	ImGuizmo::PopID();
#endif

	

}

///////////////////////////////////////////////////////////

DirectX::XMFLOAT2 UserInterface::ComputePosOnScreen(const POINT& drawAt)
{
	// in:  top left pos relatively to the top left corner of the screen
	// out: top left pos relatively to the screen center
	return
	{
		(float)(-(windowWidth_  >> 1) + drawAt.x),   // posX
		(float)(+(windowHeight_ >> 1) - drawAt.y),   // posY
	};
}

///////////////////////////////////////////////////////////

void UserInterface::RenderDebugInfo(
	ID3D11DeviceContext* pContext,
	Render::FontShaderClass& fontShader,
	const SystemState& sysState)
{
	TextureMgr& texMgr = *TextureMgr::Get();
	std::vector<ID3D11Buffer*> vbPtrs;
	std::vector<ID3D11Buffer*> ibPtrs;
	std::vector<u32> indexCounts;

	// receive a font SRV 
	SRV* const* ppFontTexSRV = font1_.GetTextureResourceViewAddress();

	textStorage_.GetRenderingData(vbPtrs, ibPtrs, indexCounts);

	fontShader.UpdatePerFrame(pContext, ppFontTexSRV);
	fontShader.Render(pContext, vbPtrs, ibPtrs, indexCounts, sizeof(VertexFont));
}