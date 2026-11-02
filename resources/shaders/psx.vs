#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// Custom uniforms for PSX effect
uniform vec2 snapResolution = vec2(320.0, 240.0); // Classic PSX resolutions

// Output vertex attributes (passed to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // 1. Calculate standard clip space position
    vec4 clipPos = mvp * vec4(vertexPosition, 1.0);
    
    // 2. Vertex Snapping (Jitter effect)
    // Convert clip space to screen space, round to grid, convert back
    vec3 ndcSpace = clipPos.xyz / clipPos.w;
    ndcSpace.xy = floor(ndcSpace.xy * snapResolution) / snapResolution;
    
    // Reconstruct snapped clip position
    vec4 snappedPos = vec4(ndcSpace * clipPos.w, clipPos.w);

    // 3. Affine Texture Warping preparation
    // Multiply by W here so the fragment shader can reverse it and break perspective
    fragTexCoord = vertexTexCoord * snappedPos.w;
    
    // Store snappedPos.w inside the alpha channel to pass depth to fragment shader
    fragColor = vec4(vertexColor.rgb, snappedPos.w);
    
    gl_Position = snappedPos;
}

