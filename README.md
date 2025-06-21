# GLEngine

This project is an OpenGL rendering engine written in C++ for learning purposes. 

https://github.com/user-attachments/assets/596eca9c-bac5-4e98-8bf2-cb8e69a2ea8f

All models used for the sample scene are public, downloaded from Sketchfab. 
Check `Content/GLTF` folder, there's the `license.txt` file for each model with links & credits to each author :D 

Stuff I eventually want to get to but I keep procrastinating:
* ~~ImGui~~ (done) 
* Maybe cvars to make my development & testing life 10x easier
* Point/Spot light support with the new pbr shaders
    * Also actually learn PBR, currently just blackboxing it for the most part.
    * I miss the simplicity of the diffuse-specular model :(
* Basic 2D / UI rendering 
* 2nd mesh "pipeline" with support for multiple UV channels
* Refactor to make more usable / extensible, maybe for a mini-game engine or something.
    * Trying to maintain a software design that makes sense while also learning all the underlying 
      algorithms, techniques, best practices & graphics API feels borderline impossible, 
      but eventually it will happen. Just cannot afford to do it currently or I would probably spend 
      more time refactoring than actually doing useful stuff. 

## Links, References and Stuff

* All used libraries are in the ThirdParty directory.
* Learning OpenGL / Graphics concepts from https://learnopengl.com/
* Used a little bit of OGL https://ogldev.org/
* Vulkan Guide, using for renderer architecture inspiration: https://vkguide.dev/
* Khronos docs for OpenGL, Vulkan and GLTF
* [Elias Daler's Bikeshed Renderer](https://github.com/eliasdaler/edbr). Been checking this project out a bit for inspiration & to see how others implement stuff. Hope my engine can look like that someday :)
