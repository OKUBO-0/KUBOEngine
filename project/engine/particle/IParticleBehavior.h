#pragma once


#include <random>        
#include "Vector3.h"

struct Particle;
struct Material; 
class IParticleBehavior
{
public:
	virtual~IParticleBehavior() = default;
	// 毎フレームの更新処理
	virtual void Update(Particle& particle, float dt,  Material* matelialData) = 0;
	// 新規生成時の初期化（Emitの中で呼ばれる）
	virtual Particle Create(std::mt19937& rng, const Vector3& pos) = 0;

};

