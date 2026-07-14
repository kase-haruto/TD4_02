#pragma once

#include <Engine/Scene/Reference/TransformReference.h>

struct LockOnStartedEvent {
	CalyxEngine::TransformRef target;
};

struct LockOnChangedEvent {
	CalyxEngine::TransformRef previousTarget;
	CalyxEngine::TransformRef currentTarget;
};

struct LockOnClearedEvent {
	CalyxEngine::TransformRef previousTarget;
};
