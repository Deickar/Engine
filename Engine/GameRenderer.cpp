#pragma once
#include "GameRenderer.h"

//Utility
#include "StructsAndEnums.h"
#include "inputclass.h"
#include "timerclass.h"
#include "controllerclass.h"
#include "fpsmeter.h"
#include "textclass.h"
#include "renderToTextureClass.h"
#include "DayNightCycle.h"
#include "DebugOverlayHUD.h"

//Managers
#include "d3dmanager.h"
#include "TerrainManager.h"
#include "TextureHandler.h"
#include "VegetationManager.h"
#include "MeshHandler.h"

//Objects
#include "modelclass.h"
#include "frustumclass.h"
#include "debugwindowclass.h"
#include "SkySphere.h"
#include "marchingCubesClass.h"
#include "TerrainNoiseSeeder.h"
#include "LSystemClass.h"
#include "NoiseClass.h"
#include "IndexedMesh.h"
#include "OBJModel.h"
#include "cameraclass.h"
#include "inputclass.h"
#include "RenderableBundle.h"

//Shaders
#include "MCGBufferTerrainShader.h"
#include "textureshaderclass.h"
#include "VertexShaderOnly.h"
#include "DepthOnlyShader.h"
#include "DRCompose.h"
#include "DRPointLight.h"
#include "DRObjModelShader.h"
#include "SSAOShader.h"
#include "DRDirectionalLight.h"
#include "fontshaderclass.h"
#include "DepthOnlyQuadShader.h"
#include "GaussianBlur.h"
#include "GeometryShaderGrass.h"
#include "DRWaterClass.h"
#include "DRGBuffer.h"
#include "UnderwaterFilterShader.h"
#include "SSAOBlur.h"

GameRenderer::GameRenderer() : SettingsDependent()
{
	previouslyInitialized = false;

	xPos = yPos = 0.0f;
	timeOfDay = 0.0f;
	timer = 10.0f;
	textureOffsetDeltaTime = 0.0f;

	toggleTextureShader = false;
	returning = false;
	toggleOtherPointLights = false;
	drawWireFrame = false;

	//Load settings
	InitializeSettings(this);
}


GameRenderer::~GameRenderer()
{
}


bool GameRenderer::Initialize(HWND hwnd, std::shared_ptr<InputClass> inputManager, std::shared_ptr<D3DManager> d3D, 
	std::shared_ptr<DebugOverlayHUD> extDebugHUD, UINT screenWidth, UINT screenHeight, UINT shadowmapWidth, UINT shadowmapHeight, float screenFar, float screenNear)
{
	bool result;


	this->toggleDebugInfo = false;
	this->inputManager = inputManager;
	this->shadowMapWidth = shadowmapWidth;
	this->shadowMapHeight = shadowmapHeight;
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->farClip = screenFar;
	this->nearClip = screenNear;
	this->d3D = d3D;
	debugHUD = extDebugHUD;

	//2 = without SSAO. 1 = only SSAO. 0 = with SSAO
	toggleSSAO = 2;
	toggleColorMode = 1;
	fogMinimum = 1.0f;

	srand((unsigned int)time(NULL));

	//Break asap, yo. Not before we've assigned external pointers though, because those might have changed
	if(previouslyInitialized == true)
	{
		return true;
	}

	result = InitializeLights(hwnd);
	if(!result)
	{
		return false;
	}

	result = InitializeEverythingElse(hwnd);
	if(!result)
	{
		return false;
	}

	result = InitializeModels(hwnd);
	if(!result)
	{
		return false;
	}

	result = InitializeShaders(hwnd);
	if(!result)
	{
		return false;
	}

	InitializeRenderingSpecifics();
	InitializeDebugText();

	previouslyInitialized = true;

	return true;
}


bool GameRenderer::InitializeShaders( HWND hwnd )
{
	bool result;

	pointLightShader = std::unique_ptr<DRPointLightShader>(new DRPointLightShader);

	result = pointLightShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Pointlight shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	vertexOnlyShader = std::unique_ptr<VertexShaderOnly>(new VertexShaderOnly);
	
	result = vertexOnlyShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Vertex only shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	depthOnlyShader = std::unique_ptr<DepthOnlyShader>(new DepthOnlyShader);

	result = depthOnlyShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Depth only shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	gbufferShader = std::unique_ptr<DRGBuffer>(new DRGBuffer);

	result = gbufferShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"GBuffer shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	dirLightShader = std::unique_ptr<DRDirLightShader>(new DRDirLightShader);

	result = dirLightShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Dir light shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	composeShader = std::unique_ptr<DRCompose>(new DRCompose);

	result = composeShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Compose shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	gaussianBlurShader = std::unique_ptr<GaussianBlur>(new GaussianBlur);

	result = gaussianBlurShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Gaussian blur shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	textureShader = std::unique_ptr<TextureShaderClass>(new TextureShaderClass);

	result = textureShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Texture shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	mcubeShader = std::unique_ptr<MCGBufferTerrainShader>(new MCGBufferTerrainShader);

	result = mcubeShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Marching cubes gbuffer shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	geometryShaderGrass = std::unique_ptr<GeometryShaderGrass>(new GeometryShaderGrass);

	result = geometryShaderGrass->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Geometry shader grass shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	waterShader = std::unique_ptr<DRWaterClass>(new DRWaterClass);

	result = waterShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Water shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	objModelShader = std::unique_ptr<DRObjModelShader>(new DRObjModelShader);

	result = objModelShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(NULL, L"OBJ Model shader couldn't be initialized.", L"Error", MB_OK);
		return false;
	}

	ssaoShader = std::unique_ptr<SSAOShader>(new SSAOShader);

	result = ssaoShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(NULL, L"Couldn't initialize SSAO shader.", L"Error", MB_OK);
		return false;
	}

	underwaterFilterShader = std::unique_ptr<UnderwaterFilterShader>(new UnderwaterFilterShader);

	result = underwaterFilterShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(NULL, L"Couldn't initialize underwater filter shader.", L"Error", MB_OK);
		return false;
	}

	ssaoBlurShader = std::unique_ptr<SSAOBlur>(new SSAOBlur);

	result = ssaoBlurShader->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(NULL, L"Couldn't initialize SSAO Blur shader.", L"Error", MB_OK);
		return false;
	}

	/************************************************************************/
	/* Shader inputs                                                        */
	/************************************************************************/
	dirLightInput = std::make_shared<ShaderInputStructs::DirectionalLightInput>();
	ssaoInput = std::make_shared<ShaderInputStructs::SSAOShaderInput>();
	composeInput = std::make_shared<ShaderInputStructs::ComposeShaderInput>();
	waterfilterInput = std::make_shared<ShaderInputStructs::WaterFilterInput>();

	return true;
}


