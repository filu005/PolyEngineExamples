#include "GameManagerSystem.hpp"

#include <Rendering/MeshRenderingComponent.hpp>
#include <Physics3D/Collider3DComponent.hpp>
#include <Physics3D/Rigidbody3DComponent.hpp>
#include <Configs/ConfigBase.hpp>
#include <Movement/FreeFloatMovementComponent.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Rendering/Camera/CameraComponent.hpp>
#include <Rendering/ViewportWorldComponent.hpp>
#include <ActorMovementComponent.hpp>
#include <Rendering/PostprocessSettingsComponent.hpp>
#include <Debugging/DebugDrawComponents.hpp>
#include <Resources/ResourceManager.hpp>
#include <Resources/MeshResource.hpp>
#include <Debugging/DebugDrawSystem.hpp>
#include <Physics3D/Physics3DShapes.hpp>

#include "GameManagerWorldComponent.hpp"
#include "ActorComponent.hpp"
#include "PlayerComponent.hpp"
#include "EnemyComponent.hpp"
#include "EnemyAIBase.hpp"
#include "TransmitterComponent.hpp"
#include "ReceiverComponent.hpp"
#include "EnemyComponent.hpp"
#include "TerrainComponent.hpp"
#include "GunComponent.hpp"
#include "LevelComponent.hpp"

#include "Rendering/SkyboxWorldComponent.hpp"
#include "Rendering/Particles/ParticleComponent.hpp"
#include "Rendering/Particles/ParticleEmitter.hpp"
#include "NavGrid.hpp"

using namespace Poly;
using namespace GGJGame;

const static float PLAYER_HEIGHT = 1.8f;
const static float PLAYER_CAPSULE_RADIUS = 0.3f;
const static float PLAYER_CAPSULE_HEIGHT = PLAYER_HEIGHT - 2 * PLAYER_CAPSULE_RADIUS;
const static float PLAYER_MASS = 80.f;
const static float PLAYER_FRICTION = 0.5f;
const static float PLAYER_LINEAR_DAMPING = 0.7f;

const static float PLAYER_CAMERA_HEIGHT = 1.25f;
const static float PLAYER_CAMERA_NEAR = 0.1f;
const static float PLAYER_CAMERA_FAR = 300.f;
const static Angle PLAYER_CAMERA_FOV = 60_deg;

const static float ENEMY_CAPSULE_RADIUS = 0.3f;
const static float ENEMY_CAPSULE_HEIGHT = 1.25f;
const static float ENEMY_MASS = 100.f;
const static float ENEMY_FRICTION = 0.5f;
const static float ENEMY_LINEAR_DAMPING = 0.7f;

static const Vector2i MAP_SIZE = Vector2i(150, 150);
static const float CELL_SIZE = 2.5f;
static const Vector2i GRID_SIZE = Vector2i(MAP_SIZE.X / CELL_SIZE, MAP_SIZE.Y / CELL_SIZE);

//------------------------------------------------------------------------------
void GameManagerSystem::InitializeScene(Poly::World* world)
{
	GameManagerWorldComponent* gameMgr = gEngine->GetWorld()->GetWorldComponent<GameManagerWorldComponent>();

	// create camera
	SpawnPlayer(world, Vector(10.f, 10.f, -10.f));

	// preload bullet mesh
	gameMgr->BulletMesh = ResourceManager<MeshResource>::Load("Models/TestBlock.fbx", eResourceSource::GAME);
	gameMgr->EnemyMesh = ResourceManager<MeshResource>::Load("Models/TestHuman.fbx", eResourceSource::GAME);

	// create light
	world->GetWorldComponent<AmbientLightWorldComponent>()->SetColor(Color(0.2f, 0.5f, 1.0f));
	world->GetWorldComponent<AmbientLightWorldComponent>()->SetIntensity(0.1f);
	Quaternion DirLightRot = Quaternion(Vector::UNIT_Y, 80_deg) * Quaternion(Vector::UNIT_X, -80_deg);

	Entity* directionalLight = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<Poly::DirectionalLightComponent>(world, directionalLight, Color(1.0f, 0.9f, 0.8f), 0.8f);
	Poly::EntityTransform& dirLightTrans = directionalLight->GetTransform();
	dirLightTrans.SetLocalRotation(DirLightRot);
}

