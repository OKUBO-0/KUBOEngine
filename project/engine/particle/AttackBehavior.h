#pragma once
#include "IParticleBehavior.h"

class AttackBehavior : public IParticleBehavior
{
	Particle Create(std::mt19937& rng, const Vector3& pos) override;

	void Update(Particle& particle, float dt, Material* matelialData) override;

};

