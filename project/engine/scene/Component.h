#pragma once

class Component
{
public:
	virtual ~Component() = default;

	virtual const char* GetTypeName() const = 0;
	virtual void DrawInspectorImGui() = 0;

	bool IsEnabled() const { return enabled_; }
	void SetEnabled(bool enabled) { enabled_ = enabled; }

private:
	bool enabled_ = true;
};
