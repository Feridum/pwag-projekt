#ifndef __HEIGHTMAP__HPP__
#define __HEIGHTMAP__HPP__

#include <vector>

#include "glm/glm.hpp"


using std::vector;
using glm::vec2;
using glm::vec3;
/*
class Heightmap
{
public:*/
	struct HillAlgorithmParameters
	{
		int rows;
		int columns;
		int numHills;
		int hillRadiusMin;
		int hillRadiusMax;
		float hillMinHeight;
		float hillMaxHeight;

		HillAlgorithmParameters(
				int rows,
				int columns,
				int numHills,
				int hillRadiusMin,
				int hillRadiusMax,
				float hillMinHeight,
				float hillMaxHeight)
		{
			this->rows = rows;
			this->columns = columns;
			this->numHills = numHills;
			this->hillRadiusMin = hillRadiusMin;
			this->hillRadiusMax = hillRadiusMax;
			this->hillMinHeight = hillMinHeight;
			this->hillMaxHeight = hillMaxHeight;
		}
	};
	vector<vector<float>> generateRandomHeightData(const HillAlgorithmParameters& params);

/*
	Heightmap(
			const HillAlgorithmParameters& params,
			bool withPositions = true,
			bool withTextureCoordinates = true,
			bool withNormals = true);

	void createFromHeightData(const vector<vector<float>>& heightData);

	int getRows() const;
	int getColumns() const;
	float getHeight(const int row, const int column) const;


private:
	void setUpVertices();
	void setUpTextureCoordinates();
	void setUpNormals();
	void setUpIndexBuffer();

	vector<vector<float>> _heightData;
	vector<vector<vec3>> _vertices;
	vector<vector<vec2>> _textureCoordinates;
	vector<vector<vec3>> _normals;
	int _rows = 0;
	int _columns = 0;
};
*/
void display(vector<vector<float>> data);

#endif//__HEIGHTMAP__HPP__

