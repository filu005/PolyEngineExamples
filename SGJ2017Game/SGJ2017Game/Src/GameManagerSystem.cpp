#include "GameManagerSystem.hpp"

#include "GameManagerWorldComponent.hpp"
#include "CameraMovementComponent.hpp"
#include "Level.hpp"

#include "PlayerControllerComponent.hpp"
#include "TileComponent.hpp"
#include <Audio/SoundEmitterComponent.hpp>
#include <Audio/SoundSystem.hpp>
#include <Debugging/DebugDrawComponents.hpp>
#include <ECS/DeferredTaskSystem.hpp>
#include <Movement/FreeFloatMovementComponent.hpp>
#include <Rendering/MeshRenderingComponent.hpp>
#include <Rendering/Camera/CameraComponent.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Physics2D/Physics2DColliders.hpp>
#include <Physics2D/Rigidbody2DComponent.hpp>
#include "Physics2D/Physics2DWorldComponent.hpp"


using namespace SGJ;
using namespace Poly;


void SGJ::GameManagerSystem::LoadLevel(Poly::World* world, const Poly::String& path)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	gameMgrCmp->Levels.PushBack(new Level(path));
}

void SGJ::GameManagerSystem::Update(Poly::World* world)
{
	GameManagerWorldComponent* manager = world->GetWorldComponent<GameManagerWorldComponent>();

	// Proces sound entities
	for (size_t i=0; i < manager->SoundSampleEntities.GetSize();)
	{
		Entity* soundEnt = manager->SoundSampleEntities[i].Get();
		if (!SoundSystem::IsEmmiterActive(world, soundEnt))
		{
			manager->SoundSampleEntities.RemoveByIdx(i);
			DeferredTaskSystem::DestroyEntity(world, soundEnt);
		}
		else
			++i;
	}

	PlayerControllerComponent* playerCmp = world->GetComponent<PlayerControllerComponent>(manager->Player.Get());
	if (playerCmp->DeathCoolDowntime > 0)
		return;

	// Proces triggers
	for (Physics2DWorldComponent::Collision col : world->GetWorldComponent<Physics2DWorldComponent>()->GetCollidingBodies(world->GetComponent<RigidBody2DComponent>(manager->Player.Get())))
	{
		TileComponent* obstacle = col.rb->GetSibling<TileComponent>();


		if (!obstacle) continue;

		switch (obstacle->GetTileType())
 		{
		case eTileType::SPIKESBOTTOM:
		case eTileType::SPIKESTOP:
		case eTileType::SPIKESLEFT:
		case eTileType::SPIKESRIGHT:
		{
			PlayerUpdateSystem::KillPlayer(world);
			PlayerUpdateSystem::PushPlayer(world, -col.Normal, 40.0f);
			return;
		}
		case eTileType::PLAYERENDPOS:
		{
			manager->CurrentLevelID = (manager->CurrentLevelID + 1) % manager->Levels.GetSize();
			SpawnLevel(world, manager->CurrentLevelID);
			return;
		}
		break;
		case eTileType::FASTERCHARACTER:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::INCREASED_SPEED);
			break;
		case eTileType::HIGHJUMP:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::HIGH_JUMP);
			break;
		case eTileType::LARGERCHARACTER:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::INCREASED_SIZE);
			break;
		case eTileType::SMALLERCHARACTER:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::DECREASED_SIZE);
			break;
		case eTileType::POGOJUMP:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::POGO_JUMP);
			break;
		case eTileType::REVERSEDCONTROLS:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::REVERSED_CONTROLS);
			break;
		case eTileType::LOWJUMP:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::LOW_JUMP);
			break;
		case eTileType::INVERSEDGRAVITY:
			PlayerUpdateSystem::PickupPowerup(world, ePowerup::INVERSED_GRAVITY);
			break;
		default:
			break;
		}
	}
}