bool GameRenderer::InitializeLights( HWND hwnd )
{
#pragma region Point light initialization
	float x, y, z, red, green, blue;
	x = 2.0f;
	z = 2.0f;
	y = 15.0f;

	for(int i = 0; i < 100; i++)
	{
		red = utility->RandomFloat();
		green = utility->RandomFloat();
		blue = utility->RandomFloat();
		// pointLights[i].Position = XMFLOAT3(2.0f+(x * 176.0f), 15.0f, 2.0f + (y * 176.0f));

		pointLights.push_back(std::unique_ptr<PointLight>(new PointLight));

		pointLights[i]->Color = XMFLOAT3(red, green, blue);
		pointLights[i]->Position = XMFLOAT3(x, y, z);
		pointLights[i]->Radius = 4.0f + (2.0f*utility->RandomFloat()); //Used to both scale the actual point light model and is a factor in the attenuation
		pointLights[i]->Intensity = 30.0f + (10.0f*utility->RandomFloat()); //Is used to control the attenuation

		x += 18.0f;

		if(x >= 178.0f) //Every 10th light gets reseted in x and z plane.
		{
			x = 2.0f;
			z += 18.0f;
		}

		if(i != 0 && i % 100 == 0) //Every 100 pointlights we reset and make another layer that is (y+8) higher up.
		{
			x = 2.0f;
			z = 2.0f;
			y += 8.0f;
		}

		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);
		XMMATRIX tempTranslation = XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y, pointLights[i]->Position.z);
		XMStoreFloat4x4(&pointLights[i]->World, (tempScale * tempTranslation));
	}

#pragma endregion

#pragma region Directional light initialization
	// Create the directional light.
	dirLight = std::make_shared<DirLight>();

	XMVECTOR lookAt = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 0.0f));
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	// Initialize the directional light.
	dirLight->Color = XMFLOAT4(0.6f, 0.4f, 0.4f, 1.0f);
	dirLight->Intensity = 1.5f;
	XMStoreFloat3(&dirLight->Position, (lookAt + XMVectorSet(50.0f, 150.0f, 0.0f, 0.0f)));

	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(lookAt, XMLoadFloat3(&dirLight->Position)));
	XMStoreFloat3(&dirLight->Direction, direction);

	//XMStoreFloat4x4(&dirLight.Projection, XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f, 5.0f, 500.0f));	//Generate PERSPECTIVE light projection matrix and store it as float4x4
	XMStoreFloat4x4(&dirLight->Projection, XMMatrixOrthographicLH(400.0f, 400.0f, 10.0f, 500.0f));					//Generate ORTHOGONAL light projection matrix and store it as float4x4
	XMStoreFloat4x4(&dirLight->View, XMMatrixLookAtLH(XMLoadFloat3(&dirLight->Position), lookAt, up));				//Generate light view matrix and store it as float4x4.
#pragma endregion

	return true;
}


bool GameRenderer::InitializeModels( HWND hwnd )
{
	bool result;

	skySphere = std::make_shared<Skysphere>();

	result = skySphere->Initialize(d3D->GetDevice(), hwnd);
	if(!result)
	{
		return false;
	}

	result = textureHandler->Load2DCubemapTextureFromFile(d3D->GetDevice(), L"../Engine/data/cloudCubeMap3.dds", skySphere->GetCloudTexturePP());
	if(!result)
	{
		return false;
	}

	//result = (proceduralTextureHandler.CreateTilingCloudTexture(d3D->GetDevice(), d3D->GetDeviceContext(), skySphere.GetCloudTexturePP(), 4096, 4096) != S_FALSE);
	//if(!result)
	//{
	//	return false;
	//}

	result = textureHandler->Load2DCubemapTextureFromFile(d3D->GetDevice(), L"../Engine/data/starCubemap.dds", skySphere->GetStarTexturePP());
	if(!result)
	{
		return false;
	}

	sphereModel = std::make_shared<ModelClass>();

	// Initialize the model object. It really doesn't matter what textures it has because it's only used for point light volume culling.
	result = sphereModel->Initialize(d3D->GetDevice(), "../Engine/data/skydome.txt", L"../Engine/data/dirt.dds", L"../Engine/data/dirt.dds", L"../Engine/data/dirt.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	numtrees = 50;

	for(int i = 0; i < numtrees; i++)
	{
		XMFLOAT4X4 temp;

		//Order of operations should be Scaling * Rotation * Translation
		XMStoreFloat4x4(&temp, (XMMatrixRotationY(utility->RandomFloat()*360.0f) * XMMatrixTranslation(utility->RandomFloat()*100.0f, 15.0f, utility->RandomFloat()*100.0f)));

		treeMatrices.push_back(temp);
	}

	return true;
}


bool GameRenderer::InitializeDebugText()
{
	debugHUD->AddNewWindowWithoutHandle("DeltaTime: ", &timer, DataTypeEnumMappings::Float);

	debugHUD->AddNewWindowWithoutHandle("Directional light position: ", &dirLight->Position, DataTypeEnumMappings::Float3);
	debugHUD->AddNewWindowWithoutHandle("Point lights are turned on: ", &toggleOtherPointLights,		DataTypeEnumMappings::Bool);
	debugHUD->AddNewWindowWithoutHandle("Point light #1: ", &pointLights[0]->Position.y,		DataTypeEnumMappings::Float);

	debugHUD->AddNewWindowWithoutHandle("SSAO mode: ", &toggleSSAO,		DataTypeEnumMappings::Int32);

	return true;
}


//TODO:Fix name and split it up...
bool GameRenderer::InitializeEverythingElse( HWND hwnd )
{
	bool result;

	noise = std::make_shared<NoiseClass>();

	textureCreator = std::make_shared<TextureCreator>();

	textureHandler = std::make_shared<TextureHandler>();
	result = textureHandler->Initialize(d3D->GetDevice(), d3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	materialHandler = std::make_shared<MaterialHandler>();
	result = materialHandler->Initialize(d3D->GetDevice(), d3D->GetDeviceContext(), textureCreator.get(), noise.get());

	proceduralTextureHandler = std::make_shared<ProceduralTextureHandler>();
	result = proceduralTextureHandler->Initialize(d3D->GetDevice(), d3D->GetDeviceContext(), textureCreator.get(), noise.get(), utility.get());
	proceduralTextureHandler->SetupWindtextures(d3D->GetDevice(), d3D->GetDeviceContext(), 0, 0, 1024, 1024, 0.6f);

	meshHandler = std::make_shared<MeshHandler>();

	result = meshHandler->Initialize(textureHandler.get(), materialHandler.get());
	if(!result)
	{
		return false;
	}

	dayNightCycle = std::make_shared<DayNightCycle>();
	result = dayNightCycle->Initialize(DAY); //86400.0f/6 <-- This is realistic day/night cycle. 86400 seconds in a day.
	if(!result)
	{
		return false;
	}

	//dayNightCycle->Update(50.0f, dirLight, skySphere);

	for(int i = 0; i < 7; i++)
	{
		debugWindows.push_back(std::unique_ptr<DebugWindowClass>(new DebugWindowClass));
		debugWindows[i]->Initialize(d3D->GetDevice(), screenWidth, screenHeight, 200, 200);
	}

	fullScreenQuad = std::unique_ptr<DebugWindowClass>(new DebugWindowClass);

	fullScreenQuad->Initialize(d3D->GetDevice(), screenWidth, screenHeight, screenWidth, screenHeight);

	colorRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	materialRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	normalRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	depthRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	lightRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	shadowRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	R16G16PingPongRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	ssaoRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	R32PingPongRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);
	ARGB8PingPongRT = std::unique_ptr<RenderTarget2D>(new RenderTarget2D);

	colorRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	materialRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R10G10B10A2_UNORM);

	normalRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R11G11B10_FLOAT);
	depthRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R32_FLOAT);

	lightRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	shadowRT->Initialize(d3D->GetDevice(), shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R16G16_FLOAT);
	R16G16PingPongRT->Initialize(d3D->GetDevice(), shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R16G16_FLOAT); //Needs to be identical to shadowRT

	ssaoRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R32_FLOAT);
	R32PingPongRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R32_FLOAT);

	ARGB8PingPongRT->Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	//TODO:
	/*

	for(int i = 0; i < 3; i++)
	{
	if(!debugWindows[i].Render(deviceContext, 200+200*i, 0))
	{
	return false;
	}

	if(!textureShader.Render(deviceContext, debugWindows[i].GetIndexCount(), 
	worldBaseViewOrthoProj, gbufferTextures[i]))
	{
	return false;
	}
	}

	if(!debugWindows[4].Render(deviceContext, 800, 0))
	{
	return false;
	}

	if(!textureShader.Render(deviceContext, debugWindows[4].GetIndexCount(), 
	worldBaseViewOrthoProj, shadowRT.SRView))
	{
	return false;
	}

	if(!debugWindows[3].Render(deviceContext, 200, 200))
	{
	return false;
	}

	if(!textureShader.Render(deviceContext, debugWindows[3].GetIndexCount(), 
	worldBaseViewOrthoProj, lightRT.SRView))
	{
	return false;
	}

	if(!debugWindows[5].Render(deviceContext, 400, 200))
	{
	return false;
	}

	if(!textureShader.Render(deviceContext, debugWindows[5].GetIndexCount(), 
	worldBaseViewOrthoProj, *textureAndMaterialHandler.GetWindNormalMap()))
	{
	return false;
	}

	if(!debugWindows[6].Render(deviceContext, 600, 200))
	{
	return false;
	}

	if(!textureShader.Render(deviceContext, debugWindows[6].GetIndexCount(), 
	worldBaseViewOrthoProj, *textureAndMaterialHandler.GetWindTexture()))
	{
	return false;
	}

	*/

	return true;
}


