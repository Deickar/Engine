#include "VegetationManager.h"

VegetationManager::VegetationManager()
{
	vegetationShader = 0;
	vertexBuffer = 0;
	instanceBuffer = 0;
	textureArray[0] = 0;
	textureArray[1] = 0;
}

VegetationManager::VegetationManager( const VegetationManager& )
{

}

VegetationManager::~VegetationManager()
{

}

bool VegetationManager::Initialize(ID3D11Device* device, HWND hwnd, WCHAR* filename1, WCHAR* filename2)
{
	HRESULT result;


	result = D3DX11CreateShaderResourceViewFromFile(device, filename1, NULL, NULL, &textureArray[0], NULL);
	if(FAILED(result))
	{
		return false;
	}

	result = D3DX11CreateShaderResourceViewFromFile(device, filename2, NULL, NULL, &textureArray[1], NULL);
	if(FAILED(result))
	{
		return false;
	}

	vegetationShader = new VegetationShader();
	if(!vegetationShader)
	{
		return false;
	}

	result = vegetationShader->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	return true;
}

bool VegetationManager::SetupQuads( ID3D11Device* device, std::vector<XMFLOAT4>* positions )
{
	if(!BuildVertexBuffer(device))
	{
		return false;
	}

	if(!BuildIndexBuffer(device, positions))
	{
		return false;
	}

	return true;
}

void VegetationManager::Shutdown()
{
	// Release the instance buffer.
	if(instanceBuffer)
	{
		instanceBuffer->Release();
		instanceBuffer = 0;
	}

	// Release the vertex buffer.
	if(vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = 0;
	}

	if(textureArray[0])
	{
		textureArray[0]->Release();
		textureArray[0] = 0;
	}

	if(textureArray[1])
	{
		textureArray[1]->Release();
		textureArray[1] = 0;
	}

	if(vegetationShader)
	{
		vegetationShader->Shutdown();
		vegetationShader = 0;
	}
}

bool VegetationManager::Render(ID3D11DeviceContext* deviceContext, XMMATRIX* worldViewProjection, ID3D11ShaderResourceView** textures)
{
	RenderBuffers(deviceContext);

	if(!vegetationShader->Render(deviceContext, vertexCount, instanceCount, worldViewProjection, textures))
	{
		return false;
	}

	return true;
}