Entity* GameManagerSystem::CreateTileObject(Poly::World* world, const Poly::Vector& position, eTileType tileType, String meshSource,
	eRigidBody2DType physicsProperties = eRigidBody2DType::STATIC, const Vector& size = Vector(1, 1, 1), const Color& color = Color(0, 0, 0), bool colliding = true)
{
	eDebugDrawPreset ddrawPreset = physicsProperties == eRigidBody2DType::STATIC ? eDebugDrawPreset::STATIC : eDebugDrawPreset::DYNAMIC;

	Entity* tile = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DebugDrawableComponent>(world, tile, ddrawPreset);
	DeferredTaskSystem::AddComponentImmediate<TileComponent>(world, tile, tileType);
	DeferredTaskSystem::AddComponentImmediate<Box2DColliderComponent>(world, tile, size * 2);

	if (colliding)
	{
		DeferredTaskSystem::AddComponentImmediate<RigidBody2DComponent>(world, tile, world, physicsProperties, tileType == eTileType::RIGIDBODYGROUND ? 0.5f : 1.0f);
	}
	else
	{
		DeferredTaskSystem::AddComponentImmediate<RigidBody2DComponent>(world, tile, world, physicsProperties, RigidBody2DSensorTag());
	}

	EntityTransform& tileTrans = tile->GetTransform();

	Entity* mesh = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DebugDrawableComponent>(world, mesh, ddrawPreset);
	DeferredTaskSystem::AddComponentImmediate<MeshRenderingComponent>(world, mesh, meshSource, eResourceSource::GAME);
	world->GetComponent<MeshRenderingComponent>(mesh)->SetMaterial(0, Material(Color::BLACK, color, 1.0f, 1.0f, 0.5f));
	world->GetComponent<MeshRenderingComponent>(mesh)->SetShadingModel(eShadingModel::PBR);
	
	EntityTransform& meshTrans = mesh->GetTransform();
	mesh->SetParent(tile);

	switch (tileType)
	{
	case eTileType::SPIKESBOTTOM:
		meshTrans.SetLocalScale(size * 2.5f);
		break;
	case eTileType::SPIKESTOP:
		meshTrans.SetLocalRotation(Quaternion(Vector::UNIT_Z, 180_deg));
		meshTrans.SetLocalScale(size * 2.5f);
		break;
	case eTileType::SPIKESLEFT:
		meshTrans.SetLocalRotation(Quaternion(Vector::UNIT_Z, -90_deg));
		meshTrans.SetLocalScale(size * 2.5f);
		break;
	case eTileType::SPIKESRIGHT:
		meshTrans.SetLocalRotation(Quaternion(Vector::UNIT_Z, 90_deg));
		meshTrans.SetLocalScale(size * 2.5f);
		break;
	default:;
		meshTrans.SetLocalScale(size);
	}

	tileTrans.SetLocalTranslation(position);
	return tile;
}

Entity* GameManagerSystem::SpawnPlayer(Poly::World* world, const Poly::Vector& position)
{
	Entity* player = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DebugDrawableComponent>(world, player, eDebugDrawPreset::PLAYER);
	DeferredTaskSystem::AddComponentImmediate<PlayerControllerComponent>(world, player);
	PlayerControllerComponent* p = world->GetComponent<PlayerControllerComponent>(player);
	p->SpawnPoint = position;
	EntityTransform& playerTrans = player->GetTransform();

	DeferredTaskSystem::AddComponentImmediate<Poly::Circle2DColliderComponent>(world, player, 0.4f);
	DeferredTaskSystem::AddComponentImmediate<Poly::RigidBody2DComponent>(world, player, world, eRigidBody2DType::DYNAMIC, 1.0f, 0.5f);
	Poly::RigidBody2DComponent* rigidBody = world->GetComponent<Poly::RigidBody2DComponent>(player);
	rigidBody->SetLinearDamping(3);
	rigidBody->SetAngularDamping(10);

	Entity* body = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<DebugDrawableComponent>(world, body, eDebugDrawPreset::STATIC);
	EntityTransform& bodyTrans = body->GetTransform();
	body->SetParent(player);
	//bodyTrans->SetLocalRotation(Quaternion(Vector::UNIT_X, 90_deg));
	// Vector correctedSize = Vector(0.4f, 0.4f, 0.1f);
	// bodyTrans.SetLocalScale(correctedSize);
	Color bodyColor = Color(0.0f, 1.0f, 0.0f, 1.0f);
	DeferredTaskSystem::AddComponentImmediate<Poly::MeshRenderingComponent>(world, body, "Models/player.obj", eResourceSource::GAME);
	world->GetComponent<MeshRenderingComponent>(body)->SetMaterial(0, Material(Color::BLACK, bodyColor, 1.0f, 1.0f, 0.5f));
	world->GetComponent<MeshRenderingComponent>(body)->SetShadingModel(eShadingModel::PBR);

	playerTrans.SetLocalTranslation(position);
	return player;
}

