////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontclass.h"
#include "fontshaderclass.h"
#include <vector>
#include <windows.h>
#include <xnamath.h> 


////////////////////////////////////////////////////////////////////////////////
// Class name: TextClass
////////////////////////////////////////////////////////////////////////////////
class TextClass
{
private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	TextClass();
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, XMMATRIX* worldViewProjection);

	bool SetMousePosition(int, int, ID3D11DeviceContext*);
	bool SetFps(int, ID3D11DeviceContext*);
	bool SetCpu(int, ID3D11DeviceContext*);
	bool SetRendercount(int, ID3D11DeviceContext*);
	bool SetCameraPosition(int, int, int, ID3D11DeviceContext*);
	bool SetCameraRotation(int, int, int, ID3D11DeviceContext*);
	bool SetRenderCount(int, ID3D11DeviceContext*);
	bool SetLastChar(char, ID3D11DeviceContext*);

private:
	bool InitializeSentence(SentenceType**, int, ID3D11Device*);
	bool UpdateSentence(SentenceType* sentence, char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(SentenceType**);
	void ReleaseSentences(vector<SentenceType*> sentences);
	bool RenderSentence(SentenceType*, ID3D11DeviceContext*, XMMATRIX* worldViewProjection);

private:
	vector<SentenceType*> sentences;

	FontClass* font;
	FontShaderClass* fontShader;
	int screenWidth, screenHeight;

	SentenceType* fpsCount;
	SentenceType* cpuLoad;
	SentenceType* mousePosX;
	SentenceType* mousePosY;

	SentenceType* cameraPosX;
	SentenceType* cameraPosY;
	SentenceType* cameraPosZ;
	SentenceType* cameraRotX;
	SentenceType* cameraRotY;
	SentenceType* cameraRotZ;
	SentenceType* numberOfModelsDrawn;
	SentenceType* numberOfTrianglesRendered; //For the terrain/quadtree
	SentenceType* lastCharPressed;
};

#endif