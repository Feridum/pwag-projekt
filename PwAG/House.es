#version 410 core

//layout(triangles, equal_spacing, ccw) in;
layout(quads, equal_spacing, ccw) in;

in vec2 TexGeoCoord_ES_in[];
in vec3 WorldPos_ES_in[];
//in float houseX_ES_in[];
//in float houseY_ES_in[];
//in vec4 posA_ES_in[];

//uniform float wallH;


out vec2 TexGeoCoord_FS_in;
out vec4 HousePosition_FS_in;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec4 v0, vec4 v1, vec4 v2)
{
    return vec3(gl_TessCoord.x) * vec3(v0) + vec3(gl_TessCoord.y) * vec3(v1) + vec3(gl_TessCoord.z) * vec3(v2);
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

/*
3	7	11	15
2	6	10	14
1	5	9	13
0	4	8	12
*/
int checkPatch(float u, float v)
{
	int x = 0;
	int y = 0;

	if(u < 0.25)
	{
		x = 0;
	}
	else if (u < 0.5)
	{
		x = 1;
	}
	else if (u < 0.75)
	{
		x = 2;
	}
	else
	{
		x = 3;
	}

	if(v < 0.25)
	{
		y = 0;
	}
	else if (v < 0.5)
	{
		y = 1;
	}
	else if (v < 0.75)
	{
		y = 2;
	}
	else
	{
		y = 3;
	}

	return (x * 4 + y);
}

void main(){

	
	TexGeoCoord_FS_in = interpolate2D(TexGeoCoord_ES_in[0], TexGeoCoord_ES_in[1], TexGeoCoord_ES_in[2]);

	// Interpolate along bottom edge using x component of the tessellation coordinate
//    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    // Interpolate along top edge using x component of the tessellation coordinate
//    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    // Now interpolate those two results using the y component of tessellation coordinate
//    vec4 pos = mix(p1, p2, gl_TessCoord.y);
//	HousePosition_FS_in = pos;
//	gl_Position = pos;

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	int our_patch;
	/*
	1	3
	0	2
	*/
	int v0, v1, v2, v3;
	our_patch = checkPatch(u, v);
	//our_patch = gl_PrimitiveID;
	
	if (our_patch < 4)
	{
		v0 = our_patch;
		v1 = 1 + our_patch;
		v2 = 5 + our_patch;
		v3 = 6 + our_patch;
	}
	else if (our_patch < 8)
	{
		v0 = 1 + our_patch;
		v1 = 2 + our_patch;
		v2 = 6 + our_patch;
		v3 = 7 + our_patch;
	}
	else if (our_patch < 12)
	{
		v0 = 2 + our_patch;
		v1 = 3 + our_patch;
		v2 = 7 + our_patch;
		v3 = 8 + our_patch;
	}
	else
	{
		v0 = 3 + our_patch;
		v1 = 4 + our_patch;
		v2 = 8 + our_patch;
		v3 = 9 + our_patch;
	}

	vec4 p1 = mix(gl_in[v0].gl_Position, gl_in[v2].gl_Position, u);
	vec4 p2 = mix(gl_in[v1].gl_Position, gl_in[v3].gl_Position, u);
	vec4 pos = mix(p1, p2, v);
	HousePosition_FS_in = pos;
	gl_Position = pos;

}






/*	ŒMIETNIK

bool sameVertex(int vNum)
{
	float X = gl_TessCoord.x;
	float Y = gl_TessCoord.y;
	float Z = gl_TessCoord.z;
	if (gl_in[vNum].gl_Position.x == X)
	{
		if (gl_in[vNum].gl_Position.y == Y)
		{
			if (gl_in[vNum].gl_Position.z == Z)
			{
				return true;
			}
		}
	}
	return false;
}

int checkVertex()
{
	int vNum = -1;
	for (int i = 0; i < 25; i++)
	{
	 	if ( sameVertex(i) == true)
		{
			vNum = i;
			break;
		}
	}
	return vNum;
}







//Najpierw musimy sprawdziæ, z którym wierzcho³kiem mamy doczynienia:

	int vNum = checkVertex();

	
	4	9	14	19	24
	3	8	13	18	23
	2	7	12	17	22
	1	6	11	16	21
	0	5	10	15	20

	Pos³uguj¹c siê terminologi¹ (punktologi¹?) z main'a mo¿emy przypisaæ wierzcho³ki do nastêpuj¹cych punktów: 
	B - 0,		Bp - 24
	C - 20,		Cp - 4
	D - 6,		Dp - 18
	E - 16,		Ep - 8
	UWAGA! ZMIANA KONCEPCJI! DOMEK BÊDIZE MIA£ JEDEN, CENTRALNY PUNKT DACHU F - 12, ZAMIAST		F - 11,		Fp - 13

	Pozosata³e 15 wierzcho³ków nale¿y ustawiæ odpowiednio pomiêdzy owymi punktami:
	1,2,3:		Pomiêdzy B i Cp, ka¿dy oddalony o 1/4 odleg³oœci pomiêdzy tymi punktami
	5,10,15:	Pomiêdzy B i C (jak wy¿ej)
	9,14,19:	Pomiêdzy Cp i Bp (jak wy¿ej)
	21,22,23:	Pomiêdzy C i Bp (jak wy¿ej)
	7:			W po³owie odleg³oœci pomiêdzy D i Ep
	11:			W po³owie odleg³oœci pomiêdzy D i E
	13:			W po³owie odleg³oœci pomiêdzy Dp i Ep
	16:			W po³owie odleg³oœci pomiêdzy E i Dp
	12:			<~!W po³owie odleg³oœci pomiêdzy F i Fp~>	Punkt 12 jest teraz centrum dachu - po³o¿ony jest nad punktem A o pó³tora wysokoœci œciany 
	
	vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);
	switch(vNum)
	{
		case 0:								//punkt B
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 1:	// 1/4 dystansu miêdzy B i Cp
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y + (0.5 * houseY);
			pos.z = posA_ES_in[0].z;
			break;

		case 2:	// 2/4 dystansu miêdzy B i Cp
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y;
			pos.z = posA_ES_in[0].z;
			break;

		case 3:	// 3/4 dystansu miêdzy B i Cp
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y - (0.5 * houseY);
			pos.z = posA_ES_in[0].z;
			break;
				
		case 4:								//punkt Cp
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z;						
			break;

		case 5: // 1/4 dystansu miêdzy B i C
			pos.x = posA_ES_in[0].x + (0.5 * houseX);
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 6:								//punkt D
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 7:	// 1/2 dystansu miêdzy D i Ep
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 8:								//punkt Ep
			pos.x = posA_ES_in[0].x + houseX;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 9: // 1/4 dystansu miêdzy Cp i Bp
			pos.x = posA_ES_in[0].x + (0.5 * houseX);
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 10: // 2/4 dystansu miêdzy B i C
			pos.x = posA_ES_in[0].x;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 11: // 1/2 dystansu miêdzy D i E
			pos.x = posA_ES_in[0].x;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 12:							//punkt F
			pos.x = posA_ES_in[0].x;
			pos.y = posA_ES_in[0].y;
			pos.z = posA_ES_in[0].z + (1.5*wallH);
			break;

		case 13: // 1/2 dystansu miêdzy Ep i Dp
			pos.x = posA_ES_in[0].x;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 14: // 2/4 dystansu miêdzy Cp i Bp
			pos.x = posA_ES_in[0].x;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 15: // 3/4 dystansu miêdzy B i C
			pos.x = posA_ES_in[0].x - (0.5 * houseX);
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 16:							//punkt E
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 17: // 1/2 dystansu miêdzy E i Dp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 18:							//punkt Dp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z + wallH;
			break;

		case 19: // 3/4 dystansu miêdzy Cp i Bp	
			pos.x = posA_ES_in[0].x - (0.5 * houseX);
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 20:							//punkt C	
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y + houseY;
			pos.z = posA_ES_in[0].z;
			break;

		case 21: // 1/4 dystansu miêdzy C i Bp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y + (0.5 * houseY);
			pos.z = posA_ES_in[0].z;
			break;

		case 22: // 2/4 dystansu miêdzy C i Bp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y;
			pos.z = posA_ES_in[0].z;
			break;

		case 23: // 3/4 dystansu miêdzy C i Bp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y - (0.5 * houseY);
			pos.z = posA_ES_in[0].z;
			break;

		case 24:							//punkt Bp
			pos.x = posA_ES_in[0].x - houseX;
			pos.y = posA_ES_in[0].y - houseY;
			pos.z = posA_ES_in[0].z;
			break;

		default:
			//error! Nie znaleziono punktu na p³acie
			break;
	}


*/