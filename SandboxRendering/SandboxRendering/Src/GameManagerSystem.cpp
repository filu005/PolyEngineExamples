#include "GameManagerSystem.hpp"

#include "GameManagerWorldComponent.hpp"

#include <Core.hpp>
#include <Debugging/DebugDrawComponents.hpp>
#include <Debugging/DebugDrawSystem.hpp>
#include <ECS/DeferredTaskSystem.hpp>
#include <Input/InputWorldComponent.hpp>
#include <Movement/FreeFloatMovementComponent.hpp>
#include <Rendering/MeshRenderingComponent.hpp>
#include <Rendering/PostprocessSettingsComponent.hpp>
#include <Rendering/SkyboxWorldComponent.hpp>
#include <Rendering/ViewportWorldComponent.hpp>
#include <Rendering/Camera/CameraComponent.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Resources/ResourceManager.hpp>
#include <Time/TimeWorldComponent.hpp>
#include <UI/ScreenSpaceTextComponent.hpp>

using namespace Poly;

void GameManagerSystem::CreateScene(World* world)
{
	gConsole.LogInfo("GameManagerSystem::CreateScene");

	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();	

	srand(42);
	
	CreateCamera(world);

	// gameMgrCmp->GameEntities.PushBack(Ground = CreateModel(world, "Models/Ground/Ground.fbx"));

	DeferredTaskSystem::AddWorldComponentImmediate<SkyboxWorldComponent>(world, "HDR/HDR.hdr", eResourceSource::GAME);
	
	gameMgrCmp->Model = CreateModel(world, "Models/leather_shoes/Leather_Shoes.obj");
	gameMgrCmp->Model->GetTransform().SetGlobalTranslation(Vector(500.0f, 0.0f, 0.0f));

	// gameMgrCmp->Model = CreateModel(world, "Models/kv-2-heavy-tank-1940/model.obj");
	// gameMgrCmp->Model->GetTransform().SetGlobalScale(Vector(5.0f, 5.0f, 5.0f));
	// gameMgrCmp->Model->GetTransform().SetGlobalTranslation(Vector(0.0f, 10.0f, 0.0f));

	// gameMgrCmp->Model = CreateModel(world, "Models/1972-datsun-240k-gt/model.obj");
	// gameMgrCmp->Model->GetTransform().SetGlobalScale(Vector::ONE * 20.0f);

	// gameMgrCmp->Model = CreateModel(world, "Models/leather_shoes/Leather_Shoes.obj");

	CreatePBRShpereGrid(world, Vector(0.0f, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f, 1.0f));
	CreatePBRShpereGrid(world, Vector(-300.0f, 0.0f, 0.0f), Color(0.5f, 0.5f, 0.5f, 1.0f));
	CreatePBRShpereGrid(world, Vector(-600.0f, 0.0f, 0.0f), Color(1.0f, 1.0f, 1.0f, 1.0f));

	CreateSponza(world);

	CreateTextUI(world);

	// CreateTranslucent(world);

	CreatePointLights(world, 128);

	// SpawnParticles(world);
}

void GameManagerSystem::CreateTextUI(World* world)
{
	Entity* text = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<ScreenSpaceTextComponent>(
		world,
		text,
		Vector2i(50, 50),
		"Fonts/Raleway/Raleway-Regular.ttf",
		eResourceSource::ENGINE,
		40.0f,
		"powered by PolyEngine"
	);
}

Entity* GameManagerSystem::CreateModel(World* world, String path)
{
	Entity* model = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DebugDrawableComponent>(world, model, eDebugDrawPreset::STATIC);
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, model, path, eResourceSource::GAME);
	return model;
}

void GameManagerSystem::CreateCamera(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* camera = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<CameraComponent>(world, camera, 35_deg, 1.0f, 5000.f);
	DeferredTaskSystem::AddComponentImmediate<FreeFloatMovementComponent>(world, camera, 100.0f, 0.003f, 10.0f);
	gameMgrCmp->PostCmp = DeferredTaskSystem::AddComponentImmediate<PostprocessSettingsComponent>(world, camera);
	gameMgrCmp->PostCmp->Exposure = 3.0f;

	EntityTransform& cameraTrans = camera->GetTransform();
	cameraTrans.SetGlobalTranslation(Vector(-550.0f, 180.0f, 0.0f));
	cameraTrans.SetGlobalRotation(Quaternion(Vector::UNIT_Y, -90.0_deg) * Quaternion(Vector::UNIT_X, -10.0_deg));
	// cameraTrans.SetGlobalRotation(Quaternion(Matrix(cameraTrans.GetGlobalTranslation(), Vector(0.0f, 0.0f, 0.0f))));
	world->GetWorldComponent<ViewportWorldComponent>()->SetCamera(0, world->GetComponent<CameraComponent>(camera));
	gameMgrCmp->Camera = camera;

	Entity* keyDirLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DirectionalLightComponent>(world, keyDirLight, Color(1.0f, 1.0f, 1.0f), 5.0f);
	keyDirLight->GetTransform().SetGlobalRotation(Quaternion(Vector::UNIT_Y, -45_deg) * Quaternion(Vector::UNIT_X, 65_deg));
	gameMgrCmp->GameEntities.PushBack(keyDirLight);
	
}

