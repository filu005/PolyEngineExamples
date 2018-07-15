#pragma once

#include "ECS/ComponentBase.hpp"
#include "ECS/Entity.hpp"
#include <Collections/Dynarray.hpp>

namespace BT
{
	class GAME_DLLEXPORT GameManagerWorldComponent : public Poly::ComponentBase
	{
	public:
		GameManagerWorldComponent() = default;

		Poly::Entity* Camera;
		Poly::Entity* DirectionalLight;

		Poly::Entity* Ground;
		Poly::Dynarray<Poly::Entity*> Bricks;
		Poly::Dynarray<Poly::Entity*> Bullets;
	};
}