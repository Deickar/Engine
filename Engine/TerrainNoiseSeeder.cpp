#include "TerrainNoiseSeeder.h"

TerrainNoiseSeeder::TerrainNoiseSeeder()
{
}

TerrainNoiseSeeder::~TerrainNoiseSeeder()
{
}

static const float minDensity = -10.0f;
static const float maxDensity = 10.0f;

void TerrainNoiseSeeder::Initialize(int sizeX, int sizeY, int sizeZ, NoiseClass* simplexNoise, TerrainTypes::Type terrainType)
{
	terrainMode = terrainType;
	noise = simplexNoise;

	densityToBeInside = 0.35f;

	worldSize = 0;
	worldSizeMargin = 1;
	worldArraySize = sizeX * sizeY;

	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;
	XFactor = 1.0f / 100.0f;
	YFactor = 1.0f / 100.0f;
	ZFactor = 1.0f / 100.0f;

	NoiseFunction seabottomNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.1f - index.y;
		density += noise->SimplexNoise3D(position.x/220, position.y/20, position.z/220) * 2.0f;
		density += noise->SimplexNoise3D(position.x/120, position.y/220, position.z/40) * 2.0f;
		density += noise->SimplexNoise3D(position.x/20, position.y/420, position.z/20) * 2.0f;

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};


	NoiseFunction plainsNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.2f - index.y;
		density += noise->SimplexNoise3D(position.x/220, position.y/20, position.z/220) * 10.0f;

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};

	NoiseFunction hillsNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.2f - index.y;
		density += noise->SimplexNoise3D(position.x/220, position.y/20,	 position.z/220) *2.0f;
		density += noise->SimplexNoise3D(position.x/80,	 position.y/220, position.z/40) *20.0f;
		density += noise->SimplexNoise3D(position.x/20,	 position.y/420, position.z/20) *10.0f;

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};

	NoiseFunction terraceNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.4f - index.y;
		density += noise->SimplexNoise3D( position.x/220,	position.y/220,	position.z/420) *20.0f;
		density += noise->SimplexNoise3D( position.x/120,	position.y/20,	position.z/20) *20.0f;
		density += noise->SimplexNoise3D( position.x/20,	position.y/20,	position.z/120) *20.0f;
		density += noise->SimplexNoise3D( position.x/120,	position.y/20,	position.z/120) *5.0f;

		//**Toplvl**//
		if(index.y > 20)
		{
			density -= 12.0f*((index.y-20)/5);
		}

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};


	NoiseFunction dramaticHillsNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.6f - index.y;

		density += noise->SimplexNoise3D(position.x/80,	 position.y/80,	 position.z/520) *30.0f;
		density += noise->SimplexNoise3D(position.x/40,	 position.y/30,	 position.z/500) *30.0f;
		density += noise->SimplexNoise3D(position.x/180, position.y/20,	 position.z/50) *20.0f;
		density += noise->SimplexNoise3D(position.x/120, position.y/20,	 position.z/120) *5.0f;
		density += noise->SimplexNoise3D(position.x/80,	 position.y/420, position.z/80) *30.0f;

		//**Toplvl**//
		if(index.y > 20)
		{
			density -= 12.0f*((index.y-20.0f)/5);
		}

		if(index.y == 1)
		{
			density += 100.0f;
		}


		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};

	NoiseFunction flyingIslandsNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 0 + index.sizeY * 0.001f - index.y*0.1f;
		density += noise->SimplexNoise3D(position.x/40, position.y/10,	position.z/25) *10.0f;
		density += noise->SimplexNoise3D(position.x/60, position.y/5,	position.z/35) *10.0f;
		density += noise->SimplexNoise3D(position.x/21, position.y/10,	position.z/43) *10.0f;

		if(density < -2)
		{
			density = -2;
		}
		else if (density > 2)
		{
			density = 2;
		}

		return density;
	};

	NoiseFunction alienNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.1f - index.y;
		density += noise->SimplexNoise3D(position.x/220, position.y/20,	 position.z/220) *20.0f;
		density += noise->SimplexNoise3D(position.x/120, position.y/220, position.z/40) *20.0f;
		density += noise->SimplexNoise3D(position.x/20,	 position.y/420, position.z/20) *10.0f;

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};


	NoiseFunction fancyNoise =
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 1 + index.sizeY * 0.1f - index.y;
		density += noise->SimplexNoise3D(position.x/220,	position.y/220,	position.z/420) *2.0f;
		density += noise->SimplexNoise3D(position.x/120,	position.y/20,	position.z/20) *20.0f;
		density += noise->SimplexNoise3D(position.x/20,		position.y/20,	position.z/120) *20.0f;
		density += noise->SimplexNoise3D(position.x/120,	position.y/20,	position.z/120) *2.0f;

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};

	NoiseFunction caveNoise = 
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		density = 7;
		density += noise->SimplexNoise3D(position.x/60,	 position.y/60,	 position.z/60) *20.0f;
		density += noise->SimplexNoise3D(position.x/20,	 position.y/20,	 position.z/120) *10.0f;
		density += noise->SimplexNoise3D(position.x/20,	 position.y/120, position.z/20) *10.0f;
		density += noise->SimplexNoise3D(position.x/120, position.y/20,	 position.z/20) *10.0f;

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}


		return density;
	};

	NoiseFunction flatTerrainNoise = 
		[](IndexingValue& index, const XMFLOAT3& position, NoiseClass* noise) -> float
	{
		float density;
		unsigned int idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

		if(position.y <= 15.0f)
		{
			density = 50.0f;
		}
		else
		{
			density = -50.0f;
		}

		if(index.y == 1)
		{
			density += 100.0f;
		}

		if(density < minDensity)
		{
			density = minDensity;
		}
		else if (density > maxDensity)
		{
			density = maxDensity;
		}

		return density;
	};

	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::SeaBottom, seabottomNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Plains, plainsNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Hills, hillsNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Terraces, terraceNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::DramaticHills, dramaticHillsNoise));

	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::FlyingIslands, flyingIslandsNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Alien, alienNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Fancy, fancyNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Cave, caveNoise));
	functionMap.insert(std::make_pair<TerrainTypes::Type, NoiseFunction>(TerrainTypes::Flat, flatTerrainNoise));
}