void GameManagerSystem::CreatePBRShpereGrid(World* world, Vector pos, Color albedo)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	// Entity* camera = DeferredTaskSystem::SpawnEntityImmediate(world);
	// DeferredTaskSystem::AddComponentImmediate<CameraComponent>(world, camera, 50_deg, 1.0f, 5000.f);
	// DeferredTaskSystem::AddComponentImmediate<FreeFloatMovementComponent>(world, camera, 10.0f, 0.003f);
	// DeferredTaskSystem::AddComponentImmediate<PostprocessSettingsComponent>(world, camera);
	// EntityTransform& cameraTrans = camera->GetTransform();
	// cameraTrans.SetGlobalTranslation(Vector(800.0f, 180.0f, 0.0f));
	// cameraTrans.SetGlobalRotation(Quaternion(Vector::UNIT_Y, 90.0_deg));
	// world->GetWorldComponent<ViewportWorldComponent>()->SetCamera(0, world->GetComponent<CameraComponent>(camera));

	// Entity* keyDirLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	// DeferredTaskSystem::AddComponentImmediate<DirectionalLightComponent>(world, keyDirLight, Color(1.0f, 1.0f, 1.0f), 1.0f);
	// keyDirLight->GetTransform().SetGlobalRotation(Quaternion(Vector::UNIT_Y, -45_deg) * Quaternion(Vector::UNIT_X, 65_deg));
	// gameMgrCmp->GameEntities.PushBack(keyDirLight);
	// Entity* fillDirLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	// DeferredTaskSystem::AddComponentImmediate<DirectionalLightComponent>(world, fillDirLight, Color(0.75f, 0.95f, 1.0f), 0.1f);
	// fillDirLight->GetTransform().SetGlobalRotation(Quaternion(Vector::UNIT_Y, -45_deg + 180_deg) * Quaternion(Vector::UNIT_X, 65_deg + 180_deg));
	// gameMgrCmp->GameEntities.PushBack(fillDirLight);

	// Entity* ground = DeferredTaskSystem::SpawnEntityImmediate(world);
	// EntityTransform& groundTrans = ground->GetTransform();
	// groundTrans.SetGlobalTranslation(Vector(0.0f, 0.0f, 0.0f));	
	// MeshRenderingComponent* meshCmp = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, ground, "Models/Ground/Ground.fbx", eResourceSource::GAME);
	// meshCmp->SetShadingModel(eShadingModel::PBR);
	// int materialsNum = meshCmp->GetMesh()->GetSubMeshes().GetSize();
	// for (int i = 0; i < materialsNum; ++i)
	// {
	// 	meshCmp->SetPBRMaterial(i, material);
	// }
	// gameMgrCmp->GameEntities.PushBack(ground);

	for (int z = 0; z < 5; ++z)
	{
		for (int y = 0; y < 5; ++y)
		{
			Entity* sphere = DeferredTaskSystem::SpawnEntityImmediate(world);
			EntityTransform& sphereTrans = sphere->GetTransform();
			sphereTrans.SetGlobalTranslation(pos + Vector(50.0f * y, 0.0f, 50.0f * z) - (Vector(50.0f * 5.0f, -100.0f, 50.0f * 5.0f) * 0.5f));
			sphereTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f) * 20.0f);
			sphereTrans.SetLocalRotation(Quaternion(Vector::UNIT_Z, 90.0_deg));
			MeshRenderingComponent* meshCmp = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, sphere, "Models/Primitives/Sphere_HighPoly.obj", eResourceSource::GAME);
			int materialsNum = meshCmp->GetMesh()->GetSubMeshes().GetSize();
			for (int i = 0; i < materialsNum; ++i)
			{
				float Roughness	= ((0.01f + z) / 5.0f);
				float Metallic	= ((0.01f + y) / 5.0f);
				meshCmp->SetMaterial(i, Material(
					Color(0.0f, 0.0f, 0.0f, 0.0f),
					albedo,
					Roughness,
					Metallic,
					0.5f
				));
			}

			gameMgrCmp->GameEntities.PushBack(sphere);
		}
	}
}