bool GameRenderer::Update( HWND hwnd, int fps, int cpuPercentage, float millisecondDeltaTime, float secondDeltaTime, XMFLOAT3* windDirection, std::shared_ptr<CameraClass> camera)
{
	//Speed up the change of day
	if(inputManager->IsKeyPressed(DIK_1))
	{
		secondDeltaTime += (millisecondDeltaTime*0.2f);
	}

	timer += secondDeltaTime;
	textureOffsetDeltaTime += secondDeltaTime;
	windDir = *windDirection;

	//Reset every once in a while.
	if(textureOffsetDeltaTime >= 300.0f)
	{
		textureOffsetDeltaTime = -textureOffsetDeltaTime;
	}

	if(inputManager->WasKeyPressed(DIK_E))
	{
		drawWireFrame = !drawWireFrame;
	}

	if(inputManager->WasKeyPressed(DIK_Q))
	{
		toggleDebugInfo = !toggleDebugInfo;

		debugHUD->SetHUDVisibility(toggleDebugInfo);
	}

	if(toggleDebugInfo)
	{
		debugHUD->Update();
	}

	//Move all point lights upward
	if(inputManager->IsKeyPressed(DIK_R))
	{
		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);

		for(int i = 0; i < (int)(pointLights.size()); i++)
		{
			pointLights[i]->Position.y += millisecondDeltaTime*0.01f;

			XMStoreFloat4x4(&pointLights[i]->World, (tempScale*XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y, pointLights[i]->Position.z)));
		}
	}

	//Move all point lights downward
	if(inputManager->IsKeyPressed(DIK_F))
	{
		XMMATRIX tempScale = XMMatrixScaling(pointLights[0]->Radius, pointLights[0]->Radius, pointLights[0]->Radius);

		for(int i = 0; i < (int)(pointLights.size()); i++)
		{
			pointLights[i]->Position.y -= millisecondDeltaTime*0.01f;

			XMStoreFloat4x4(&pointLights[i]->World, (tempScale*XMMatrixTranslation(pointLights[i]->Position.x, pointLights[i]->Position.y, pointLights[i]->Position.z)));
		}
	}

	if(inputManager->IsKeyPressed(DIK_Y))
	{
		for(int i = 0; i < numtrees; i++)
		{
			XMStoreFloat4x4(&treeMatrices[i], XMMatrixRotationRollPitchYaw(0.0f, secondDeltaTime, 0.0f) * XMLoadFloat4x4(&treeMatrices[i]));
		}
	}

	//Toggle the coloring mode of materials
	if(inputManager->WasKeyPressed(DIK_2))
	{
		toggleColorMode++;

		if(toggleColorMode > 1)
		{
			toggleColorMode = 0;
		}
	}

	//TODO: Wtf? Change these.. remove..?
	if(inputManager->WasKeyPressed(DIK_3))
	{
		fogMinimum -= 0.1f;

		if(fogMinimum < 0.0f)
		{
			fogMinimum = 0.0f;
		}
	}

	if(inputManager->WasKeyPressed(DIK_4))
	{                  
		fogMinimum += 0.1f;

		if(fogMinimum > 1.0f)
		{
			fogMinimum = 1.0f;
		}
	}

	if(inputManager->WasKeyPressed(DIK_5))
	{
		toggleOtherPointLights = !toggleOtherPointLights;
	}

	if(inputManager->WasKeyPressed(DIK_6))
	{
		toggleSSAO++;

		if(toggleSSAO > 2)
		{
			toggleSSAO = 0;
		}
	}

	timeOfDay = dayNightCycle->Update(secondDeltaTime, dirLight.get(), skySphere.get(), &camera->GetPosition());

	XMVECTOR lookAt = XMLoadFloat3(&camera->GetPosition());//XMVectorSet(30.0f, 20.0f, 30.0f, 1.0f);//XMLoadFloat3(&camera->GetPosition());//XMLoadFloat3(&camera->GetPosition());//XMLoadFloat3(&camera->GetPosition())+(camera->ForwardVector()*30.0f);//XMLoadFloat3(&camera->GetPosition());//
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR currentLightPos = XMLoadFloat3(&dirLight->Position);//XMLoadFloat3(&camera->GetPosition())-(camera->ForwardVector()*30.0f);

	XMStoreFloat3(&dirLight->Direction, XMVector3Normalize(XMVectorSubtract(lookAt, currentLightPos)));//XMLoadFloat3(&dirLight.Position)
	XMStoreFloat4x4(&dirLight->View, XMMatrixLookAtLH(currentLightPos, lookAt, up)); //Generate light view matrix

	// Generate the view matrix based on the camera's position.
	camPos = camera->GetPosition();
	XMStoreFloat3(&camDir, XMVector3Normalize(camera->ForwardVector()));

	cameraIsUnderwater = ((camPos.y-1.5f) <= waterLevel) ? true : false;

	return true;
}


