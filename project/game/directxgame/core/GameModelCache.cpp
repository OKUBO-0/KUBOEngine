#include "game/directxgame/core/GameModelCache.h"
#include "game/directxgame/core/DirectXGameResourcePaths.h"
#include "ModelManager.h"
#include "Object3D.h"
#include <cassert>
#include <filesystem>
#include <unordered_map>

namespace DirectXGame {

namespace {

struct CachedModelEntry {
	std::string requestedName;
	std::string resolvedFileName;
};

std::unordered_map<std::string, ModelHandle>& GetNameToHandle()
{
	static std::unordered_map<std::string, ModelHandle> cache;
	return cache;
}

std::unordered_map<ModelHandle, CachedModelEntry>& GetHandleToModel()
{
	static std::unordered_map<ModelHandle, CachedModelEntry> cache;
	return cache;
}

ModelHandle& GetNextHandle()
{
	static ModelHandle nextHandle = 1;
	return nextHandle;
}

std::string ResolveModelFileName(const std::string& modelName)
{
	const std::filesystem::path resourceRoot = ResourcePaths::GetModelResourceRoot();
	const std::filesystem::path requestedPath(modelName);

	const std::filesystem::path directPath = resourceRoot / requestedPath;
	if (std::filesystem::exists(directPath)) {
		return requestedPath.generic_string();
	}

	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(resourceRoot)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		const std::filesystem::path relativePath = std::filesystem::relative(entry.path(), resourceRoot);
		if (!relativePath.empty() && *relativePath.begin() == "models") {
			continue;
		}
		const bool matchesStem = !requestedPath.has_extension() && entry.path().stem() == modelName;
		const bool matchesFileName = requestedPath.has_extension() && entry.path().filename() == requestedPath.filename();
		if (matchesStem || matchesFileName) {
			return relativePath.generic_string();
		}
	}

	if (requestedPath.has_extension()) {
		return requestedPath.filename().generic_string();
	}

	const std::filesystem::path modelRoot = resourceRoot / "models";
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(modelRoot)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		if (entry.path().stem() == modelName) {
			return std::filesystem::relative(entry.path(), modelRoot).generic_string();
		}
	}

	return requestedPath.generic_string();
}

}

ModelHandle GameModelCache::Load(const std::string& modelName)
{
	auto& nameToHandle = GetNameToHandle();
	if (nameToHandle.contains(modelName)) {
		return nameToHandle.at(modelName);
	}

	const std::string resolvedFileName = ResolveModelFileName(modelName);
	Engine::Graphics3D::ModelManager::GetInstance()->LoadModelFromResourceRoot(
		ResourcePaths::GetModelResourceRoot(), resolvedFileName);

	ModelHandle handle = GetNextHandle()++;
	nameToHandle.emplace(modelName, handle);
	GetHandleToModel().emplace(handle, CachedModelEntry{ modelName, resolvedFileName });
	return handle;
}

Engine::Graphics3D::Model* GameModelCache::Get(ModelHandle handle)
{
	return Engine::Graphics3D::ModelManager::GetInstance()->FindModelFromResourceRoot(
		ResourcePaths::GetModelResourceRoot(), GetResolvedFileName(handle));
}

const std::string& GameModelCache::GetResolvedFileName(ModelHandle handle)
{
	const auto& handleToModel = GetHandleToModel();
	assert(handleToModel.contains(handle));
	return handleToModel.at(handle).resolvedFileName;
}

void GameModelCache::ApplyToObject(Engine::Graphics3D::Object3D& object, ModelHandle handle)
{
	object.SetModelFromResourceRoot(ResourcePaths::GetModelResourceRoot(), GetResolvedFileName(handle));
}

}
