#pragma once

#include "RenderingData.h"

namespace Engine::Graphics3D {
class Object3D;
}

namespace DirectXGame {

namespace GameLightDefaults {
	inline const Vector3 kPlayerLightOffset{ 0.0f, 40.0f, 22.5f };
	inline const Vector4 kDirectionalColor{ 1.05f, 1.0f, 0.95f, 1.0f };
	inline const Vector3 kDirectionalDirection{ -0.35f, -1.0f, -0.4f };
	inline constexpr float kDirectionalIntensity = 0.18f;
	inline const Vector4 kPointColor{ 1.0f, 0.96f, 0.82f, 1.0f };
	inline constexpr float kPointIntensity = 1.015f;
	inline constexpr float kPointRadius = 220.0f;
	inline constexpr float kPointDecay = 2.05f;
}

class GameLightSettings {
public:
	GameLightSettings();

	void SetLightingEnabled(bool enabled) { lightingEnabled_ = enabled; }
	bool IsLightingEnabled() const { return lightingEnabled_; }

	void SetDirectionalLight(const DirectionalLight& light) { directionalLight_ = light; }
	const DirectionalLight& GetDirectionalLight() const { return directionalLight_; }

	void SetPointLight(const PointLight& light) { pointLight_ = light; }
	const PointLight& GetPointLight() const { return pointLight_; }

	void SetSpotLight(const SpotLight& light) { spotLight_ = light; }
	const SpotLight& GetSpotLight() const { return spotLight_; }

	void ApplyTo(Engine::Graphics3D::Object3D& object) const;

private:
	bool lightingEnabled_ = true;
	DirectionalLight directionalLight_{};
	PointLight pointLight_{};
	SpotLight spotLight_{};
};

}