void GameRenderer::InitializeRenderingSpecifics()
{
	deviceContext = d3D->GetDeviceContext();
	device = d3D->GetDevice();

	shadowDepthStencil = d3D->GetShadowManager()->GetShadowmapDSV();
	depthStencil = d3D->GetDepthStencilManager()->GetDepthStencilView();

	//For shadow pre-gbuffer pass
	shadowTarget[0] = shadowRT->RTView;

	//Name should be pretty self-explanatory
	gaussianBlurTarget[0] = R16G16PingPongRT->RTView;

	//For underwater filter pass
	waterTarget[0] = ARGB8PingPongRT->RTView;

	//For gbuffer pass
	gbufferRenderTargets[0] = colorRT->RTView;
	gbufferRenderTargets[1] = normalRT->RTView;
	gbufferRenderTargets[2] = depthRT->RTView;
	gbufferRenderTargets[3] = materialRT->RTView;

	//For lighting pass
	lightTarget[0] = lightRT->RTView; 

	//For SSAO pass
	ssaoTarget[0] = R32PingPongRT->RTView;
	ssaoTarget[1] = ssaoRT->RTView;

	//For GBuffer pass
	gbufferTextures[0] = colorRT->SRView; 
	gbufferTextures[1] = normalRT->SRView;
	gbufferTextures[2] = depthRT->SRView;
	gbufferTextures[3] = materialRT->SRView;

	//For directional light pass
	dirLightTextures[0] = normalRT->SRView;
	dirLightTextures[1] = depthRT->SRView;
	dirLightTextures[2] = shadowRT->SRView;
	dirLightTextures[3] = materialRT->SRView;

	//For the the final composition pass
	finalTextures[0] = ARGB8PingPongRT->SRView;
	finalTextures[1] = lightRT->SRView;
	finalTextures[2] = depthRT->SRView;
	finalTextures[3] = ssaoRT->SRView;
	finalTextures[4] = materialRT->SRView;

	gaussianBlurTexture[0] = R16G16PingPongRT->SRView;

	ssaoView = ssaoRT->SRView;

	ssaoTextures[0] = depthRT->SRView;
	ssaoTextures[1] = normalRT->SRView;
	ssaoTextures[2] = *proceduralTextureHandler->GetSSAORandomTexture();

	ssaoBlurTextures[0] = ssaoRT->SRView;
	ssaoBlurTextures[1] = depthRT->SRView;
	ssaoBlurTextures[2] = normalRT->SRView;


	waterInputTextures[0] = colorRT->SRView;
	waterInputTextures[1] = depthRT->SRView;
}


bool GameRenderer::Render(HWND hwnd, RenderableBundle* renderableBundle, std::shared_ptr<CameraClass> camera)
{
	// Clear the scene.
	d3D->BeginScene(0.1f, 0.1f, 0.45f, 0.0f);

#pragma region Matrix preparations
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, viewProjection, invertedViewProjection, invertedView, 
		lightView, lightProj, lightViewProj, baseView, worldBaseViewOrthoProj, identityWorldViewProj, lightWorldViewProj, 
		invertedProjection, lightWorldView, invertedWorldView, worldBaseView, invertedBaseView, lightViewAndInvertedCameraView;

	//XMMATRIX shadowScaleBiasMatrix = new XMMATRIX
	//	(
	//	0.5f, 0.0f, 0.0f, 0.5f,
	//	0.0f, 0.5f, 0.0f, 0.5f,
	//	0.0f, 0.0f, 0.5f, 0.5f,
	//	0.0f, 0.0f, 0.0f, 1.0f
	//	);

	worldMatrix = XMMatrixIdentity(); //XMLoadFloat4x4(camera->GetWorldMatrix());//
	camera->GetOrthographicProjection(orthoMatrix);
	camera->GetViewMatrix(viewMatrix);
	camera->GetProjectionMatrix(projectionMatrix);
	baseView = XMLoadFloat4x4(camera->GetBaseViewMatrix());
	lightView = XMLoadFloat4x4(&dirLight->View);
	lightProj = XMLoadFloat4x4(&dirLight->Projection);

	lightViewProj = lightView * lightProj;

	viewProjection = viewMatrix * projectionMatrix;

	invertedView = XMMatrixInverse(&XMVectorSplatOne(), viewMatrix);
	invertedProjection = XMMatrixInverse(&XMVectorSplatOne(), projectionMatrix);
	invertedViewProjection = XMMatrixInverse(&XMVectorSplatOne(), viewProjection);
	invertedWorldView = XMMatrixInverse(&XMVectorSplatOne(), worldMatrix*viewMatrix);

	lightWorldView = worldMatrix * lightView;
	lightWorldViewProj = (worldMatrix * lightView) * lightProj;

	identityWorldViewProj = (worldMatrix * viewMatrix) * projectionMatrix;
	worldBaseView = worldMatrix * baseView;

	lightViewAndInvertedCameraView = XMMatrixTranspose(XMMatrixMultiply(lightView, invertedView));

	lightWorldView =			XMMatrixTranspose(lightWorldView);
	lightWorldViewProj =		XMMatrixTranspose(lightWorldViewProj);
	lightView =					XMMatrixTranspose(lightView);
	lightProj =					XMMatrixTranspose(lightProj);
	lightViewProj =				XMMatrixTranspose(lightViewProj);

	invertedView =				XMMatrixTranspose(invertedView);
	invertedWorldView =			XMMatrixTranspose(invertedWorldView);
	invertedProjection =		XMMatrixTranspose(invertedProjection);
	invertedViewProjection =	XMMatrixTranspose(invertedViewProjection);

	worldMatrix =				XMMatrixTranspose(worldMatrix);
	//viewMatrix =				XMMatrixTranspose(viewMatrix);
	//projectionMatrix =		XMMatrixTranspose(projectionMatrix);
	invertedBaseView =			XMMatrixTranspose(XMMatrixInverse(&XMVectorSplatOne(), 	invertedBaseView));

	worldBaseView =				XMMatrixTranspose(worldBaseView);
	//viewProjection =			XMMatrixTranspose(viewProjection);
	identityWorldViewProj =		XMMatrixTranspose(identityWorldViewProj);

	//Post-transpose matrix multiplications for the calculations that contain orthogonal projections
	worldBaseViewOrthoProj =	(worldMatrix * baseView) * orthoMatrix;

	//baseView = XMMatrixTranspose(baseView);
#pragma endregion

#pragma region Prepare input structs...
	//Directional light
	dirLightInput->worldViewProjection = &worldBaseViewOrthoProj;
	dirLightInput->worldView = &worldBaseView;
	dirLightInput->world = &XMMatrixTranspose(XMMatrixIdentity());
	dirLightInput->view = &XMMatrixTranspose(viewMatrix);
	dirLightInput->invertedView = &invertedView;
	dirLightInput->invertedProjection = &invertedProjection;
	dirLightInput->lightViewAndInvertedCameraView = &lightViewAndInvertedCameraView;
	dirLightInput->lightView = &lightView;
	dirLightInput->lightProj = &lightProj;
	dirLightInput->lightViewProj = &lightViewProj;
	dirLightInput->dirLight = dirLight.get();
	dirLightInput->textureArray = &dirLightTextures[0].p;
	dirLightInput->materialTextureArray = materialHandler->GetMaterialTextureArray();
	dirLightInput->ambienceColor = dayNightCycle->GetAmbientLightColor();
	dirLightInput->cameraPosition = camPos;

	//Underwater shader
	waterfilterInput->cameraHeight = camPos.y;
	waterfilterInput->fogColor = &dayNightCycle->GetAmbientLightColor();
	waterfilterInput->textureArray = &waterInputTextures[0].p;
	waterfilterInput->WorldViewProjection = &worldBaseViewOrthoProj;

	//Compose shader
	composeInput->worldViewProjection = &worldBaseViewOrthoProj;
	composeInput->worldView = &worldBaseView;
	composeInput->view = &baseView; 
	composeInput->invertedProjection = &invertedProjection;
	composeInput->invViewProjection = &invertedViewProjection;
	composeInput->fogColor = &dayNightCycle->GetAmbientLightColor();
	composeInput->fogMinimum = fogMinimum;
	composeInput->textureArray = &finalTextures[0].p;
	composeInput->toggle = toggleSSAO;
	composeInput->lightIntensity = dirLight->Intensity;
	composeInput->cameraHeight = camPos.y;

	ssaoInput->rtTextureArray = &ssaoTextures[0].p;
	ssaoInput->worldViewProjection = &worldBaseViewOrthoProj;
	ssaoInput->invertedProjection = &invertedProjection; //&XMMatrixTranspose(projectionMatrix); //&orthoMatrix; //
	ssaoInput->view = &XMMatrixTranspose(XMMatrixIdentity() * viewMatrix); //XMMatrixTranspose(viewMatrix); //&XMMatrixTranspose(viewMatrix);//&baseView;
#pragma endregion

	//Send untransposed lightView/lightProj here
	if(!RenderShadowmap(&lightWorldViewProj, &lightWorldView, &XMLoadFloat4x4(&dirLight->View), &XMLoadFloat4x4(&dirLight->Projection), renderableBundle))
	{
		return false;
	}

	if(!RenderTwoPassGaussianBlur(&worldBaseViewOrthoProj))
	{
		return false;
	}

	if(!RenderGBuffer(&viewMatrix, &baseView, &projectionMatrix, &identityWorldViewProj, renderableBundle))
	{
		return false;
	}

	if(!RenderPointLight(&viewMatrix, &invertedView, &invertedProjection, &viewProjection))
	{
		return false;
	}

	if(!RenderDirectionalLight(dirLightInput.get())) 
	{
		return false;
	}

	//OK TODO: First of all make sure to change color render target at start of GBuffer to ColorRT
	//And in beginning of underwater filter change RT to tempARGB8Target

	if(cameraIsUnderwater)
	{
		finalTextures[0] = ARGB8PingPongRT->SRView;

		if(!RenderUnderwaterFilter(waterfilterInput.get()))
		{
			return false;
		}
	}
	else
	{
		finalTextures[0] = colorRT->SRView;
	}

	if(!RenderSSAO(ssaoInput.get()))
	{	
		return false;
	}

	if(!RenderComposedScene(composeInput.get()))
	{
		return false;
	}

	if(!RenderGUI(&worldBaseViewOrthoProj))
	{
		return false;
	}

	return true;
}


