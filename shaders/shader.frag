#version 450

layout(push_constant) uniform Push {
  mat4 transform;
//  vec2 offset;
  vec3 color;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(fragColor /*push.color*/, 1.0);

  //outColor = vec4( vec3(1.f, 1.f, 1.f) /*fragColor*/, 1.0);
  //outColor = vec4(fragColor, 1.0);
}
