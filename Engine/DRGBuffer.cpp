////////////////////////////////////////////////////////////////////////////////
// Filename: bumpmapshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "DRGBuffer.h"

DRGBuffer::DRGBuffer() : SettingsDependent()
{
	//textureScale = tighten = 1.0f;

	//vertexShader = 0;
	//pixelShader = 0;
	//layout = 0;
	//matrixBuffer = 0;
	//pixelFarZBuffer = 0;
	//samplers[0] = 0;
	//samplers[1] = 0;
}


DRGBuffer::DRGBuffer(const DRGBuffer& other)
{
}


DRGBuffer::~DRGBuffer()
{
}

bool DRGBuffer::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//Load settings from file
	InitializeSettings(this);

	// Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, L"../Engine/Shaders/DRGbuffer.vsh", L"../Engine/Shaders/DRGbuffer.psh");
	if(!result)
	{
		return false;
	}

	return true;
}

void DRGBuffer::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool DRGBuffer::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX* worldMatrix, XMMATRIX* viewMatrix, 
	XMMATRIX* projectionMatrix, ID3D11ShaderResourceView* texture, float farZ)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, farZ);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool DRGBuffer::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	CComPtr<ID3D10Blob> errorMessage;
	CComPtr<ID3D10Blob> vertexShaderBuffer;
	CComPtr<ID3D10Blob> pixelShaderBuffer;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "GBufferVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
		0, NULL, &vertexShaderBuffer, &errorMessage.p, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have written something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "GBufferPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
		0, NULL, &pixelShaderBuffer, &errorMessage.p, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have written something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, 
		&vertexShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, 
		&pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType structure in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	//polygonLayout[3].SemanticName = "TANGENT";
	//polygonLayout[3].SemanticIndex = 0;
	//polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//polygonLayout[3].InputSlot = 0;
	//polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	//polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[3].InstanceDataStepRate = 0;

	//polygonLayout[4].SemanticName = "BINORMAL";
	//polygonLayout[4].SemanticIndex = 0;
	//polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	//polygonLayout[4].InputSlot = 0;
	//polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	//polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
		vertexShaderBuffer->GetBufferSize(), &layout);
	if(FAILED(result))
	{
		return false;
	}

	//// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	//vertexShaderBuffer->Release();
	//vertexShaderBuffer = 0;

	//pixelShaderBuffer->Release();
	//pixelShaderBuffer = 0;

	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer.p);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the pixel shader constant buffer that is in the vertex shader.
	variableBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	variableBufferDesc.ByteWidth = sizeof(VariableBuffer);
	variableBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	variableBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	variableBufferDesc.MiscFlags = 0;
	variableBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;

	data.pSysMem = &variables;
	data.SysMemPitch = sizeof(VariableBuffer);
	data.SysMemSlicePitch = 0;


	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&variableBufferDesc, NULL, &variableBuffer.p);
	if(FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &sampler.p);
	if(FAILED(result))
	{
		return false;
	}

	//// Create a texture sampler state description.
	//samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.MipLODBias = 0.0f;
	//samplerDesc.MaxAnisotropy = 1;
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	//samplerDesc.BorderColor[0] = 0;
	//samplerDesc.BorderColor[1] = 0;
	//samplerDesc.BorderColor[2] = 0;
	//samplerDesc.BorderColor[3] = 0;
	//samplerDesc.MinLOD = 1;
	//samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//// Create the texture sampler state.
	//result = device->CreateSamplerState(&samplerDesc, &samplers[1].p);
	//if(FAILED(result))
	//{
	//	return false;
	//}

	return true;
}

void DRGBuffer::ShutdownShader()
{
	//// Release the sampler state.
	//if(samplers[0])
	//{
	//	samplers[0]->Release();
	//	samplers[0] = 0;
	//}
	//if(samplers[1])
	//{
	//	samplers[1]->Release();
	//	samplers[1] = 0;
	//}

	//// Release the matrix constant buffer.
	//if(matrixBuffer)
	//{
	//	matrixBuffer->Release();
	//	matrixBuffer = 0;
	//}

	//if(pixelFarZBuffer)
	//{
	//	pixelFarZBuffer->Release();
	//	pixelFarZBuffer = 0;
	//}

	//// Release the layout.
	//if(layout)
	//{
	//	layout->Release();
	//	layout = 0;
	//}

	//// Release the pixel shader.
	//if(pixelShader)
	//{
	//	pixelShader->Release();
	//	pixelShader = 0;
	//}

	//// Release the vertex shader.
	//if(vertexShader)
	//{
	//	vertexShader->Release();
	//	vertexShader = 0;
	//}

	return;
}

void DRGBuffer::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool DRGBuffer::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX* worldMatrix, 
	XMMATRIX* viewMatrix, XMMATRIX* projectionMatrix, ID3D11ShaderResourceView* texture, float farZ)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	VariableBuffer* dataPtr2;
	unsigned int bufferNumber;

	///////////// #1

	// Lock the matrix constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->World = *worldMatrix;
	dataPtr->View = *viewMatrix;
	dataPtr->Projection = *projectionMatrix;

	// Unlock the matrix constant buffer.
	deviceContext->Unmap(matrixBuffer, 0);

	// Set the position of the matrix constant buffer in the vertex sh�ader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer.p);

	///////////// #2

	if(bufferNeedsUpdate)
	{
		// Lock the light constant buffer so it can be written to.
		result = deviceContext->Map(variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if(FAILED(result))
		{
			return false;
		}

		// Get a pointer to the data in the constant buffer.
		dataPtr2 = (VariableBuffer*)mappedResource.pData;

		// Copy the lighting variables into the constant buffer.
		dataPtr2->farClip		= variables.farClip;
		dataPtr2->textureScale	= variables.textureScale;
		dataPtr2->tighten		= variables.tighten;

		// Unlock the constant buffer.
		deviceContext->Unmap(variableBuffer, 0);

		bufferNeedsUpdate = false;
	}

	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &variableBuffer.p);

	//Assort shit.

	// Set shader texture array resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

void DRGBuffer::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(layout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &sampler.p);

	// Render the triangles.
	deviceContext->DrawIndexed(indexCount, 0, 0);


	return;
}

void DRGBuffer::OnSettingsReload( Config* cfg )
{
	bufferNeedsUpdate = true;

	const Setting& settings = cfg->getRoot()["shaders"]["gbufferModel"];

	settings.lookupValue("tighten", variables.tighten);
	settings.lookupValue("textureScale", variables.textureScale);

	const Setting& settings2 = cfg->getRoot()["rendering"];

	settings2.lookupValue("farClip", variables.farClip);
}
