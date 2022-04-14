#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
  mat4 transform; // projection * view * model
  //mat4 modelMatrix;
  mat4 normalMatrix; // mat4 instead of mat3 because of alignment
} push;

void main()
{
  outColor = vec4(fragColor /*push.color*/, 1.0);

  //outColor = vec4( vec3(1.f, 1.f, 1.f) /*fragColor*/, 1.0);
  //outColor = vec4(fragColor, 1.0);
}
