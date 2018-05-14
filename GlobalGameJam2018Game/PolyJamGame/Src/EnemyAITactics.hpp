#pragma once
#include "EnemyAIBase.hpp"

namespace GGJGame
{
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