//------------------------------------------------------------------------------
void GGJGame::GameManagerSystem::DenitializeScene(Poly::World* world)
{
	GameManagerWorldComponent* gameMgr = gEngine->GetWorld()->GetWorldComponent<GameManagerWorldComponent>();
	ResourceManager<MeshResource>::Release(gameMgr->BulletMesh);
	ResourceManager<MeshResource>::Release(gameMgr->EnemyMesh);
}

//------------------------------------------------------------------------------
void GameManagerSystem::Update(Poly::World* world)
{
	//float deltaTime = (float)(TimeSystem::GetTimerDeltaTime(world, Poly::eEngineTimer::GAMEPLAY));
	GameManagerWorldComponent* gameManager = Poly::gEngine->GetWorld()->GetWorldComponent<GameManagerWorldComponent>();
	InputWorldComponent* inputCmp = world->GetWorldComponent<InputWorldComponent>();

	
	// Proces sound entities
	for (size_t i = 0; i < gameManager->SoundSampleEntities.GetSize();)
	{
		Entity* soundEnt = gameManager->SoundSampleEntities[i].Get();
		if (!SoundSystem::IsEmmiterActive(world, soundEnt))
		{
			gameManager->SoundSampleEntities.RemoveByIdx(i);
			DeferredTaskSystem::DestroyEntity(world, soundEnt);
		}
		else
			++i;
	}
	
	// levels swap

	if (inputCmp->IsPressed(eKey::KEY_1))
	{
		Cleanup(world);
		SpawnLevel(world, eLevels::MOVEMENT_TEST);
	}

	if (!gameManager->Level->GetComponent<LevelComponent>()->NavigationGrid)
	{
		// TODO: generate grid size from meszh size


		EntityTransform& levelTransform = gameManager->Level->GetTransform();
		NavGrid* grid = gameManager->Level->GetComponent<LevelComponent>()->NavigationGrid = new NavGrid({ levelTransform.GetGlobalTranslation().X - GRID_SIZE.X * CELL_SIZE / 2, levelTransform.GetGlobalTranslation().Z - GRID_SIZE.Y * CELL_SIZE / 2 }, GRID_SIZE, CELL_SIZE);

		for (int x = 0; x < GRID_SIZE.X; ++x)
		{
			for (int y = 0; y < GRID_SIZE.Y; ++y)
			{
				Vector from = grid->GetNodeWorldPosition(grid->GetNodeFromGridPosition({ x, y }));
				Vector to = from;

				RaycastResult result = Physics3DSystem::AllHitsRaycast(world, Vector(from.X, 50, from.Z), Vector(to.X, -50, to.Z), eCollisionGroup::RIGIDBODY_GREEN, eCollisionGroup::RIGIDBODY_GREEN);

				bool occupied = true;
				for (size_t i = 0; i < result.Hits.GetSize(); ++i)
				{
					const TerrainComponent* terrain = result.Hits[i].HitEntity->GetComponent<TerrainComponent>();

					if (!terrain)
						continue;

					if (terrain->Passability == eTerrainPassability::WALKABLE_GROUND)
					{
						occupied = false;
					}
					else if (terrain->Passability == eTerrainPassability::IMPASSABLE)
					{
						occupied = true;
						break;
					}
				}

				grid->GetNodeFromGridPosition({ x, y })->Occupied = occupied;
			}
		}
	}

	if (gameManager->Level->GetComponent<LevelComponent>()->NavigationGrid)
	{
		NavGrid* grid = gameManager->Level->GetComponent<LevelComponent>()->NavigationGrid;

		for (int x = 0; x < GRID_SIZE.X; ++x)
		{
			for (int y = 0; y < GRID_SIZE.Y; ++y)
			{
				Vector from = grid->GetNodeWorldPosition(grid->GetNodeFromGridPosition({ x, y }));
				Vector to = from;
	
				if (!grid->GetNodeFromGridPosition({ x, y })->Occupied)
					DebugDrawSystem::DrawLine(world, Vector(from.X, 0, from.Z), Vector(to.X, 0.3f, to.Z), Color::GREEN);
				else
					DebugDrawSystem::DrawLine(world, Vector(from.X, 0, from.Z), Vector(to.X, 0.3f, to.Z), Color::RED);
			}
		}
	}
}

