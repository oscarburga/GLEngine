#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	{
		int numAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
		std::cout << "Max number of vertex attributes: " << numAttributes << " 4-component vertex attributes\n";
	}

	float vertices[] = {
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // top left 
	};

	GLuint indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};  

	// Vertex array objects store all the calls to glEnableVertexAttribArray or glDisableVertexAttribArray, glVertexAttribPointer, and also stores the buffer objects associated.
	// Use them to not have to re-specify these attributes every time you want to use a certain buffer with a layout. 
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	GLuint VBO;
	glGenBuffers(1, &VBO); // Create a buffer and get a handle to it
	GLuint EBO; // Element buffer object
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// Start "recording" all the buffer bindings and attributes for this vertex array object.
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // OpenGL state machine has a slot to use an Array Buffer. Set it to use our Array Buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy our vertex data into the currently bound array buffer (our buffer).
	// glBufferData requires specifying buffer usage: static draw sets the data only once and uses it many times.

	// Specify wthe layout of each vertex data in the array for opengl to interpret before rendering.
	// In this case, we specify that the first attribute (attrib at location 0) has 3 components (must be 1-4), 
	// we don't want to auto-clamp it, it takes the size of 3 floats, and there's no offset on the first component of the first attribute in the array.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr); 
	glEnableVertexAttribArray(0); // remember to enable the attrib lol
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Bind the element buffer object to draw the vertices by index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Unbind the VAO, VBO and EBO when done setting them up.
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	Shader shader("Shaders/shader.vert", "Shaders/shader.frag");
	shader.use();
	glBindVertexArray(VAO);
	// glPolygonMode(GL_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		const float time = (float)glfwGetTime();
		shader.SetUniform("time", time);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
