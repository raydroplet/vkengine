#version 450

const vec2 offsets[6] = {
  vec2(-1.0, -1.0),
  vec2(-1.0,  1.0),
  vec2( 1.0, -1.0),

  vec2( 1.0, -1.0),
  vec2(-1.0,  1.0),
  vec2( 1.0,  1.0)
};

layout(location = 0) out vec2 outFragOffset;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
  vec4 cameraPosition;
} ubo;

const float LIGHT_RADIUS = 0.075;

void secondMethod()
{
  outFragOffset = offsets[gl_VertexIndex];

  // Transforms the light position to camera space, then apply the offset.
  // It is doing the same as the first method, but with xyzw instead of xy alone.
  vec4 cameraSpaceLightPos = ubo.view * vec4(ubo.lightPosition.xyz, 1.0);
  vec4 cameraSpaceVertPos = cameraSpaceLightPos + LIGHT_RADIUS * vec4(outFragOffset, 0.0, 0.0);
  gl_Position = ubo.projection * cameraSpaceVertPos, 1.0;
}

void firstMethod()
{
  // Extract camera up and right vectors (in world space). Used to calculate the vertex position in world space.

  // the first 3 elements of the first row make the camera right vector, and
  // the first 3 elements of the second row make the camera up vector.

  // To understand the calculation, simply remember how a matrix-vector multiplication takes each vector as a scalar to each column and adds them (It is doing that in the worldPos).

  outFragOffset = offsets[gl_VertexIndex];

  vec3 cameraWorldRightDir = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 cameraWorldUpDir = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 worldPos = ubo.lightPosition.xyz
    + LIGHT_RADIUS * outFragOffset.x * cameraWorldRightDir
    + LIGHT_RADIUS * outFragOffset.y * cameraWorldUpDir;

  // outFragOffset.xy (that is in the -1 to 1 range) shrinks the cameraWorldDir.xy, while LIGHT_RADIUS scales it.

  gl_Position = ubo.projection * ubo.view * vec4(worldPos, 1.0);
}

void main()
{
  secondMethod();
}
