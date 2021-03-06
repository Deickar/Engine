#pragma once
#include "fontshaderclass.h"
#include <vector>
#include <windows.h>
#include <xnamath.h> 
#include <string>
#include <iosfwd>
#include <functional>
#include "SentenceManager.h"

class TextClass
{
public:
	TextClass(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight);
	~TextClass();

	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, XMMATRIX* worldViewProjection);

	bool SetMousePosition(int, int,			ID3D11DeviceContext*);
	bool SetFps(int,						ID3D11DeviceContext*);
	bool SetCpu(int,						ID3D11DeviceContext*);
	bool SetRendercount(int,				ID3D11DeviceContext*);
	bool SetCameraPosition(int, int, int,	ID3D11DeviceContext*);
	bool SetCameraRotation(int, int, int,	ID3D11DeviceContext*);
	bool SetLastChar(char,					ID3D11DeviceContext*);

	//Update values and reciprocate to sentenceManager.
	void SetScreenWidth(int width) { screenWidth = width; sentenceManager.SetScreenWidth(width); }
	void SetScreenHeight(int height) { screenHeight = height; sentenceManager.SetScreenHeight(height); }

private:
	//This stays in text class because this is where we have fontshader class
	bool RenderSentence(const SentenceManager::SentenceType* sentence, FontClass* font, ID3D11DeviceContext* deviceContext, XMMATRIX* worldViewProjection);

private:
	FontShaderClass fontShader;
	SentenceManager sentenceManager;
	int screenWidth, screenHeight;

	//TODO CHANGE ALL OF THE BELOW TO IDS
	//THEN MOVE ALL SENTENCE MANAGING TO SENTENCEMANAGER

	SentenceManager::Handle fpsCountTextID;
	SentenceManager::Handle cpuLoadTextID;
	SentenceManager::Handle mousePosXTextID;
	SentenceManager::Handle mousePosYTextID;
	
	SentenceManager::Handle cameraPosXTextID;
	SentenceManager::Handle cameraPosYTextID;
	SentenceManager::Handle cameraPosZTextID;
	SentenceManager::Handle cameraRotXTextID;
	SentenceManager::Handle cameraRotYTextID;
	SentenceManager::Handle cameraRotZTextID;

	SentenceManager::Handle numberOfModelsDrawnTextID;
	SentenceManager::Handle numberOfTrianglesRenderedTextID; //For the terrain/quadtree
	SentenceManager::Handle lastCharPressedTextID;

	XMFLOAT3 white, black, red, green, blue, yellow, cyan, magenta;
};