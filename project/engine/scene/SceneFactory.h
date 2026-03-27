#pragma once

#include "AbstractSceneFactory.h"
#include <memory>
class SceneFactory
	: public AbstractSceneFactory
{

public:
	/// <summary>
	/// シーンの生成
	/// </summary>
	std::unique_ptr<BaseScene> CreateScene(const std::string&sceneName)override;





};

