# GLEngine

This project is an OpenGL rendering engine written in C++ for learning purposes. 

Current interesting features include:
* Programmable vertex pulling
* Skinning
* Cascading shadow maps
* Frustum culling
* PBR lightning (kind-of)

https://github.com/user-attachments/assets/596eca9c-bac5-4e98-8bf2-cb8e69a2ea8f

All models used for the sample scene are public, downloaded from Sketchfab. 
Check `Content/GLTF` folder, there's the `license.txt` file for each model with links & credits to each author :D 

High on the TODO list, but in no specific order: 
* Refactor renderer & render passes into something more tidy
* Make stuff more bindless (setup as if for doing GPU-driven rendering)
    * Look into the bindless texture extension
* Compute shader stuff:
    * Compute culling?
    * Draw indirect?

Other wishlist / TODO:
* ~~ImGui~~ (done) 
* Point/Spot light support with the new pbr shaders
    * Also actually learn PBR, currently just blackboxing it for the most part.
* Basic 2D / UI rendering 
* Maybe cvars
* Lua scripting support
    * Not sure what I'd even use it for right now but I've always wanted to integrate
      Lua scripting to a project :D 
* BIG refactor to make more usable / extensible, maybe for a mini-game engine or something.

## Links, References and Stuff

* All used libraries are in the ThirdParty directory.
* Learning OpenGL / Graphics concepts from https://learnopengl.com/
* Used a little bit of OGL https://ogldev.org/
* Vulkan Guide, using for renderer architecture inspiration: https://vkguide.dev/
* Khronos docs for OpenGL, Vulkan and GLTF
* [Elias Daler's Bikeshed Renderer](https://github.com/eliasdaler/edbr). Been checking this project out a bit for inspiration & to see how others implement stuff. Hope my engine can look like that someday :)
