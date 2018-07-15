#pragma once

#include <ECS/World.hpp>
#include <Collections/String.hpp>
#include <Math/Vector.hpp>
#include <Rendering/Particles/ParticleComponent.hpp>

using namespace Poly;

namespace GameManagerSystem
{
	void CreateScene(World* world);


	Entity* CreateModel(World* world, String path);
	
	void CreateTextUI(World* world);
	void CreateCamera(World* world);
	void CreatePBRShpereGrid(World* world, Vector pos, Color albedo);
	void CreateTranslucent(World* world);
	void CreateSponza(World* world);
	void CreateSponzaSample(World* world);
	
	void CreatePointLights(World* world, int quota);
	Entity* CreatePointLight(World* world, Vector& position, float Range);
	void CreateSpotLight(World* world, float Range);
	void SpawnParticles(World* world);
	void SpawnSpritesSheets(World* world);
	void SpawnShaderball(World* world);

	void SpawnSpritesheet11(World* world, Vector pos);
	void SpawnSpritesheet22(World* world, Vector pos);
	void SpawnSpritesheet44(World* world, Vector pos);
	void SpawnSpritesheet42(World* world, Vector pos);
	void SpawnSpritesheet41(World* world, Vector pos);
	void SpawnSpritesheet44Random(World* world, Vector pos);
	void SpawnSpritesheetGandalf(World* world, Vector pos);

	ParticleComponent* SpawnEmitterDefault(World* world, Vector pos);
	ParticleComponent* SpawnEmitterWorldSpace(World* world, Vector pos);
	ParticleComponent* SpawnEmitterLocalSpace(World* world, Vector pos);
	ParticleComponent* SpawnEmitterAmbient(World* world, Vector pos);
	ParticleComponent* SpawnEmitterAmbientWind(World* world, Vector pos);
	ParticleComponent* SpawnEmitterHeart(World* world, Vector pos);
	ParticleComponent* SpawnEmitterHeartImpact(World* world, Vector pos);
	ParticleComponent* SpawnEmitterHeartImpact2(World* world, Vector pos);
	void SpawnHeartSystem(World* world);

	void Update(World* world);
	void UpdateParticles(World* world);
	void UpdateLights(World* world);
	void UpdateModel(World* world);
	void UpdatePostProcess(World* world);
	
	void Deinit(World* world);
}
