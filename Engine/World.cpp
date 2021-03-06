#include "World.h"

#include "DebugOverlayHUD.h"
#include "TerrainManager.h"
#include "d3dmanager.h"
#include "cameraclass.h"
#include "frustumclass.h"
#include "GameRenderer.h"
#include "inputclass.h"
#include "MeshHandler.h"
#include "WeatherSystem.h"
#include "RenderableBundle.h"
#include "controllerclass.h"

GameWorld::GameWorld() : SettingsDependent()
{
}

GameWorld::~GameWorld()
{
	CleanUp();
}

XMFLOAT3* GameWorld::GetWindDirection()
{
	return weatherSystem->GetWindDirection();
}

void GameWorld::CleanUp()
{
	terrainManager->Shutdown();

	if(dynamicsWorld)
	{
		auto objArray = dynamicsWorld->getCollisionObjectArray();

		//remove the rigidbodies from the dynamics world and delete them
		for (int i = dynamicsWorld->getNumCollisionObjects()-1; i >= 0; i--)
		{
			dynamicsWorld->removeCollisionObject(objArray[i]);
		}
	}
}

bool GameWorld::Initialize(std::shared_ptr<D3DManager> extD3DManager, std::shared_ptr<InputClass> extInput, std::shared_ptr<GameRenderer> gameRenderer, std::shared_ptr<DebugOverlayHUD> debugHud)
{
	inputManager = extInput;
	d3D = extD3DManager;
	renderer = gameRenderer;
	debugHUD = debugHud;

	bool result = true;

	//Load settings from file
	InitializeSettings(this);

	weatherSystem = std::make_shared<WeatherSystem>();

	debugHud->AddNewWindowWithoutHandle("Chunks currently being drawn: ", &currentlyActiveChunks, DataTypeEnumMappings::UInt32);
	debugHud->AddNewWindowWithoutHandle("Chunks currently in production: ", &chunksInProduction, DataTypeEnumMappings::UInt32);

	result = InitializeRenderables();
	if(!result)
	{
		return false;
	}

	result = InitializeCollision();
	if(!result)
	{
		return false;
	}

	result = InitializeCamera();
	if(!result)
	{
		return false;
	}

	result = InitializeTerrain();
	if(!result)
	{
		return false;
	}

	return true;
}

bool GameWorld::InitializeRenderables()
{
	renderableBundle = std::make_shared<RenderableBundle>();

	std::wstring objFilepath = L"../Engine/data/Models/";
	std::wstring treeModelFilepath = objFilepath + L"LushTree.obj";

	OBJModel tempModelPtr;

	//If load succeeds
	if(renderer->GetMeshHandler()->LoadModelFromOBJFile(d3D->GetDevice(), treeModelFilepath, &tempModelPtr))
	{
		//Add model by value into renderable bundle.
		renderableBundle->objModels.push_back(tempModelPtr);

		return true;
	}

	return false;
}

bool GameWorld::InitializeCamera()
{
	//Create camera and the camera controller
	cameraController = std::make_shared<ControllerClass>(dynamicsWorld, inputManager, 0.05f); //0.03f
	camera = std::make_shared<CameraClass>(cameraController);
	camera->Initialize();

	camera->SetPosition(XMFLOAT3(0.0f, 25.0f, -100.0f));
	camera->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	camera->SetPerspectiveProjection(screenWidth, screenHeight, XM_PIDIV2, nearClip, farClip);

	debugHUD->AddNewWindowWithoutHandle("Camera position: ", camera->GetPositionPtr(),		DataTypeEnumMappings::Float3);
	debugHUD->AddNewWindowWithoutHandle("Camera rotation: ", camera->GetRotationPtr(),		DataTypeEnumMappings::Float3);

	return true;
}

bool GameWorld::InitializeCollision()
{
	broadphase				=	std::make_shared<btDbvtBroadphase>();
	collisionConfiguration	=	std::make_shared<btDefaultCollisionConfiguration>();

	dispatcher				=	std::make_shared<btCollisionDispatcher>(collisionConfiguration.get());
	solver					=	std::make_shared<btSequentialImpulseConstraintSolver>();

	dynamicsWorld			=	std::make_shared<btDiscreteDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get());

	//Gravity is probably -9.78: http://en.wikipedia.org/wiki/Gravity_of_Earth
	dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y,  gravity.z));
	dynamicsWorld->stepSimulation(bulletTimestepScale, maxSubSteps);

	frustum = std::make_shared<FrustumClass>();
	frustumAABB = std::make_shared<Lemmi2DAABB>(XMFLOAT2(-1, -1), XMFLOAT2(1, 1));

	//1.77f is 16:9 aspect ratio
	frustum->SetInternals((float)(screenWidth / screenHeight), XM_PIDIV2, nearClip, farClip);

	return true;
}

void GameWorld::ResetCamera()
{
	cameraController->ResetBody();
}

bool GameWorld::InitializeTerrain()
{
	//Initialize terrain manager
	terrainManager = std::make_shared<TerrainManager>();

	if(!terrainManager->Initialize(d3D->GetDevice(), dynamicsWorld, d3D->GetHwnd(), camera->GetPosition()))
	{
		return false;
	}

	return true;
}