//------------------------------------------------------------------------------
Entity* GameManagerSystem::SpawnPlayer(Poly::World* world, const Poly::Vector& position)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	world->GetWorldComponent<GameManagerWorldComponent>()->Player = entity;

	// collider
	Collider3DComponentTemplate colliderTemplate;
	colliderTemplate.Shape = std::make_unique<Physics3DCapsuleShape>(PLAYER_CAPSULE_RADIUS, PLAYER_CAPSULE_HEIGHT);
	colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), std::move(colliderTemplate));

	//rigidbody
	Rigidbody3DComponentTemplate rigidbodyTemplate;
	rigidbodyTemplate.Mass = PLAYER_MASS;
	rigidbodyTemplate.AngularFactor = Vector(0.f, 0.f, 0.f);
	rigidbodyTemplate.RigidbodyType = eRigidBody3DType::DYNAMIC;
	rigidbodyTemplate.Friction = PLAYER_FRICTION;
	rigidbodyTemplate.LinearDamping = PLAYER_LINEAR_DAMPING;
	DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), rigidbodyTemplate);

	// actor
	DeferredTaskSystem::AddComponentImmediate<ActorComponent>(gEngine->GetWorld(), entity);
	// player
	DeferredTaskSystem::AddComponentImmediate<PlayerComponent>(gEngine->GetWorld(), entity);

	// receiver
	DeferredTaskSystem::AddComponentImmediate<ReceiverComponent>(gEngine->GetWorld(), entity);

	//camera
	Entity* camera = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	camera->SetParent(entity);
	DeferredTaskSystem::AddComponentImmediate<Poly::CameraComponent>(gEngine->GetWorld(), camera, PLAYER_CAMERA_FOV, PLAYER_CAMERA_NEAR, PLAYER_CAMERA_FAR);
	gEngine->GetWorld()->GetWorldComponent<ViewportWorldComponent>()->SetCamera(0, gEngine->GetWorld()->GetComponent<CameraComponent>(camera));
	DeferredTaskSystem::AddComponentImmediate<PostprocessSettingsComponent>(gEngine->GetWorld(), camera);
	PostprocessSettingsComponent* postCmp = world->GetComponent<PostprocessSettingsComponent>(camera);
	postCmp->UseBgShader = false;
	postCmp->UseFgShader = true;
// 	postCmp->Distortion = 0.5f;
// 	postCmp->ColorTempValue = 6500.0f;
// 	postCmp->Saturation = 1.0f;
// 	postCmp->Grain = 1.0f;
// 	postCmp->Stripes = 0.0f;
// 	postCmp->Vignette = 1.0f;

	// gun
	Entity* gun = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	DeferredTaskSystem::AddComponentImmediate<Poly::MeshRenderingComponent>(gEngine->GetWorld(), gun, "Models/Gun.fbx", eResourceSource::GAME);
	DeferredTaskSystem::AddComponentImmediate<GGJGame::GunComponent>(gEngine->GetWorld(), gun);
	gun->SetParent(camera);
	gun->GetTransform().SetLocalScale(0.1f);
	gun->GetTransform().SetLocalTranslation(Vector(0.1f, -0.1f, -0.4f));
	gun->GetTransform().SetLocalRotation(Quaternion(Vector::UNIT_Z, 90_deg) * Quaternion(Vector::UNIT_Y, 90_deg));

	// walking sound
	Entity* soundEmmiter = DeferredTaskSystem::SpawnEntityImmediate(world);
	soundEmmiter->SetParent(entity);
	DeferredTaskSystem::AddComponentImmediate<Poly::SoundEmitterComponent>(world, soundEmmiter, "Audio/footsteps.ogg", eResourceSource::GAME);
	SoundSystem::SetEmitterFrequency(world, soundEmmiter, 1.5f);
	SoundSystem::SetEmitterGain(world, soundEmmiter, 2.f);

	// music
	Entity* musicEmmiter = DeferredTaskSystem::SpawnEntityImmediate(world);
	musicEmmiter->SetParent(camera);
	DeferredTaskSystem::AddComponentImmediate<Poly::SoundEmitterComponent>(world, musicEmmiter, "Audio/music.ogg", eResourceSource::GAME);
	SoundSystem::SetEmitterFrequency(world, musicEmmiter, 1.0f);
	SoundSystem::SetEmitterGain(world, musicEmmiter, 1.0f);
	SoundSystem::PlayEmitter(world, musicEmmiter);
	SoundSystem::LoopEmitter(world, musicEmmiter);

	// transform
	EntityTransform& transform = entity->GetTransform();
	transform.SetLocalTranslation(position);

	camera->GetTransform().SetLocalTranslation(Vector(0, PLAYER_CAMERA_HEIGHT - PLAYER_CAPSULE_RADIUS - PLAYER_CAPSULE_HEIGHT * 0.5f, 0));

	return entity;
}

