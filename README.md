# GLEngine

This project is an OpenGL rendering engine written in C++ for learning purposes. 

I have decided to NOT include any screenshots OR any of the sample models I use during 
development & testing to avoid any licensing concerns for the future since they are not 
self-made (its mostly random handpicked stuff from sketchfab).

TODO: List all features/scope here.

Coming up next:
* ~~Fit shadow mapping to camera frustum~~ (done)
    * CSM still pending
* ~~Skinning~~(done)

Stuff I eventually want to get to but I keep procrastinating:
* ~~ImGui~~ (done) and maybe cvars to make my development & testing life 10x easier
* Point/Spot light support with the new pbr shaders
    * Also actually learn PBR, currently just blackboxing it for the most part.
    * I miss the simplicity of the diffuse-specular model :(
* Basic 2D / UI rendering 
* 2nd mesh "pipeline" with support for multiple UV channels
* Refactor to make more maintainable/independent/modular/whatever.
    * Trying to maintain a software design that makes sense while also learning all the underlying 
      algorithms, techniques, best practices & graphics API feels borderline impossible, 
      but eventually it will happen. Just cannot afford to do it currently or I would probably spend 
      more time refactoring than actually doing useful stuff. 
* Figure out license stuff for sample models and screenshots

## Links, References and Stuff

* All used libraries are in the ThirdParty directory.
* Learning OpenGL / Graphics concepts from https://learnopengl.com/
* Used a little bit of OGL https://ogldev.org/
* Vulkan Guide, using for renderer architecture inspiration: https://vkguide.dev/
* Khronos docs for OpenGL, Vulkan and GLTF
* [Elias Daler's Bikeshed Renderer](https://github.com/eliasdaler/edbr). Been checking this project out a bit for inspiration & to see how others implement stuff. Hope my engine can look like that someday :)