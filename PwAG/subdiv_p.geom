#version 430 core
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

layout( triangles ) in;

//wyjsciowym prymitywem powinny byc punkty!!!
//layout( points, max_vertices=200 ) out;
layout(triangle_strip, max_vertices = 150) out;


vec4 CG;

in vec2 TexGeoCoord[];
out vec2 TexCoord;

void main() {
	
	CG = ( gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position ) / 3.; //srodek

	vec4 v0 = gl_in[0].gl_Position;
	vec4 v1 = gl_in[1].gl_Position;
	vec4 v2 = gl_in[2].gl_Position;

	float division = pow(2, 3);

	float step = 1/division;

	for(float s =0; s<=1; s=s+step){
		for(float t =0; t<=1-s; t=t+step){
			vec4 pos = v0 + s*(v1-v0) + t*(v2-v0); 
			gl_Position = vec4(sin(pos.x), cos(pos.x)*2 + sin(pos.z * 2)-2, cos(pos.z), 1.0);
			TexCoord = vec2(s, t);
			EmitVertex();
		}
	}

	EndPrimitive();
}