void GameManagerSystem::CreateTranslucent(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Material material(
		Color(0.01f, 0.01f, 0.01f, 1.0f),
		Color(1.0f, 0.5f, 0.5f, 0.75f),
		1.0f,
		1.0f,
		0.5f
	);

	for (int i = 0; i < 8; ++i)
	{
		Entity* translucent = DeferredTaskSystem::SpawnEntityImmediate(world);
		EntityTransform& translucentTrans = translucent->GetTransform();
		Vector randomOffset = RandomVectorRange(-1.0f, 1.0f) * 200.0f;
		translucentTrans.SetGlobalTranslation(Vector(0.0f, 200.0f, 0.0f) + randomOffset);
		translucentTrans.SetLocalScale(Vector(100.0f, 100.0f, 100.0f));
		MeshRenderingComponent* meshCmp = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, translucent, "Models/Primitives/Sphere_HighPoly.obj", eResourceSource::GAME);
		int materialsNum = meshCmp->GetMesh()->GetSubMeshes().GetSize();
		for (int i = 0; i < materialsNum; ++i)
		{
			meshCmp->SetMaterial(i, material);
		}

		gameMgrCmp->GameEntities.PushBack(translucent);
	}
}

void GameManagerSystem::CreateSponza(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	// Entity* camera = DeferredTaskSystem::SpawnEntityImmediate(world);
	// CameraComponent* cameraCmp = DeferredTaskSystem::AddComponentImmediate<CameraComponent>(world, camera, 50_deg, 1.0f, 5000.f);
	// cameraCmp->SetRenderingMode(eRenderingModeType::IMMEDIATE_DEBUG);
	// DeferredTaskSystem::AddComponentImmediate<FreeFloatMovementComponent>(world, camera, 10.0f, 0.003f);
	// EntityTransform& cameraTrans = camera->GetTransform();
	// cameraTrans.SetGlobalTranslation(Vector(800.0f, 180.0f, 0.0f));
	// cameraTrans.SetGlobalRotation(Quaternion(Vector::UNIT_Y, 90.0_deg));
	// world->GetWorldComponent<ViewportWorldComponent>()->SetCamera(0, world->GetComponent<CameraComponent>(camera));

	// Entity* keyDirLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	// DeferredTaskSystem::AddComponentImmediate<DirectionalLightComponent>(world, keyDirLight, Color(0.1f, 0.095f, 0.075f), 1.0f);
	// EntityTransform& dirLightTrans = keyDirLight->GetTransform();
	// dirLightTrans.SetLocalRotation(Quaternion(Vector::UNIT_Y, -45_deg) * Quaternion(Vector::UNIT_X, 65_deg));
	// gameMgrCmp->GameEntities.PushBack(keyDirLight);

	Entity* sponza = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, sponza, "Models/Sponza/sponza.obj", eResourceSource::GAME);

	gameMgrCmp->GameEntities.PushBack(sponza);
}

void GameManagerSystem::CreateSponzaSample(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* camera = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<CameraComponent>(world, camera, 40_deg, 1.0f, 5000.f);
	DeferredTaskSystem::AddComponentImmediate<FreeFloatMovementComponent>(world, camera, 10.0f, 0.003f);
	EntityTransform& cameraTrans = camera->GetTransform();
	cameraTrans.SetGlobalTranslation(Vector(300.0f, 0.0f, 0.0f));
	cameraTrans.SetGlobalRotation(Quaternion(Vector::UNIT_Y, 90.0_deg));

	world->GetWorldComponent<ViewportWorldComponent>()->SetCamera(0, world->GetComponent<CameraComponent>(camera));

	Entity* sponza = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, sponza, "Models/Sponza/sponza_lion.obj", eResourceSource::GAME);
	gameMgrCmp->GameEntities.PushBack(sponza);

	Vector lightPosition = Vector(50.0f, 0.0f, 50.0f);
	Entity* lightEntity = CreatePointLight(world, lightPosition, 100.0f);
	gameMgrCmp->LightsStartPositions.PushBack(lightPosition);
	gameMgrCmp->PointLightEntities.PushBack(lightEntity);
}

