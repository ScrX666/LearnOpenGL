#version 420 core

// your first render target which is used to accumulate pre-multiplied color values
layout (location = 0) out vec4 accum;

// your second render target which is used to store pixel revealage
layout (location = 1) out float reveal;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{          
    vec4 color = vec4(1,1,1,1);
    color = texture(texture1, TexCoords);
    // -------------O*I*T-----------------
    // insert your favorite weighting function here. the color-based factor
    // avoids color pollution from the edges of wispy clouds. the z-based
    // factor gives precedence to nearer surfaces
    // weight function
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * 
                         pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    // store pixel color accumulation
    accum = vec4(color.rgb * color.a, color.a) * weight;

    // store pixel revealage threshold
    reveal = color.a;
}