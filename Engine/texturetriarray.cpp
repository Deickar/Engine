////////////////////////////////////////////////////////////////////////////////
// Filename: TextureTriArray.cpp
////////////////////////////////////////////////////////////////////////////////
#include "TextureTriArray.h"

ID3D11ShaderResourceView** TextureTriArray::GetTextureArray()
{
	return textures;
}

TextureTriArray::TextureTriArray()
{
	textures[0] = 0;
	textures[1] = 0;
	textures[2] = 0;
}


TextureTriArray::TextureTriArray(const TextureTriArray& other)
{
}


TextureTriArray::~TextureTriArray()
{
}

bool TextureTriArray::Initialize(ID3D11Device* device, WCHAR* filename1, WCHAR* filename2, WCHAR* filename3)
{
	HRESULT result;


	// Load the first texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename1, NULL, NULL, &textures[0], NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Load the second texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename2, NULL, NULL, &textures[1], NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Load the third texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename3, NULL, NULL, &textures[2], NULL);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void TextureTriArray::Shutdown()
{
	// Release the texture resources.
	if(textures[0])
	{
		textures[0]->Release();
		textures[0] = 0;
	}

	if(textures[1])
	{
		textures[1]->Release();
		textures[1] = 0;
	}

	if(textures[2])
	{
		textures[2]->Release();
		textures[2] = 0;
	}

	return;
}