#pragma once

#include "BaseScene.h"
#include <memory>
#include <string>
class AbstractSceneFactory
{

public:
	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& Scenename) = 0;
	virtual ~AbstractSceneFactory() = default;

};

