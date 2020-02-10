#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;


uniform mat4 view_matrix;

out vec2 TexGeoCoord_CS_in;
out vec3 WorldPos_CS_in;


void main()
{
  //gl_Position = view_matrix * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
  gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
  WorldPos_CS_in = vPosition;

  TexGeoCoord_CS_in = texPosition;
}

