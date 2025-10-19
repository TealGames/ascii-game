#version 430 core

//The local size is the work group size -> how many threads there are per group 
//(the number of groups is determined with compute shader call)
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Bindings: input and output images
uniform sampler2D uTextureInput;
layout (rgba16f, binding = 1) uniform writeonly image2D uTextureOutput;

uniform bool uIsHorizontal;
uniform float uWeights[5];

void main() 
{
    //Since we force input and output texture to be the same size
    ivec2 texSize = imageSize(uTextureOutput);
    //Invocation is the thread (x, y, z group) and since we dispatch 
    // one thread per pixel -> thread id is the same as pixel coord
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    // Guard: don’t go out of bounds
    if (pixel.x >= texSize.x || pixel.y >= texSize.y)
        return;

    vec4 color = texelFetch(uTextureInput, pixel, 0);

    if (uIsHorizontal) 
    {
        for (int i = 1; i < 5; i++) 
        {
            //We clamp to ensure when we invoke on edge pixels it does not cause problems
            ivec2 right = clamp(pixel + ivec2(i, 0), ivec2(0), texSize - 1);
            ivec2 left  = clamp(pixel - ivec2(i, 0), ivec2(0), texSize - 1);
            color += texelFetch(uTextureInput, right, 0).rgb * uWeights[i];
            color += texelFetch(uTextureInput, left, 0).rgb * uWeights[i];
        }
    } 
    else 
    {
        for (int i = 1; i < 5; i++) 
        {
            ivec2 down = clamp(pixel + ivec2(0, i), ivec2(0), texSize - 1);
            ivec2 up   = clamp(pixel - ivec2(0, i), ivec2(0), texSize - 1);
            color += texelFetch(uTextureInput, down, 0).rgb * uWeights[i];
            color += texelFetch(uTextureInput, up, 0).rgb * uWeights[i];
        }
    }

    imageStore(uTextureOutput, pixel, color);
}