void GameManagerSystem::Update(World* world)
{
	UpdateParticles(world);

	UpdateLights(world);

	// UpdateModel(world);

	Vector offset = Vector(800.0f, 0.0f, 0.0f);
	DebugDrawSystem::DrawLine(world, offset, offset + Vector::UNIT_Y * 1000.0f, Color::RED);
	DebugDrawSystem::DrawBox(world, offset + Vector(-100.0f, 0.0f, -100.0f), offset + Vector(100.0f, 200.0f, 100.0f), Color::RED);

	// UpdatePostProcess(world);
}

void GameManagerSystem::UpdatePostProcess(World* world)
{
	float time = (float)(world->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	time *= 0.1f;
	float exposure = 0.25f * floor(10.0f*4.0f * (time - floor(time)));
	gameMgrCmp->PostCmp->Exposure = exposure;
	gConsole.LogInfo("GameManagerSystem::Update exposure: {}", gameMgrCmp->PostCmp->Exposure);
}

void GameManagerSystem::Deinit(World* world)
{
	gConsole.LogInfo("GameManagerSystem::Cleanup");
}

void GameManagerSystem::CreatePointLights(World* world, int quota)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	gameMgrCmp->LightsStartPositions.Reserve(quota);

	for (int i = 0; i < quota; ++i)
	{
		Vector position = Vector(RandomRange(-1.0f, 1.0f)*1000.0f, RandomRange(0.0f, 800.0f), RandomRange(-1.0f, 1.0f)*500.0f);
		float rangeRnd = pow(RandomRange(0.0f, 1.0f), 8.0f);
		float range = 100.0f + rangeRnd * 2000.0f;
		Entity* lightEntity = CreatePointLight(world, position, range);

		gameMgrCmp->LightsStartPositions.PushBack(position);
		gameMgrCmp->PointLightEntities.PushBack(lightEntity);

		// gConsole.LogInfo("GameManagerSystem::CreateTestLights #{}: Positon: {}, Radius: {}",
		// 	i, position, Range);
	}

	for (size_t i = 0; i < gameMgrCmp->PointLightEntities.GetSize(); ++i)
	{
		gameMgrCmp->GameEntities.PushBack(gameMgrCmp->PointLightEntities[i]);
	}
}

void GameManagerSystem::UpdateLights(World* world)
{
	float time = (float)(world->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	for (size_t i = 0; i < gameMgrCmp->LightsStartPositions.GetSize(); ++i)
	{
		Entity* pointLight = gameMgrCmp->PointLightEntities[i];
		Vector startPosition = gameMgrCmp->LightsStartPositions[i];
		Vector offset = Vector::UNIT_Y * Sin(2.0_rad * time + 1.0_rad * startPosition.X) * 100.0f;
		EntityTransform& pointLightTrans = pointLight->GetTransform();
		pointLightTrans.SetGlobalTranslation(startPosition + offset);
	}
}

void GameManagerSystem::UpdateModel(World* world)
{
	float time = (float)(world->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	if (gameMgrCmp->Model)
	{
		EntityTransform& modelTrans = gameMgrCmp->Model->GetTransform();
		modelTrans.SetLocalRotation(Quaternion(Vector::UNIT_Y, 10.0_deg * time));
	}
}

Entity* GameManagerSystem::CreatePointLight(World* world, Vector& position, float Range)
{
	Color lightColor = Color(RandomRange(0.0f, 1.0f), RandomRange(0.0f, 1.0f), RandomRange(0.0f, 1.0f));

	Entity* pointLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	PointLightComponent* pointLightCmp = DeferredTaskSystem::AddComponentImmediate<PointLightComponent>(world, pointLight, lightColor, 100.0f * 1000.0f, Range);
	MeshRenderingComponent* meshCmp = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, pointLight, "Models/Primitives/Sphere_LowPoly.obj", eResourceSource::GAME);
	meshCmp->SetMaterial(0, Material(Color(0.0f, 0.0f, 0.0f, 0.0f), lightColor, 0.0f, 1.0f, 0.5f));
	EntityTransform& pointLightTrans = pointLight->GetTransform();
	pointLightTrans.SetGlobalScale(Vector::ONE * 5.0f);
	pointLightTrans.SetGlobalTranslation(position);

	return pointLight;
}

void GameManagerSystem::CreateSpotLight(World* world, float Range)
{
	Vector spotLightPos = Vector(50.0f, 50.0f, 0.0f);
	Color lightColor = Color(1.0f, 0.5f, 0.0f) + Color(RandomRange(0.0f, 1.0f), RandomRange(0.0f, 0.5f), RandomRange(0.0f, 0.2f));
	Quaternion spotLightRot = Quaternion(Vector::UNIT_Y, -45_deg) * Quaternion(Vector::UNIT_X, -35_deg);
	// float pointLightRange = 100.0f;
	Entity* spotLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<SpotLightComponent>(world, spotLight, lightColor, 1.0f, Range, 5.0f, 17.0f);
	EntityTransform& spotLightTrans = spotLight->GetTransform();
	spotLightTrans.SetLocalTranslation(spotLightPos);
	spotLightTrans.SetLocalRotation(spotLightRot);

	Entity* spotLightDebugSource = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, spotLightDebugSource, "Models/Primitives/Sphere_LowPoly.obj", eResourceSource::GAME);
	MeshRenderingComponent* spotLightMesh = world->GetComponent<MeshRenderingComponent>(spotLightDebugSource);
	spotLightMesh->SetMaterial(0, Material(lightColor, lightColor, 1.0f, 1.0f, 0.5f));
	EntityTransform& spotLightDebugSourceTrans = spotLightDebugSource->GetTransform();
	spotLightDebugSource->SetParent(spotLight);
	spotLightDebugSourceTrans.SetLocalScale(2.0f);
	spotLightDebugSourceTrans.SetLocalTranslation(Vector(0.0f, 0.0f, 0.0f));
}

