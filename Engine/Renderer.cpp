////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Renderer.h"

/*
THE LINK DUNGEON

Geometry shader quads:
http://www.braynzarsoft.net/index.php?p=D3D11BILLBOARDS
http://rastergrid.com/blog/2010/02/instance-culling-using-geometry-shaders/

Linear depth:
http://www.gamerendering.com/2008/09/28/linear-depth-texture/
http://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/

Inf�r cleana upp kod:
http://gamedev.stackexchange.com/questions/24615/managing-shaders-and-objects-in-directx-11
https://graphics.stanford.edu/wikis/cs448s-11/FrontPage?action=AttachFile&do=get&target=05-GPU_Arch_I.pdf


Directional light lens flare:
http://www.madgamedev.com/post/2010/04/21/Article-Sun-and-Lens-Flare-as-a-Post-Process.aspx
http://stackoverflow.com/questions/14161727/hlsl-drawing-a-centered-circle
if cross product (cameraDirection, lightDirection) == 0 then they're both facing the same way? I think.

Multithreading:
http://gamedev.stackexchange.com/questions/2116/multi-threaded-game-engine-design-resources
http://www.gamasutra.com/view/feature/1830/multithreaded_game_engine_.php
http://www.gamasutra.com/view/feature/2463/threading_3d_game_engine_basics.php
http://bitsquid.blogspot.se/2010/03/task-management-practical-example.html
*/

Renderer::Renderer()
{
	d3D = 0;
	camera  = 0;
	text = 0;

	frustum = 0;
	gbufferShader = 0;
	textureShader = 0;
	pointLightShader = 0;

	vertexOnlyShader = 0;
	depthOnlyShader = 0;
	composeShader = 0;
	gaussianBlurShader = 0;

	dirLightShader = 0;
	dirLight = 0;

	sphereModel = 0;
	skySphere = 0;
	vegetationManager = 0;
	depthOnlyQuadShader = 0;

	colorRT = 0;
	normalRT = 0;
	depthRT = 0;
	shadowRT = 0;
	lightRT = 0;
	gaussianBlurPingPongRT = 0;

	mcubeShader = 0;
	marchingCubes = 0;
	mcTerrain = 0;

	utility = 0;
	textureAndMaterialHandler = 0;

	lSystemSRV = 0;
	ssaoRandomTextureSRV = 0;

	timeOfDay = 0.0f;
	timer = 10.0f;
	lodState = 0;
	previousLodState = 0;

	toggleTextureShader = false;
	returning = false;
	toggleCameraPointLight = false;
	toggleOtherPointLights = false;
}


Renderer::Renderer(const Renderer& other)
{
}


Renderer::~Renderer()
{
	noise->~SimplexNoise();
	delete noise;
	noise = 0;
}


bool Renderer::Initialize(HWND hwnd, CameraClass* camera, InputClass* input, D3DClass* d3D, UINT screenWidth, UINT screenHeight,
	UINT shadowmapWidth, UINT shadowmapHeight, float screenFar, float screenNear, bool toggleDebug)
{
	srand((unsigned int)time(NULL));
	bool result;

	this->toggleDebugInfo = toggleDebug;
	this->inputManager = input;
	this->shadowMapWidth = shadowmapWidth;
	this->shadowMapHeight = shadowmapHeight;
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->screenFar = screenFar;
	this->screenNear = screenNear;
	this->d3D = d3D;
	this->camera = camera;

	toggleSSAO = 1;
	toggleColorMode = 1;
	fogMinimum = 1.0f;

	utility = new Utility();

	XMStoreFloat4x4(&baseViewMatrix, camera->GetView());

	ID3D11Device* device = d3D->GetDevice();

	result = InitializeShaders(hwnd, device);
	if(!result)
	{
		return false;
	}

	result = InitializeLights(hwnd, device);
	if(!result)
	{
		return false;
	}

	result = InitializeModels(hwnd, device);
	if(!result)
	{
		return false;
	}

	result = InitializeEverythingElse(hwnd, device);
	if(!result)
	{
		return false;
	}

	device = 0;

	return true;
}

