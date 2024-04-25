# StaticEngine

This project is a very basic game engine written in C++, with very ~~unnecessary and silly~~ interesting development restrictions.

The whole purpose of this project, for me, is to learn more about computer graphics, game engine architectures, modern C++ features, and other good stuff.
Working daily at my job with Unreal Engine, a lot of rendering & engine concepts are abstracted away and it is easy to forget about them or get really rusty. 
I want to brush up on the skills I already had and learn new stuff :) 

## Core Project Goals

Create a basic game engine in C++ using ONLY static polymorphism (no virtual functions, dynamic_cast, etc.).
* Main source of polymorphism should come from the use of templates and C++20's concepts. 
* Inheritance/interfaces, pointers to base-class, etc. are allowed, but virtual functions and dynamic casts are not.
* Switch cases with type enums as a replacement for polymorphism should also be avoided.

## Bonus / Longer Term Goals

In no particular order:
* Implement multithreading in some meaningful way into the engine.
* Implement physics in some meaningful way into the engine. 
    * Probably would use Jolt for it.
* Vulkan rendering
    * Starting with OpenGL since it is far simpler and quicker to get up-and-running and to learn with.
    * Learning a bit of Vulkan in parallel but won't implement into this project until way further down the road.
    * Possibly DirectX after Vulkan, although currently I'm not interested in it at all to be honest.
* Implement my own, very basic window system to replace GLFW3
    * This one might end up being a task too big to fit as a part of this project. 
    * Also would have to write a lot of platform-specific code. Not prioritizing it at all.
* Use the engine to create a small Kingdom Rush game clone 
	* Not releasing as a finished game or anything, just for funzies & learning.
    * Not affiliated in any way, I just like the KR games a lot :)
* CMake setup and proper modularization
    * This is probably one of the first things I should do, but making this project multiplatform / compilable for other people is not at all a priority for me right now. We'll see, might change my mind.

## Links, References and Stuff

* Learning OpenGL / Graphics concepts from https://learnopengl.com/
* Vulkan Guide and some Game Engine architecture from https://vkguide.dev/
* Khronos docs for OpenGL and Vulkan
* [Elias Daler's Bikeshed Renderer](https://github.com/eliasdaler/edbr). Been checking him out as an example for engine/API architecture. Hope my engine can look like his someday :)
* Indirectly using Unreal Engine as reference / example, since I've used it on my job for 2-3 years now.
