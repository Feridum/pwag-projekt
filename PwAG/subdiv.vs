#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;

uniform int houseNum;
//uniform vec3 houseFirstPart[2];
//uniform vec4 houseSecoundPart[2];
//uniform float params[3];
//uniform vec4 t[2];

out vec2 TexGeoCoord_CS_in;
out vec3 WorldPos_CS_in;


void main()
{
  gl_Position = vec4(vPosition.x, vPosition.z, vPosition.y, 1.0);

  TexGeoCoord_CS_in = texPosition;

}