bool Renderer::InitializeShaders(HWND hwnd, ID3D11Device* device)
{
	bool result;

	gbufferShader = new DRGBuffer();
	if(!gbufferShader)
	{
		return false;
	}

	result = gbufferShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"GBuffer shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	pointLightShader = new DRPointLight();
	if(!pointLightShader)
	{
		return false;
	}

	result = pointLightShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Pointlight shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	vertexOnlyShader = new VertexShaderOnly();
	if(!vertexOnlyShader)
	{
		return false;
	}

	result = vertexOnlyShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Vertex only shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	depthOnlyShader = new DepthOnlyShader();
	if(!vertexOnlyShader)
	{
		return false;
	}

	result = depthOnlyShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Depth only shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	depthOnlyQuadShader = new DepthOnlyQuadShader();
	if(!depthOnlyQuadShader)
	{
		return false;
	}

	result = depthOnlyQuadShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Depth only quad shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	composeShader = new DRCompose();
	if(!composeShader)
	{
		return false;
	}

	result = composeShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Compose shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	gaussianBlurShader = new GaussianBlur();
	if(!gaussianBlurShader)
	{
		return false;
	}

	result = gaussianBlurShader->Initialize(device, hwnd);



	textureShader = new TextureShaderClass();
	if(!textureShader)
	{
		return false;
	}

	result = textureShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Texture shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	mcubeShader = new MCubesGBufferShader();
	if(!mcubeShader)
	{
		return false;
	}

	result = mcubeShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Marching cubes gbuffer shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Renderer::InitializeLights(HWND hwnd, ID3D11Device* device)
{
	bool result;

#pragma region Point light initialization
	float x, y, z;
	//x = 2.0f;
	//z = 2.0f;
	//y = 40.0f;

	for(int i = 0; i < 300; i++)
	{
		x = utility->RandomFloat();
		y = utility->RandomFloat();
		z = utility->RandomFloat();

		pointLights.push_back(new PointLight());
		pointLights[i]->Color = XMFLOAT3(x, y, z);
		pointLights[i]->Position = XMFLOAT3(2.0f+(utility->RandomFloat() * 176.0f), 40.0f, 2.0f + (utility->RandomFloat() * 176.0f));
		pointLights[i]->Radius = 3.0f; //Used to both scale the actual point light model and is a factor in the attenuation
		pointLights[i]->Intensity = 1.0f; //Is used to control the attenuation

		//x += 12.0f;

		//if(x > 60.0f) //Every 10th light gets reseted in x and z plane.
		//{
		//	x = 0.0f;
		//	z += 6.0f;
		//}

		//if(i != 0 && i % 100 == 0) //Every 100 pointlights we reset and make another layer that is (y+8) higher up.
		//{
		//	x = -10.0f;
		//	z = -10.0f;
		//	y += 8.0f;
		//}

		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);
		XMMATRIX tempTranslation = XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y, pointLights[i]->Position.z);
		XMStoreFloat4x4(&pointLights[i]->World, XMMatrixTranspose(tempScale * tempTranslation));
	}

	cameraPointLight.Color = XMFLOAT3(0.9f, 0.9f, 0.2f);
	cameraPointLight.Position = camera->GetPosition();
	cameraPointLight.Radius = 30.0f; //Used to both scale the actual point light model and is a factor in the attenuation
	cameraPointLight.Intensity = 1.0f; //Is used to control the attenuation

#pragma endregion

#pragma region Directional light initialization
	// Create the directional light.
	dirLight = new DirLight();
	if(!dirLight)
	{
		return false;
	}

	dirLightShader = new DRDirLight();
	if(!dirLightShader)
	{
		return false;
	}

	result = dirLightShader->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Dir light shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);//LookAt for dir light. We always want thi0s to be (0,0,0), because it's the easiest to visualize.
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	// Initialize the directional light.
	dirLight->Color = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight->Intensity = 1.0f;
	dirLight->Position = XMFLOAT3(150.0f, 0.0f, 0.0f);

	XMVECTOR direction = XMVector3Normalize(lookAt - XMLoadFloat3(&dirLight->Position));
	XMStoreFloat3(&dirLight->Direction, direction);

	//XMStoreFloat4x4(&dirLight->Projection, XMMatrixPerspectiveFovLH(((float)D3DX_PI/2.0f), 1.0f, 10.0f, 300.0f)); //Generate perspective light projection matrix and store it as float4x4
	XMStoreFloat4x4(&dirLight->Projection, XMMatrixOrthographicLH(200.0f, 200.0f, 5.0f, 200.0f)); //Generate orthogonal light projection matrix and store it as float4x4

	XMStoreFloat4x4(&dirLight->View, XMMatrixLookAtLH(XMLoadFloat3(&dirLight->Position), lookAt, up)); //Generate light view matrix and store it as float4x4.
#pragma endregion

	return true;
}

