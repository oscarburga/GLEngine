#version 460 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
	FragColor = vec4(objectColor * lightColor, 1.0f);
	// FragColor = vec4(gl_FragCoord.xy, 0.0f, 1.0f);
}