bool GameRenderer::RenderShadowmap(XMMATRIX* lightWorldViewProj, XMMATRIX* lightWorldView, XMMATRIX* lightView, XMMATRIX* lightProj, RenderableBundle* renderableBundle )
{
	XMMATRIX world, WVP, WV;

	//Early depth pass for shadowmap
	d3D->GetShadowManager()->SetShadowViewport();

	deviceContext->OMSetRenderTargets(1, &shadowTarget[0].p, shadowDepthStencil.p);
	deviceContext->ClearDepthStencilView(shadowDepthStencil.p, D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->ClearRenderTargetView(shadowTarget[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	// Turn off alpha blending after rendering the text->
	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();

	// Turn the Z buffer back on now that all 2D rendering has completed.
	d3D->GetDepthStencilManager()->SetDefaultDepthStencilView();

	d3D->GetRasterizerStateManager()->SetNoCullRasterizer();

	for(unsigned int i = 0; i < renderableBundle->terrainChunks.size(); i++)
	{	
		renderableBundle->terrainChunks[i]->GetTerrainMesh()->Render(deviceContext);

		if(!depthOnlyShader->Render(deviceContext, renderableBundle->terrainChunks[i]->GetTerrainMesh()->GetIndexCount(), 0, lightWorldViewProj, lightWorldView))
		{
			return false;
		}
	}

	////This is where we'll render trees.
	//const auto& objModels = renderableBundle->objModels;
	//const auto& chunks = renderableBundle->terrainChunks;
	//unsigned int vecSize = objModels.size();

	////Should be temporary ...
	//bool textureNeedsUpdate = true;
	//bool materialNeedsUpdate = true;

	//auto& model = renderableBundle->objModels[0];
	//IndexedMesh* tempMesh = meshHandler->GetMesh(model.GetMeshHandle());

	//for(unsigned int i = 0; i < chunks.size(); ++i)
	//{
	//	//Dumb hack to see viability of rendering many trees
	//	for(unsigned int k = 0; k < chunks[i]->GetBushCount(); ++k)
	//	{
	//		world = XMLoadFloat4x4(&chunks[i]->GetBushTransforms()[k]); /*XMMatrixTranspose(XMLoadFloat4x4(&(treeMatrices.at(k))));*/
	//		WVP = XMMatrixTranspose(world * *lightView * *lightProj);
	//		WV = XMMatrixTranspose(world * *lightView);

	//		for(int j = 0; j < model.GetSubsetCount()-1; ++j)
	//		{
	//			tempMesh->Render(deviceContext);

	//			//This will always work, because when we load in the model we add one last set of index subsets that contains the entire count (the end of the indices, essentially).
	//			int indexCount = model.GetSubSetIndices().at(j+1) - model.GetSubSetIndices().at(j);

	//			depthOnlyShader.Render(deviceContext, indexCount, model.GetSubSetIndices().at(j), &WVP, &WV);
	//		}
	//	}
	//}

	return true;
}

//TODO: change this function to just take two generic render targets and a depth stencil, as to make it possible to blur anything
bool GameRenderer::RenderTwoPassGaussianBlur(XMMATRIX* worldBaseViewOrthoProj )
{
	//Shadow map blur stage
	//Change render target to prepare for ping-ponging
	deviceContext->OMSetRenderTargets(1, &gaussianBlurTarget[0].p, shadowDepthStencil.p);
	deviceContext->ClearRenderTargetView(gaussianBlurTarget[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));
	deviceContext->ClearDepthStencilView(shadowDepthStencil.p, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Blur shadow map texture horizontally
	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	if(!gaussianBlurShader->RenderBlurX(deviceContext, fullScreenQuad->GetIndexCount(), worldBaseViewOrthoProj, &dirLightTextures[2].p))
	{
		return false;
	}

	//Change render target back to our shadow map to render the second blur and get the final result
	deviceContext->OMSetRenderTargets(1, &shadowTarget[0].p, shadowDepthStencil.p);
	deviceContext->ClearDepthStencilView(shadowDepthStencil.p, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Blur shadow map texture vertically
	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	if(!gaussianBlurShader->RenderBlurY(deviceContext, fullScreenQuad->GetIndexCount(), worldBaseViewOrthoProj, &gaussianBlurTexture[0].p))
	{
		return false;
	}

	return true;
}


bool GameRenderer::RenderGBuffer( XMMATRIX* viewMatrix, XMMATRIX* baseView, XMMATRIX* projectionMatrix, XMMATRIX* identityWorldViewProj, RenderableBundle* renderableBundle )
{
	XMMATRIX worldViewProjMatrix, worldMatrix, worldView, view, proj;

	//GBuffer building stage
	d3D->SetDefaultViewport();

	//depthStencil.p = d3D->GetDepthStencilManager()->GetDepthStencilView();
	deviceContext->OMSetRenderTargets(4, &gbufferRenderTargets[0].p, depthStencil);
	deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

	deviceContext->ClearRenderTargetView(gbufferRenderTargets[0], D3DXVECTOR4(0.0f, 0.125f, 0.3f, 1.0f));
	deviceContext->ClearRenderTargetView(gbufferRenderTargets[1], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	deviceContext->ClearRenderTargetView(gbufferRenderTargets[2], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	deviceContext->ClearRenderTargetView(gbufferRenderTargets[3], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	d3D->GetRasterizerStateManager()->SetNoCullRasterizer();
	d3D->GetDepthStencilManager()->SetDepthDisabledStencilState();

	worldMatrix = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);

	//Scale skysphere by arbitrary value because camera nearClip is 2.0f. Nearclip is 2.0f because else I get precision issues when rendering water.
	worldViewProjMatrix = (XMMatrixScaling(5.0f, 5.0f, 5.0f) * worldMatrix) * ((*viewMatrix) * (*projectionMatrix));
	worldViewProjMatrix = XMMatrixTranspose(worldViewProjMatrix);

	skySphere->Render(deviceContext, &worldMatrix, &worldViewProjMatrix, camPos.y, &dayNightCycle->GetAmbientLightColor(), timeOfDay, dayNightCycle->GetCurrentStageOfDay(), dirLight->Intensity);

	d3D->GetDepthStencilManager()->SetDefaultDepthStencilView();
	d3D->GetRasterizerStateManager()->SetBackFaceCullingRasterizer();
	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();

	deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//worldMatrix = (XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMLoadFloat4x4((&renderableBundle->testSphere.world)));
	//worldMatrix = XMMatrixTranspose(worldMatrix);
	//view = XMMatrixTranspose(*viewMatrix);
	//proj = XMMatrixTranspose(*projectionMatrix);

	//renderableBundle->testSphere.mesh.Render(deviceContext);

	//gbufferShader.Render(deviceContext, renderableBundle->testSphere.mesh.GetIndexCount(), &worldMatrix, &view, &proj, sphereModel.GetTexture(), farClip);

	worldMatrix = XMMatrixIdentity();
	worldView = XMMatrixTranspose(worldMatrix * (*baseView)); //
	worldMatrix = XMMatrixTranspose(worldMatrix);

	if(drawWireFrame)
	{	
		d3D->GetRasterizerStateManager()->SetWireframeRasterizer();
	}

	unsigned int vecSize = renderableBundle->terrainChunks.size();

	const auto& chunks = renderableBundle->terrainChunks;

	for(unsigned int i = 0; i < vecSize; i++)
	{	
		chunks[i]->GetTerrainMesh()->Render(deviceContext);

		if(!mcubeShader->Render(d3D->GetDeviceContext(),  chunks[i]->GetTerrainMesh()->GetIndexCount(), &worldMatrix, &worldView, 
			identityWorldViewProj, textureHandler->GetTerrainTextureArray(), materialHandler->GetMaterialLookupTexture(), toggleColorMode))
		{
			return false;
		}
	}

	d3D->GetBlendStateManager()->TurnOnTransparencyBlending();
	//d3D->GetRasterizerStateManager()->SetBackFaceCullingRasterizer();
	d3D->GetRasterizerStateManager()->SetNoCullRasterizer();

	for(unsigned int i = 0; i < vecSize; i++)
	{	
		chunks[i]->GetWaterMesh()->Render(deviceContext);

		if(!waterShader->Render(d3D->GetDeviceContext(),  chunks[i]->GetWaterMesh()->GetIndexCount(), &worldMatrix, &worldView, identityWorldViewProj, 
			textureHandler->GetVegetationTextureArray(), proceduralTextureHandler->GetWindTexture(), proceduralTextureHandler->GetWindNormalMap(), 
			textureOffsetDeltaTime, &windDir))
		{
			return false;
		}
	}

	d3D->GetBlendStateManager()->TurnOnAlphaBlending();
	d3D->GetRasterizerStateManager()->SetNoCullRasterizer();

	for(unsigned int i = 0; i < vecSize; i++)
	{	
		chunks[i]->GetTerrainMesh()->Render(deviceContext);

		if(!geometryShaderGrass->Render(d3D->GetDeviceContext(),  chunks[i]->GetTerrainMesh()->GetIndexCount(), &worldMatrix, &worldView, 
			identityWorldViewProj, textureHandler->GetVegetationTextureArray(), materialHandler->GetMaterialLookupTexture(), 
			proceduralTextureHandler->GetWindTexture(), toggleColorMode, textureOffsetDeltaTime, &windDir))
		{
			return false;
		}
	}

	//This is where we'll render trees.
	auto& objModels = renderableBundle->objModels;
	vecSize = renderableBundle->objModels.size();

	//Should be temporary ...
	bool textureNeedsUpdate = true;
	bool materialNeedsUpdate = true;

	auto& model = renderableBundle->objModels[0];
	IndexedMesh* tempMesh = meshHandler->GetMesh(model.GetMeshHandle());

	for(unsigned int i = 0; i < chunks.size(); ++i)
	{
		unsigned int bushCount = chunks[i]->GetVegetationTransforms()->size();
		auto vegTransforms = chunks[i]->GetVegetationTransforms()->data();

		//Retarded little hack to see viability of rendering many trees
		for(unsigned int k = 0; k < bushCount; ++k)
		{
			worldMatrix = XMLoadFloat4x4(&vegTransforms[k]); /*XMMatrixTranspose(XMLoadFloat4x4(&(treeMatrices.at(k))));*/
			worldView = worldMatrix * (*viewMatrix);
			worldViewProjMatrix = worldView * (*projectionMatrix);

			worldMatrix = XMMatrixTranspose(worldMatrix);
			worldView = XMMatrixTranspose(worldView);
			worldViewProjMatrix = XMMatrixTranspose(worldViewProjMatrix);
			
			objModelShader->UpdateMatrixBuffer(deviceContext, &worldMatrix, &worldView, &worldViewProjMatrix);

			for(int j = 0; j < model.GetSubsetCount()-1; ++j)
			{
				//See if we need to update texture
				auto tex = textureHandler->GetTexture(model.GetTextureHandles().at(j));

				//See if we need to update material
				auto mat = materialHandler->GetMaterial(model.GetMaterialHandles().at(j));

				if(textureNeedsUpdate)
				{
					objModelShader->SetNewTexture(deviceContext, tex, 1);
				}

				if(materialNeedsUpdate)
				{
					objModelShader->SetNewMaterial(deviceContext, mat);
				}

				tempMesh->Render(deviceContext);

				//This will always work, because when we load in the model we add one last set of index subsets that contains the entire count (the end of the indices, essentially).
				int indexCount = model.GetSubSetIndices().at(j+1) - model.GetSubSetIndices().at(j);

				objModelShader->RenderShader(deviceContext, indexCount, model.GetSubSetIndices().at(j));
			}
		}
	}

	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();
	d3D->GetRasterizerStateManager()->SetBackFaceCullingRasterizer();

	return true;
}


bool GameRenderer::RenderPointLight(XMMATRIX* view, XMMATRIX* invertedView, XMMATRIX* invertedProjection, XMMATRIX* viewProjection )
{
	XMMATRIX world, worldView, worldViewProj;

	//Point light stage
	deviceContext->OMSetRenderTargets(1, &lightTarget[0].p, depthStencil);
	deviceContext->ClearRenderTargetView(lightTarget[0], D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_STENCIL, 1.0f, 0);

	d3D->GetBlendStateManager()->TurnOnLightBlending();

	if(toggleOtherPointLights)
	{
		for(unsigned int i = 0; i < pointLights.size(); i++)
		{	
			world = XMLoadFloat4x4(&pointLights[i]->World);
			worldView = world * (*view);
			worldViewProj =  world * (*viewProjection);

			world = XMMatrixTranspose(world);
			worldView = XMMatrixTranspose(worldView);
			worldViewProj = XMMatrixTranspose(worldViewProj);

			//Phase one, draw sphere with vertex-only shader.
			d3D->GetDepthStencilManager()->SetLightStencilMethod1Phase1();
			d3D->GetRasterizerStateManager()->SetNoCullRasterizer();

			sphereModel->Render(deviceContext);

			if(!vertexOnlyShader->Render(deviceContext, sphereModel->GetIndexCount(), &worldViewProj))
			{
				return false;
			}

			//Phase two, draw sphere with light algorithm
			d3D->GetDepthStencilManager()->SetLightStencilMethod1Phase2();
			d3D->GetRasterizerStateManager()->SetFrontFaceCullingRasterizer();

			sphereModel->Render(deviceContext);

			if(!pointLightShader->Render(deviceContext, sphereModel->GetIndexCount(), &worldViewProj, &worldView, &world, invertedView, 
				invertedProjection, pointLights[i].get(), &gbufferTextures[0].p, materialHandler->GetMaterialTextureArray(), camPos))
			{
				return false;
			}

			//if(!textureShader.Render(deviceContext, sphereModel->GetIndexCount(), &worldViewProj, sphereModel->GetTexture()))
			//{
			//	return false;
			//}

			deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}

	return true;
}


bool GameRenderer::RenderDirectionalLight(ShaderInputStructs::DirectionalLightInput* input)
{
	//Directional light stage
	/*TODO: Create a directional light stencilstate that does a NOTEQUAL==0 stencil check.*/
	d3D->GetDepthStencilManager()->SetDepthDisabledStencilState();
	d3D->GetRasterizerStateManager()->SetNoCullRasterizer();

	deviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	if(!dirLightShader->Render(deviceContext, fullScreenQuad->GetIndexCount(), input))
	{
		return false;
	}

	return true;
}

bool GameRenderer::RenderUnderwaterFilter(ShaderInputStructs::WaterFilterInput* input)
{
	deviceContext->OMSetRenderTargets(1, &waterTarget[0].p, depthStencil);
	deviceContext->ClearRenderTargetView(waterTarget[0].p, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();

	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	if(!underwaterFilterShader->Render(deviceContext, fullScreenQuad->GetIndexCount(), waterfilterInput.get()))
	{
		return false;
	}


	return true;
}

bool GameRenderer::RenderSSAO(ShaderInputStructs::SSAOShaderInput* input )
{
	//First, set render target to real target
	deviceContext->OMSetRenderTargets(1, &ssaoTarget[1].p, depthStencil);
	deviceContext->ClearRenderTargetView(ssaoTarget[1].p, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	//Write SSAO to real target
	if(!ssaoShader->Render(deviceContext, fullScreenQuad->GetIndexCount(), input))
	{
		return false;
	}

	////Change target to our bounce target, because we'll be using the other target to read from while blurring
	//deviceContext->OMSetRenderTargets(1, &ssaoTarget[0].p, depthStencil);
	//deviceContext->ClearRenderTargetView(ssaoTarget[0].p, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	////Change what resource to sample from
	//ssaoBlurTextures[0] = ssaoRT->SRView;

	//if(!fullScreenQuad->Render(deviceContext, 0, 0))
	//{
	//	return false;
	//}

	//if(!ssaoBlurShader->RenderBlurX(deviceContext, fullScreenQuad->GetIndexCount(), input->worldViewProjection, &ssaoBlurTextures[0].p))
	//{
	//	return false;
	//}

	////Aaaand... Set RT back to real target again to render out final result
	//deviceContext->OMSetRenderTargets(1, &ssaoTarget[1].p, depthStencil);
	//deviceContext->ClearRenderTargetView(ssaoTarget[1].p, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));

	////Change what resource to sample from
	//ssaoBlurTextures[0] = R32PingPongRT->SRView;

	//if(!fullScreenQuad->Render(deviceContext, 0, 0))
	//{
	//	return false;
	//}

	//if(!ssaoBlurShader->RenderBlurY(deviceContext, fullScreenQuad->GetIndexCount(), input->worldViewProjection, &ssaoBlurTextures[0].p))
	//{
	//	return false;
	//}


	return true;
}

bool GameRenderer::RenderComposedScene(ShaderInputStructs::ComposeShaderInput* input)
{
	//Render final composed scene that is the sum of all the previous scene
	d3D->ResetBackBufferRenderTarget();

	//d3D->GetDepthStencilManager()->SetDefaultDepthStencilView();
	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();
	deviceContext->ClearDepthStencilView(depthStencil,  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if(!fullScreenQuad->Render(deviceContext, 0, 0))
	{
		return false;
	}

	if(!composeShader->Render(deviceContext, fullScreenQuad->GetIndexCount(), input))
	{
		return false;
	}

	return true;
}


bool GameRenderer::RenderGUI(XMMATRIX* worldBaseViewOrthoProj )
{
	XMMATRIX worldMatrix;

	deviceContext->ClearDepthStencilView(depthStencil,  D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	worldMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixTranspose(worldMatrix);

	if(toggleDebugInfo)
	{

		d3D->GetDepthStencilManager()->SetDepthDisabledStencilState();

		d3D->GetBlendStateManager()->TurnOnAlphaBlending();

		for(int i = 0; i < 3; i++)
		{
			if(!debugWindows[i]->Render(deviceContext, 200+200*i, 0))
			{
				return false;
			}

			if(!textureShader->Render(deviceContext, debugWindows[i]->GetIndexCount(), 
				worldBaseViewOrthoProj, gbufferTextures[i]))
			{
				return false;
			}
		}

		if(!debugWindows[4]->Render(deviceContext, 800, 0))
		{
			return false;
		}

		if(!textureShader->Render(deviceContext, debugWindows[4]->GetIndexCount(), 
			worldBaseViewOrthoProj, shadowRT->SRView))
		{
			return false;
		}

		if(!debugWindows[3]->Render(deviceContext, 200, 200))
		{
			return false;
		}

		if(!textureShader->Render(deviceContext, debugWindows[3]->GetIndexCount(), 
			worldBaseViewOrthoProj, lightRT->SRView))
		{
			return false;
		}

		if(!debugWindows[6]->Render(deviceContext, 400, 200))
		{
			return false;
		}

		if(!textureShader->Render(deviceContext, debugWindows[6]->GetIndexCount(), 
			worldBaseViewOrthoProj, ssaoView.p))
		{
			return false;
		}

		//if(!debugWindows[5].Render(deviceContext, 600, 200))
		//{
		//	return false;
		//}

		//if(!textureShader.Render(deviceContext, debugWindows[5].GetIndexCount(), 
		//	worldBaseViewOrthoProj, *skySphere.GetCloudTexturePP()))
		//{
		//	return false;
		//}
	}

	// Turn off alpha blending after rendering the text->
	d3D->GetBlendStateManager()->TurnOnDefaultBlendState();

	// Turn the Z buffer back on now that all 2D rendering has completed.
	d3D->GetDepthStencilManager()->SetDefaultDepthStencilView();

	return true;
}


void GameRenderer::Shutdown()
{
	return;
}

void GameRenderer::OnSettingsReload(Config* cfg)
{
	const Setting& settings = cfg->getRoot()["rendering"];

	//let's NOT do this. take all of this from d3dManager, which in turns gets it from engine where it is decided on startup

	settings.lookupValue("windowWidth", screenWidth);
	settings.lookupValue("windowHeight", screenHeight);

	const Setting& waterSettings = cfg->getRoot()["shaders"]["waterShader"];

	waterSettings.lookupValue("waterLevels", waterLevel);

	//result = dayNightCycle.Initialize(500.0f, DAY); //86400.0f/6 <-- This is realistic day/night cycle. 86400 seconds in a day.
	//if(!result)
	//{
	//	return false;
	//}

	////dayNightCycle->Update(50.0f, dirLight, skySphere);

	//for(int i = 0; i < 7; i++)
	//{
	//	debugWindows[i].Initialize(d3D->GetDevice(), screenWidth, screenHeight, 200, 200);
	//}

	//fullScreenQuad.Initialize(d3D->GetDevice(), screenWidth, screenHeight, screenWidth, screenHeight);

	//colorRT.Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	//normalRT.Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	//depthRT.Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R32_FLOAT);
	//lightRT.Initialize(d3D->GetDevice(), screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
	//shadowRT.Initialize(d3D->GetDevice(), shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R32G32_FLOAT);
	//gaussianBlurPingPongRT.Initialize(d3D->GetDevice(), shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R32G32_FLOAT); //Needs to be identical to shadowRT
}

#pragma region Hidden stuff.. for now
//if(inputManager->WasKeyPressed(DIK_U))
//{
//	textureAndMaterialHandler.RebuildRandom2DTexture(d3D->GetDevice(), d3D->GetDeviceContext());
//}
//
//if(inputManager->WasKeyPressed(DIK_I))
//{
//	textureAndMaterialHandler.RebuildMirroredSimplex2DTexture(d3D->GetDevice(), d3D->GetDeviceContext());
//}
//
//if(inputManager->WasKeyPressed(DIK_O))
//{
//	textureAndMaterialHandler.RebuildSeamlessSimplex2DTexture(d3D->GetDevice(), d3D->GetDeviceContext(), 0, 0, 100, 100);
//}
//
//if(inputManager->WasKeyPressed(DIK_J))
//{
//	xPos += 30.0f;
//
//	textureAndMaterialHandler.RebuildTexture(d3D->GetDevice(), d3D->GetDeviceContext(), TEXTURE_GRASS, 1024, 1024, xPos, yPos, true);
//}
//
//if(inputManager->WasKeyPressed(DIK_K))
//{
//	yPos += 30.0f;
//
//	textureAndMaterialHandler.RebuildTexture(d3D->GetDevice(), d3D->GetDeviceContext(), TEXTURE_DIRT, 1024, 1024, xPos, yPos, true);
//}
//
//if(inputManager->WasKeyPressed(DIK_P))
//{
//	//Create and initialize our time... things.
//	const time_t timeObject = time(NULL);
//	struct tm parts;
//	localtime_s(&parts, &timeObject );
//
//	std::ostringstream std::stringStream;
//
//	//Create the std::string that will hold the screenshot's name when it gets pooped out into the directory
//	std::stringStream << "SavedTexture_" << (1+parts.tm_mon) << "-" << parts.tm_mday <<  "-" << parts.tm_min << "-" << parts.tm_sec << ".bmp";
//
//	LPCSTR fileName;
//	std::string temp = std::stringStream.str();
//	fileName = (temp).c_str();
//
//	textureAndMaterialHandler.SaveTextureToFile(d3D->GetDeviceContext(), *textureAndMaterialHandler.GetNoiseTexture(), D3DX11_IFF_BMP, fileName);
//}
//
//if(inputManager->WasKeyPressed(DIK_L))
//{
//	//Create and initialize our time... things.
//	const time_t timeObject = time(NULL);
//	struct tm parts;
//	localtime_s(&parts, &timeObject );
//
//	std::ostringstream std::stringStream;
//
//	//Create the std::string that will hold the screenshot's name when it gets pooped out into the directory
//	std::stringStream << "SavedTexture_" << (1+parts.tm_mon) << "-" << parts.tm_mday <<  "-" << parts.tm_min << "-" << parts.tm_sec << ".bmp";
//
//	LPCSTR fileName;
//	std::string temp = stringStream.str();
//	fileName = (temp).c_str();
//
//	textureAndMaterialHandler.SaveTextureToFile(d3D->GetDeviceContext(), *textureAndMaterialHandler.GetTerrainTexture(), D3DX11_IFF_BMP, fileName);
//}

#pragma endregion