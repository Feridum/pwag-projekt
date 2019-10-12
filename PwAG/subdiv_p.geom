#version 430 core
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

layout( triangles ) in;

//wyjsciowym prymitywem powinny byc punkty!!!
//layout( points, max_vertices=200 ) out;
layout(triangle_strip, max_vertices = 6) out;

uniform float time;
uniform int level;

vec4 CG;



void main() {
	
	CG = ( gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position ) / 3.; //srodek

	for(int i = 0; i < gl_in.length(); i++) {
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}