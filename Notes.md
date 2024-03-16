
## Alpha test and blending stage

After the fragment shader calculates the final pixel color, alpha test and blending is performed. 
This stage checks the depth and stencil values of the fragment and uses those to check if the resulting fragment is in front or behind other objects, and should be discarded accordingly.
This stage also blends translucent values accordingly.

## Normalized device coordinates (NDC)

OpenGL only process 3D coordinates when they're in the (-1, 1) range on all 3 axes (x, y, z). 
* (x = 0, y = 0) is the middle of the screen
* (x = 1) is the right edge of the screen
* (x = -1) is the left edge of the screen
* (y = -1) is the bottom edge of the screen
* (y = 1) is the top edge of the screen

NDC coordinates then get converted to screen-space coordinates through the viewport transform using the glViewport data.
The resulting screen-space coordinates is what is fed as input to the fragment shader.

## GL buffers

Create buffers that OpenGL can manage and move in and out of the GPU memory when necessary.
Sending data from CPU to GPU is slow so ideally we minimize data transfers.

OpenGL doesn't allow super granular control over this. When you write memory to a buffer, you can specify an usage type:
* GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
* GL_STATIC_DRAW: the data is set only once and used many times.
* GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

OpenGL will then create and manage the buffer accordingly to the use case. Dynamic draw may place the memory in a region that allows for faster writes for example.

## Vertex array object and vertex buffer object

The vertex array object holds the bindings to all the buffers that we want to use as data for a draw call.
The VAO remembers all the calls to bind buffers and VertexAttribPointer you make while it is bound.
VAO binds attributes to locations for input variables in shader code.

* Vertex array object contains one or more vertex buffer objects, and is designed to store the information for a complete rendered object.
* Vertex buffer object is a memory buffer to hold information about vertices (coordinates, colors, normals, texcoords, etc etc)


