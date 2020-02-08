#version 410 core

layout(triangles, equal_spacing, ccw) in;

in vec2 TexGeoCoord_ES_in[];
//in vec3 HouseCoord_ES_in[];



out vec2 TexGeoCoord_FS_in;
out vec4 HousePosition_FS_in;
//out vec4 HouseCoord_FS_in[100];



vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

void main(){
	TexGeoCoord_FS_in = interpolate2D(TexGeoCoord_ES_in[0], TexGeoCoord_ES_in[1], TexGeoCoord_ES_in[2]);

	/*
	vec4 oPosY = gl_TessCoord.y*gl_in[1].gl_Position;
	vec4 posY = sin(oPosY)*2;

	vec4 oPosZ = gl_TessCoord.z*gl_in[2].gl_Position;
	vec4 posZ = oPosZ;
	*/
	//gl_Position=(gl_TessCoord.x*gl_in[0].gl_Position+posY+posZ);
	gl_Position=(gl_TessCoord.x*gl_in[0].gl_Position+gl_TessCoord.y*gl_in[1].gl_Position+gl_TessCoord.z*gl_in[2].gl_Position);
	HousePosition_FS_in = gl_Position;
}