//------------------------------------------------------------------------------
Poly::Entity* GameManagerSystem::SpawnLevel(Poly::World* world, eLevels level)
{
	Entity* levelEntity;

	SpawnAmbientFX(world);

	switch (level)
	{
	case eLevels::MOVEMENT_TEST:
		levelEntity = SpawnMovementTestLevel(world);
		break;

	case eLevels::RELEASE_LEVEL:
		levelEntity = SpawnReleaseLevel(world);
		break;
	default:
		ASSERTE(false, "Invalid level type!");
	}

	world->GetWorldComponent<GameManagerWorldComponent>()->Level = levelEntity;

	return levelEntity;
}

//------------------------------------------------------------------------------
void GGJGame::GameManagerSystem::Cleanup(Poly::World* world)
{
	GameManagerWorldComponent* gameMgr = world->GetWorldComponent<GameManagerWorldComponent>();

	DeferredTaskSystem::DestroyEntity(world, gameMgr->Level);
}

//------------------------------------------------------------------------------
void GGJGame::GameManagerSystem::PlaySample(Poly::World* world, const Poly::String& file, const Poly::Vector& position, float pitch, float gain, bool loop)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* ent = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& trans = ent->GetTransform();
	trans.SetLocalTranslation(position);
	DeferredTaskSystem::AddComponentImmediate<Poly::SoundEmitterComponent>(world, ent, file, eResourceSource::GAME);

	SoundSystem::SetEmitterFrequency(world, ent, pitch);
	SoundSystem::SetEmitterGain(world, ent, gain);
	SoundSystem::PlayEmitter(world, ent);
	if(loop)
		SoundSystem::LoopEmitter(world, ent);
	gameMgrCmp->SoundSampleEntities.PushBack(ent);
}

//------------------------------------------------------------------------------
Poly::Entity * GGJGame::GameManagerSystem::SpawnReleaseLevel(Poly::World * world)
{
	//GameManagerWorldComponent* gameMgr = world->GetWorldComponent<GameManagerWorldComponent>();

	// spawn level
	Entity* level = DeferredTaskSystem::SpawnEntityImmediate(world);

	// level component
	LevelComponent* levelCmp = DeferredTaskSystem::AddComponentImmediate<LevelComponent>(world, level);

	// spawn level meshes without colliders
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (1)/Map_1.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (2)/Map_2.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (3)/Map_3.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (4)/Map_4.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (5)/Map_5.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (6)/Map_6.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (7)/Map_7.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (8)/Map_8.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (9)/Map_9.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (10)/Map_10.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (12)/Map_12.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (13)/Map_13.fbx");
	SpawnLevelMesh(world, level, "Levels/ReleaseLevel/Map (14)/Map_14.fbx");

	// spawn ground with collider
	levelCmp->Ground = SpawnPlaneGround(world, level, "Levels/ReleaseLevel/Map (11)/Map_11.fbx");

	// spawn walls collider without mesh
	SpawnCustomCollider(world, level, "Levels/ReleaseLevel/WallsColliderLayout.fbx");

	// spawn navMesh generation mask
	SpawnCustomCollider(world, level, "Levels/ReleaseLevel/GroundColliderLayout.fbx", eResourceSource::GAME, eTerrainPassability::WALKABLE_GROUND);

	Entity* b = SpawnBeacon(world, level, Vector(16, 0.9, 5.2));
	SpawnBeacon(world, level, Vector(-39, 0.9, -0.6));
	SpawnBeacon(world, level, Vector(16.8, 0.9, -52));

	b->GetTransform().SetLocalTranslation(b->GetTransform().GetLocalTranslation() + Vector(0, 0, 2.f));

	return level;
}

