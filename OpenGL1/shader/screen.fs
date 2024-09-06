#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float _Time;
uniform sampler2D screenTexture;

void main()
{ 
    float offset = sin(_Time *4) * 0.02;
    vec2 uv0 = vec2(TexCoords.x - offset, TexCoords.y);
    vec2 uv1 = vec2(TexCoords.x + offset, TexCoords.y);
    FragColor.r = texture(screenTexture, uv0).r;
    FragColor.g = texture(screenTexture, TexCoords).g;
    FragColor.b = texture(screenTexture, uv1).b;
    FragColor = vec4(FragColor.rgb,1);
}