void TerrainNoiseSeeder::Noise3D(unsigned int startX, unsigned int startY, unsigned int startZ, unsigned int endX, unsigned int endY, unsigned int endZ, std::vector<MarchingCubeVoxel>* verts)
{
	unsigned int idx;

	//Initialize index to some default values.
	IndexingValue index(0, 0, 0, sizeX, sizeY, sizeZ);

	//Select which noise function to use depending on what terrain type we currently have selected.
	NoiseFunction noiseFunction = functionMap[terrainMode];

	for(index.y = startY; index.y < (endY-1); ++index.y)
	{
		for (index.z = startZ; index.z < (endZ-1); ++index.z)
		{
			for (index.x = startX; index.x < (endX-1); ++index.x)
			{
				//Index value into the vertex voxel field
				idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

				//Extract density value from selected noise function
				(*verts)[idx].density = noiseFunction(index, (*verts)[idx].position, noise);

				//Decide if the vertex is considered inside
				(*verts)[idx].inside = ((*verts)[idx].density >= densityToBeInside) ? true : false;
			}
		}
	}

	//Calculate normals for each point in terrain voxel field. 
	//** This needs to be done in a second loop because we need all the values to be noised before we do normal creation. **
	for(index.y = startY; index.y < (endY-1); ++index.y)
	{
		for (index.z = startZ; index.z < (endZ-1); ++index.z)
		{
			for (index.x = startX; index.x < (endX-1); ++index.x)
			{
				//Index value into the vertex voxel field
				idx = index.x + (index.y*index.sizeY) + (index.z * index.sizeY * index.sizeZ);

				(*verts)[idx].normal.x = ((*verts)[idx - 1								].density	-	(*verts)[idx + 1							].density)	* XFactor;
				(*verts)[idx].normal.y = ((*verts)[idx - index.sizeY					].density	-	(*verts)[idx + index.sizeY					].density)	* YFactor;
				(*verts)[idx].normal.z = ((*verts)[idx - (index.sizeY * index.sizeZ)	].density	-	(*verts)[idx + (index.sizeY * index.sizeZ)	].density)	* ZFactor;

				//Normalize results.
				float vectorLength = sqrt(((*verts)[idx].normal.x*(*verts)[idx].normal.x) + ((*verts)[idx].normal.y*(*verts)[idx].normal.y) + ((*verts)[idx].normal.z*(*verts)[idx].normal.z));

				//If length is too small we don't need to normalize...
				if(vectorLength >= 0.01f)
				{
					(*verts)[idx].normal.x = (*verts)[idx].normal.x/vectorLength;
					(*verts)[idx].normal.y = (*verts)[idx].normal.y/vectorLength;
					(*verts)[idx].normal.z = (*verts)[idx].normal.z/vectorLength;
				}
				else
				{
					(*verts)[idx].normal.x	+= RandomFloat() * 0.1f; //.. yeah... dirty.
					(*verts)[idx].normal.y	+= RandomFloat() * 0.1f; //.. yeah... dirty.
					(*verts)[idx].normal.z	+= RandomFloat() * 0.1f; //.. yeah... dirty.
				}
				//CreateNormal(verts, index, idx);
			}
		}
	}
}

