////////////////////////////////////////////////////////////////////
// Filename:     RenderWindow.h
// Description:  this class is responsible for creation, initialization,
//               and showing of the window; 
//               also it dispatches some messages about window events to 
//               special handler which is in the WindowContainer class;
// Revising:     30.09.22
////////////////////////////////////////////////////////////////////
#pragma once

#include <string>

namespace Core
{

class RenderWindow
{
public:
	~RenderWindow();

	// initializes the private members and registers the window class
	bool Initialize(
		HINSTANCE hInstance, 
		HWND& mainWnd,
		const bool isFullScreen,
		const std::string & windowTitle,
		const std::string & windowClass,
		const int width, 
		const int height);

	bool ProcessMessages(HINSTANCE & hInstance, HWND & hwnd);
	
	void UnregisterWindowClass(HINSTANCE & hInstance);

	inline UINT GetWndWidth()     const { return windowWidth_; }
	inline UINT GetWndHeight()    const { return windowHeight_; }
	inline UINT GetClientWidth()  const { return clientWidth_; };
	inline UINT GetClientHeight() const { return clientHeight_; };
	inline float GetAspectRatio() const { return (float)clientWidth_ / (float)clientHeight_; }

	void UpdateWindowDimensions(const UINT newWidth, const UINT newHeight);
	void UpdateClientDimensions(const UINT newWidth, const UINT newHeight);
	

private:
	void RegisterWindowClass(const HINSTANCE hInstance);            
	bool CreateWindowExtended(const HINSTANCE hInstance, HWND& hwnd, const bool isFullScreen);

private:
	std::wstring windowTitleWide_{ L"" }; // wide string representation of window title
	std::wstring windowClassWide_{ L"" }; // wide string representation of window class name

	std::string windowTitle_{ "" };
	std::string windowClass_{ "" };

	// window's zone width/height
	UINT windowWidth_ = 0;
	UINT windowHeight_ = 0;

	// client's zone width/height
	UINT clientWidth_ = 0;
	UINT clientHeight_ = 0;
};

} // namespace Doors