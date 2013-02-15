#ifndef MCUBES_TYPES_H
#define MCUBES_TYPES_H

#include <d3d11.h>
#include <d3dx10math.h>

struct MarchingCubeVertex
{
	float posX;
	float posY;
	float posZ;

	float normalX;
	float normalY;
	float normalZ;

	float flux;

	bool inside;
};

struct Metapoint
{
	float posX;
	float posY;
	float posZ;

	float power;
};

struct MarchingCubeVectors
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
};

#endif