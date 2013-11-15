////////////////////////////////////////////////////////////////////////////////
// Filename: DRFinalComposition.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _DRFINALCOMPOSITION_H_
#define _DRFINALCOMPOSITION_H_


//////////////
// INCLUDES //
//////////////
////////////// 
#pragma once

#include <d3d11.h>
#include <windows.h>
#include <xnamath.h> 
#include <d3dx11async.h>
#include <fstream>
#include <atlcomcli.h>

using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: DRFinalComposition
////////////////////////////////////////////////////////////////////////////////
class DRCompose
{
private:
	struct VertexMatrixBuffer
	{
		XMMATRIX WorldViewProjection;
		XMMATRIX WorldView;
	};

	struct PixelMatrixBuffer
	{
		XMMATRIX View;
		XMMATRIX InvertedProjection;
		XMFLOAT4 FogColor;
		int toggleSSAO;
	};


public:
	DRCompose();
	DRCompose(const DRCompose&);
	~DRCompose();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX* worldViewProjection, XMMATRIX* worldView, XMMATRIX* view, XMMATRIX* invertedProjection, XMMATRIX* invViewProjection,
		XMFLOAT4* fogColor, float fogMinimum, ID3D11ShaderResourceView** textureArray, ID3D11ShaderResourceView* randomTexture, int toggle);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX* worldViewProjection, XMMATRIX* worldView, XMMATRIX* view, XMMATRIX* invertedProjection, XMMATRIX* invViewProjection, 
		XMFLOAT4* fogColor, float fogMinimum, ID3D11ShaderResourceView** textureArray, ID3D11ShaderResourceView* randomTexture, int toggle);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	CComPtr<ID3D11VertexShader> vertexShader;
	CComPtr<ID3D11PixelShader> pixelShader;
	CComPtr<ID3D11InputLayout> layout;
	CComPtr<ID3D11SamplerState> samplers[2];

	CComPtr<ID3D11Buffer> vertexMatrixBuffer;
	CComPtr<ID3D11Buffer> pixelMatrixBuffer;
};

#endif