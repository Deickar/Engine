#pragma once
#include <math.h>
#include "customStructs.h"
#include "SimplexNoise.h"
#include <stdlib.h> //srand
#include <time.h>
#include <vector>

using std::vector;

class MCTerrainClass
{
private:

	//Terrain modes

	int sizeX;
	int sizeY;
	int sizeZ;
	int worldSize;
	int worldSizeMargin;
	int x,y,z, idx;
	int worldArraySize;
	int *marchingCubesFluxArray;
	int terrainMode;
	float densityToBeInside;
	float maxDensity, minDensity, densityRangeUpper,densityRangeLower;

	bool pulvirize;


	MarchingCubeVertex *marchingCubeVertices;
	SimplexNoise *noise;

	vector<vector<vector<float>>> densityArray3D;
	float* densityArray1D;


public:
	MCTerrainClass(void);

	void Initialize
		(
		int sizeX,
		int sizeY,
		int sizeZ,
		MarchingCubeVertex *marchingCubeVertices
		);

	~MCTerrainClass(void);


	void LSystem();


	void LSystemTree();

	void MCHeightMap();

	void Noise3D();

	MarchingCubeVertex* &GetMarchingCubeVertices()
	{
		return marchingCubeVertices;
	}

	float GetDensityRangeUpper()
	{
		return densityRangeUpper;
	}

	float GetDensityRangeLower()
	{
		return densityRangeLower;
	}

	float* &getDensetyArray1D()
	{
		return densityArray1D;
	}
	
	void SetTerrainType(int terrainMode)
	{
		this->terrainMode = terrainMode;
	}

	void PulvirizeWorldToggle()
	{
		if (pulvirize)
		pulvirize = false;
		else
		pulvirize = true;
	}

	bool GetPulverizeWorld()
	{
		return pulvirize;
	}

	void SetPulverizeWorld(bool pulverize)
	{
		this->pulvirize = pulverize;
	}

	int getTerrainType()
	{
		return terrainMode;
	}

	vector<vector<vector<float>>> const &getDensityArray3D() const
	{
		return densityArray3D;
	}



	float GetHighestPositionOfCoordinate(int x, int z);

	float GetHighestPositionOfCoordinateBruteforce(float x, float z);

private:
	void CreateMCVerts();



};