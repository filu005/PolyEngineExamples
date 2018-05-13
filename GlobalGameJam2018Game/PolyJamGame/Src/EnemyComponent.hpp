#pragma once

#include <ECS/ComponentBase.hpp>
#include <EnemyAIBase.hpp>

#include <deque>

using namespace Poly;

namespace GGJGame
{
	class GAME_DLLEXPORT EnemyComponent : public ComponentBase
	{
	public:
		EnemyComponent(std::unique_ptr<EnemyAIBase> enemyAI) : EnemyAI(std::move(enemyAI))
		{ }

		std::deque<Vector> delineatedPath;
		std::unique_ptr<EnemyAIBase> EnemyAI;
		const float lineOfSight = 20.0f;
		float TransmitterDestroySpeed = 0.5f;
	};
}
