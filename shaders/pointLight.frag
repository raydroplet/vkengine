#version 450

layout(location = 0) in vec2 inFragOffset;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
  vec4 cameraPosition;
} ubo;

void main()
{
  // Discards any pixels outside the point light radius to create a circle.
  // Based on how we set up our offsets, the inFragOffset x and y will always be in the -1 to 1 range, no matter how large the billboard is made.
  float distance = sqrt(dot(inFragOffset, inFragOffset)); // sqrt(magnitude^2)
  if(distance >= 1.0)
  {
    // Special keyword for fragment shaders.
    // It tells the pipeline to throw away this fragment and return.
    discard;
  }

  outColor = vec4(ubo.lightColor.xyz, 1.0);
}
