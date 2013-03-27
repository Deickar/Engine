#include "MCTerrainClass.h"


MCTerrainClass::MCTerrainClass(void)
{
}

MCTerrainClass::MCTerrainClass
	(
	int sizeX,
	int sizeY,
	int sizeZ,
	MarchingCubeVertex *marchingCubeVertices

	)
{

	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;
	marchingCubeVertices2 = marchingCubeVertices;


	worldSize = 0;
	worldSizeMargin = 16;
	worldArraySize = sizeX * sizeY;
	noise = new SimplexNoise();
}

MCTerrainClass::~MCTerrainClass(void)
{
	// //	delete marchingCubeVertices2; 
	// //	marchingCubeVertices2 = 0;

	//	noise->~SimplexNoise();
	//	delete noise;
}

void MCTerrainClass::MCHeightMap()
{
	//int worldArray[worldArraySize];


	float* worldArray = new float[worldArraySize];

	for(int i = 0; i < worldArraySize; i++)
	{
		float r = noise->noise2D(this->marchingCubeVertices2[i].posX,marchingCubeVertices2[i].posZ);
		worldArray[i] = r;
	}

	int i = -2;

	for (z = worldSizeMargin; z < (this->sizeZ - worldSizeMargin); z++)
	{
		i++;
		for (y = worldSizeMargin; y < (this->sizeY - worldSizeMargin); y++)
		{
			for (x = worldSizeMargin; x < (this->sizeX - worldSizeMargin); x++)
			{
				i++;

				if (worldArray[i] > (float)y)
				{
				}
				idx = x + y*this->sizeY + z * this->sizeY * this->sizeZ;



				this->marchingCubeVertices2[idx].flux = 100.0f;



				if (this->marchingCubeVertices2[idx].flux > 0.01f)
				{
					this->marchingCubeVertices2[idx].inside = true;
				}
				else
				{
					this->marchingCubeVertices2[idx].inside = false;
				}


				this->marchingCubeVertices2[idx].normalX = this->marchingCubeVertices2[idx - 1].flux - this->marchingCubeVertices2[idx+1].flux;
				this->marchingCubeVertices2[idx].normalY = this->marchingCubeVertices2[idx - this->sizeY].flux - this->marchingCubeVertices2[idx + this->sizeY].flux;
				this->marchingCubeVertices2[idx].normalZ = this->marchingCubeVertices2[idx - (this->sizeY * this->sizeZ)].flux - this->marchingCubeVertices2[idx + (this->sizeY * this->sizeZ)].flux;

			}
		}
	}

	if (worldArray != 0)
	{
		delete worldArray;
	}

}


void MCTerrainClass::Noise3D()
{
	float max, min;
	max = 0;
	min = 1;
	for (z = worldSizeMargin; z < (this->sizeZ - worldSizeMargin); z++)
	{

		for (z = 16; z < (this->sizeZ - 16); z++)
		{
			for (y = 16; y < (this->sizeY - 16); y++)
			{
				for (x = 16; x < (this->sizeX - 16); x++)
				{

					idx = x + y*this->sizeY + z * this->sizeY * this->sizeZ;


					//float r = (float)rand()/(float)RAND_MAX;
					float r = noise->noise3D2(this->marchingCubeVertices2[idx].posX,marchingCubeVertices2[idx].posY,marchingCubeVertices2[idx].posZ + 0.005f);
					if(max < r){max = r;}
					if(min > r){min = r;}
					

					this->marchingCubeVertices2[idx].flux = r * 100.0f;



					if (this->marchingCubeVertices2[idx].flux > 0.01f)
					{
						this->marchingCubeVertices2[idx].inside = true;
					}
					else
					{
						this->marchingCubeVertices2[idx].inside = false;
					}


					this->marchingCubeVertices2[idx].normalX = this->marchingCubeVertices2[idx - 1].flux - this->marchingCubeVertices2[idx+1].flux;
					this->marchingCubeVertices2[idx].normalY = this->marchingCubeVertices2[idx - this->sizeY].flux - this->marchingCubeVertices2[idx + this->sizeY].flux;
					this->marchingCubeVertices2[idx].normalZ = this->marchingCubeVertices2[idx - (this->sizeY * this->sizeZ)].flux - this->marchingCubeVertices2[idx + (this->sizeY * this->sizeZ)].flux;
				}
			}
		}
	}
	max = max;
	min = min;
}