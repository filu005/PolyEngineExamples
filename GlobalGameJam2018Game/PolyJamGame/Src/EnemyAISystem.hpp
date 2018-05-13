#pragma once

#include <functional>
#include <ECS/World.hpp>
#include <EnemyAIBase.hpp>

using namespace Poly;

namespace GGJGame
{
	namespace EnemyAISystem
	{
		// functions in query namespace always(!) return EnemyAI::State.
		// it means that they can be in intermediate (async-like) state - State::RUNNING.
		namespace query
		{
			EnemyAI::State GAME_DLLEXPORT SetEntityToFollow(World* world, Entity* entity, std::function<Vector(World*, Entity*)> EntityPositionPredicate);
		}

		void DebugDrawPath(Entity* ent);
		void GAME_DLLEXPORT Update(World* world);
		void DeathParticle(Poly::Entity * ParticlesEnt, Poly::World * &world);
	}
}
