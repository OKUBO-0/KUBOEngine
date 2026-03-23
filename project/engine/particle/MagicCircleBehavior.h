#pragma once
#include "IParticleBehavior.h"


class MagicCircleBehavior : public IParticleBehavior
{
public:
	
	Particle Create(std::mt19937& rng, const Vector3& pos) override;
		
	void Update(Particle& particle, float dt, Material* matelialData) override;

	
	



};

