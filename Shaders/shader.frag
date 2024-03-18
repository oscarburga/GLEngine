#version 330 core

out vec4 FragColor;

in vec4 vertexColor;
in vec2 texCoord;

uniform float time;
uniform sampler2D woodTexture;
uniform sampler2D faceTexture;

void main()
{
    vec2 doubleTexCoord = texCoord * 2.0;
    float timeSineRemapped = (sin(time) + 1.0) * 0.5;
    float invTimeSine = 1.0 - timeSineRemapped;
    vec4 woodColor = texture(woodTexture, doubleTexCoord);

    vec2 doubleTexCoordFract = doubleTexCoord - ivec2(doubleTexCoord.xy);
    vec2 xFlippedTexCoord = vec2(1.0 - texCoord.x, texCoord.y);
    // vec4 faceColor = texture(faceTexture, xFlippedTexCoord);
    vec4 faceColor = texture(faceTexture, doubleTexCoordFract);
    vec4 texColor = mix(woodColor, faceColor, 0.2);
    FragColor = texColor;
    // FragColor = (texColor * invTimeSine) + (vertexColor * timeSineRemapped);
    FragColor.a = 1;
}
