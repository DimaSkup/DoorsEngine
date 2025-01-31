// =================================================================================
// Filename:  Engine.cpp
// Created:   05.10.22
// =================================================================================
#include "Engine.h"

#include "../Common/FileSystemPaths.h"
#include "ProjectSaver.h"

#include "ImGuizmo.h"
#include "../UI/UICommon/FacadeEngineToUI.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <winuser.h>
//#include <Psapi.h>    // This header is used by Process Status API (PSAPI)


namespace Doors
{


Engine::Engine()
{
	Log::Debug();
	timer_.Reset();       // reset the engine/game timer
}


Engine::~Engine()
{
	Log::Print();
	Log::Print("-------------------------------------------------", ConsoleColor::YELLOW);
	Log::Print("            START OF THE DESTROYMENT:            ", ConsoleColor::YELLOW);
	Log::Print("-------------------------------------------------", ConsoleColor::YELLOW);

	ProjectSaver projSaver;

	projSaver.StoreModels(graphics_.GetD3DClass().GetDevice());

	delete pFacadeEngineToUI_;

	// unregister the window class, destroys the window,
	// reset the responsible members;

	if (hwnd_ != NULL)
	{
		ChangeDisplaySettings(NULL, 0);  // before destroying the window we need to set it to the windowed mode
		DestroyWindow(hwnd_);            // Remove the window
		hwnd_ = NULL;
		hInstance_ = NULL;

		Log::Debug("engine desctuctor");
	}

	imGuiLayer_.Shutdown();

	Log::Print("the engine is shut down successfully");
}


// =================================================================================
//                            public methods
// =================================================================================

bool Engine::Initialize(
	HINSTANCE hInstance,
	HWND mainWnd,
	const Settings& settings,
	const std::string& windowTitle)
{
	// this function initializes all the main parts of the engine

	try
	{
		// check support for SSE2 (Pentium4, AMD K8, and above)
		Assert::True(DirectX::XMVerifyCPUSupport(), "XNA math not supported");


		// WINDOW: store a handle to the application instance
		hInstance_ = hInstance;  
		hwnd_ = mainWnd;
		windowTitle_ = windowTitle;


		// GRAPHICS SYSTEM: initialize the graphics system
		bool result = graphics_.Initialize(
			hwnd_,
			systemState_, 
			settings);
		Assert::True(result, "can't initialize the graphics system");

		D3DClass& d3d = graphics_.GetD3DClass();
		ID3D11Device* pDevice = d3d.GetDevice();
		ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

		systemState_.wndWidth_ = d3d.GetWindowWidth();
		systemState_.wndHeight_ = d3d.GetWindowHeight();
		

#if 0
		// create a texture which can be used as a render target
		FrameBufferSpecification fbSpec;

		fbSpec.width = 800;
		fbSpec.height = 600;
		fbSpec.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		fbSpec.screenNear = d3d.GetScreenNear();
		fbSpec.screenDepth = d3d.GetScreenDepth();

		result = frameBuffer_.Initialize(pDevice, fbSpec);
		Assert::True(result, "can't initialize the render to texture object");
		
#endif
#if 0
		// SOUND SYSTEM: initialize the sound obj
		result = sound_.Initialize(hwnd_);
		Assert::True(result, "can't initialize the sound system");
#endif


		// INPUT SYSTEM: setup keyboard input params
		keyboard_.EnableAutoRepeatKeys();
		keyboard_.EnableAutoRepeatChars();

		// TIMERS: (game timer, CPU)
		timer_.Tick();                 // execute tick so we will be able to receive the initialization time
		cpu_.Initialize();


		// create a facade btw the UI and the engine parts
		pFacadeEngineToUI_ = new FacadeEngineToUI(
			pContext,
			&graphics_.GetRender(),
			&graphics_.GetEntityMgr(),
			&graphics_.GetTextureMgr());


		imGuiLayer_.Initialize(hwnd_, pDevice, pContext);
		
		// initialize the main UserInterface class
		InitializeGUI(d3d, settings);

		// create a str with duration time of the engine initialization process
		std::string initTimeStr = { "Init time: " + std::to_string(timer_.GetGameTime()) + " s" };
		userInterface_.CreateConstStr(pDevice, initTimeStr, { 10, 325 });
		

		Log::Print("is initialized!");
	}
	catch (EngineException& e)
	{
		Log::Error(e, true);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////

bool Engine::InitializeGUI(D3DClass& d3d, const Settings& settings)
{
	// this function initializes the GUI of the game/engine (interface elements, text, etc.);

	Log::Print();
	Log::Print("----------------------------------------------------------", ConsoleColor::YELLOW);
	Log::Print("                   INITIALIZATION: GUI                    ", ConsoleColor::YELLOW);
	Log::Print("----------------------------------------------------------", ConsoleColor::YELLOW);

	try
	{
		const std::string fontDataFullFilePath = g_DataDir + settings.GetString("FONT_DATA_FILE_PATH");
		const std::string fontTexFullFilePath = g_DataDir + settings.GetString("FONT_TEXTURE_FILE_PATH");
		std::string videoCardName{ "" };
		int videoCardMemory = 0;

		d3d.GetVideoCardInfo(videoCardName, videoCardMemory);
	
		// initialize the user interface
		userInterface_.Initialize(
			d3d.GetDevice(),
			d3d.GetDeviceContext(),
			pFacadeEngineToUI_,
			fontDataFullFilePath,
			fontTexFullFilePath,
			settings.GetInt("WINDOW_WIDTH"),
			settings.GetInt("WINDOW_HEIGHT"),
			videoCardMemory,
			videoCardName);
	}
	catch (EngineException& e)
	{
		Log::Error(e, true);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

void Engine::Update()
{
	timer_.Tick();
	
#if 0
	// to update the system stats each of timers classes we needs to call its 
	// own Update function for each frame of execution the application goes through
	cpu_.Update();

	// update the percentage of total cpu use that is occuring each second
	systemState_.cpu = cpu_->GetCpuPercentage();
#endif

	// get the time which passed since the previous frame
	deltaTime_ = timer_.GetDeltaTime();
	deltaTime_ = (deltaTime_ > 16.6f) ? 16.6f : deltaTime_;

	// compute fps and frame time (ms)
	CalculateFrameStats();

	// maybe hack: handle also here to prevent delaying after pressing the key (for instance W)
	graphics_.HandleKeyboardInput(keyboardEvent_, deltaTime_);

	
	graphics_.Update(systemState_, deltaTime_, timer_.GetGameTime());
	userInterface_.Update(graphics_.GetD3DClass().GetDeviceContext(), systemState_);
}

///////////////////////////////////////////////////////////

void Engine::CalculateFrameStats()
{
	// measure the number of frames being rendered per second (FPS);
	// this method would be called every frame in order to count the frame

	// code computes the avetage frames per second, and also the average time it takes
	// to render one frame. These stats are appended to the window caption bar

	static int frameCount = 0;
	static int timeElapsed = 0;
	
	frameCount++;

	// compute averages over one second period
	if ((timer_.GetGameTime() - timeElapsed) >= 1.0f)
	{
		// store the fps value for later using (for example: render this value as text onto the screen)
		systemState_.fps = frameCount;
		systemState_.frameTime = 1000.0f / (float)frameCount;  // ms per frame

		// reset for next average
		frameCount = 0;
		++timeElapsed;

		// print FPS/frame_time as the window caption
#if 0
		PROCESS_MEMORY_COUNTERS pmc;
		DWORD processID;
		GetWindowThreadProcessId(hwnd_, &processID);

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		Assert::True(hProcess != NULL, "can't get a process handle of the window");
		
		GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

		std::wostringstream outs;
		outs.precision(6);
		outs << windowTitle_ << L" "
			 << L"FPS: " << systemState_.fps << L" "
			 << L"Frame Time: " << systemState_.frameTime << L" (ms); "
			 << L"RAM Usage: " << pmc.WorkingSetSize / 1024 / 1024 << L" (mb)";
		SetWindowText(hwnd_, outs.str().c_str());		
#endif
	}
}

///////////////////////////////////////////////////////////

void Engine::RenderFrame()
{
	// this function executes rendering of each frame;

	try
	{
		using namespace DirectX;

		D3DClass& d3d = graphics_.GetD3DClass();
		ID3D11DeviceContext* pContext = d3d.GetDeviceContext();
		
		if (systemState_.isEditorMode)
		{
			// Clear all the buffers before frame rendering and render our 3D scene
			d3d.BeginScene();
			graphics_.Render3D();

			// begin rendering of the editor elements
			imGuiLayer_.Begin();

			RenderUI();

			ImGui::End();
			imGuiLayer_.End();
		}

		// we aren't in the editor mode
		else  
		{
			// Clear all the buffers before frame rendering and render our 3D scene
			d3d.BeginScene();
			graphics_.Render3D();
			RenderUI();
		}

		// Show the rendered stuff on the screen
		d3d.EndScene();

		// before next frame
		graphics_.ClearRenderingDataBeforeFrame();
	}
	catch (EngineException & e)
	{
		Log::Error(e, true);
		Log::Error("can't render a frame");
		
		// exit after it
		isExit_ = true;
	}
}

///////////////////////////////////////////////////////////

void Engine::RenderUI()
{
	D3DClass& d3d = graphics_.GetD3DClass();

	// preparation before 2D rendering
	d3d.TurnZBufferOff();
	d3d.TurnOnBlending(RenderStates::STATES::ALPHA_ENABLE);
	d3d.TurnOnRSfor2Drendering();

	if (isEditorMode_)
	{
		// all render the scene view space and gizmos (if any entt is selected)
		userInterface_.RenderSceneWnd(systemState_);

		// HACK: we set background color for ImGui elements (except of scene windows)
		//       each fucking time because if we doesn't it we will have 
		//       scene objects which are using blending to be mixed with 
		//       ImGui bg color; so we want to have proper scene colors;
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = imGuiLayer_.GetBackgroundColor();

		
		userInterface_.RenderEditor(systemState_);

		// reset: ImGui window bg color to fully invisible since we
		//        want to see the scene through the window
		colors[ImGuiCol_WindowBg] = { 0,0,0,0 };

	}
	// we're in the game mode
	else
	{
		userInterface_.RenderGameUI(
			d3d.GetDeviceContext(),
			graphics_.GetRender().GetShadersContainer().fontShader_,
			systemState_);
	}

	// reset after 2D rendering
	d3d.TurnOffBlending();     
	d3d.TurnZBufferOn(); 
	d3d.TurnOffRSfor2Drendering();
}


// =================================================================================
//                        public API: event handlers 
// =================================================================================

void Engine::EventActivate(const APP_STATE state)
{
	// define that the app is curretly running or paused

	if (state == APP_STATE::ACTIVATED)
	{
		isPaused_ = false;
		timer_.Start();
	}
	else if (state == APP_STATE::DEACTIVATED)
	{
		isPaused_ = true;
		timer_.Stop();
	}
}

///////////////////////////////////////////////////////////

void Engine::EventWindowMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//assert("TODO: implement it!" && 0);
}

///////////////////////////////////////////////////////////

void Engine::EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SIZE newSize{ LOWORD(lParam), HIWORD(lParam) };

	// try to resize the window
	if (!graphics_.GetD3DClass().ResizeSwapChain(hwnd, newSize))
		PostQuitMessage(0);
}

///////////////////////////////////////////////////////////

void Engine::EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT wndRect;
	GetWindowRect(hwnd, &wndRect);   // get the window dimensions

	switch (wParam)
	{
		case WMSZ_LEFT:
		case WMSZ_RIGHT:
		case WMSZ_TOP:
		case WMSZ_BOTTOM:
		{
			// resize by X or by Y
			const int posX   = wndRect.left;
			const int posY   = wndRect.top;
			const int width  = wndRect.right - posX;
			const int height = wndRect.bottom - posY;

			// Set new dimensions
			SetWindowPos(hwnd, NULL, posX, posY,
				width, height,
				SWP_NOMOVE | SWP_NOZORDER);

			// try to resize the window
			if (!graphics_.GetD3DClass().ResizeSwapChain(hwnd, { width, height }))
				PostQuitMessage(0);

			systemState_.wndWidth_  = width;
			systemState_.wndHeight_ = height;

			break;
		}
	}
}

///////////////////////////////////////////////////////////

void Engine::HandleEditorEventKeyboard()
{
	static bool isKeyCtrlDown = false;
	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

	// define if some keys (ctrl,alt,shift) are currently pressed down
	// so it gives us more opportunities to control using the keyboard
	isKeyCtrlDown = (1 < keyboardState[VK_CONTROL]);

	const UCHAR keyCode = keyboardEvent_.GetKeyCode();
	static UCHAR prevKeyCode = 0;

	// handle pressing of some keys
	if (keyboardEvent_.IsPress())
	{
		switch (keyCode)
		{
			case KEY_X:
			{
				int kek = 0;
				kek++;
				break;
			}
			case KEY_Q:
			{
				// guizmo: turn OFF any operation
				if (isKeyCtrlDown)
					userInterface_.SetGizmoOperation(-1);
				break;
			}
			case KEY_T:
			{
				if (isKeyCtrlDown)
					userInterface_.SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
				break;
			}
			case KEY_R:
			{
				if (isKeyCtrlDown)
					userInterface_.SetGizmoOperation(ImGuizmo::OPERATION::ROTATE);
				break;
			}
			case KEY_S:
			{
				if (isKeyCtrlDown)
					userInterface_.SetGizmoOperation(ImGuizmo::OPERATION::SCALE);
				break;
			}
			case KEY_F1:
			{
				// switch to the game mode
				graphics_.GetD3DClass().ToggleFullscreen(hwnd_, true);
				graphics_.SwitchGameMode(true);

				systemState_.isEditorMode = false;
				isEditorMode_ = false;
				ShowCursor(FALSE);

				break;
			}
			case VK_ESCAPE:
			{
				// if we pressed the ESC button we exit from the application
				Log::Debug("Esc is pressed");
				isExit_ = true;
				break;
			}
		} // switch

		// store the values of currently pressed key for the next frame
		prevKeyCode = keyCode;

	} // if is press

	// handle releasing of some keys
	if (keyboardEvent_.IsRelease())
	{
		UCHAR keyCode = keyboardEvent_.GetKeyCode();

		prevKeyCode = 0;
	}

	// store what type of the keyboard event we have 
	keyboardEvent_ = keyboard_.ReadKey();
}

///////////////////////////////////////////////////////////

void Engine::HandleGameEventKeyboard()
{
	const UCHAR keyCode = keyboardEvent_.GetKeyCode();
	static UCHAR prevKeyCode = 0;

	// handle pressing of some keys
	if (keyboardEvent_.IsPress())
	{
		switch (keyCode)
		{

			case KEY_F1:
			{
				// switch to the editor mode
				D3DClass& d3d = graphics_.GetD3DClass();
				Camera& editorCamera = graphics_.GetEditorCamera();

				d3d.ToggleFullscreen(hwnd_, false);
				graphics_.SwitchGameMode(false);

				// update the camera proj matrix according to new window size
				editorCamera.SetProjectionValues(
					editorCamera.GetFovInRad(),
					d3d.GetAspectRatio(),
					d3d.GetScreenNear(),
					d3d.GetScreenDepth());

				systemState_.isEditorMode = true;
				isEditorMode_ = true;
				ShowCursor(TRUE);

				break;
			}
			case KEY_F3:
			{
				// show/hide debug info in the game mode
				systemState_.isShowDbgInfo = !systemState_.isShowDbgInfo;

				if (systemState_.isShowDbgInfo)
					Log::Debug("F3 key is pressed: show dbg text info:");
				else
					Log::Debug("F3 key is pressed: hide dbg text info:");

				break;
			}
			case KEY_L:
			{
				// switch the flashlight
				static bool flashLightState = true;

				if (prevKeyCode != KEY_L)
				{
					flashLightState = !flashLightState;

					graphics_.GetRender().SwitchFlashLight(
						graphics_.GetD3DClass().GetDeviceContext(),
						flashLightState);
				}

				Log::Debug("key L is pressed");

				break;
			}
			case VK_ESCAPE:
			{
				// if we pressed the ESC button we exit from the application
				Log::Debug("Esc is pressed");
				isExit_ = true;
				break;
			}
		} // switch

		// store the values of currently pressed key for the next frame
		prevKeyCode = keyCode;

	} // if is press

	// handle releasing of some keys
	if (keyboardEvent_.IsRelease())
	{
		UCHAR keyCode = keyboardEvent_.GetKeyCode();

		prevKeyCode = 0;
	}

	// store what type of the keyboard event we have 
	keyboardEvent_ = keyboard_.ReadKey();
}

///////////////////////////////////////////////////////////

void Engine::EventKeyboard(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// a handler for all the keyboard events

	inputMgr_.HandleKeyboardMessage(keyboard_, uMsg, wParam, lParam);

	// according to the engine mode we call a respective keyboard handler
	if (isEditorMode_)
		while (!keyboard_.KeyBufferIsEmpty())
			HandleEditorEventKeyboard();
	else
		while (!keyboard_.KeyBufferIsEmpty())
			HandleGameEventKeyboard();		
}

///////////////////////////////////////////////////////////

void Engine::EventMouse(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// handler for all the mouse events;

	inputMgr_.HandleMouseMessage(mouse_, uMsg, wParam, lParam);

	while (!mouse_.EventBufferIsEmpty())
	{
		mouseEvent_ = mouse_.ReadEvent();

		switch (mouseEvent_.GetEventType())
		{
			case MouseEvent::EventType::Move:
			{
				// update mouse position data because we need to print mouse position on the screen
				systemState_.mouseX = mouseEvent_.GetPosX();
				systemState_.mouseY = mouseEvent_.GetPosY();
				break;
			}
			case MouseEvent::EventType::LPress:
			{
				// if we currenly hovering the scene windows with our mouse
				// and we don't hover any gizmo we execute entity picking (selection) test
				if (userInterface_.IsSceneWndHovered() && !userInterface_.IsGizmoHovered())
				{
					EntityID selectedEnttID = 0;
					selectedEnttID = graphics_.TestEnttSelection(mouseEvent_.GetPosX(), mouseEvent_.GetPosY());

					// update the UI about selection of the entity
					if (selectedEnttID)
						userInterface_.SetSelectedEntt(selectedEnttID);
				}
				break;
			}
			default:
			{
				graphics_.HandleMouseInput(systemState_, mouseEvent_, deltaTime_);
				break;
			}
		} // switch
	} // while
}

} // namespace Doors