bool TerrainNoiseSeeder::GetHighestPositionOfCoordinate(int x, int z, MarchingCubeChunk* chunk, std::vector<MarchingCubeVoxel>* marchingCubeVertices, float* outHeightValue)
{
	int idx;
	int i = 0;

	//OR stepSize? can't remember
	int sizeY = chunk->GetStepCountY();
	int sizeZ = chunk->GetStepCountZ();

	int idxPreCalc = x + z * sizeY * sizeZ;

	int yBreakPoint = 0;

	//So what this function does is that it takes a point in our voxel field, then it starts from the absolute highest point in the field, 
	// and goes downward until it hits a point with a density that is high enough to be considered solid.
	for (i = sizeY; i > 0; i--)
	{
		idx = i*sizeY + idxPreCalc;

		//When we find our first solid voxel, we break, because we've found the right point
		if((*marchingCubeVertices)[idx].inside)
		{
			yBreakPoint = i;
			break;
		} 
	}

	//If this point is below water level.... return false
	if(yBreakPoint <= ((int)chunk->GetWaterLevel()))
	{
		return false;
	}

	//Now we want the point below it
	int idx2 = idx+sizeY;

	//Calculate an average
	float j = ((*marchingCubeVertices)[idx].density + (*marchingCubeVertices)[idx2].density)*0.5f;

	//Upvector to compare vertex normals against
	XMVECTOR upVector = XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f));

	XMVECTOR vertexNormal1 = XMLoadFloat3(&(*marchingCubeVertices)[idx].normal);
	XMVECTOR vertexNormal2 = XMLoadFloat3(&(*marchingCubeVertices)[idx2].normal);

	//http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.geometric.xmvector3dot(v=vs.85).aspx
	//Return value: Returns a vector. The dot product between V1 and V2 is replicated into each component.
	//							//Take average of the two closest vertices
	XMVECTOR result = XMVector3Dot(((vertexNormal1+vertexNormal2)*0.5f), upVector);

	//We check the dot result between an UpNormal and the average of the two vertex normals
	//If this value is too low, it means that the angle of the surface we're trying to 
	// place something on is too slanted. Hence we break early and return false.
	if(result.m128_f32[0] < 0.95f)
	{
		return false;
	}

	//Then we add some fuckin' magic numbers because we're BAD BOYS who don't PLAY BY THE RULES
	*outHeightValue = (i + 0.5f + j);

	return true;
}

void TerrainNoiseSeeder::MCHeightMap()
{
	//float* worldArray = new float[worldArraySize];

	//for(int i = 0; i < worldArraySize; i++)
	//{
	//	float density = noise->Noise2D(marchingCubeVertices[i].position.x, marchingCubeVertices[i].position.z);
	//	worldArray[i] = density;
	//}

	//int i = -2;

	//for (z = worldSizeMargin; z < (sizeZ - worldSizeMargin); z++)
	//{
	//	i++;
	//	for (y = worldSizeMargin; y < (sizeY - worldSizeMargin); y++)
	//	{
	//		for (x = worldSizeMargin; x < (sizeX - worldSizeMargin); x++)
	//		{
	//			i++;

	//			if (worldArray[i] > (float)y)
	//			{
	//			}
	//			idx = x + y*sizeY + z * sizeY * sizeZ;



	//			(*marchingCubeVertices)[idx].density = 100.0f;



	//			if ((*marchingCubeVertices)[idx].density > 0.1f)
	//			{
	//				(*marchingCubeVertices)[idx].inside = true;
	//			}
	//			else
	//			{
	//				(*marchingCubeVertices)[idx].inside = false;
	//			}


	//			(*marchingCubeVertices)[idx].normal.x = marchingCubeVertices[idx - 1].density - marchingCubeVertices[idx+1].density;
	//			(*marchingCubeVertices)[idx].normal.y = marchingCubeVertices[idx - sizeY].density - marchingCubeVertices[idx + sizeY].density;
	//			(*marchingCubeVertices)[idx].normal.z = marchingCubeVertices[idx - (sizeY * sizeZ)].density - marchingCubeVertices[idx + (sizeY * sizeZ)].density;

	//		}
	//	}
	//}

	//if (worldArray != 0)
	//{
	//	delete worldArray;
	//}
}