bool Renderer::InitializeModels(HWND hwnd, ID3D11Device* device)
{
	bool result;

	metaBalls = new MetaballsClass();
	marchingCubes = new MarchingCubesClass(0.0f, 0.0f, 0.0f, 180.0f, 180.0f, 180.0f, 1.0f, 1.0f, 1.0f);
	marchingCubes->SetMetaBalls(metaBalls, 0.2f);
	marchingCubes->GetTerrain()->SetTerrainType(2);


	marchingCubes->GetTerrain()->Noise3D();
	marchingCubes->CalculateMesh(device);


	lSystem = new LSystemClass();
	lSystem->initialize();

	skySphere = new Skysphere();
	if(!skySphere)
	{
		return false;
	}

	result = skySphere->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	vegetationManager = new VegetationManager();
	if(!vegetationManager)
	{
		return false;
	}

	result = vegetationManager->Initialize(device, hwnd);
	if(!result)
	{
		return false;
	}

	GenerateVegetation(device, true);

	// Create the model object.
	sphereModel = new ModelClass();
	if(!sphereModel)
	{
		return false;
	}

	// Initialize the model object. It really doesn't matter what textures it has because it's only used for point light volume culling.
	result = sphereModel->Initialize(device, "../Engine/data/skydome.txt", L"../Engine/data/grass.dds", L"../Engine/data/dirt.dds", L"../Engine/data/rock.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool Renderer::InitializeEverythingElse(HWND hwnd, ID3D11Device* device)
{
	bool result;

	textureAndMaterialHandler = new TextureAndMaterialHandler();
	if(!textureAndMaterialHandler)
	{
		return false;
	}

	result = textureAndMaterialHandler->Initialize(device, d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	noise = new SimplexNoise();
	if(!noise)
	{
		return false;
	}

	if(FAILED(textureAndMaterialHandler->CreateSimplex2DTexture(device, d3D->GetDeviceContext(), noise, &lSystemSRV)))
	{
		return false;
	}

	if(FAILED(textureAndMaterialHandler->CreateRandom2DTexture(d3D->GetDevice(), d3D->GetDeviceContext(), &ssaoRandomTextureSRV)))
	{
		return false;
	}

	dayNightCycle = new DayNightCycle();
	if(!dayNightCycle)
	{
		return false;
	}

	result = dayNightCycle->Initialize(300.0f, DAY); //86400.0f/6
	if(!result)
	{
		return false;
	}

	dayNightCycle->Update(50.0f, dirLight, skySphere);

	// Create the text object.
	text = new TextClass();
	if(!text)
	{
		return false;
	}

	// Initialize the text object.
	result = text->Initialize(device, d3D->GetDeviceContext(), hwnd, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object. Look in graphicsclass.", L"Error", MB_OK);
		return false;
	}

	for(int i = 0; i < 6; i++)
	{
		debugWindows[i].Initialize(device, screenWidth, screenHeight, 200, 200);
	}

	fullScreenQuad.Initialize(device, screenWidth, screenHeight, screenWidth, screenHeight);

	colorRT = new RenderTarget2D();
	normalRT = new RenderTarget2D();
	depthRT = new RenderTarget2D();
	shadowRT = new RenderTarget2D();
	lightRT = new RenderTarget2D();
	gaussianBlurPingPongRT = new RenderTarget2D();

	colorRT->Initialize(device, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	normalRT->Initialize(device, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	depthRT->Initialize(device, screenWidth, screenHeight, DXGI_FORMAT_R32_FLOAT);
	lightRT->Initialize(device, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	shadowRT->Initialize(device, shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R32G32_FLOAT);
	gaussianBlurPingPongRT->Initialize(device, shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R32G32_FLOAT); //Needs to be identical to shadowRT

	// Create the frustum object.
	frustum = new FrustumClass;
	if(!frustum)
	{
		return false;
	}

	frustum->SetInternals(((float)D3DX_PI/2.0f), 1.0f, 0.5f, 150.0f);
	testBoundingbox = Lemmi2DAABB(XMFLOAT2(0, 0), XMFLOAT2(60, 60));

	return true;
}

bool Renderer::Update(HWND hwnd, int fps, int cpu, float frameTime, float seconds)
{
	bool result;

	timer += seconds;

	if(inputManager->WasKeyPressed(DIK_Q))
	{
		toggleDebugInfo = !toggleDebugInfo;
	}

	result = text->SetFps(fps, d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	result = text->SetCpu(cpu, d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	XMFLOAT3 temp = camera->GetPosition();
	result = text->SetCameraPosition((int)temp.x, (int)temp.y, (int)temp.z, d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	temp = camera->GetRotation();
	result = text->SetCameraRotation((int)temp.x, (int)temp.y, (int)temp.z, d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}
	
	//Move all point lights upward
	if(inputManager->IsKeyPressed(DIK_R))
	{
		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);

		for(int i = 0; i < (int)(pointLights.size()-1); i++)
		{
			pointLights[i]->Position.y += frameTime*0.01f;

			XMStoreFloat4x4(&pointLights[i]->World, XMMatrixTranspose(tempScale*XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y-0.5f, pointLights[i]->Position.z)));
		}
	}

	//Move all point lights downwarf
	if(inputManager->IsKeyPressed(DIK_F))
	{
		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);

		for(int i = 0; i < (int)(pointLights.size()-1); i++)
		{
			pointLights[i]->Position.y -= frameTime*0.01f;

			XMStoreFloat4x4(&pointLights[i]->World, XMMatrixTranspose(tempScale*XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y-0.5f, pointLights[i]->Position.z)));
		}
	}

	if(inputManager->WasKeyPressed(DIK_U))
	{
		if(FAILED(textureAndMaterialHandler->CreateRandom2DTexture(d3D->GetDevice(), d3D->GetDeviceContext(), &ssaoRandomTextureSRV)))
		{
			return false;
		}
	}

	if(inputManager->WasKeyPressed(DIK_I))
	{
		if(FAILED(textureAndMaterialHandler->CreateMirroredSimplex2DTexture(d3D->GetDevice(), d3D->GetDeviceContext(), noise, &lSystemSRV)))
		{
			return false;
		}
	}

	if(inputManager->WasKeyPressed(DIK_O))
	{
		if(FAILED(textureAndMaterialHandler->CreateSimplex2DTexture(d3D->GetDevice(), d3D->GetDeviceContext(), noise, &lSystemSRV)))
		{
			return false;
		}

	}

	if(inputManager->WasKeyPressed(DIK_P))
	{
		//Create and initialize our time... things.
		const time_t timeObject = time(NULL);
		struct tm parts;
		localtime_s(&parts, &timeObject );

		std::ostringstream stringStream;

		//Create the string that will hold the screenshot's name when it gets pooped out into the directory
		stringStream << "SavedTexture_" << (1+parts.tm_mon) << "-" << parts.tm_mday <<  "-" << parts.tm_min << "-" << parts.tm_sec << ".bmp";

		LPCSTR fileName;
		string temp = stringStream.str();
		fileName = (temp).c_str();


		//Call save-to-hdd function. If it returns false we break the update loop and the game dies hard.
		if(!textureAndMaterialHandler->SaveLTreeTextureToFile(d3D->GetDeviceContext(), D3DX11_IFF_BMP, fileName))
		{
			MessageBox(NULL, L"Could not save random texture to hdd. Look in textureAndMaterialHandler.SaveLTreeTextureToFile()", L"Error", MB_OK);
			return false;
		}
	}

	if(inputManager->IsKeyPressed(DIK_1))
	{
		seconds = timeOfDay += frameTime;
	}

	if(inputManager->WasKeyPressed(DIK_2))
	{
		toggleColorMode++;

		if(toggleColorMode > 1)
		{
			toggleColorMode = 0;
		}
	}

	if(inputManager->WasKeyPressed(DIK_3))
	{
		fogMinimum -= 0.1f;

		if(fogMinimum < 0.2f)
		{
			fogMinimum = 0.2f;
		}
	}

	if(inputManager->WasKeyPressed(DIK_4))
	{                  
		fogMinimum *= 1.1f;

		if(fogMinimum > 1.0f)
		{
			fogMinimum = 1.0f;
		}

		//fogMinimum = max(150.0f, fogMinimum);
	}

	if(inputManager->WasKeyPressed(DIK_5))
	{
		toggleOtherPointLights = !toggleOtherPointLights;
	}

	if(inputManager->WasKeyPressed(DIK_6))
	{
		toggleCameraPointLight = !toggleCameraPointLight;

	}

	if(inputManager->WasKeyPressed(DIK_7))
	{
		toggleSSAO++;

		if(toggleSSAO > 3)
		{
			toggleSSAO = 0;
		}
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD0))
	{
		marchingCubes->GetTerrain()->PulvirizeWorldToggle();
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD1))
	{
		marchingCubes->GetTerrain()->SetTerrainType(1);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD2))
	{
		marchingCubes->GetTerrain()->SetTerrainType(2);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD3))
	{
		marchingCubes->GetTerrain()->SetTerrainType(3);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD4))
	{
		marchingCubes->GetTerrain()->SetTerrainType(4);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD5))
	{
		marchingCubes->GetTerrain()->SetTerrainType(5);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD6))
	{
		marchingCubes->GetTerrain()->SetTerrainType(6);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD7))
	{
		marchingCubes->GetTerrain()->SetTerrainType(7);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD8))
	{
		marchingCubes->GetTerrain()->SetTerrainType(8);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD9))
	{
		marchingCubes->GetTerrain()->SetTerrainType(9);
	}


