#include "EnginePCH.hpp"

#include "ActorSystem.hpp"
#include "EnemyAISystem.hpp"
#include "EnemyComponent.hpp"
#include "EnemyAITactics.hpp"
#include "GameManagerWorldComponent.hpp"

#include "LevelComponent.hpp"
#include "ActorComponent.hpp"
#include "Rendering/Particles/ParticleEmitter.hpp"
#include "Rendering/Particles/ParticleComponent.hpp"
#include "GameManagerSystem.hpp"
#include "NavGrid.hpp"

using namespace Poly;
using namespace GGJGame;

constexpr float SPAWNRATE = 5.0f;
static const float ENEMY_MOVEMENT_SPEED_MULT = 0.4f;
static constexpr auto PATH_NODES_REACH_DISTANCE_THRESHOLD = 1.0f;

EnemyAI::State GAME_DLLEXPORT EnemyAISystem::query::MoveEntityToPosition(World * world, Entity * entity,
																		 std::function<Vector(World*, Entity*)> PositionPredicate)
{
	using namespace EnemyAI;

	auto path = PathfindingSystem::QueryPathToPosition(world, entity, PositionPredicate);
	if(path.IsEmpty())
	{
		return State::RUNNING;
	}
	else
	{
		EnemyComponent* enemyCmp = entity->GetComponent<EnemyComponent>();

		if(enemyCmp->delineatedPath.empty())
		{
			for(auto& node : path)
				enemyCmp->delineatedPath.push_front(node);
		}

		Vector nextNode = enemyCmp->delineatedPath.back();
		Vector entityPosition = entity->GetTransform().GetGlobalTranslation();
		if(Vector(nextNode - entityPosition).LengthSquared() < PATH_NODES_REACH_DISTANCE_THRESHOLD)
		{
			enemyCmp->delineatedPath.pop_back();
			if( !enemyCmp->delineatedPath.empty() )
			{
				nextNode = enemyCmp->delineatedPath.back();
			}
		}
		ActorSystem::Move(entity, nextNode - entityPosition, ENEMY_MOVEMENT_SPEED_MULT);
		// when should pathfindingCmp->ResetDestination() ??

		return State::SUCCESS;
	}
}

void GGJGame::EnemyAISystem::DebugDrawPath(Entity * ent)
{
	PathfindingComponent* cmp = ent->GetComponent<PathfindingComponent>();
	auto& path = cmp->GetPath();
	if(path.GetSize())
	{
		for(size_t i = 1; i < path.GetSize(); ++i)
		{
			DebugDrawSystem::DrawLine(ent->GetWorld(), Vector(path[i - 1].X, 0.1f, path[i - 1].Z), Vector(path[i].X, 0.1f, path[i].Z), Color::RED);
		}
	}
}

void EnemyAISystem::Update(Poly::World* world)
{
	float deltaTime = (float)(TimeSystem::GetTimerDeltaTime(world, eEngineTimer::GAMEPLAY));
	for(auto tuple : world->IterateComponents<EnemyComponent>())
	{
		auto enemyCmp = std::get<EnemyComponent*>(tuple);
		auto actorCmp = enemyCmp->GetSibling<ActorComponent>();
		if (actorCmp->IsDead()) 
		{
			DeathParticle(enemyCmp->GetOwner(), world);
			DeferredTaskSystem::DestroyEntity(world, enemyCmp->GetOwner());
			continue;
		}

		enemyCmp->EnemyAI->IterateActions(world, enemyCmp->GetOwner());
	}

	// hack; sori
	static int noEnemies = 0;

	if(noEnemies > 1)
		return;

	GameManagerWorldComponent* gmComp = world->GetWorldComponent<GameManagerWorldComponent>();
	NavGrid* nav = gmComp->Level->GetComponent<LevelComponent>()->NavigationGrid;
	if (!nav) { return; }
	if (gmComp->Level->GetComponent<LevelComponent>()->TimeSinceLastEnemySpawn >= SPAWNRATE)
	{
		// -42, 1, 2
		float cellSize = nav->GetCellSize();
		for (int i = 0; i < nav->GetGridSize().Y; i += cellSize)
		{
			for (int j = 0; j < nav->GetGridSize().X; j += cellSize)
			{
				NavGridCell* cell = nav->GetNodeFromGridPosition(Vector2i(j, i));
				if (!cell->Occupied)
				{
					//gConsole.LogDebug("Spaaawned at {}", Vector(j, i, 0));
					GameManagerSystem::SpawnEnemy<EnemyAIAssailant>(world, gmComp->Level, Vector(GameManagerSystem::Random(-45.0f, -35.0f), 1.0f, GameManagerSystem::Random(-5.0f, 5.0f))); //TODO::Tweak
					++noEnemies;
					//cell->Occupied = true;
					gmComp->Level->GetComponent<LevelComponent>()->TimeSinceLastEnemySpawn = 0;
					return;
				}
			}
		}
	}
	else
	{
		gmComp->Level->GetComponent<LevelComponent>()->TimeSinceLastEnemySpawn += deltaTime;
	}
}

void GGJGame::EnemyAISystem::DeathParticle(Poly::Entity* enemy, Poly::World * &world)
{
	Entity* ParticlesEnt = DeferredTaskSystem::SpawnEntityImmediate(world);
	EntityTransform& ParticlesEnt1Trans = ParticlesEnt->GetTransform();
	ParticlesEnt1Trans.SetLocalTranslation(Vector(-20.0f, 0.0f, 0.0f));
	Vector EnemyPos = enemy->GetTransform().GetGlobalTranslation();
	ParticleEmitter::Settings settings;
	settings.InitialSize = 10;
	settings.BurstSizeMin = 5;
	settings.BurstSizeMax = 10;
	settings.BurstTimeMin = 0.1f;
	settings.BurstTimeMax = 0.2f;
	settings.BaseColor = Color(1.0f, 0.8f, 0.8f, 1.0f);
	settings.ParticleInitFunc = [=](ParticleEmitter::Particle* p) {
		p->Position = EnemyPos + GameManagerSystem::RandomVector(0.2f, 0.2f);
		Vector accel = GameManagerSystem::RandomVector(-1.0f, 1.0f);
		p->Acceleration = Vector(0.5f * accel.X, 1.0f * Abs(accel.Y), 0.5f * accel.Z) * 0.5f;
		p->LifeTime = GameManagerSystem::Random(0.1f, 0.2f);
		p->Scale = Vector(1.0f, 1.0f, 1.0f);
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Position += p->Acceleration;
		float life = Lerp(p->Age / p->LifeTime, 0.1f, 1.0f);
		p->Scale = Vector(1.0f, 1.0f, 1.0f) * (1.0f - life * life);
	};

    settings.SprsheetSettings.SpritePath = "Textures/puff_512.png";
    settings.SprsheetSettings.Source = eResourceSource::GAME;
	DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, ParticlesEnt, settings);
}