//------------------------------------------------------------------------------
Poly::Entity* GameManagerSystem::SpawnMovementTestLevel(Poly::World* world)
{
	//GameManagerWorldComponent* gameMgr = world->GetWorldComponent<GameManagerWorldComponent>();
	
	// spawn level
	Entity* level = DeferredTaskSystem::SpawnEntityImmediate(world);

	// level component
	LevelComponent* levelCmp = DeferredTaskSystem::AddComponentImmediate<LevelComponent>(world, level);

	// spawn ground
	levelCmp->Ground = SpawnPlaneGround(world, level);

	// build the wall
	for (int x = 0; x < 10; ++x)
	{
		for (int y = 0; y < 5; ++y)
		{
			// entity
			Entity* brick = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
			brick->SetParent(level);
			// transform
			EntityTransform& brickTransform = brick->GetTransform();
			brickTransform.SetLocalTranslation(Vector(-5 + x * 2, 15 + y * 2, -5 * 2));
			// mesh
			MeshRenderingComponent* brickMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, brick, "Models/Cube.fbx", eResourceSource::GAME);
			brickMesh->SetMaterial(0, PhongMaterial(Color(0, 0, 1), Color(0, 0, 1), Color(0, 0, 1), 8.0f));
			brickMesh->SetShadingModel(eShadingModel::LIT);
			// collider
			Collider3DComponentTemplate colliderTemplate;
			colliderTemplate.Shape = std::make_unique<Physics3DBoxShape>(Vector(1, 1, 1));
			colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
			colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
			DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(gEngine->GetWorld(), brick, gEngine->GetWorld(), std::move(colliderTemplate));
			//rigidbody
			Rigidbody3DComponentTemplate rigidbodyTemplate;
			rigidbodyTemplate.Mass = 10;
			rigidbodyTemplate.AngularDamping = 0.1f;
			DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(gEngine->GetWorld(), brick, gEngine->GetWorld(), rigidbodyTemplate);
		}
	}

	return level;
}

//------------------------------------------------------------------------------
Poly::Entity* GameManagerSystem::SpawnPlaneGround(Poly::World* world, Poly::Entity* parent, const String& meshPath, eResourceSource meshResource, PhongMaterial material, const Vector& colliderHalfExtents, eTerrainPassability passability)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	entity->SetParent(parent);

	// transform
	EntityTransform& groundTransform = entity->GetTransform();
	groundTransform.SetLocalTranslation(Vector(0, 0, 0));
	groundTransform.SetLocalRotation(Quaternion({ -90_deg, 0_deg, 0_deg }));

	// mesh
	MeshRenderingComponent* groundMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, entity, meshPath, meshResource);
	groundMesh->SetMaterial(0, material);
	groundMesh->SetShadingModel(eShadingModel::LIT);

	// collider
	Collider3DComponentTemplate colliderTemplate;
	colliderTemplate.Shape = std::make_unique<Physics3DBoxShape>(colliderHalfExtents);
	colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(world, entity, gEngine->GetWorld(), std::move(colliderTemplate));

	//rigidbody
	Rigidbody3DComponentTemplate rigidbodyTemplate;
	rigidbodyTemplate.RigidbodyType = eRigidBody3DType::STATIC;
	DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(world, entity, world, rigidbodyTemplate);

	// terrain component
	DeferredTaskSystem::AddComponentImmediate<TerrainComponent>(world, entity, passability);

	return entity;
}