#pragma region LOD stuff
	//Distance between camera and middle of mcube chunk. We'll have to do this for each chunk, and keep an individual lodState for each chunk.
	if(timer >= 0.2f)
	{
		int distance = (int)utility->VectorDistance(camera->GetPosition(), XMFLOAT3(30.0f, 40.0f, 30.0f));

		//if(distance <= 100)
		//{
		//	lodState = 3;
		//}
		//else if(distance <= 150)
		//{
		//	lodState = 2;
		//}
		//else if(distance <= 200)
		//{
		//	lodState = 1;
		//}
		//else
		//{
		//	lodState = 0;
		//}
		lodState = 3;

		timer = 0.0f;
	}

	//If the lod state has changed since last update, switch and rebuild vegetation instance buffers
	if(lodState != previousLodState)
	{
		switch (lodState)
		{
		case 0:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector500);
			break;

		case 1:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector2500);
			break;

		case 2:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector5000);
			break;

		case 3:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector10000);
			break;
		}
	}


#pragma endregion

#pragma region Generate new marching cubes world
	if(inputManager->WasKeyPressed(DIK_N))
	{
		previousLodState = 0; //We set previous lod state to something != lodState so that it'll trigger an instancebuffer rebuild
		lodState = 3;

		marchingCubes->Reset();
		marchingCubes->GetTerrain()->Noise3D();
		marchingCubes->CalculateMesh(d3D->GetDevice());

		GenerateVegetation(d3D->GetDevice(), false);

		switch (lodState)
		{
		case 0:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector500);
			break;

		case 1:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector2500);
			break;

		case 2:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector5000);
			break;

		case 3:
			vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector10000);
		}
	}
#pragma endregion

	timeOfDay = dayNightCycle->Update(seconds, dirLight, skySphere);

	XMVECTOR lookAt = XMLoadFloat3(&camera->GetPosition())+(camera->ForwardVector()*3.0f);//XMVectorSet(30.0f, 20.0f, 30.0f, 1.0f);//XMLoadFloat3(&camera->GetPosition());//XMLoadFloat3(&camera->GetPosition());//XMLoadFloat3(&camera->GetPosition())+(camera->ForwardVector()*30.0f);//XMLoadFloat3(&camera->GetPosition());//
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR currentLightPos = (XMLoadFloat3(&camera->GetPosition())+XMLoadFloat3(&dirLight->Position));//XMLoadFloat3(&camera->GetPosition())-(camera->ForwardVector()*30.0f);

	XMStoreFloat3(&dirLight->Direction, XMVector3Normalize((lookAt - currentLightPos)));//XMLoadFloat3(&dirLight->Position)
	XMStoreFloat4x4(&dirLight->View, XMMatrixLookAtLH(currentLightPos, lookAt, up)); //Generate light view matrix


	//Camera point light updates
	XMMATRIX tempScale = XMMatrixScaling(cameraPointLight.Radius, cameraPointLight.Radius, cameraPointLight.Radius);
	XMStoreFloat3(&cameraPointLight.Position, XMLoadFloat3(&camera->GetPosition())+(camera->ForwardVector()*6.0f));
	//pointLights[501]->Position.y = ;camera->GetPosition().y;

	XMStoreFloat4x4(&cameraPointLight.World, XMMatrixTranspose(tempScale*XMMatrixTranslation(cameraPointLight.Position.x, cameraPointLight.Position.y, cameraPointLight.Position.z)));


	previousLodState = lodState;

	return true;
}

