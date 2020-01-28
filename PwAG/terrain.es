#version 410 core

layout(triangles, equal_spacing, ccw) in;

in vec2 TexGeoCoord_ES_in[];
in vec3 WorldPos_ES_in[];

out vec2 TexGeoCoord_FS_in;
out vec3 WorldPos_FS_in;

uniform mat4 view_matrix;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec4 v0, vec4 v1, vec4 v2)
{
    return vec3(gl_TessCoord.x) * vec3(v0) + vec3(gl_TessCoord.y) * vec3(v1) + vec3(gl_TessCoord.z) * vec3(v2);
}

void main(){
	TexGeoCoord_FS_in = interpolate2D(TexGeoCoord_ES_in[0], TexGeoCoord_ES_in[1], TexGeoCoord_ES_in[2]);
	//WorldPos_FS_in = interpolate3D(WorldPos_ES_in[0], WorldPos_ES_in[1], WorldPos_ES_in[2]);
	WorldPos_FS_in = interpolate3D(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);

	vec4 oPosY = gl_TessCoord.y*gl_in[1].gl_Position;
	vec4 posY = sin(oPosY)*2;

	vec4 oPosZ = gl_TessCoord.z*gl_in[2].gl_Position;
	vec4 posZ = oPosZ;

	//gl_Position = WorldPos_FS_in;

	double wX = WorldPos_FS_in.x;
	//double wY = sin((WorldPos_FS_in.x + WorldPos_FS_in.z)*2)*0.3;
	double wY = WorldPos_FS_in.y;
	double wZ = WorldPos_FS_in.z;
	gl_Position = vec4(wX, wY, wZ , 1.0) * view_matrix;
}

