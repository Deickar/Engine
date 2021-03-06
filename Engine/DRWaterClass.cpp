#include "DRWaterClass.h"

DRWaterClass::DRWaterClass() : SettingsDependent()
{
}


DRWaterClass::~DRWaterClass()
{
}

bool DRWaterClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//Load settings from file
	InitializeSettings(this);

	// Initialize the vertex and pixel shaders.
	//result = InitializeShader(device, hwnd, L"../Engine/Shaders/WaterShader.vsh", L"../Engine/Shaders/WaterShader.gsh", L"../Engine/Shaders/WaterShader.psh");
	result = InitializeShader(device, hwnd, L"../Engine/Shaders/WaterAnimationVertexShader.vsh", L"CURRENTLY UNUSED!", L"../Engine/Shaders/WaterShader.psh");
	if(!result)
	{
		return false;
	}

	return true;
}

void DRWaterClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool DRWaterClass::Render( ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX* worldMatrix, XMMATRIX* worldViewMatrix, XMMATRIX* worldViewProjection, 
							ID3D11ShaderResourceView** waterTexture, ID3D11ShaderResourceView** offsetNoiseTexture, 
							ID3D11ShaderResourceView** offsetNoiseNormalTexture, float deltaTime, XMFLOAT3* windDirection )
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, worldViewMatrix, worldViewProjection, waterTexture, offsetNoiseTexture, offsetNoiseNormalTexture, deltaTime, windDirection);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool DRWaterClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* gsFilename, WCHAR* psFilename)
{
	HRESULT result;
	CComPtr<ID3D10Blob> errorMessage;
	CComPtr<ID3D10Blob> vertexShaderBuffer;
	CComPtr<ID3D10Blob> geometryShaderBuffer;
	CComPtr<ID3D10Blob> pixelShaderBuffer;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

		result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "WaterAnimationVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
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


	//// Compile the vertex shader code.
	//result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "WaterShaderVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
	//	0, NULL, &vertexShaderBuffer, &errorMessage.p, NULL);
	//if(FAILED(result))
	//{
	//	// If the shader failed to compile it should have written something to the error message.
	//	if(errorMessage)
	//	{
	//		OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
	//	}
	//	// If there was  nothing in the error message then it simply could not find the shader file itself.
	//	else
	//	{
	//		MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	//// Compile the geometry shader code.
	//result = D3DX11CompileFromFile(gsFilename, NULL, NULL, "WaterShaderGS", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
	//	0, NULL, &geometryShaderBuffer, &errorMessage.p, NULL);
	//if(FAILED(result))
	//{
	//	// If the shader failed to compile it should have written something to the error message.
	//	if(errorMessage)
	//	{
	//		OutputShaderErrorMessage(errorMessage, hwnd, gsFilename);
	//	}
	//	// If there was  nothing in the error message then it simply could not find the shader file itself.
	//	else
	//	{
	//		MessageBox(hwnd, gsFilename, L"Missing Shader File", MB_OK);
	//	}

	//	return false;
	//}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "WaterShaderPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 
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

	//result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), NULL, 
	//	&geometryShader);
	//if(FAILED(result))
	//{
	//	return false;
	//}

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

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
		vertexShaderBuffer->GetBufferSize(), &layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
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

	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(PixelMatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &pixelMatrixBuffer.p);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(WaterAnimationVertexBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &animationVariableBuffer.p);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the matrix dynamic constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(GeometryVariableBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the matrix constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &variableBuffer.p);
	if(FAILED(result))
	{
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &sampler);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

void DRWaterClass::ShutdownShader()
{

	return;
}

void DRWaterClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

	//// Release the error message.
	//errorMessage->Release();
	//errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool DRWaterClass::SetShaderParameters( ID3D11DeviceContext* deviceContext, XMMATRIX* worldMatrix, XMMATRIX* worldViewMatrix, 
	XMMATRIX* worldViewProjection, ID3D11ShaderResourceView** waterTexture, ID3D11ShaderResourceView** offsetNoiseTexture, 
	ID3D11ShaderResourceView** offsetNoiseNormalTexture, float deltaTime, XMFLOAT3* windDirection)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//MatrixBufferType* dataPtr;
	//GeometryVariableBuffer* dataPtr2;
	WaterAnimationVertexBuffer* dataPtr1;
	PixelMatrixBuffer* dataPtr3;
	unsigned int bufferNumber;

	//////////////////////////////////////////////////////////////////////////// #1

	//// Lock the matrix constant buffer so it can be written to.
	//result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if(FAILED(result))
	//{
	//	return false;
	//}

	//// Get a pointer to the data in the constant buffer.
	//dataPtr = (MatrixBufferType*)mappedResource.pData;

	//// Copy the matrices into the constant buffer.
	//dataPtr->World = *worldMatrix;
	//dataPtr->WorldView = *worldViewMatrix;
	//dataPtr->WorldViewProjection = *worldViewProjection;

	//// Unlock the matrix constant buffer.
	//deviceContext->Unmap(matrixBuffer, 0);

	//// Set the position of the matrix constant buffer in the vertex sh�ader.
	//bufferNumber = 0;

	//// Now set the matrix constant buffer in the vertex shader with the updated values.
	//deviceContext->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer.p);

	//////////////////////////////////////////////////////////////////////////// #2

	//// Lock the matrix constant buffer so it can be written to.
	//result = deviceContext->Map(variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//if(FAILED(result))
	//{
	//	return false;
	//}

	//// Get a pointer to the data in the constant buffer.
	//dataPtr2 = (GeometryVariableBuffer*)mappedResource.pData;

	//// Copy the matrices into the constant buffer.
	//dataPtr2->WorldViewProjection		= *worldViewProjection;
	//dataPtr2->DeltaTime					= deltaTime;
	//dataPtr2->farClip 					= variables.farClip;
	//dataPtr2->heightScaling				= variables.heightScaling;
	//dataPtr2->positionSamplingOffset	= variables.positionSamplingOffset;
	//dataPtr2->windDirection				= XMFLOAT3(-0.2f, 0.0f, 0.8f); //*windDirection
	//dataPtr2->timeScaling				= variables.timeScaling;
	//dataPtr2->waterHeight				= variables.waterHeight;

	//// Unlock the matrix constant buffer.
	//deviceContext->Unmap(variableBuffer, 0);

	//// Set the position of the matrix constant buffer in the pixel sh�ader.
	//bufferNumber = 0;

	//// Now set the matrix constant buffer in the vertex shader with the updated values.
	//deviceContext->GSSetConstantBuffers(bufferNumber, 1, &variableBuffer.p);
	
	// Lock the matrix constant buffer so it can be written to.

	result = deviceContext->Map(animationVariableBuffer.p, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr1 = (WaterAnimationVertexBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr1->World						= *worldMatrix;
	dataPtr1->WorldView					= *worldViewMatrix;
	dataPtr1->WorldViewProjection		= *worldViewProjection;
	dataPtr1->DeltaTime					= deltaTime;
	dataPtr1->farClip 					= variables.farClip;
	dataPtr1->heightScaling				= variables.heightScaling;
	dataPtr1->positionSamplingOffset	= variables.positionSamplingOffset;
	dataPtr1->windDirection				= XMFLOAT3(-0.2f, 0.0f, 0.8f); //*windDirection
	dataPtr1->timeScaling				= variables.timeScaling;
	dataPtr1->waterHeight				= variables.waterHeight;
	dataPtr1->largeWaveHeightOffset		= animationVariables.largeWaveHeightOffset;
	dataPtr1->largeWavePositionSamplingOffset = animationVariables.largeWavePositionSamplingOffset;
	dataPtr1->largeWaveTimeSamplingOffset = animationVariables.largeWaveTimeSamplingOffset;

	// Unlock the matrix constant buffer.
	deviceContext->Unmap(animationVariableBuffer, 0);

	// Set the position of the matrix constant buffer in the pixel sh�ader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &animationVariableBuffer.p);


	////////////////////////////////////////////////////////////////////////// #3

	// Lock the matrix constant buffer so it can be written to.

	result = deviceContext->Map(pixelMatrixBuffer.p, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (PixelMatrixBuffer*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr3->World = *worldMatrix;
	dataPtr3->waterColorMultiplier = waterColorMultiplier;
	dataPtr3->waterColorStartOffset = waterColorStartOffset;

	// Unlock the matrix constant buffer.
	deviceContext->Unmap(pixelMatrixBuffer, 0);

	// Set the position of the matrix constant buffer in the pixel sh�ader.
	bufferNumber = 0;

	// Now set the matrix constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &pixelMatrixBuffer.p);

	//Set shader texture array resource in the vertex shader.
	deviceContext->VSSetShaderResources(0, 1, offsetNoiseTexture);

	//Set pixel shader resource
	deviceContext->PSSetShaderResources(0, 1, offsetNoiseNormalTexture);
	deviceContext->PSSetShaderResources(1, 1, waterTexture);

	return true;
}

void DRWaterClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(layout);

	// Set the shaders that will be used to render this triangle.
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	//deviceContext->GSSetShader(geometryShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	
	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &sampler.p);

	// ... And in the geometry shader
	deviceContext->VSSetSamplers(0, 1, &sampler.p);

	// Render the triangles.
	deviceContext->DrawIndexed(indexCount, 0, 0);
	
	//And reset geometry shader again
	//deviceContext->GSSetShader(NULL, NULL, 0);

	return;
}

void DRWaterClass::OnSettingsReload(Config* cfg)
{
	bufferNeedsUpdating = true;

	variables.positionSamplingOffset	= 0.2f;
	variables.heightScaling				= 0.3f;
	variables.timeScaling				= 0.2f;
	waterColorStartOffset				= 0.2f;
	waterColorMultiplier				= 0.8f;

	Setting& settings = cfg->getRoot()["shaders"]["waterShader"];

	settings.lookupValue("positionSamplingOffset",	variables.positionSamplingOffset	);
	settings.lookupValue("heightScaling",			variables.heightScaling				);
	settings.lookupValue("timeScaling",				variables.timeScaling				);
	settings.lookupValue("waterColorStartOffset",	waterColorStartOffset				);
	settings.lookupValue("waterColorMultiplier",	waterColorMultiplier				);
	settings.lookupValue("waterLevels",				variables.waterHeight				);

	settings.lookupValue("largeWavePositionSamplingOffset",				animationVariables.largeWavePositionSamplingOffset);
	settings.lookupValue("largeWaveTimeSamplingOffset",					animationVariables.largeWaveTimeSamplingOffset);
	settings.lookupValue("largeWaveHeightOffset",						animationVariables.largeWaveHeightOffset);

	const Setting& settings2 = cfg->getRoot()["rendering"];

	settings2.lookupValue("farClip", variables.farClip);
}