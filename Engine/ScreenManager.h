#pragma once
#include "CEGUI/CEGUI.h"
#include "CEGUI/RendererModules/Direct3D11/Renderer.h"

#include <functional>
#include "SettingsDependent.h"

#include "timerclass.h"
#include "GameStates.h"
#include <map>
#include <string>

class InputClass;
class D3DManager;

class TextOverlayScreen;
class GenericScreen;

class ScreenManager : public SettingsDependent
{
private:
//How often we'll update mouse input
#define MouseInputUpdateRate 0.2f
#define KeyboardInputUpdateRate 0.1f

public:
	ScreenManager();
	~ScreenManager();

	bool Initialize(HWND extHwnd, HINSTANCE hInst, int screenWidth, int screenHeight, int centerPosX, int centerPosY, bool vsyncEnabled, bool fullScreen);
	void InitializeGUI();

	//Provides smooth transition out of current state and into new state
	void ChangeState(GameStates::Type state);

	bool Update();
	void HandleMessages(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	bool HandleInputs();

	virtual void OnSettingsReload(Config* cfg);

	//CEGUI::utf32 KeycodeToUTF32(unsigned int scanCode);

	//Returns true if escape was pressed this frame
	bool Quitting() { return isQuitting; }

private:
	void DrawOverlayText(const std::string& text);

	void AddNewScreen(std::shared_ptr<GenericScreen> screen, GameStates::Type gameState, std::wstring name);
	void Quit() { isQuitting = true; }


private:
	HWND hwnd;
	float nearClip, farClip, shadowMapWidth, shadowMapHeight, clickUpdateTimer, keypressUpdateTimer;

	std::shared_ptr<D3DManager> d3D;
	std::shared_ptr<InputClass> input;
	TimerClass timer;

	//TODO: Lambda system again... this time in debug mode to see what's happening
	GameStates::Type previousState, currentState;
	std::shared_ptr<GenericScreen> currentScreen;

	std::map<GameStates::Type, std::wstring> stateNames;
	std::map<GameStates::Type, std::shared_ptr<GenericScreen>> stateToScreenMap;
	std::shared_ptr<TextOverlayScreen> textOverlayScreen;

	bool isQuitting, showCursor, paused;
};

