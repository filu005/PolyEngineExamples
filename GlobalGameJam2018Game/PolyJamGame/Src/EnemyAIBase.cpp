#include "EnginePCH.hpp"

#include "ActorSystem.hpp"
#include "AI/PathfindingComponent.hpp"
#include "TransmitterComponent.hpp"
#include "ActorComponent.hpp"
#include "PlayerComponent.hpp"
#include "EnemyComponent.hpp"
#include "EnemyAIBase.hpp"

static const float ENEMY_MOVEMENT_SPEED_MULT = 0.1f;

void DebugDrawPath(Entity* ent)
{
	PathfindingComponent* cmp = ent->GetComponent<PathfindingComponent>();
	auto& path = cmp->GetPath();
	if (path.GetSize())
	{
		for (size_t i = 1; i < path.GetSize(); ++i)
		{
			DebugDrawSystem::DrawLine(ent->GetWorld(), Vector(path[i - 1].X, 0.1f, path[i - 1].Z), Vector(path[i].X, 0.1f, path[i].Z), Color::RED);
		}
	}
}

namespace Actions
{
	using namespace GGJGame;
	// wander around in direction of transmitter
	GGJGame::EnemyAIBase::ActionSignature lookForTransmitter = [](World* world, Entity* selfEntity)
	{
		PathfindingComponent* pathfindingCmp = selfEntity->GetComponent<PathfindingComponent>();

		auto path = pathfindingCmp->GetPath();
		if(path.GetSize() == 0u)
		{
			for(auto transmitterTuple : world->IterateComponents<TransmitterComponent>())
			{
				auto transmitterCmp = std::get<TransmitterComponent*>(transmitterTuple);

				auto transmitterPosition = transmitterCmp->GetTransform().GetGlobalTranslation();

				pathfindingCmp->SetDestination(transmitterPosition);
			}

			return State::FAILURE;
		}
		else
		{
			auto entityTranslation = selfEntity->GetTransform().GetGlobalTranslation();
			//auto actorCmp = selfEntity->GetComponent<ActorComponent>();
			auto movementVector = Vector(path[0].X, 0.0f, path[0].Y) - entityTranslation;
			ActorSystem::Move(selfEntity, movementVector, ENEMY_MOVEMENT_SPEED_MULT);
		}

		return State::SUCCESS;
	};

	GGJGame::EnemyAIBase::ActionSignature attackPlayer = [](World* world, Entity* selfEntity)
	{
		// if in line of sight
		for(auto playerTuple : world->IterateComponents<PlayerComponent>())
		{
			PlayerComponent* playerCmp = std::get<PlayerComponent*>(playerTuple);
			auto playerTransform = playerCmp->GetTransform();
			auto enemyTransform = selfEntity->GetTransform();

			auto playerPosition = playerTransform.GetGlobalTranslation();
			auto enemyPosition = enemyTransform.GetGlobalTranslation();
			auto enemyForward = MovementSystem::GetLocalForward(enemyTransform);

			auto playerEnemyVec = enemyPosition - playerPosition;
			float playerEnemyDist = playerEnemyVec.Length();
			if(playerEnemyDist <= selfEntity->GetComponent<EnemyComponent>()->lineOfSight)
			{
				// shoot bullets
				ActorSystem::Shoot(world, selfEntity, enemyForward.GetNormalized() * 1.5f, playerEnemyVec.GetNormalized());
			}
			else
				return State::FAILURE;
		}
		return State::SUCCESS;
	};

	GGJGame::EnemyAIBase::ActionSignature goAfterPlayer = [](World* world, Entity* selfEntity)
	{
		PathfindingComponent* pathfindingCmp = selfEntity->GetComponent<PathfindingComponent>();

		for (auto playerTuple : world->IterateComponents<PlayerComponent>())
		{
			auto playerCmp = std::get<PlayerComponent*>(playerTuple);
			auto playerPosition = playerCmp->GetTransform().GetGlobalTranslation();
			pathfindingCmp->SetDestination(playerPosition);
			break;
		}

		auto& path = pathfindingCmp->GetPath();
		if(path.GetSize() > 0)
		{
			DebugDrawPath(selfEntity);
			auto entityTranslation = selfEntity->GetTransform().GetGlobalTranslation();
			//auto actorCmp = selfEntity->GetComponent<ActorComponent>();
			auto movementDirection = Vector(path[1].X, 0.0f, path[1].Y) - entityTranslation;
			ActorSystem::Move(selfEntity, movementDirection, ENEMY_MOVEMENT_SPEED_MULT);
		}

		return State::SUCCESS;
	};
}

void GGJGame::EnemyAIEngineer::InitActions()
{
	AddAction(10, Actions::lookForTransmitter);
}

void GGJGame::EnemyAIAssailant::InitActions()
{
	// priority: lower is more important
	//AddAction(10, Actions::attackPlayer);
	AddAction(20, Actions::goAfterPlayer);
}
