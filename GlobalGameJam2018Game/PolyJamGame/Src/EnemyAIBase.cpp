#include "EnginePCH.hpp"

#include "AI/PathfindingComponent.hpp"
#include "TransmitterComponent.hpp"
#include "ActorComponent.hpp"
#include "PlayerComponent.hpp"
#include "EnemyComponent.hpp"
#include "EnemyAISystem.hpp"
#include "EnemyAIBase.hpp"
#include "EnemyAIAction.hpp"

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

		State state = EnemyAISystem::query::SetEntityToFollow(world, selfEntity, findPositionOfClosestTransmitter);

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
				// shoot bullets
				//ActorSystem::Shoot(world, selfEntity, enemyForward.GetNormalized() * 1.5f, playerEnemyVec.GetNormalized());
				;
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

		State state = EnemyAISystem::query::SetEntityToFollow(world, selfEntity, getPlayerPosition);

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

void GGJGame::EnemyAIBase::IterateActions(World * world, Entity * selfEntity)
{
	using namespace EnemyAI;

	while(true)
	{
		auto currentAction = actions.front();
		actions.pop_front();

		// we reached 'end of actions' marker
		if(currentAction.IsNull())
		{
			actions.push_back(currentAction);
			return;
		}

		State s = currentAction.run(world, selfEntity);

		switch(s)
		{
		case State::INVALID:
			actions.push_back(currentAction);
			return;
		case State::RUNNING:
			// if action is still running, then push it back to the front
			// so we could come back to it in the next tick
			actions.push_front(currentAction);
			return;
		case State::SUCCESS:
			// successfully ran the action in present tick
			actions.push_back(currentAction);
			RefreshActionsDeque();
			return;
		case State::FAILURE:
			actions.push_back(currentAction);
			continue; // ??
		default:
			break;
		}

		// we should never be here
		assert(false);
	}
}