void GameManagerSystem::UpdateParticles(World* world)
{
	float time = (float)(world->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());
	// float deltaTime = (float)(TimeSystem::GetTimerDeltaTime(world, Poly::eEngineTimer::GAMEPLAY));

	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Vector translation = Vector::UNIT_Y * (0.2f*Abs(Sin(1.0_rad*3.1415f *time + 0.5_rad)));
	Vector scale = Vector::ONE * (1.0f + 0.1f*(0.5f + 0.5f*Sin(2.0_rad*3.1415f *time + 0.5_rad)));
	Quaternion rotation = Quaternion(Vector::UNIT_Y, 0.5_rad * time);

	if (gameMgrCmp->particleHeart)
	{
		gameMgrCmp->particleHeart->GetTransform().SetGlobalTranslation(translation);
		gameMgrCmp->particleHeart->GetTransform().SetGlobalScale(scale);
		gameMgrCmp->particleHeart->GetTransform().SetGlobalRotation(rotation);
	}

	if (gameMgrCmp->particleHeartImpact0)
	{
		gameMgrCmp->particleHeartImpact0->GetTransform().SetGlobalTranslation(translation);
		gameMgrCmp->particleHeartImpact0->GetTransform().SetGlobalScale(scale);
		gameMgrCmp->particleHeartImpact0->GetTransform().SetGlobalRotation(rotation);
	}

	if (gameMgrCmp->particleHeartImpact1)
	{
		gameMgrCmp->particleHeartImpact1->GetTransform().SetGlobalRotation(rotation);
	}

	if (gameMgrCmp->particleHeartImpact2)
	{
		gameMgrCmp->particleHeartImpact2->GetTransform().SetGlobalTranslation(Vector::UNIT_Y * 0.2f);
		gameMgrCmp->particleHeartImpact2->GetTransform().SetGlobalRotation(rotation);
	}

	if (gameMgrCmp->particleLocalSpace)
	{
		gameMgrCmp->particleLocalSpace->GetTransform().SetGlobalTranslation(Vector(0.0f, 4.0f, 0.0f) + Vector(Cos(100.0_deg * time), 0.0f, Sin(100.0_deg * time)) * 6.0f);
	}

	if (gameMgrCmp->particleWorldSpace)
	{
		gameMgrCmp->particleWorldSpace->GetTransform().SetGlobalTranslation(Vector(0.0f, 4.0f, 0.0f) + Vector(Cos(100.0_deg * time + 180.0_deg), 0.0f, Sin(100.0_deg * time + 180.0_deg)) * 6.0f);
	}
}

