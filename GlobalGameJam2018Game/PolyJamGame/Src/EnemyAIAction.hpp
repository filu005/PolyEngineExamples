#pragma once

#include <functional>

#include <ECS/World.hpp>

using namespace Poly;

namespace GGJGame
{
	namespace EnemyAI
	{
		enum class State
		{
			INVALID,
			SUCCESS,
			FAILURE,
			RUNNING
		};
	}

	class Action
	{
	public:
		using ActionSignature = std::function<EnemyAI::State(World* world, Entity* entity)>;

		Action(int priority, ActionSignature action) : priority(priority), action(action)
		{ }

		bool operator<(const Action& a) const
		{
			return this->priority < a.priority;
		}

		EnemyAI::State run(World* world, Entity* selfEntity) const
		{
			return action(world, selfEntity);
		}

		bool IsNull() const
		{
			return action == nullptr;
		}

	private:
		ActionSignature action;
		int priority;
	};
}