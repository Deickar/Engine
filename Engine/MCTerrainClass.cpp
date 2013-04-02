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
	static const int a = sizeX;

	marchingCubesFluxArray = new int[sizeX*sizeY*sizeZ];
	

	//static const float[a][sizeY][sizeZ] katt;

	worldSize = 0;
	worldSizeMargin = 1;
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



				if (this->marchingCubeVertices2[idx].flux > 0.1f)
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
			for (y = worldSizeMargin; y < (this->sizeY - worldSizeMargin); y++)
		{
				for (x = worldSizeMargin; x < (this->sizeX - worldSizeMargin); x++)
			{
				idx = x + y*this->sizeY + z * this->sizeY * this->sizeZ;

					float r =  + sizeY * 0.8f - y;
					r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *4.0f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *2.0f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/30,marchingCubeVertices2[idx].posZ/30) + 1.0f) *2.0f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/120,marchingCubeVertices2[idx].posY/20,marchingCubeVertices2[idx].posZ/120) + 1.0f) *5.0f;
					r -= (noise->noise3D2(this->marchingCubeVertices2[idx].posX/320,marchingCubeVertices2[idx].posY/20,marchingCubeVertices2[idx].posZ/320) + 1.0f) *10.0f;
					/**shelves
					
					////***
					//***terraces
					r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *2.0f;
					r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *5.0f;
					r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/130,marchingCubeVertices2[idx].posZ/30) + 1.0f) *5.0f;
					r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/230,marchingCubeVertices2[idx].posZ/30) + 1.0f) *5.0f;
					r -= (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/30,marchingCubeVertices2[idx].posZ/300) + 1.0f) *10.0f;
					//***

					//Hardfloor
					if(y < 5)
					{
						r += 40.0f;
					}

					//Toplvl
					if(y > 10)
					{
						r -= 7.0f;
					}
					if(y > 15)
					{
						r -= 7.0f;
					}
					if(y > 20)
					{
						r -= 7.0f;
					}
					if(y > 25)
					{
						r -= 7.0f;
					}
					
					
					//r -= (noise->noise3D2(this->marchingCubeVertices2[idx].posX/230,marchingCubeVertices2[idx].posY/30,marchingCubeVertices2[idx].posZ/30) + 1.0f) *5.0f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/120,marchingCubeVertices2[idx].posY/20,marchingCubeVertices2[idx].posZ/120) + 1.0f) *5.0f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX/15,marchingCubeVertices2[idx].posY/15,marchingCubeVertices2[idx].posZ/15) + 1.0f) *0.5f;
					//r += 4 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.25f;
					//r += 4 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.25f;
					//r += 4 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.25f;
					//r += 4 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/260,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.25f;
					//r -= 10 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/20,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.25f;
					//r -= 3 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/30,marchingCubeVertices2[idx].posZ/30) + 1.0f) *0.5f;
					//r -= 3 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/30,marchingCubeVertices2[idx].posY/30,marchingCubeVertices2[idx].posZ/30) + 1.0f) *0.5f;
					//r += 8 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/120,marchingCubeVertices2[idx].posY/20,marchingCubeVertices2[idx].posZ/120) + 1.0f) *0.5f;
					//r += 2 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/180,marchingCubeVertices2[idx].posZ/20) + 1.0f) *0.5f;
					//r += 2 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/20,marchingCubeVertices2[idx].posY/140,marchingCubeVertices2[idx].posZ/20) + 1.0f) *1.0f;
					//r -= 1 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/10,marchingCubeVertices2[idx].posY/10,marchingCubeVertices2[idx].posZ/10) + 1.0f) *0.5f;
					//r -= 2 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/10,marchingCubeVertices2[idx].posY/10,marchingCubeVertices2[idx].posZ/10) + 1.0f) *0.5f;
					//r -= 2 * (noise->noise3D2(this->marchingCubeVertices2[idx].posX/10,marchingCubeVertices2[idx].posY/10,marchingCubeVertices2[idx].posZ/10) + 1.0f) *0.5f;
					//r -= (noise->noise3D2(this->marchingCubeVertices2[idx].posX/5,marchingCubeVertices2[idx].posY/5,marchingCubeVertices2[idx].posZ/5) + 1.0f) *0.5f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX,marchingCubeVertices2[idx].posY,marchingCubeVertices2[idx].posZ) + 0.0f) *0.5f;
					//r += (noise->noise3D2(this->marchingCubeVertices2[idx].posX,marchingCubeVertices2[idx].posY,marchingCubeVertices2[idx].posZ) - 0.3f) *0.5f;


					//r = (noise->noise3D2(this->marchingCubeVertices2[idx].posX,marchingCubeVertices2[idx].posY,marchingCubeVertices2[idx].posZ) + 1.0f) *0.5f;

					/*if(r<0)
						{r = 0.0f;}*/

					this->marchingCubeVertices2[idx].flux = r * 1;
					
					//r *= 1000 *3;

				if(max < r)
					{max = r;}
				if(min > r)
					{min = r;}

					
				if (r > 0.2f)
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
	max = max;
	min = min;
}


void MCTerrainClass::CreateMCVerts()
{

	
					this->marchingCubeVertices2[idx].normalX = this->marchingCubeVertices2[idx - 1].flux - this->marchingCubeVertices2[idx+1].flux;
					this->marchingCubeVertices2[idx].normalY = this->marchingCubeVertices2[idx - this->sizeY].flux - this->marchingCubeVertices2[idx + this->sizeY].flux;
					this->marchingCubeVertices2[idx].normalZ = this->marchingCubeVertices2[idx - (this->sizeY * this->sizeZ)].flux - this->marchingCubeVertices2[idx + (this->sizeY * this->sizeZ)].flux;
}