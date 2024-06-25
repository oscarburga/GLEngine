#pragma once

#include "Utils/MultiTypeContainer.h"
#include "InputConcepts.h"
#include "Camera.h"

#define INPUT_PROCESSOR_CLASSES GCamera

DeclareConceptContainer(InputProcessors, HandlesInput)

class WorldContainers
{
public:
	static InputProcessors<INPUT_PROCESSOR_CLASSES> InputProcessors;
};