void GameManagerSystem::SpawnParticles(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	Vector particlesPosition = Vector(0.0f, 0.0f, 0.0f);
	gameMgrCmp->particleDefault = SpawnEmitterDefault(world, particlesPosition);
	gameMgrCmp->particleWorldSpace = SpawnEmitterWorldSpace(world, particlesPosition);
	gameMgrCmp->particleLocalSpace = SpawnEmitterLocalSpace(world, particlesPosition);
	gameMgrCmp->particleAmbient = SpawnEmitterAmbient(world, particlesPosition);
	gameMgrCmp->particleAmbientWind = SpawnEmitterAmbientWind(world, particlesPosition);

	// SpawnHeartSystem(world);
}

void GameManagerSystem::SpawnSpritesSheets(World* world)
{
	Vector spritesheetsPosition = Vector(-15.0f, 0.0f, 0.0f);
	SpawnSpritesheet11(world, spritesheetsPosition + Vector(-5.0f, 4.0f, 0.0f));
	SpawnSpritesheet22(world, spritesheetsPosition + Vector(0.0f, 4.0f, 0.0f));
	SpawnSpritesheet44(world, spritesheetsPosition + Vector(5.0f, 4.0f, 0.0f));
	SpawnSpritesheet42(world, spritesheetsPosition + Vector(-5.0f, 8.0f, 0.0f));
	SpawnSpritesheet41(world, spritesheetsPosition + Vector(0.0f, 8.0f, 0.0f));
	SpawnSpritesheet44Random(world, spritesheetsPosition + Vector(5.0f, 8.0f, 0.0f));
	SpawnSpritesheetGandalf(world, spritesheetsPosition + Vector(0.0f, 12.0f, 0.0f));
}

void GameManagerSystem::SpawnShaderball(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* shaderball = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& shaderballTrans = shaderball->GetTransform();
	shaderballTrans.SetLocalTranslation(Vector(0.0f, 5.0f, 0.0f));
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, shaderball, "Models/shaderball/PolyEngine_shaderball.fbx", eResourceSource::GAME);
	MeshRenderingComponent* ballMesh = world->GetComponent<MeshRenderingComponent>(shaderball);
	ballMesh->SetMaterial(0, Material(Color(1.0f, 1.0f, 1.0f), Color(1.0f, 1.0f, 0.0f), 1.0f, 1.0f, 0.5f));
	ballMesh->SetMaterial(1, Material(Color(1.0f, 1.0f, 1.0f), Color(0.4f, 0.4f, 0.4f), 1.0f, 1.0f, 0.5f));
	shaderballTrans.SetLocalScale(0.1f);
	gameMgrCmp->GameEntities.PushBack(shaderball);
}


// #pragma region Spritesheet examples

