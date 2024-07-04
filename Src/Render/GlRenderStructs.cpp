#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"

void SMeshNode::Draw(const glm::mat4& topMatrix)
{
	SNode::Draw(topMatrix);
}
