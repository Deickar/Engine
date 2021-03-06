#pragma once
#include <d3d11.h>
#include <windows.h>
#include <xnamath.h>
#include <atlcomcli.h>
#include <vector>

class DebugWindowClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	DebugWindowClass();
	DebugWindowClass(const DebugWindowClass&);
	~DebugWindowClass();

	bool Initialize(ID3D11Device* device, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();
	void SetTextureToRender(ID3D11ShaderResourceView* texture) { textureToRender = texture; }
	ID3D11ShaderResourceView* GetTexture() { return textureToRender; }

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

private:
	CComPtr<ID3D11Buffer> vertexBuffer;
	CComPtr<ID3D11Buffer> indexBuffer;
	ID3D11ShaderResourceView* textureToRender;
	int vertexCount, indexCount;
	int screenWidth, screenHeight;
	int bitmapWidth, bitmapHeight;
	int previousPosX, previousPosY;
};