Poly::Entity * GGJGame::GameManagerSystem::SpawnLevelMesh(Poly::World* world, Poly::Entity* parent, const String & meshPath, eResourceSource meshResource, PhongMaterial material)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	entity->SetParent(parent);

	// transform
	EntityTransform& groundTransform = entity->GetTransform();
	groundTransform.SetLocalTranslation(Vector(0, 0, 0));
	groundTransform.SetLocalRotation(Quaternion({ -90_deg, 0_deg, 0_deg }));

	// mesh
	MeshRenderingComponent* groundMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, entity, meshPath, meshResource);
	groundMesh->SetMaterial(0, material);
	groundMesh->SetShadingModel(eShadingModel::LIT);

	return entity;
}

//------------------------------------------------------------------------------
Poly::Entity* GGJGame::GameManagerSystem::SpawnCustomCollider(Poly::World* world, Poly::Entity* parent, const String& meshPath, eResourceSource meshResource, eTerrainPassability passability)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	entity->SetParent(parent);

	// transform
	EntityTransform& groundTransform = entity->GetTransform();
	groundTransform.SetLocalTranslation(Vector(0, 0, 0));
	groundTransform.SetLocalRotation(Quaternion({ -90_deg, 0_deg, 0_deg }));

	// mesh
	//MeshRenderingComponent* groundMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, entity, meshPath, meshResource);
	//groundMesh->SetMaterial(0, PhongMaterial(Color(1, 0.25, 0.25), Color(1, 0.25, 0.25), Color(1, 0.25, 0.25), 8.0f));
	//groundMesh->SetShadingModel(eShadingModel::LIT);
	
	// collider
	Collider3DComponentTemplate colliderTemplate;
	Physics3DStaticMeshSource src;
	src.LoadMesh(meshPath, eResourceSource::GAME);
	colliderTemplate.Shape = std::make_unique<Physics3DStaticMeshShape>(src);
	colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), std::move(colliderTemplate));

	//rigidbody
	Rigidbody3DComponentTemplate rigidbodyTemplate;
	rigidbodyTemplate.RigidbodyType = eRigidBody3DType::STATIC;
	DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(world, entity, world, rigidbodyTemplate);

	// terrain component
	DeferredTaskSystem::AddComponentImmediate<TerrainComponent>(world, entity, passability);

	return entity;
}

//------------------------------------------------------------------------------
template <typename T>
Poly::Entity* GGJGame::GameManagerSystem::SpawnEnemy(Poly::World* world, Poly::Entity* parent, const Poly::Vector& position)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	entity->SetParent(parent);

	world->GetWorldComponent<GameManagerWorldComponent>()->Level->GetComponent<LevelComponent>()->Enemy.PushFront(entity);
	EntityTransform& enemyTransform = entity->GetTransform();
	enemyTransform.SetLocalTranslation(position);
	enemyTransform.SetLocalRotation(Quaternion({ -90_deg, 0_deg, 0_deg }));
	enemyTransform.SetLocalScale(0.8f);

	// collider
	Collider3DComponentTemplate colliderTemplate;
	colliderTemplate.Shape = std::make_unique<Physics3DCapsuleShape>(ENEMY_CAPSULE_RADIUS, ENEMY_CAPSULE_HEIGHT);
	colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), std::move(colliderTemplate));

	//rigidbody
	Rigidbody3DComponentTemplate rigidbodyTemplate;
	rigidbodyTemplate.Mass = ENEMY_MASS;
	rigidbodyTemplate.AngularFactor = Vector(0.f, 0.f, 0.f);
	rigidbodyTemplate.Friction = ENEMY_FRICTION;
	rigidbodyTemplate.LinearDamping = ENEMY_LINEAR_DAMPING;
	rigidbodyTemplate.RigidbodyType = eRigidBody3DType::DYNAMIC;
	DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), rigidbodyTemplate);

	//mesh
	MeshRenderingComponent* enemyMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, entity, "Models/Robot/Robot.fbx", eResourceSource::GAME);
	enemyMesh->SetMaterial(0, PhongMaterial(Color(1, 0, 1), Color(1, 0, 1), Color(1, 0, 1), 8.0f));
	enemyMesh->SetShadingModel(eShadingModel::LIT);

	// actor
	ActorComponent* actorCmp = DeferredTaskSystem::AddComponentImmediate<ActorComponent>(gEngine->GetWorld(), entity);

	//enemy
	EnemyComponent* enemyCmp = DeferredTaskSystem::AddComponentImmediate<EnemyComponent>(world, entity, std::make_unique<T>());

	GameManagerWorldComponent* gameManager = Poly::gEngine->GetWorld()->GetWorldComponent<GameManagerWorldComponent>();
	DeferredTaskSystem::AddComponentImmediate<PathfindingComponent>(gEngine->GetWorld(), entity, gameManager->Level->GetComponent<LevelComponent>()->NavigationGrid);

	return entity;
}

