#include "EnginePCH.hpp"

#include "AI/PathfindingComponent.hpp"
#include "TransmitterComponent.hpp"
#include "ActorComponent.hpp"
#include "ActorSystem.hpp"
#include "PlayerComponent.hpp"
#include "EnemyComponent.hpp"
#include "EnemyAIAction.hpp"
#include "EnemyAISystem.hpp"
#include "EnemyAITactics.hpp"

namespace Actions
{
	using namespace GGJGame;
	// wander around in direction of transmitter
	GGJGame::Action::ActionSignature lookForTransmitter = [](World* world, Entity* selfEntity)
	{
		using namespace EnemyAI;

		auto findPositionOfClosestTransmitter = [](World* world, Entity* entity)
		{
			Vector closestTransmitterPosition;
			auto shortestDistanceSquared = std::numeric_limits<float>::min();

			for(auto transmitterTuple : world->IterateComponents<TransmitterComponent>())
			{
				auto transmitterCmp = std::get<TransmitterComponent*>(transmitterTuple);

				Vector entityPosition = entity->GetTransform().GetGlobalTranslation();
				auto transmitterPosition = transmitterCmp->GetTransform().GetGlobalTranslation();

				auto distanceSquared = Vector(transmitterPosition - entityPosition).LengthSquared();
				if(distanceSquared > shortestDistanceSquared)
				{
					closestTransmitterPosition = transmitterPosition;
					shortestDistanceSquared = distanceSquared;
				}
			}

			return closestTransmitterPosition;
		};

		State state = EnemyAISystem::query::MoveEntityToPosition(world, selfEntity, findPositionOfClosestTransmitter);

		return state;
	};

	GGJGame::Action::ActionSignature attackPlayer = [](World* world, Entity* selfEntity)
	{
		using namespace EnemyAI;

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
				// if player is not obstructed by geometry
				auto rayCastRes = Physics3DSystem::ClosestHitRaycast(world, enemyPosition, playerPosition, eCollisionGroup::RIGIDBODY_GREEN, eCollisionGroup::RIGIDBODY_GREEN);

				for(auto hit : rayCastRes.Hits)
				{
					Entity* ent = hit.HitEntity;
					if(PlayerComponent* playerCmp = ent->GetComponent<PlayerComponent>())
					{
						// shoot bullets
						ActorSystem::Shoot(world, selfEntity, enemyForward.GetNormalized() * 1.5f, playerEnemyVec.GetNormalized());
						gConsole.LogDebug("Shooting at player!", playerCmp->GetSibling<ActorComponent>()->GetHitPoints());
						break;
					}
				}
			}
			else
				return State::FAILURE;
		}
		return State::SUCCESS;
	};

	GGJGame::Action::ActionSignature goAfterPlayer = [](World* world, Entity* selfEntity)
	{
		using namespace EnemyAI;

		auto getPlayerPosition = [](World* world, Entity* entity)
		{
			Vector playerPosition;
			for(auto playerTuple : world->IterateComponents<PlayerComponent>())
			{
				auto playerCmp = std::get<PlayerComponent*>(playerTuple);
				playerPosition = playerCmp->GetTransform().GetGlobalTranslation();
				break;
			}
			return playerPosition;
		};

		State state = EnemyAISystem::query::MoveEntityToPosition(world, selfEntity, getPlayerPosition);

		return state;
	};
}

void GGJGame::EnemyAIEngineer::InitActions()
{
	AddAction(Action(0, Actions::lookForTransmitter));
	AddAction(Action(std::numeric_limits<int>::min(), nullptr)); // 'end of actions' marker
}

void GGJGame::EnemyAIAssailant::InitActions()
{
	// priority: lower is more important
	//AddAction(Action(20, Actions::attackPlayer));
	AddAction(Action(10, Actions::goAfterPlayer));
	AddAction(Action(std::numeric_limits<int>::min(), nullptr)); // 'end of actions' marker
}