void GameWorld::Update( float deltaTimeSeconds, float deltaTimeMilliseconds )
{
	chunksInProduction = terrainManager->GetChunkInProductionCount();
	currentlyActiveChunks = terrainManager->GetActiveChunkCount();

	cameraController->Update(deltaTimeMilliseconds, camera->GetWorldMatrix());
	camera->Update();

	//Advance bullet world simulation stepping
	dynamicsWorld->stepSimulation(deltaTimeSeconds, maxSubSteps);
	dynamicsWorld->clearForces();


	//Update wind system, used for wind direction and other fun things.
	weatherSystem->Update(deltaTimeSeconds);

	HandleInput();
	UpdateVisibility(deltaTimeSeconds);
}

void GameWorld::HandleInput()
{
	if(inputManager->IsKeyPressed(DIK_G))
	{
		SettingsManager::GetInstance().ReloadSettings();
	}

	if(inputManager->WasKeyPressed(DIK_N))
	{
		terrainManager->ResetTerrain();
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD1) || inputManager->WasKeyPressed(DIK_F1))
	{
		terrainManager->SetTerrainType(TerrainTypes::SeaBottom);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD2) || inputManager->WasKeyPressed(DIK_F2))
	{
		terrainManager->SetTerrainType(TerrainTypes::Plains);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD3) || inputManager->WasKeyPressed(DIK_F3))
	{
		terrainManager->SetTerrainType(TerrainTypes::Hills);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD4) || inputManager->WasKeyPressed(DIK_F4))
	{
		terrainManager->SetTerrainType(TerrainTypes::Terraces);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD5) || inputManager->WasKeyPressed(DIK_F5))
	{
		terrainManager->SetTerrainType(TerrainTypes::DramaticHills);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD6) || inputManager->WasKeyPressed(DIK_F6))
	{
		terrainManager->SetTerrainType(TerrainTypes::FlyingIslands);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD7) || inputManager->WasKeyPressed(DIK_F7))
	{
		terrainManager->SetTerrainType(TerrainTypes::Alien);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD8) || inputManager->WasKeyPressed(DIK_F8))
	{
		terrainManager->SetTerrainType(TerrainTypes::Fancy);
	}

	if(inputManager->WasKeyPressed(DIK_NUMPAD9) || inputManager->WasKeyPressed(DIK_F9))
	{
		terrainManager->SetTerrainType(TerrainTypes::Cave);
	}
}

void GameWorld::UpdateVisibility(float deltaTime)
{
	frustum->ConstructFrustum(camera->GetFarClip(), &camera->GetProj(), &camera->GetView());
	frustum->CalculateFrustumExtents(frustumAABB.get(), XMLoadFloat3(&camera->GetPosition()), camera->ForwardVector(), camera->UpVector());

	//If this function returns false, it means that we haven't moved far enough to have loaded in any new chunks, hence we don't need to clear and regrow the terrainChunks vector
	if(terrainManager->UpdateAgainstAABB(frustumAABB.get(), deltaTime))
	{
		renderableBundle->terrainChunks.clear();
		renderableBundle->terrainChunks = terrainManager->GetActiveChunks();
	}
}

void GameWorld::OnSettingsReload( Config* cfg )
{
	const Setting& settings = cfg->getRoot()["physics"];

	settings.lookupValue("timeStep", bulletTimestepScale);
	settings.lookupValue("maxSubsteps", maxSubSteps);

	settings.lookupValue("gravityX", gravity.x);
	settings.lookupValue("gravityY", gravity.y);
	settings.lookupValue("gravityZ", gravity.z);

	const Setting& settings2 = cfg->getRoot()["rendering"];

	settings2.lookupValue("windowWidth", screenWidth);
	settings2.lookupValue("windowHeight", screenHeight);
	settings2.lookupValue("farClip", farClip);
	settings2.lookupValue("nearClip", nearClip);
}

//TODO: camera body that collides with meshes
//		Generate meshes from marching cubes class and add them to dynamicsWorld in the smoothest way possible.
//		

//TODO: Keep list of renderables here? Then we have a GetRenderableBundle or something along those lines that is sent to renderer
// RenderableBundle should probably just contain a vector of terrainchunks
//
// Further idea: Maybe renderable bundle should keep an array or a big collection of textures... or should everything be fetched from textureAndMaterial manager?
// Maybe all sorts of meshes should keep a textureID handle. TextureAndMaterialManager has a... LoadTexture(string filePath); that returns a handle
// 
//TextureAndMaterial handler has an unsorted map or smth that takes string keys and returns handles. The keys are the strings that were used to load the textures from the HDD.
//
// This all means that whenever we add a texture, it first checks the map to see if that exact texture has already been added, if that's the case, return handle to that texture. 
// If that's NOT the case then we do the whole DOD container thing where we put it in the next free array slot and assign a new index etcetc.
// 
//Could probably apply exactly the same pattern with models loaded in from hdd. Could call it MeshHandler.
// 


//So regarding the whole RenderableBundle thing, maybe just make the whole thing into a full blown class/manager that has different functions for adding/removing meshes, coupled with DOD containers behind.