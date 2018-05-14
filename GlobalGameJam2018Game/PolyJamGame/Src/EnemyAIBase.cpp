#include "EnginePCH.hpp"

#include "EnemyAIBase.hpp"

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
