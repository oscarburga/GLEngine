#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"
#include "EngineMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "RefIgnore.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float deltaTime);

struct World
{
	static const vec3 front;
	static const vec3 up;
	static const vec3 right;
};

const vec3 World::front = vec3(0.0f, 0.0f, 1.0f);
const vec3 World::up = vec3(0.0f, 1.0f, 0.0f);
const vec3 World::right = vec3(1.0f, 0.0f, 0.0f);

class Camera
{
	mat4 lookAt;
public:
	vec3 pos;
	vec3 front;
	vec3 up;
	float yaw { 0.0f };
	float pitch { 0.0f };
	float speed { 5.0f };
	float yawSens { 100.01f };
	float pitchSens { 100.01f };

	Camera(const vec3& _pos, const vec3& _lookAtLoc) : pos(_pos)
	{
		front = glm::normalize(_lookAtLoc - _pos);
		const vec3 right = glm::cross(front, World::up); // Both unit vectors, don't need renormalize
		up = glm::cross(right, front); // Both unit vectors, ddon't need renormalize
		lookAt = glm::lookAt(_pos, _lookAtLoc, up);
	}

	Camera() : Camera(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f)) {}

	vec3 GetRightVector() { return glm::normalize(glm::cross(front, up)); }
	mat4& GetWorldToCamera() { return lookAt; }
	mat4& UpdateAndGetWorldToCamera() 
	{
		lookAt = glm::lookAt(pos, pos + front, up); 
		return lookAt; 
	}

	void UpdateAxesFromYawPitch()
	{
		const float yawr = glm::radians(yaw);
		const float pitchr = glm::radians(pitch);
		front.x = cos(yawr) * cos(pitchr);
		front.y = sin(pitchr);
		front.z = sin(yawr) * cos(pitchr);
		front = glm::normalize(front);
		// const vec3 right = glm::normalize(glm::cross(front, World::up));
		// up = glm::cross(right, front);
	}
} camera;

uint32_t winSizeX = 0, winSizeY = 0;
float aspectRatio = 0;

