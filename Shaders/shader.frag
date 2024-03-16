#version 330 core

out vec4 FragColor;

in vec4 vertexColor;
in vec2 texCoord;

uniform float time;
uniform sampler2D texSampler;

void main()
{
    float timeSineRemapped = (sin(time) + 1.0) * 0.5;
    float invTimeSine = 1.0 - timeSineRemapped;
    vec4 texColor = texture(texSampler, texCoord);
    FragColor = (texColor * invTimeSine) + (vertexColor * timeSineRemapped);
    // FragColor = texColor;
}
