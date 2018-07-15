#include "CollisionSystem.hpp"
#include "PlayerComponent.hpp"
#include "EnemyComponent.hpp"
#include "GameManagerWorldComponent.hpp"
#include "ActorComponent.hpp"
#include "LevelComponent.hpp"
#include "Rendering/Particles/ParticleComponent.hpp"
#include "Rendering/Particles/ParticleEmitter.hpp"
#include "GameManagerSystem.hpp"

using namespace GGJGame;

void HandlePlayerEnemyCollision(World* world, Entity* player, Entity* enemy, const Vector& normal)
{
	if (enemy->GetComponent<ActorComponent>())
	{
		player->GetComponent<ActorComponent>()->SetHitPoints(player->GetComponent<ActorComponent>()->GetHitPoints() - 34);
		enemy->GetComponent<ActorComponent>()->SetHitPoints(0);

		if(player->GetComponent<ActorComponent>()->IsDead())
			Kaboom(world);
	}
}

void Kaboom(Poly::World * &world)
{
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
	settings.BaseColor = Color(1.0f, 1.0f, 1.0f, 0.1f);
	settings.ParticleInitFunc = [](ParticleEmitter::Particle* p) {
		Vector rndPos = (GameManagerSystem::RandomVector(-1.0f, 1.0f) * 50.0f);
		p->Position = Vector(1.0f * rndPos.X, 0.01f * Abs(rndPos.Y), 1.0f * rndPos.Z);
		Vector rndAccel = GameManagerSystem::RandomVector(0.5f, 1.0f) * 0.05f;
		p->Acceleration = Vector(0.1f * rndAccel.X, 0.2f * rndAccel.Y, 0.1f * rndAccel.Z);
		p->LifeTime = GameManagerSystem::Random(1.0f, 2.0f);
		p->Scale = Vector(1.0f, 1.0f, 1.0f) * GameManagerSystem::Random(5.0f, 20.0f);
	};
	settings.ParticleUpdateFunc = [](ParticleEmitter::Particle* p) {
		p->Position += p->Acceleration;
	};

    settings.SprsheetSettings.SpritePath = "Textures/puff_512.png";
    settings.SprsheetSettings.Source = eResourceSource::GAME;
	DeferredTaskSystem::AddComponentImmediate<ParticleComponent>(world, ParticlesEnt, settings);
}

void HandlePlayerGenericCollision(World* world, Entity* player, Entity* collider, const Vector& normal)
{
	GameManagerWorldComponent* managerCmp = world->GetWorldComponent<GameManagerWorldComponent>();
	if (managerCmp->Level->GetComponent<LevelComponent>()->Ground == collider)
	{
		if (Vector::UNIT_Y.Dot(normal) < 0.8f)
		{
			player->GetComponent<ActorComponent>()->ResetJumpState();
		}
	}
}

void GGJGame::CollisionSystem::Update(World* world)
{
	Poly::ContactPairResults results = Poly::Physics3DSystem::GetAllContactPairs(world);
	for (const auto& res : results.ContactPairs)
	{
		if (res.FirstEntity->GetComponent<PlayerComponent>() && res.SecondEntity->GetComponent<EnemyComponent>())
			HandlePlayerEnemyCollision(world, res.FirstEntity, res.SecondEntity, res.Normal);
		else if (res.SecondEntity->GetComponent<PlayerComponent>() && res.FirstEntity->GetComponent<EnemyComponent>())
			HandlePlayerEnemyCollision(world, res.SecondEntity, res.FirstEntity, res.Normal * -1.0f);
		
		else if(res.FirstEntity->GetComponent<PlayerComponent>())
			HandlePlayerGenericCollision(world, res.FirstEntity, res.SecondEntity, res.Normal);
		else if(res.SecondEntity->GetComponent<PlayerComponent>())
			HandlePlayerGenericCollision(world, res.SecondEntity, res.FirstEntity, res.Normal * -1.0f);
	}
}