int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "GLEngine", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create glfw window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to init GLAD\n";
		return -1;
	}

	framebuffer_size_callback(window, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	{
		int numAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
		std::cout << "Max number of vertex attributes: " << numAttributes << " 4-component vertex attributes\n";
	}

	const float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	const vec3 cubePositions[] = {
		vec3(0.0f,  0.0f,  0.0f),
		vec3(2.0f,  5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -2.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3(2.4f, -0.4f, -3.5f),
		vec3(-1.7f,  3.0f, -7.5f),
		vec3(1.3f, -2.0f, -2.5f),
		vec3(1.5f,  2.0f, -2.5f),
		vec3(1.5f,  0.2f, -1.5f),
		vec3(-1.3f,  1.0f, -1.5f)
	};

	// opengl tex coords are measured with (0,0) being bottom left corner.
	// regular images have (0,0) coordinate being top left corner. Make sure to vertically flip images when reading them
	// float texCoords[] = {  // unused texcoords now  that we have the cube
	// 	1.f, 1.f, // top right
	// 	1.f, 0.f, // bot right
	// 	0.f, 0.f, // bot left
	// 	0.f, 1.f // top left
	// };

	// GLuint indices[] = {  // note that we start from 0!
	// 	0, 1, 3,   // first triangle
	// 	1, 2, 3    // second triangle
	// };  

	// Vertex array objects store all the calls to glEnableVertexAttribArray or glDisableVertexAttribArray, glVertexAttribPointer, and also stores the buffer objects associated.
	// Use them to not have to re-specify these attributes every time you want to use a certain buffer with a layout. 
	GLuint VAO;
	glCreateVertexArrays(1, &VAO);
	GLuint buffers[3];
	glCreateBuffers(3, buffers);
	// glGenBuffers(2, buffers); // Create a buffer and get a handle to it
	// auto [VBO, texCoordBuffer, IBO] = buffers; 
	GLuint VBO = buffers[0];

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// glNamedBufferData(texCoordBuffer, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	// glNamedBufferData(IBO, sizeof(indices), indices, GL_STATIC_DRAW);

	const int vboBindIndex = 0;// , texCoordsBindIndex = 1;

	// glVertexArrayElementBuffer(VAO, IBO); // Set the VAO to use IndexBufferObject for the element indices.

	// What buffers it uses in each slot for vertex data
	glVertexArrayVertexBuffer(VAO, vboBindIndex, VBO, 0, 5 * sizeof(float));
	// glVertexArrayVertexBuffer(VAO, texCoordsBindIndex, texCoordBuffer, 0, 2 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 0); // enable VAO's array attrib 0
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO's array attrib 0 has 3 floats,
	glVertexArrayAttribBinding(VAO, 0, vboBindIndex); // VAO's array attrib 0 reads from the buffer bound at vboBindIndex (0)

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // array attrib has 3 floats, starting after 3 floats (from attrib 0)
	glVertexArrayAttribBinding(VAO, 1, vboBindIndex); // VAO's array attrib 1 reads from the buffer bound at vboBindIndex (0)

	// glEnableVertexArrayAttrib(VAO, 2);
	// glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, 0); // array attrib has 2 floats
	// glVertexArrayAttribBinding(VAO, 2, texCoordsBindIndex); // vao's array attrib 2 reads from buffer bound at texCoordsBindIndex(1)


	// glBindVertexArray(VAO);
	// Start "recording" all the buffer bindings and attributes for this vertex array object.
	// glBindBuffer(GL_ARRAY_BUFFER, VBO); // OpenGL state machine has a slot to use an Array Buffer. Set it to use our Array Buffer
	// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy our vertex data into the currently bound array buffer (our buffer).
	// glBufferData requires specifying buffer usage: static draw sets the data only once and uses it many times.

	// Specify wthe layout of each vertex data in the array for opengl to interpret before rendering.
	// In this case, we specify that the first attribute (attrib at location 0) has 3 components (must be 1-4), 
	// we don't want to auto-clamp it, it takes the size of 3 floats, and there's no offset on the first component of the first attribute in the array.
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr); 
	// glEnableVertexAttribArray(0); // remember to enable the attrib lol
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);

	// glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	// glEnableVertexAttribArray(2);

	// Bind the element buffer object to draw the vertices by index
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Unbind the VAO, VBO and EBO when done setting them up.
	// glBindVertexArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Textures
	GLuint textures[2];
	const char* paths[2] = { "Textures/container.jpg", "Textures/awesomeface.png" };
	GLenum inputFormat[2] = { GL_RGB, GL_RGBA };
	glCreateTextures(GL_TEXTURE_2D, 2, textures);
	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < 2; i++)
	{
		int w, h, c;
		if (stbi_uc* texData = stbi_load(paths[i], &w, &h, &c, 0))
		{
			GLuint texture = textures[i];
			const int numLevels = 1 + floor(log2(std::max(w, h))); // TODO use count leading zero to calculate num mipmaps
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float texBorderColor[] = { 0.0f, 1.1f, 0.08f, 1.0f };
			glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, texBorderColor);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // downscale texture filtering: nearest
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // upscale texture filtering: linear interp
			glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Take closest mipmap and linearly interpolate

			glTextureStorage2D(texture, numLevels, GL_RGB8, w, h);
			glTextureSubImage2D(texture, 0, 0, 0, w, h, inputFormat[i], GL_UNSIGNED_BYTE, texData);
			glGenerateTextureMipmap(texture);
			// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, inputFormat[i], GL_UNSIGNED_BYTE, texData);
			// glGenerateMipmap(GL_TEXTURE_2D);
			// glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(texData);
		}
		else
		{
			std::cerr << "failed to load texture\n";
			std::abort();
		}
	}
	auto [woodTexture, faceTexture] = textures;
	
	Shader shader("Shaders/shader.vert", "Shaders/shader.frag");
	shader.use();
	glBindVertexArray(VAO);
	glBindTextureUnit(0, woodTexture);
	glBindTextureUnit(1, faceTexture);
	shader.SetUniform("woodTexture", int(0));
	shader.SetUniform("faceTexture", int(1));
	// glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	float lastFrameTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const float time = (float)glfwGetTime();
		const float deltaTime = time - lastFrameTime;
		lastFrameTime = time;
		shader.SetUniform("time", time);
		processInput(window, deltaTime);
		mat4 cameraToPerspective = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.f);
		shader.SetUniform("worldToCamera", camera.UpdateAndGetWorldToCamera());
		shader.SetVarUniform(cameraToPerspective);

		for (int i = 0; i < 10; i++)
		{
			mat4 localToWorld = mat4(1.0f);
			const float angle = 20.0f * i;
			localToWorld = glm::translate(localToWorld, cubePositions[i]);
			localToWorld = glm::rotate(localToWorld, time * glm::radians(angle), vec3(1.0f, 0.0f, 0.0f));
			// localToWorld = glm::scale(localToWorld, vec3(1.0f, 1.0f, 1.0f));
			shader.SetVarUniform(localToWorld);
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(float));
		}

		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	winSizeX = width;
	winSizeY = height;
	aspectRatio = float(width) / float(height);
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += camera.front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= camera.front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= camera.GetRightVector();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += camera.GetRightVector();
	camera.pos += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(camera.speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);


	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	static float lastx = xpos;
	static float lasty = ypos;

	float deltax = (float(xpos) - lastx);
	float deltay = (float(ypos) - lasty);

	lastx = (float)xpos;
	lasty = (float)ypos;

	camera.yaw += deltax * deltaTime * camera.yawSens;
	camera.pitch -= deltay * deltaTime * camera.pitchSens;
	camera.pitch = glm::clamp(camera.pitch, -89.f, 89.f);

	camera.UpdateAxesFromYawPitch();
	std::cout << camera.yaw << " " << camera.pitch << std::endl;
}