void SGJ::GameManagerSystem::SpawnLevel(Poly::World* world, size_t idx)
{
	// cleanup previous level
	DespawnLevel(world);

	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	Level* level = gameMgrCmp->Levels[idx];

	gameMgrCmp->CurrentLevelID = idx;

	// calculate level center
	gameMgrCmp->MinLevelWidth = float(level->Width);
	gameMgrCmp->MaxLevelWidth = 0.f;
	gameMgrCmp->MinLevelHeight = float(level->Height);
	gameMgrCmp->MaxLevelHeight = 0.f;
	for (size_t idx = 0; idx < level->Tiles.GetSize(); ++idx)
	{
		if (level->Tiles[idx] != SGJ::eTileType::NOTHING)
		{
			auto w = float(size_t(idx % level->Width));
			auto h = float(size_t(idx / level->Width));

			if (w < gameMgrCmp->MinLevelWidth)
				gameMgrCmp->MinLevelWidth = w;
			if (w > gameMgrCmp->MaxLevelWidth)
				gameMgrCmp->MaxLevelWidth = w;

			if (h < gameMgrCmp->MinLevelHeight)
				gameMgrCmp->MinLevelHeight = h;
			if (h > gameMgrCmp->MaxLevelHeight)
				gameMgrCmp->MaxLevelHeight = h;
		}
	}
	float meanW = (gameMgrCmp->MaxLevelWidth - gameMgrCmp->MinLevelWidth + 1)/2;
	float meanH = (gameMgrCmp->MaxLevelHeight - gameMgrCmp->MinLevelHeight + 1)/2;

	gameMgrCmp->MinLevelWidth -= meanW;
	gameMgrCmp->MaxLevelWidth -= meanW;
	gameMgrCmp->MinLevelHeight -= meanH;
	gameMgrCmp->MaxLevelHeight -= meanH;

	// spawn level tiles
	for (size_t idx = 0; idx < level->Tiles.GetSize(); ++idx)
	{
		if (level->Tiles[idx] != SGJ::eTileType::NOTHING)
		{
			auto posW = float(int((idx % level->Width) - meanW));
			auto posH = float(int((idx / level->Width) - meanH));

			switch (level->Tiles[idx])
			{
			case eTileType::PLAYERSTARTPOS:
				if (gameMgrCmp->Player)
					PlayerUpdateSystem::ResetPlayer(world, Vector(posW, -posH, 0));
				else
					gameMgrCmp->Player = SpawnPlayer(world, Vector(posW, -posH, 0));
				break;
			case eTileType::PLAYERENDPOS:
				gameMgrCmp->LevelEntities.PushBack(CreateTileObject(world, Vector(posW, -posH, 0.f), level->Tiles[idx], "Models/cube.obj", eRigidBody2DType::STATIC, Vector(0.5f, 0.5f, 0.5f), Color(0.f, 0.f, 1.5f)));
				break;

			case eTileType::STATICGROUND:
			case eTileType::RIGIDBODYGROUND:
				gameMgrCmp->LevelEntities.PushBack(CreateTileObject(world, Vector(posW, -posH, 0.f), level->Tiles[idx], "Models/cube.obj",
					level->Tiles[idx] == eTileType::STATICGROUND ? eRigidBody2DType::STATIC : eRigidBody2DType::DYNAMIC,
					level->Tiles[idx] == eTileType::STATICGROUND ? Vector(0.5f, 0.5f, 0.5f) : Vector(0.4f, 0.4f, 0.4f),
					level->Tiles[idx] == eTileType::STATICGROUND ? Color(0.05f, 0.f, 0.125f) : Color(0.5f, 0.5f, 0.5f)));
				break;

			case eTileType::SPIKESBOTTOM:
			case eTileType::SPIKESTOP:
			case eTileType::SPIKESLEFT:
			case eTileType::SPIKESRIGHT:
				gameMgrCmp->LevelEntities.PushBack(CreateTileObject(world, Vector(posW, -posH, 0.f), level->Tiles[idx], "Models/spikes.obj", eRigidBody2DType::STATIC, Vector(0.4f, 0.4f, 0.25f), Color(1.2f, 0.f, 0.f)));
				break;

			default:
				gameMgrCmp->LevelEntities.PushBack(CreateTileObject(world, Vector(posW, -posH, 0.f), level->Tiles[idx], "Models/cube.obj", eRigidBody2DType::STATIC, Vector(0.5f, 0.5f, 0.5f), Color(0.25f, 0, 0.125f), false));
				break;
			}
		}
	}
}