template
Poly::Entity* GGJGame::GameManagerSystem::SpawnEnemy<EnemyAIAssailant>(Poly::World* world, Poly::Entity* parent, const Poly::Vector& position);

template
Poly::Entity* GGJGame::GameManagerSystem::SpawnEnemy<EnemyAIEngineer>(Poly::World* world, Poly::Entity* parent, const Poly::Vector& position);


Poly::Entity * GGJGame::GameManagerSystem::SpawnBeacon(Poly::World* world, Poly::Entity* parent, const Poly::Vector& position)
{
	// entity
	Entity* entity = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	entity->GetTransform().SetLocalTranslation(position);
	entity->SetParent(parent);

	entity->GetTransform().SetLocalRotation(Quaternion({ -90_deg, 0_deg, 0_deg }));
	entity->GetTransform().SetLocalTranslation(Vector( 0, -1.f, 0 ) + entity->GetTransform().GetLocalTranslation());

	// mesh
	MeshRenderingComponent* brickMesh = DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, entity, "Levels/ReleaseLevel/Antenna/Antenna.fbx", eResourceSource::GAME);
	brickMesh->SetMaterial(0, PhongMaterial(Color(1, 0, 1), Color(1, 0, 1), Color(1, 0, 1), 8.0f));
	brickMesh->SetShadingModel(eShadingModel::LIT);

	// collider
	Collider3DComponentTemplate colliderTemplate;
	colliderTemplate.Shape = std::make_unique<Physics3DSphereShape>(1.f);
	colliderTemplate.CollisionGroup = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	colliderTemplate.CollisionMask = EnumFlags<eCollisionGroup>(eCollisionGroup::RIGIDBODY_GREEN);
	DeferredTaskSystem::AddComponentImmediate<Collider3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), std::move(colliderTemplate));

	// rigidbody
	Rigidbody3DComponentTemplate rigidbodyTemplate;
	rigidbodyTemplate.Mass = 0;
	rigidbodyTemplate.AngularFactor = Vector(0, 0, 0);
	rigidbodyTemplate.RigidbodyType = eRigidBody3DType::STATIC;
	DeferredTaskSystem::AddComponentImmediate<Rigidbody3DComponent>(gEngine->GetWorld(), entity, gEngine->GetWorld(), rigidbodyTemplate);

	DeferredTaskSystem::AddComponentImmediate<TransmitterComponent>(world, entity);

	return entity;
}

void GameManagerSystem::SpawnAmbientFX(World* world)
{
	EnumArray<String, eCubemapSide> miramar{
		{ eCubemapSide::RIGHT, "Cubemaps/stormydays/stormydays_rt.jpg" },
		{ eCubemapSide::LEFT , "Cubemaps/stormydays/stormydays_lt.jpg" },
		{ eCubemapSide::TOP  , "Cubemaps/stormydays/stormydays_up.jpg" },
		{ eCubemapSide::DOWN , "Cubemaps/stormydays/stormydays_dn.jpg" },
		{ eCubemapSide::BACK , "Cubemaps/stormydays/stormydays_bk.jpg" },
		{ eCubemapSide::FRONT, "Cubemaps/stormydays/stormydays_ft.jpg" }
	};
	DeferredTaskSystem::AddWorldComponentImmediate<SkyboxWorldComponent>(world, miramar);

	SpawnParticleAmbientSmall(world);
	SpawnParticleAmbientBig(world);
}

