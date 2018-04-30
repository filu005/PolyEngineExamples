#pragma once

#include <functional>

#include <ECS/World.hpp>
#include <Collections/OrderedMap.hpp>

using namespace Poly;

namespace GGJGame
{
	enum class State
	{
		SUCCESS,
		FAILURE,
		RUNNING
	};

	struct Action
	{
		using ActionSignature = std::function<State(World* world, Entity* selfEntity)>;
		bool operator<(const Action& a)
		{
			return this->priority < a.priority;
		}

		ActionSignature action;
		int priority;
	};

	class GAME_DLLEXPORT EnemyAIBase : public BaseObject<>
	{
	public:
		using ActionSignature = std::function<State(World* world, Entity* selfEntity)>;

		EnemyAIBase() : actions(OrderedMap<int, ActionSignature>())
		{ }

		void AddAction(int priority, ActionSignature action)
		{
			actions.Insert(priority, action);
		}

		void IterateActions(World* world, Entity* selfEntity)
		{
			for(auto action : actions)
			{
				State s = action.value(world, selfEntity);

				// if action is still running, then save it so we could come back here in next tick
				if(s == State::RUNNING)
				{
					// bind some observer
				}

				// If the child succeeds, or keeps running
				if(s != State::FAILURE)
					return;
			}

			// if we're here then all actions have failed
			//RecreateActionsScript();
		}

	private:
		virtual void InitActions() = 0;

		void RecreateActionsScript()
		{
			actions.Clear();

			InitActions();
		}
		OrderedMap<int, ActionSignature> actions;
	};

	class GAME_DLLEXPORT EnemyAIEngineer : public EnemyAIBase
	{
	public:
		EnemyAIEngineer()
		{
			InitActions();
		}
	private:
		virtual void InitActions() override final;
	};

	class GAME_DLLEXPORT EnemyAIAssailant : public EnemyAIBase
	{
	public:
		EnemyAIAssailant()
		{
			InitActions();
		}
	private:
		virtual void InitActions() override final;
	};
}