void SGJ::GameManagerSystem::DespawnLevel(Poly::World* world)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	for (auto ent : gameMgrCmp->LevelEntities)
		DeferredTaskSystem::DestroyEntityImmediate(world, ent.Get());
	gameMgrCmp->LevelEntities.Clear();
}

void SGJ::GameManagerSystem::PlaySample(Poly::World* world, const String& file, const Vector& position, float pitch, float gain)
{
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();

	Entity* ent = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& trans = ent->GetTransform();
	trans.SetLocalTranslation(position);
	DeferredTaskSystem::AddComponentImmediate<Poly::SoundEmitterComponent>(world, ent, file, eResourceSource::GAME);

	SoundSystem::SetEmitterFrequency(world, ent, pitch);
	SoundSystem::SetEmitterGain(world, ent, gain);
	SoundSystem::PlayEmitter(world, ent);
	gameMgrCmp->SoundSampleEntities.PushBack(ent);
}

void SGJ::GameManagerSystem::PrepareNonlevelObjects(Poly::World * world)
{
	// Spawn entities
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	gameMgrCmp->Camera = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	DeferredTaskSystem::AddComponentImmediate<Poly::CameraComponent>(gEngine->GetWorld(), gameMgrCmp->Camera.Get(), 60_deg, 1.0f, 1000.f);
	DeferredTaskSystem::AddComponentImmediate<SGJ::CameraMovementComponent>(gEngine->GetWorld(), gameMgrCmp->Camera.Get());
	// Set some camera position
	EntityTransform& cameraTrans = gameMgrCmp->Camera->GetTransform();
	cameraTrans.SetLocalTranslation(Vector(0, 0, 50.f));

	// Set background
	//double Time = gEngine->GetWorld()->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime();
	//Background = DeferredTaskSystem::SpawnEntityImmediate(gEngine->GetWorld());
	//DeferredTaskSystem::AddComponentImmediate<Poly::BackgroundComponent>(gEngine->GetWorld(), Background, Time);
	//GameManagerComponent* gameManagerComponent = Engine->GetWorld()->GetComponent<GameManagerComponent>(GameManager);
	//gEngine->RegisterGameUpdatePhase(BackgroundSystem::BackgroundSystemSystemPhase);

	// SETUP SCENE HERE

	Entity* backgroundPlayer = DeferredTaskSystem::SpawnEntityImmediate(world);
	DeferredTaskSystem::AddComponentImmediate<SoundEmitterComponent>(world, backgroundPlayer, "Audio/Pursuit_cut.ogg", eResourceSource::GAME);
	SoundSystem::PlayEmitter(world, backgroundPlayer);
	SoundSystem::LoopEmitter(world, backgroundPlayer);
	SoundSystem::SetEmitterGain(world, backgroundPlayer, 0.1f);
}

void SGJ::GameManagerSystem::Cleanup(Poly::World* world)
{
	DespawnLevel(world);
	GameManagerWorldComponent* gameMgrCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	for (auto ent : gameMgrCmp->OtherEntities)
		DeferredTaskSystem::DestroyEntityImmediate(world, ent.Get());

	DeferredTaskSystem::DestroyEntityImmediate(world, gameMgrCmp->Player.Get());
	DeferredTaskSystem::DestroyEntityImmediate(world, gameMgrCmp->Camera.Get());
}
