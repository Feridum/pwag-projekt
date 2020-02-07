#include <iostream>
#include <random>
#include <vector>

#include "glm/glm.hpp"

#include "Heightmap.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
using glm::vec2;
using glm::vec3;

vector<vector<float>> generateRandomHeightData(const HillAlgorithmParameters& params)
{
	vector<vector<float>> heightData(params.rows, vector<float>(params.columns, 0.0f));

	std::random_device rd;
	std::mt19937 generator(rd());
	uniform_int_distribution<int> hillRadiusDistribution(params.hillRadiusMin, params.hillRadiusMax);
	uniform_real_distribution<float> hillHeightDistribution(params.hillMinHeight, params.hillMaxHeight);
	uniform_int_distribution<int> hillCenterRowIntDistribution(0, params.rows - 1);
	uniform_int_distribution<int> hillCenterColIntDistribution(0, params.columns - 1);

	for (int i = 0; i < params.numHills; i++) {
		const auto hillCenterRow = hillCenterRowIntDistribution(generator);
		const auto hillCenterCol = hillCenterColIntDistribution(generator);
		const auto hillRadius = hillRadiusDistribution(generator);
		const auto hillHeight = hillHeightDistribution(generator);

		for (auto r = hillCenterRow - hillRadius; r < hillCenterRow + hillRadius; r++) {
			for (auto c = hillCenterCol - hillRadius; c < hillCenterCol + hillRadius; c++) {
				if (r < 0 || r >= params.rows || c < 0 || c >= params.columns) {
					continue;
				}
				const auto r2 = hillRadius * hillRadius; // r*r ter
				const auto x2x1 = hillCenterCol - c; // (x2-x1) ter
				const auto y2y1 = hillCenterRow - r; // (y2-y1) ter
				const auto height = float(r2 - x2x1 * x2x1 - y2y1 * y2y1);

				const auto factor = height / r2;
				heightData[r][c] += hillHeight * factor;
				if (heightData[r][c] > 1.0f) {
					heightData[r][c] = 1.0f;
				}
			}
		}
	}
	return heightData;
}

void display(vector<vector<float>> data)
{
	int i, j, n, m;
	for(i = 0, n = data.size(); i < n; ++i) {
		for(j = 0, m = data[i].size(); j < m; ++j) {
			cout << data[i][j] << ' ';
		}
		cout << '\n';
	}
}

#ifdef  __HEIGHTMAP_TEST__
int main()
{
	Heightmap::HillAlgorithmParameters hil = Heightmap::HillAlgorithmParameters(10,10,4,2,4,0.5,1);
	display(Heightmap::generateRandomHeightData(hil));
}
#endif//__HEIGHTMAP_TEST__