void GameManagerSystem::SpawnSpritesheet11(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(1.0f, 1.0f);
	settings.SpritePath = "Textures/test_1_1.png";
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheet22(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(2.0f, 2.0f);
	settings.SpritePath = "Textures/test_2_2.png";
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheet44(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(4.0f, 4.0f);
	settings.SpritePath = "Textures/test_4_4.png";
	settings.SpriteColor = Color(0.0f, 1.0f, 0.0f, 0.5f);
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheet42(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(4.0f, 2.0f);
	settings.SpritePath = "Textures/test_4_2.png";
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheet41(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(4.0f, 1.0f);
	settings.SpritePath = "Textures/test_4_1.png";
	settings.SpriteColor = Color::RED;
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheet44Random(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(4.0f, 4.0f);
	settings.SpritePath = "Textures/test_4_4.png";
	settings.IsRandomStartFrame = true;
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

void GameManagerSystem::SpawnSpritesheetGandalf(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* spriteSheetEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& SpriteSheetTrans = spriteSheetEnt->GetTransform();
	SpriteSheetTrans.SetLocalTranslation(pos);
	SpriteSheetTrans.SetLocalScale(Vector(1.0f, 1.0f, 1.0f));
	SpritesheetSettings settings;
	settings.SubImages = Vector2f(4.0f, 4.0f);
	settings.SpritePath = "Textures/gandalf_anim.png";
	settings.Speed = 0.2f;
	DeferredTaskSystem::AddComponentImmediate<SpritesheetComponent>(world, spriteSheetEnt, settings);
	gameMgrCmp->GameEntities.PushBack(spriteSheetEnt);
}

// #pragma endregion


// #pragma region Particle Examples

ParticleComponent* GameManagerSystem::SpawnEmitterDefault(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& particlesTrans = particlesEnt->GetTransform();
	particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(4.0f, 4.0f);
	spriteSettings.SpritePath = "Textures/none.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 20;
	settings.BurstSizeMin = 2;
	settings.BurstSizeMax = 5;
	settings.BurstTimeMax = 1.0f;
	settings.BurstTimeMin = 5.0f;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::LOCAL_SPACE;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		p->Position += RandomVectorRange(-10.0f, 10.0f);
		p->Velocity = Vector(0.0f, 0.0f, 0.0f);
		p->LifeTime = RandomRange(1.0f, 1.0f);
		p->Scale = Vector::ONE * RandomRange(10.0f, 10.0f);
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterWorldSpace(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& particlesTrans = particlesEnt->GetTransform();
	particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/test_2_2.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 50;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::WORLD_SPACE;
	settings.BurstTimeMin = 0.05f;
	settings.BurstTimeMax = 0.20f;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		p->Position += RandomVectorRange(-1.0f, 1.0f);
		p->Velocity = RandomVectorRange(0.5f, 1.0f) * 0.001f;
		p->LifeTime = RandomRange(0.5f, 2.0f);
		p->Scale = Vector::ONE * RandomRange(1.0f, 2.0f);
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterLocalSpace(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& particlesTrans = particlesEnt->GetTransform();
	particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(4.0f, 4.0f);
	spriteSettings.SpritePath = "Textures/test_4_4.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 20;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::LOCAL_SPACE;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		p->Position = RandomVectorRange(-1.0f, 1.0f);
		p->Velocity = RandomVectorRange(0.5f, 1.0f) * 0.001f;
		p->LifeTime = RandomRange(1.0f, 5.0f);
		p->Scale = Vector::ONE * RandomRange(1.0f, 2.0f);
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterAmbient(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& particlesTrans = particlesEnt->GetTransform();
	particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/test_2_2.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 500;
	settings.InitialSize = 500;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::WORLD_SPACE;
	settings.BurstTimeMin = 1.0f;
	settings.BurstTimeMax = 2.0f;
	settings.BurstSizeMin = 10;
	settings.BurstSizeMax = 20;
	settings.BaseColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		p->Position += RandomVectorRange(-1.0f, 1.0f) * 10.0f;
		p->Velocity = RandomVectorRange(-1.0f, 1.0f) * 0.001f;
		p->LifeTime = RandomRange(5.0f, 10.0f);
		p->Scale = Vector::ONE * RandomRange(0.025f, 0.05f);
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterAmbientWind(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& particlesTrans = particlesEnt->GetTransform();
	particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/test_2_2.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 200;
	settings.InitialSize = 0;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::WORLD_SPACE;
	settings.BurstTimeMin = 1.0f;
	settings.BurstTimeMax = 2.0f;
	settings.BurstSizeMin = 200;
	settings.BurstSizeMax = 400;
	settings.BaseColor = Color(1.0f, 1.0f, 1.0f, 0.1f);
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		p->Position += Vector(-20.0f, 2.0f, 0.0f) + RandomVectorRange(-1.0f, 1.0f) * 10.0f;
		p->Velocity = Vector(RandomRange(0.75f, 1.0f) * 0.5f, 0.0f, 0.0f);
		p->LifeTime = RandomRange(0.75f, 1.0f) * 2.0f;
		p->Scale = Vector::ONE * RandomRange(0.75f, 1.0f) * 4.0f;
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterHeart(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	// EntityTransform& particlesTrans = particlesEnt->GetTransform();
	// particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/strokes_2_2.png";
	spriteSettings.Speed = 0.0f;

	ParticleEmitter::Settings settings;
	settings.MaxSize = 1000;
	settings.BaseColor = Color(1.2f, 0.8f, 0.8f, 0.5f);
	settings.BurstTimeMin = 0.01f;
	settings.BurstTimeMax = 0.05f;
	settings.BurstSizeMin = 10;
	settings.BurstSizeMax = 30;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::LOCAL_SPACE;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rnd = RandomVectorRange(-1.0f, 1.0f);
		rnd.Normalize();
		rnd.Z = rnd.Z * 0.5f;
		rnd.Z = rnd.Z * (1.0f - rnd.Y * 1.0f);
		rnd.Y = 0.9f*rnd.Y - Abs(rnd.X) * sqrt((20.0f - Abs(rnd.X)) / 15.0f);
		rnd.Y *= -0.8f;
		p->Position += rnd;
		p->Velocity = RandomVectorRange(0.5f, 1.0f) * 0.001f;
		p->LifeTime = RandomRange(3.0f, 5.0f);
		p->Scale = Vector::ONE * RandomRange(0.02f, 0.2f);
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterHeartImpact(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	// EntityTransform& particlesTrans = particlesEnt->GetTransform();
	// particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/strokes_2_2.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 1000;
	settings.BaseColor = Color(1.5f, 1.0f, 1.0f, 0.95f);
	settings.BurstTimeMin = 0.1f;
	settings.BurstTimeMax = 0.5f;
	settings.BurstSizeMin = 10;
	settings.BurstSizeMax = 30;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::LOCAL_SPACE;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rndPos = RandomVectorRange(-1.0f, 1.0f);
		rndPos.Normalize();
		rndPos.Z = rndPos.Z * 0.5f;
		rndPos.Z = rndPos.Z * (1.0f - rndPos.Y * 1.0f);
		rndPos.Y = 0.9f*rndPos.Y - Abs(rndPos.X) * sqrt((20.0f - Abs(rndPos.X)) / 15.0f);
		rndPos.Y *= -0.8f;
		p->Position += rndPos;
		Vector rndVel = RandomVectorRange(-1.0f, 1.0f);
		rndVel.Normalize();
		p->Velocity = rndVel * 0.01f;
		p->Acceleration = rndVel * 0.0001f;
		p->LifeTime = RandomRange(8.0f, 10.0f);
		p->Scale = Vector::ONE * RandomRange(0.01f, 0.02f);
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Scale = Vector::ONE * Lerp(0.04f, 0.01f, pow(p->Age / p->LifeTime, 16.0f));
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

ParticleComponent* GameManagerSystem::SpawnEmitterHeartImpact2(World* world, Vector pos)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* particlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	// EntityTransform& particlesTrans = particlesEnt->GetTransform();
	// particlesTrans.SetLocalTranslation(pos);

	SpritesheetSettings spriteSettings;
	spriteSettings.SubImages = Vector2f(2.0f, 2.0f);
	spriteSettings.SpritePath = "Textures/strokes_2_2.png";

	ParticleEmitter::Settings settings;
	settings.MaxSize = 1000;
	settings.BaseColor = Color(2.0f, 0.5f, 0.5f, 0.2f);
	settings.BurstTimeMin = 1.0f;
	settings.BurstTimeMax = 1.0f;
	settings.BurstSizeMin = 200;
	settings.BurstSizeMax = 200;
	settings.Spritesheet = spriteSettings;
	settings.SimulationSpace = ParticleEmitter::eSimulationSpace::LOCAL_SPACE;
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rndPos = RandomVectorRange(-1.0f, 1.0f);
		rndPos.Normalize();
		rndPos.Z = rndPos.Z * 0.5f;
		rndPos.Z = rndPos.Z * (1.0f - rndPos.Y * 1.0f);
		rndPos.Y = 0.9f*rndPos.Y - Abs(rndPos.X) * sqrt((20.0f - Abs(rndPos.X)) / 15.0f);
		rndPos.Y *= -0.8f;
		rndPos.Y += 0.02f;
		// rndPos *= 1.2f;
		Vector rndVel = rndPos;
		Vector rndAccel = RandomVectorRange(0.5f, 1.0f);
		p->Position += rndPos;
		p->Velocity = rndVel * 0.01f;
		p->Acceleration = rndVel * 0.001f + Vector(-1.5f * rndAccel.X, 1.0f * rndAccel.Y, 0.5f * rndAccel.Z) * 0.001f;
		p->LifeTime = RandomRange(0.2f, 0.75f);
		p->Scale = Vector::ONE * RandomRange(0.04f, 0.05f);
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Scale = Vector::ONE * Lerp(0.4f, 0.001f, pow(p->Age / p->LifeTime, 8.0f));
	};

	gameMgrCmp->GameEntities.PushBack(particlesEnt);

	ParticleComponent* particleCmp = DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, particlesEnt, settings);
	return particleCmp;
}

void GameManagerSystem::SpawnHeartSystem(World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	gameMgrCmp->particleHeart = SpawnEmitterHeart(world, Vector(0.0f, 4.0f, 0.0f));
	gameMgrCmp->particleHeartImpact0 = SpawnEmitterHeartImpact(world, Vector(0.0f, 4.0f, 0.0f));
	gameMgrCmp->particleHeartImpact1 = SpawnEmitterHeartImpact(world, Vector(0.0f, 4.0f, 0.0f));
	gameMgrCmp->particleHeartImpact2 = SpawnEmitterHeartImpact2(world, Vector(0.0f, 4.0f, 0.0f));
}

// #pragma endregion