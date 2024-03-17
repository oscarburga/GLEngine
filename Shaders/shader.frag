#version 330 core

out vec4 FragColor;

in vec4 vertexColor;
in vec2 texCoord;

uniform float time;
uniform sampler2D woodTexture;
uniform sampler2D faceTexture;

void main()
{
    float timeSineRemapped = (sin(time) + 1.0) * 0.5;
    float invTimeSine = 1.0 - timeSineRemapped;
    vec4 woodColor = texture(woodTexture, texCoord);
    vec4 faceColor = texture(faceTexture, texCoord);
    vec4 texColor = mix(woodColor, faceColor, 0.5);
    FragColor = texColor;
    // FragColor = (texColor * invTimeSine) + (vertexColor * timeSineRemapped);
    // FragColor.a = 1;
}
