#pragma once

#include <ECS/World.hpp>

namespace BT
{
	namespace GameManagerSystem
	{
		void InitializeDemoWorld(Poly::World* world);

		void SpawnBullet(Poly::World* world);

		void Update(Poly::World* world);
	}
}