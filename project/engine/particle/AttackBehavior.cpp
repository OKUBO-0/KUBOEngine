#include "AttackBehavior.h"
#include "ParticleManager.h"
#include <numbers>

Particle AttackBehavior::Create(std::mt19937& randomEngine, const Vector3& pos)
{
	std::uniform_real_distribution<float>distribution(-1.0, 1.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);
	std::uniform_real_distribution<float>disRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float>disScale(0.4f, 1.5f);

	Particle particle;
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };

	particle.transform.scale = { 0.5f,disScale(randomEngine),1.0f };
	//particle.transform.scale = { 1.0f,1.0f,1.0f };
	//particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.rotate = { disRotate(randomEngine),disRotate(randomEngine),disRotate(randomEngine) };
	//particle.transform.translate = translate + randomTranslate;
	particle.transform.translate = pos;
	//particle.Velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.Velocity = { 0.0f,0.0f,0.0f };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	//particle.color = { 1.0f,1.0f,1.0f,1.0f };
	//particle.lifetime = distTime(randomEngine);
	particle.lifetime = 1.0f;
	particle.currentTime = 0;
	return particle;
}

void AttackBehavior::Update(Particle& particle, float dt, Material* matelialData)
{

	matelialData= matelialData; // X方向スクロール
	
	//パーティクルの位置を更新
	particle.transform.translate += particle.Velocity * 1.0f / 60.0f;
	//パーティクルの寿命を減らす
	particle.currentTime += 1.0f / 60.0f;
}
