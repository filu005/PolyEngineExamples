#pragma once

#include <functional>

#include <ECS/World.hpp>
#include <EnemyAIAction.hpp>
#include <deque>

using namespace Poly;

namespace GGJGame
{
	class GAME_DLLEXPORT EnemyAIBase : public BaseObject<>
	{
	public:
		EnemyAIBase()
		{ }

		void AddAction(Action && action)
		{
			actions.emplace_back( std::move(action) );
		}

		void IterateActions(World* world, Entity* selfEntity);

	private:
		virtual void InitActions() = 0;

		void RefreshActionsDeque()
		{
			while(true)
			{
				auto action = actions.front();
				actions.pop_front();
				actions.push_back(action);
				if(action.IsNull())
					break;
			}
			//std::sort(actions.begin(), actions.end(), std::less<Action>());
		}

		void RecreateActionsScript()
		{
			actions.erase(actions.begin(), actions.end());

			InitActions();

			std::sort(actions.begin(), actions.end(), std::less<Action>());
		}

		std::deque<Action> actions;
	};
}