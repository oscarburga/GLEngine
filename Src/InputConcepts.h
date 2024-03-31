#pragma once

struct GLFWwindow;

template<typename T>
concept HandlesInput = requires(T inputHandler, GLFWwindow* window, float pointerDeltaX, float pointerDeltaY, float deltaTime)
{
	inputHandler.HandleInput(window, pointerDeltaX, pointerDeltaY, deltaTime);
};
