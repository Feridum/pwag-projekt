#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;
layout(location = 7) uniform mat4 view_matrix;

out vec2 TexGeoCoord_CS_in;

void main()
{
  gl_Position = view_matrix * vec4(vPosition.x, vPosition.z, vPosition.y, 1.0);

  TexGeoCoord_CS_in = texPosition;
}