void VegetationManager::RenderBuffers( ID3D11DeviceContext* deviceContext)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	//Set the buffer strides.
	strides[0] = sizeof(VertexType); 
	strides[1] = sizeof(InstanceType); 

	//Set the buffer offsets.
	offsets[0] = 0;
	offsets[1] = 0;

	// Set the array of pointers to the vertex and instance buffers.
	bufferPointers[0] = vertexBuffer;	
	bufferPointers[1] = instanceBuffer;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool VegetationManager::BuildVertexBuffer( ID3D11Device* device )
{
	VertexType* vertices;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	HRESULT result;

#pragma region Setting up the vertices

	// Set the number of vertices in the vertex array.
	vertexCount = 18;

	// Create the vertex array.
	vertices = new VertexType[vertexCount];
	if(!vertices)
	{
		return false;
	}

	XMFLOAT2 quad1Left, quad1Right;
	quad1Left = XMFLOAT2(-0.6f, -0.2f);
	quad1Right = XMFLOAT2(0.5f, 0.0f);

	XMFLOAT2 quad2Left, quad2Right;
	quad2Left = XMFLOAT2(0.0f, -0.7f);
	quad2Right = XMFLOAT2(0.1f, 0.3f);

	XMFLOAT2 quad3Left, quad3Right;
	quad3Left = XMFLOAT2(-0.45f, 0.15f);
	quad3Right = XMFLOAT2(0.35f, -0.6f);

	//Quad #1
	vertices[0].position = XMFLOAT3(quad1Left.x, 0.0f, quad1Left.y);  // Bottom left.
	vertices[0].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(quad1Left.x, 1.0f, quad1Left.y);  // Top left.
	vertices[1].texCoord = XMFLOAT2(0.0f, 0.0f);

	vertices[2].position = XMFLOAT3(quad1Right.x, 1.0f, quad1Right.y);  // Top right.
	vertices[2].texCoord = XMFLOAT2(1.0f, 0.0f);

	vertices[3].position = XMFLOAT3(quad1Right.x, 0.0f, quad1Right.y);  // Bottom right.
	vertices[3].texCoord = XMFLOAT2(1.0f, 1.0f);

	vertices[4].position = XMFLOAT3(quad1Left.x, 0.0f, quad1Left.y);  // Bottom left.
	vertices[4].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[5].position = XMFLOAT3(quad1Right.x, 1.0f, quad1Right.y);  // Top right.
	vertices[5].texCoord = XMFLOAT2(1.0f, 0.0f);


	//Quad #2
	vertices[6].position = XMFLOAT3(quad2Left.x, 0.0f, quad2Left.y);  // Bottom left.
	vertices[6].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[7].position = XMFLOAT3(quad2Left.x, 1.0f, quad2Left.y);  // Top left.
	vertices[7].texCoord = XMFLOAT2(0.0f, 0.0f);

	vertices[8].position = XMFLOAT3(quad2Right.x, 1.0f, quad2Right.y);  // Top right.
	vertices[8].texCoord = XMFLOAT2(1.0f, 0.0f);

	vertices[9].position = XMFLOAT3(quad2Right.x, 0.0f, quad2Right.y);  // Bottom right.
	vertices[9].texCoord = XMFLOAT2(1.0f, 1.0f);

	vertices[10].position = XMFLOAT3(quad2Left.x, 0.0f, quad2Left.y);  // Bottom left.
	vertices[10].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[11].position = XMFLOAT3(quad2Right.x, 1.0f, quad2Right.y);  // Top right.
	vertices[11].texCoord = XMFLOAT2(1.0f, 0.0f);


	//Quad #3
	vertices[12].position = XMFLOAT3(quad3Left.x, 0.0f, quad3Left.y);  // Bottom left.
	vertices[12].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[13].position = XMFLOAT3(quad3Left.x, 1.0f, quad3Left.y);  // Top left.
	vertices[13].texCoord = XMFLOAT2(0.0f, 0.0f);

	vertices[14].position = XMFLOAT3(quad3Right.x, 1.0f, quad3Right.y);  // Top right.
	vertices[14].texCoord = XMFLOAT2(1.0f, 0.0f);

	vertices[15].position = XMFLOAT3(quad3Right.x, 0.0f, quad3Right.y);  // Bottom right.
	vertices[15].texCoord = XMFLOAT2(1.0f, 1.0f);

	vertices[16].position = XMFLOAT3(quad3Left.x, 0.0f, quad3Left.y);  // Bottom left.
	vertices[16].texCoord = XMFLOAT2(0.0f, 1.0f);

	vertices[17].position = XMFLOAT3(quad3Right.x, 1.0f, quad3Right.y);  // Top right.
	vertices[17].texCoord = XMFLOAT2(1.0f, 0.0f);

#pragma endregion

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex array now that the vertex buffer has been created and loaded.
	delete [] vertices;
	vertices = 0;

	return true;
}

bool VegetationManager::BuildIndexBuffer( ID3D11Device* device, std::vector<XMFLOAT4>* positions )
{
	InstanceType* instances;
	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA instanceData;
	HRESULT result;

	// Set the number of instances in the array.
	instanceCount = positions->size();

	// Create the instance array.
	instances = new InstanceType[instanceCount];
	if(!instances)
	{
		return false;
	}

	// Load the instance array with data.
	for(unsigned int i = 0; i < positions->size(); i++)
	{
		instances[i].position = positions->at(i); //positions contains an XMFLOAT4 that consists of a float3 position and a float/int that contains texture ID
	}

	// Set up the description of the instance buffer.
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the instance data.
	instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.
	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the instance array now that the instance buffer has been created and loaded.
	delete [] instances;
	instances = 0;

	return true;
}