void GameManagerSystem::SpawnParticleAmbientSmall(World* world)
{
	// GameManagerWorldComponent* GameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* ParticlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& ParticlesEnt0Trans = ParticlesEnt->GetTransform();
	ParticlesEnt0Trans.SetLocalTranslation(Vector(0.0f, 0.0f, 0.0f));

	ParticleEmitter::Settings settings;
	settings.InitialSize = 1000;
	settings.InitialSize = 500;
	settings.BurstSizeMin = 100;
	settings.BurstSizeMax = 200;
	settings.BurstTimeMin = 0.1f;
	settings.BurstTimeMax = 0.2f;
	settings.BaseColor = Color(0.1f, 0.4f, 0.4f, 1.0f);
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rndPos = (GameManagerSystem::RandomVector(-1.0f, 1.0f) * 50.0f);
		p->Position = Vector(1.0f * rndPos.X, 0.01f * Abs(rndPos.Y), 1.0f * rndPos.Z);
		Vector rndAccel = GameManagerSystem::RandomVector(0.5f, 1.0f) * 0.1f;
		p->Acceleration = Vector(10.0f * rndAccel.X, 0.1f * rndAccel.Y, 0.1f * rndAccel.Z);
		p->LifeTime = Random(100.0f, 200.0f);
		p->Scale = Vector(1.0f, 1.0f, 1.0f) * (0.05f + Random(0.0f, 0.05f));
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Position += p->Acceleration;
	};

	DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, ParticlesEnt, settings);
	//ParticleComponent* ParticleComp = world->GetComponent<ParticleComponent>(ParticlesEnt);
	// GameMgrCmp->GameEntities.PushBack(ParticlesEnt);
}

void GameManagerSystem::SpawnParticleAmbientBig(World* world)
{
	// GameManagerWorldComponent* GameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* ParticlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& ParticlesEnt0Trans = ParticlesEnt->GetTransform();
	ParticlesEnt0Trans.SetLocalTranslation(Vector(0.0f, 0.0f, 0.0f));

	ParticleEmitter::Settings settings;
	settings.Speed = 0.1f;
	settings.InitialSize = 500;
	settings.BurstSizeMin = 100;
	settings.BurstSizeMax = 200;
	settings.BurstTimeMin = 0.1f;
	settings.BurstTimeMax = 0.2f;
	settings.BaseColor = Color(0.1f, 0.7f, 0.8f, 0.01f);
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rndPos = (GameManagerSystem::RandomVector(-1.0f, 1.0f) * 50.0f);
		p->Position = Vector(1.0f * rndPos.X, 0.01f * Abs(rndPos.Y), 1.0f * rndPos.Z);
		Vector rndAccel = GameManagerSystem::RandomVector(0.5f, 1.0f) * 0.05f;
		p->Acceleration = Vector(10.0f * rndAccel.X, 0.1f * rndAccel.Y, 0.1f * rndAccel.Z);
		p->LifeTime = Random(1.0f, 2.0f);
		p->Scale = Vector(1.0f, 1.0f, 1.0f) * Random(10.0f, 20.0f);
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Position += p->Acceleration;
	};

    settings.SprsheetSettings.SpritePath = "Textures/puff_512.png";
    settings.SprsheetSettings.Source = eResourceSource::GAME;
	DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, ParticlesEnt, settings);
	//ParticleComponent* ParticleComp = world->GetComponent<ParticleComponent>(ParticlesEnt);
	// GameMgrCmp->GameEntities.PushBack(ParticlesEnt);
}


float GameManagerSystem::Random()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float GameManagerSystem::Random(float min, float max)
{
	float rnd = Random();
	return Lerp(min, max, rnd);
}

Vector GameManagerSystem::RandomVector(float min, float max)
{
	float rndX = GameManagerSystem::Random();
	float rndY = GameManagerSystem::Random();
	float rndZ = GameManagerSystem::Random();
	return Vector(Lerp(min, max, rndX), Lerp(min, max, rndY), Lerp(min, max, rndZ));
}