bool Renderer::Render()
{
	// Clear the scene.
	d3D->BeginScene(0.0f, 0.0f, 0.0f, 0.0f);

#pragma region Preparation
	ID3D11DeviceContext* context;
	context = d3D->GetDeviceContext();

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, scalingMatrix, viewProjection, invertedViewProjection, invertedView, 
		lightView, lightProj, lightViewProj, baseView, worldBaseViewOrthoProj, identityWorldViewProj, lightWorldViewProj, 
		invertedProjection, untransposedViewProj, worldView, lightWorldView;

	XMFLOAT3 camPos, camDir;
	bool result;
	ID3D11RenderTargetView* gbufferRenderTargets[3] = { NULL, NULL, NULL }; //render targets for GBuffer pass
	ID3D11RenderTargetView* lightTarget[1] = { NULL };
	ID3D11RenderTargetView* shadowTarget[1] = { NULL };
	ID3D11RenderTargetView* gaussianBlurPingPongRTView[1] = { NULL };

	ID3D11ShaderResourceView* gbufferTextures[3] = { NULL, NULL, NULL };
	ID3D11ShaderResourceView* dirLightTextures[4] = { NULL, NULL, NULL, NULL };
	ID3D11ShaderResourceView* finalTextures[4] = { NULL, NULL, NULL, NULL };
	ID3D11ShaderResourceView* gaussianBlurTexture[1] = { NULL };

	ID3D11ShaderResourceView* lightMap = NULL;

	ID3D11DepthStencilView* shadowDS = d3D->GetShadowmapDSV();
	ID3D11DepthStencilView* ds; //We set it later on when we need it. Calling d3D->GetDepthStencilView() also calls a reset on DS settings to default, hence we wait with calling it.

	// Generate the view matrix based on the camera's position.
	camPos = camera->GetPosition();
	XMStoreFloat3(&camDir, XMVector3Normalize(camera->ForwardVector()));

	gbufferRenderTargets[0] = colorRT->RTView;
	gbufferRenderTargets[1] = normalRT->RTView;
	gbufferRenderTargets[2] = depthRT->RTView;

	//For lighting pass
	lightTarget[0] = lightRT->RTView; 

	//For shadow pre-gbuffer pass
	shadowTarget[0] = shadowRT->RTView;

	//Name should be pretty self-explanatory
	gaussianBlurPingPongRTView[0] = gaussianBlurPingPongRT->RTView;

	//For GBuffer pass
	gbufferTextures[0] = colorRT->SRView; 
	gbufferTextures[1] = normalRT->SRView;
	gbufferTextures[2] = depthRT->SRView;

	//For directional light pass
	dirLightTextures[0] = normalRT->SRView;
	dirLightTextures[1] = depthRT->SRView;
	dirLightTextures[2] = shadowRT->SRView;
	dirLightTextures[3] = colorRT->SRView;

	//For the the final composition pass
	finalTextures[0] = colorRT->SRView;
	finalTextures[1] = lightRT->SRView;
	finalTextures[2] = depthRT->SRView;
	finalTextures[3] = normalRT->SRView;

	gaussianBlurTexture[0] = gaussianBlurPingPongRT->SRView;
#pragma endregion

#pragma region Matrix preparations
	//XMMATRIX shadowScaleBiasMatrix = new XMMATRIX
	//	(
	//	0.5f, 0.0f, 0.0f, 0.5f,
	//	0.0f, 0.5f, 0.0f, 0.5f,
	//	0.0f, 0.0f, 0.5f, 0.5f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//	);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	d3D->GetWorldMatrix(worldMatrix);
	d3D->GetOrthoMatrix(orthoMatrix);
	camera->GetViewMatrix(viewMatrix);
	camera->GetProjectionMatrix(projectionMatrix);
	lightView = XMLoadFloat4x4(&dirLight->View);
	lightProj = XMLoadFloat4x4(&dirLight->Projection);

	// Construct the frustum.
	frustum->ConstructFrustum(screenFar, &projectionMatrix, &viewMatrix);
	frustum->CalculateXZBounds(XMLoadFloat3(&camera->GetPosition()), camera->ForwardVector(), camera->UpVector());

	XMVECTOR nullVec;
	lightViewProj = XMMatrixMultiply(lightView, lightProj);
	viewProjection = XMMatrixMultiply(viewMatrix, projectionMatrix);

	invertedView = XMMatrixInverse(&nullVec, viewMatrix);
	invertedProjection = XMMatrixInverse(&nullVec, projectionMatrix);
	invertedViewProjection = XMMatrixInverse(&nullVec, viewProjection);

	worldView = (worldMatrix*viewMatrix);
	identityWorldViewProj = (worldView * projectionMatrix);
	lightWorldViewProj = worldMatrix*lightViewProj;
	lightWorldView = XMMatrixMultiply(lightView, worldMatrix);

	lightWorldViewProj =		XMMatrixTranspose(lightWorldViewProj);
	lightWorldView =			XMMatrixTranspose(lightWorldView);
	worldView =					XMMatrixTranspose(worldView);
	identityWorldViewProj =		XMMatrixTranspose(identityWorldViewProj);
	worldMatrix =				XMMatrixTranspose(worldMatrix);
	viewProjection =			XMMatrixTranspose(viewProjection);
	lightViewProj =				XMMatrixTranspose(lightViewProj);
	invertedViewProjection =	XMMatrixTranspose(invertedViewProjection);
	baseView =					XMMatrixTranspose(XMLoadFloat4x4(&baseViewMatrix));
	invertedProjection =		XMMatrixTranspose(invertedProjection);
	worldBaseViewOrthoProj =	((baseView * worldMatrix)* orthoMatrix); //Do it post-transpose

#pragma endregion

#pragma region Early depth pass for shadowmap
	context->OMSetRenderTargets(1, shadowTarget, shadowDS);
	context->ClearDepthStencilView(shadowDS, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->ClearRenderTargetView(shadowTarget[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	d3D->SetShadowViewport();

	d3D->SetNoCullRasterizer();

	marchingCubes->Render(context);

	result = depthOnlyShader->Render(context, marchingCubes->GetIndexCount(), &lightWorldViewProj, &lightWorldView);
	if(!result)
	{
		return false;
	}

	/************************ Uncomment to enable vegetation quad shadows ************************/

	//d3D->TurnOnShadowBlendState();
	//vegetationManager->RenderBuffers(context);

	//depthOnlyQuadShader->Render(context, vegetationManager->GetVertexCount(), vegetationManager->GetInstanceCount(),
	//	&lightWorldViewProj, textureAndMaterialHandler->GetVegetationTextureArray());

	//d3D->ResetBlendState();

	/*********************************************************************************************/
#pragma endregion

#pragma region Shadow map blur stage
	//Change render target to prepare for ping-ponging
	context->OMSetRenderTargets(1, gaussianBlurPingPongRTView, shadowDS);
	context->ClearRenderTargetView(gaussianBlurPingPongRTView[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));
	context->ClearDepthStencilView(shadowDS, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Blur shadow map texture horizontally
	fullScreenQuad.Render(context, 0, 0);
	gaussianBlurShader->RenderBlurX(context, fullScreenQuad.GetIndexCount(), &worldBaseViewOrthoProj, &dirLightTextures[2]);

	//Change render target back to our shadow map to render the second blur and get the final result
	context->OMSetRenderTargets(1, shadowTarget, shadowDS);
	context->ClearDepthStencilView(shadowDS, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Blur shadow map texture vertically
	fullScreenQuad.Render(context, 0, 0);
	gaussianBlurShader->RenderBlurY(context, fullScreenQuad.GetIndexCount(), &worldBaseViewOrthoProj, &gaussianBlurTexture[0]);
#pragma endregion

#pragma region GBuffer building stage
	d3D->SetDefaultViewport();
	ds = d3D->GetDepthStencilView();
	context->OMSetRenderTargets(3, gbufferRenderTargets, ds);
	context->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->ClearRenderTargetView(gbufferRenderTargets[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	context->ClearRenderTargetView(gbufferRenderTargets[1], D3DXVECTOR4(0.5f, 0.5f, 0.5f, 0.0f));
	context->ClearRenderTargetView(gbufferRenderTargets[2], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	d3D->SetNoCullRasterizer();
	d3D->TurnZBufferOff();

	worldMatrix = (XMMatrixTranslation(camPos.x, camPos.y, camPos.z)*viewMatrix)*projectionMatrix;
	worldMatrix = XMMatrixTranspose(worldMatrix);

	skySphere->Render(context, &worldMatrix, &dayNightCycle->GetAmbientLightColor(), timeOfDay);

	d3D->SetBackFaceCullingRasterizer();
	d3D->TurnZBufferOn();

	context->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	worldMatrix = XMMatrixIdentity(); 
	worldMatrix = XMMatrixTranspose(worldMatrix);
	worldView = XMMatrixTranspose(worldMatrix*viewMatrix);

	/*if(frustum->Check2DAABB(&testBoundingbox))
	{*/
	marchingCubes->Render(context);
	result = mcubeShader->Render(d3D->GetDeviceContext(), marchingCubes->GetIndexCount(), 
		&worldMatrix, &worldView, &identityWorldViewProj, textureAndMaterialHandler->GetTerrainTextureArray(), toggleColorMode);

	d3D->SetNoCullRasterizer();
	d3D->TurnOnAlphaBlending();
	vegetationManager->Render(context, &identityWorldViewProj, &worldMatrix, textureAndMaterialHandler->GetVegetationTextureArray());
	d3D->TurnOffAlphaBlending();

	//}

#pragma endregion

#pragma region Point light stage
	context->OMSetRenderTargets(1, lightTarget, ds);
	context->ClearRenderTargetView(lightTarget[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	context->ClearDepthStencilView(ds, D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Phase one, draw sphere with vertex-only shader.
	d3D->TurnOnLightBlending();
	if(toggleOtherPointLights)
	{
		for(unsigned int i = 0; i < pointLights.size(); i++)
		{	
			XMMATRIX worldViewProj = viewProjection * (XMLoadFloat4x4(&pointLights[i]->World));

			d3D->SetLightStencilMethod1Phase1();
			d3D->SetNoCullRasterizer();

			sphereModel->Render(context);
			result = vertexOnlyShader->Render(context, sphereModel->GetIndexCount(), &worldViewProj);

			//Phase two, draw sphere with light algorithm
			d3D->SetLightStencilMethod1Phase2();
			d3D->SetFrontFaceCullingRasterizer();

			sphereModel->Render(context);

			result = pointLightShader->Render(context, sphereModel->GetIndexCount(), &worldViewProj, &invertedViewProjection, 
				pointLights[i], gbufferTextures, textureAndMaterialHandler->GetMaterialTextureArray(), camPos);
			if(!result)
			{
				return false;
			}

			context->ClearDepthStencilView(ds, D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}

	if(toggleCameraPointLight)
	{	
		XMMATRIX worldViewProj = viewProjection * (XMLoadFloat4x4(&cameraPointLight.World));

		d3D->SetLightStencilMethod1Phase1();
		d3D->SetNoCullRasterizer();

		sphereModel->Render(context);
		result = vertexOnlyShader->Render(context, sphereModel->GetIndexCount(), &worldViewProj);

		//Phase two, draw sphere with light algorithm
		d3D->SetLightStencilMethod1Phase2();
		d3D->SetFrontFaceCullingRasterizer();

		sphereModel->Render(context);

		result = pointLightShader->Render(context, sphereModel->GetIndexCount(), &worldViewProj, &invertedViewProjection, 
			&cameraPointLight, gbufferTextures, textureAndMaterialHandler->GetMaterialTextureArray(), camPos);
		if(!result)
		{
			return false;
		}

		context->ClearDepthStencilView(ds, D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
#pragma endregion

#pragma region Directional light stage
	///*TODO: Create a directional light stencilstate that does a NOTEQUAL==0 stencil check.*/
	ds = d3D->GetDepthStencilView();
	context->ClearDepthStencilView(ds, D3D11_CLEAR_STENCIL|D3D11_CLEAR_DEPTH, 1.0f, 0);
	d3D->SetBackFaceCullingRasterizer();
	d3D->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranspose(worldMatrix);

	fullScreenQuad.Render(context, 0, 0);

	result = dirLightShader->Render(context, fullScreenQuad.GetIndexCount(), &worldBaseViewOrthoProj, &invertedViewProjection, 
		dirLightTextures, textureAndMaterialHandler->GetMaterialTextureArray(), camPos, camDir, dirLight,
		dayNightCycle->GetAmbientLightColor(), &lightViewProj, &worldView);
	if(!result)
	{
		return false;
	}
#pragma endregion

#pragma region Final compose stage
	d3D->SetBackBufferRenderTarget();
	context->ClearDepthStencilView(ds,  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	fullScreenQuad.Render(context, 0, 0);

	composeShader->Render(context, fullScreenQuad.GetIndexCount(), &worldBaseViewOrthoProj, &invertedViewProjection, 
		&dayNightCycle->GetAmbientLightColor(), fogMinimum, finalTextures, ssaoRandomTextureSRV, toggleSSAO);
#pragma endregion

#pragma region Debug and text stage
	d3D->ResetRasterizerState();
	d3D->ResetBlendState();
	ds = d3D->GetDepthStencilView(); //This also resets the depth stencil state to "default".
	context->ClearDepthStencilView(ds,  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	d3D->GetWorldMatrix(worldMatrix);
	worldMatrix = XMMatrixTranspose(worldMatrix);

	if(toggleDebugInfo)
	{
		for(int i = 0; i < 3; i++)
		{
			result = debugWindows[i].Render(d3D->GetDeviceContext(), 200+200*i, 0);
			if(!result)
			{
				return false;
			}

			result = textureShader->Render(d3D->GetDeviceContext(), debugWindows[i].GetIndexCount(), 
				&worldBaseViewOrthoProj, gbufferTextures[i]);
			if(!result)
			{
				return false;
			}
		}

		result = debugWindows[3].Render(d3D->GetDeviceContext(), 200, 200);
		if(!result)
		{
			return false;
		}

		result = textureShader->Render(d3D->GetDeviceContext(), debugWindows[3].GetIndexCount(), 
			&worldBaseViewOrthoProj, lightRT->SRView);
		if(!result)
		{
			return false;
		}

		result = debugWindows[4].Render(d3D->GetDeviceContext(), 800, 0);
		if(!result)
		{
			return false;
		}

		result = textureShader->Render(d3D->GetDeviceContext(), debugWindows[4].GetIndexCount(), 
			&worldBaseViewOrthoProj, shadowRT->SRView);
		if(!result)
		{
			return false;
		}

		result = debugWindows[5].Render(d3D->GetDeviceContext(), 400, 200);
		if(!result)
		{
			return false;
		}

		result = textureShader->Render(d3D->GetDeviceContext(), debugWindows[5].GetIndexCount(), 
			&worldBaseViewOrthoProj, lSystemSRV);
		if(!result)
		{
			return false;
		}

		d3D->TurnZBufferOff();

		d3D->TurnOnAlphaBlending();

		// Render the text user interface elements.
		result = text->Render(d3D->GetDeviceContext(), &worldBaseViewOrthoProj);
		if(!result)
		{
			return false;
		}

		// Turn off alpha blending after rendering the text.
		d3D->TurnOffAlphaBlending();

		// Turn the Z buffer back on now that all 2D rendering has completed.
		d3D->TurnZBufferOn();
	}
#pragma endregion

	// Present the rendered scene to the screen.
	d3D->EndScene();

	return true;
}

void Renderer::Shutdown()
{
	if(utility)
	{
		delete utility;
		utility = 0;
	}

	if (text)
	{
		text->Shutdown();
		delete text;
		text = 0;
	}

	if(frustum)
	{
		delete frustum;
		frustum = 0;
	}

	if(textureAndMaterialHandler)
	{
		delete textureAndMaterialHandler;
		textureAndMaterialHandler = 0;
	}

	if(gbufferShader)
	{
		gbufferShader->Shutdown();
		delete gbufferShader;
		gbufferShader = 0;
	}

	if(textureShader)
	{
		textureShader->Shutdown();
		delete textureShader;
		textureShader = 0;
	}

	if(pointLightShader)
	{
		pointLightShader->Shutdown();
		delete pointLightShader;
		pointLightShader = 0;
	}

	if(dirLight)
	{
		delete dirLight;
		dirLight = 0;
	}

	if(depthOnlyShader)
	{
		depthOnlyShader->Shutdown();
		delete depthOnlyShader;
		depthOnlyShader = 0;
	}

	if(vertexOnlyShader)
	{
		vertexOnlyShader->Shutdown();
		delete vertexOnlyShader;
		vertexOnlyShader = 0;
	}

	if(dirLightShader)
	{
		dirLightShader->Shutdown();
		delete dirLightShader;
		dirLightShader = 0;
	}

	if(composeShader)
	{
		composeShader->Shutdown();
		delete composeShader;
		composeShader = 0;
	}

	if(gaussianBlurShader)
	{
		gaussianBlurShader->Shutdown();
		delete gaussianBlurShader;
		gaussianBlurShader = 0;
	}

	if(colorRT)
	{
		delete colorRT;
		colorRT = 0;
	}

	if(normalRT)
	{
		delete normalRT;
		normalRT = 0;
	}

	if(depthRT)
	{
		delete depthRT;
		depthRT = 0;
	}

	if(lightRT)
	{
		delete lightRT;
		lightRT = 0;
	}

	if(shadowRT)
	{
		delete shadowRT;
		shadowRT = 0;
	}

	if(gaussianBlurPingPongRT)
	{
		delete gaussianBlurPingPongRT;
		gaussianBlurPingPongRT  = 0;
	}

	if(sphereModel)
	{
		sphereModel->Shutdown();
		delete sphereModel;
		sphereModel = 0;
	}

	if(skySphere)
	{
		skySphere->Shutdown();
		delete skySphere;
		skySphere = 0;
	}

	if(vegetationManager)
	{
		vegetationManager->Shutdown();
		delete vegetationManager;
		vegetationManager = 0;
	}

	for(std::vector<PointLight*>::iterator tmp = pointLights.begin(); tmp != pointLights.end(); tmp++) 
	{
		PointLight* light = 0;
		swap(light, *tmp); //Added for extra safety.

		if(light)
		{
			// Release the sentence.
			delete light;
			light = 0;
		}
	}
	pointLights.clear();

	if(marchingCubes)
	{
		delete marchingCubes;
		marchingCubes = 0;
	}

	if(mcubeShader)
	{
		mcubeShader->Shutdown();
		mcubeShader = 0;
	}

	if(dayNightCycle)
	{
		delete dayNightCycle;
		dayNightCycle = 0;
	}

	if(lSystemSRV)
	{
		lSystemSRV->Release();
		lSystemSRV = 0;
	}

	if(ssaoRandomTextureSRV)
	{
		ssaoRandomTextureSRV->Release();
		ssaoRandomTextureSRV = 0;
	}

	if(noise)
	{
		delete noise;
		noise = 0;
	}

	return;
}

void Renderer::GenerateVegetation( ID3D11Device* device, bool IfSetupThenTrue_IfUpdateThenFalse)
{
	float x,z,y;
	int textureID, randValue;

	LODVector500.clear();
	LODVector500.reserve(500);
	LODVector2500.clear();
	LODVector2500.reserve(2500);
	LODVector5000.clear();
	LODVector5000.reserve(5000);
	LODVector10000.clear();
	LODVector10000.reserve(10000);

	for(int i = 0; i < 70000; i++)
	{
		x = (2.0f + (utility->RandomFloat() * 176.0f));
		z = (2.0f + (utility->RandomFloat() * 176.0f));

		//Extract highest Y at this point
		y = marchingCubes->GetTerrain()->GetHighestPositionOfCoordinate((int)x, (int)z);

		randValue = rand()%100;

		//If we are above "snow level", we only want yellow grass
		if(y <= 30.0f)
		{

		}
		else if(y >= 45.0)
		{
			//But the grass should be sparse, so there is
			//95% chance that we won't actually add this to the instance list.
			if(randValue > 95)
			{
				textureID = 0;

				//Place texture ID in .w channel
				XMFLOAT4 temp = XMFLOAT4(x, y, z, (float)textureID);

				//We use i to control how many should be added to each LOD vector
				if(i <= 500)
				{
					LODVector500.push_back(temp);
				}

				if(i <= 2500)
				{
					LODVector2500.push_back(temp);
				}

				if(i <= 5000)
				{
					LODVector5000.push_back(temp);
				}

				LODVector10000.push_back(temp);
			}
		}
		else
		{
			if(randValue <= 5)
			{
				textureID = 2; //Some kind of leaf branch that I've turned into a plant quad.
			}
			else if(randValue <= 96) //By far biggest chance that we get normal grass
			{
				textureID = 1; //Normal grass.
			}
			else if(randValue <= 98) //If 97-98
			{
				textureID = 4; //Bush.
			}
			else //If 99-100.
			{
				textureID = 3; //Flower.
			}

			//Place texture ID in .w channel
			XMFLOAT4 temp = XMFLOAT4(x, y, z, (float)textureID);

			//We use i to control how many should be added to each LOD vector
			if(i <= 500)
			{
				LODVector500.push_back(temp);
			}

			if(i <= 2500)
			{
				LODVector2500.push_back(temp);
			}

			if(i <= 5000)
			{
				LODVector5000.push_back(temp);
			}

			LODVector10000.push_back(temp);
		}
	}

	if(IfSetupThenTrue_IfUpdateThenFalse)
	{
		vegetationManager->SetupQuads(d3D->GetDevice(), &LODVector500);
	}
	else
	{
		vegetationManager->BuildInstanceBuffer(d3D->GetDevice(), &LODVector10000);
	}
}

bool Renderer::RenderShadowmap()
{

	return true;
}

bool Renderer::RenderTwoPassGaussianBlur()
{

	return true;
}

bool Renderer::RenderGBuffer()
{

	return true;
}

bool Renderer::RenderDirectionalLight()
{

	return true;
}

bool Renderer::RenderPointLight()
{

	return true;
}

bool Renderer::RenderFinalScene()
{

	return true;
}

bool Renderer::RenderDebugInfoAndText()
{

	return true;
}
