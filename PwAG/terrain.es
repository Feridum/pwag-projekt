#version 410 core

layout(triangles, equal_spacing, ccw) in;

in vec2 TexGeoCoord_ES_in[];
in vec3 WorldPos_ES_in[];

out vec2 TexGeoCoord_FS_in;
out float Heigth_FS_in;
out vec3 WorldPos_FS_in;

uniform mat4 view_matrix;

uniform vec2 hill_centers[5];
uniform float hill_height;
uniform float hill_radius;

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

double calculateHeight(double x, double z)
{
	double maxHeight = 0;

	for(int i = 0; i < 5; i++){
		double r2 = hill_radius * hill_radius; 
		double x2x1 = hill_centers[i].x - x;
		double y2y1 = hill_centers[i].y - z;
		double height = double(r2 - x2x1 * x2x1 - y2y1 * y2y1);

		double factor = height / r2;
		height = hill_height * factor;

		maxHeight = max(maxHeight, height);
	}

	return maxHeight;
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

	//WorldPos_FS_in += texture(heightMap, TexGeoCoord_FS_in.xy).x;

	double wX = WorldPos_FS_in.x;
	//double wY = sin((WorldPos_FS_in.x + WorldPos_FS_in.z)*8)*0.1;
	//double wY = WorldPos_FS_in.y;
	double wZ = WorldPos_FS_in.z;
	double wY = calculateHeight(wX, wZ);
	//double wY = WorldPos_FS_in.y;
	Heigth_FS_in = float(wY);
	gl_Position = view_matrix * vec4(wX, wY, wZ + wY/2 , 1.0);
}

