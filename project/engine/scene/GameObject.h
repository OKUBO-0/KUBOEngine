#pragma once

#include "Component.h"
#include <memory>
#include <string>
#include <vector>

class GameObject
{
public:
	explicit GameObject(std::string name) : name_(std::move(name)) {}

	const std::string& GetName() const { return name_; }
	void SetName(std::string name) { name_ = std::move(name); }

	bool IsActive() const { return active_; }
	void SetActive(bool active) { active_ = active; }

	template <class T, class... Args>
	T* AddComponent(Args&&... args)
	{
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T* rawPtr = component.get();
		components_.push_back(std::move(component));
		return rawPtr;
	}

	const std::vector<std::unique_ptr<Component>>& GetComponents() const { return components_; }
	std::vector<std::unique_ptr<Component>>& GetComponents() { return components_; }

	void AddChild(GameObject* child)
	{
		if (child == nullptr) {
			return;
		}
		children_.push_back(child);
	}

	const std::vector<GameObject*>& GetChildren() const { return children_; }

private:
	std::string name_;
	bool active_ = true;
	std::vector<std::unique_ptr<Component>> components_;
	std::vector<GameObject*> children_;
};
