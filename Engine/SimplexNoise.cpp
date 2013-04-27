#include "SimplexNoise.h"

//http://webstaff.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java

////0..255, randomized
//static int p[] = {151,160,137,91,90,15,
//	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
//	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
//	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
//	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
//	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
//	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
//	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
//	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
//	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
//	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
//	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
//	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

static Grads grad3 [] = 
{
	Grads(1,1,0),
	Grads(-1,1,0),
	Grads(1,-1,0),
	Grads(-1,-1,0),
	Grads(1,0,1),
	Grads(-1,0,1),
	Grads(1,0,-1),
	Grads(-1,0,-1),
	Grads(0,1,1),
	Grads(0,-1,1),
	Grads(0,1,-1),
	Grads(0,-1,-1)
};


SimplexNoise::SimplexNoise()
{
	for(int i=0; i<256; i++)
	{
		p[i] = 1 + (rand() % 10);
	}

	for(int i=0; i<512; i++)
	{
		perm[i] = p[i & 255];
	}
}

void SimplexNoise::ReseedRandom()
{
	for(int i=0; i<256; i++)
	{
		p[i] = 1 + (rand() % 10);
	}

	for(int i=0; i<512; i++)
	{
		perm[i] = p[i & 255];
	}
}

SimplexNoise::~SimplexNoise()
{
}

//2D noise
float SimplexNoise::noise2D(float xin, float yin)
{
	float n0, n1, n2;

	// Noise contributions from the three corners
	// Skew the input space to determine which simplex cell we're in
	float F2 = 0.5f*(float)(sqrt(3.0f)-1.0f);
	float s = (xin+yin)*F2;

	// Hairy factor for 2D
	int i = fastfloor(xin+s);
	int j = fastfloor(yin+s);
	float G2 = (float)(3.0f-sqrt(3.0f))/6.0f;
	float t = (i+j)*G2;
	float X0 = i-t;

	// Unskew the cell origin back to (x,y) space
	float Y0 = j-t;
	float x0 = xin-X0;

	// The x,y distances from the cell origin
	float y0 = yin-Y0;

	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1;

	// Ofsets for second (middle) corner of simplex in (i,j) coords
	if(x0>y0) {i1=1; j1=0;}

	// lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {i1=0; j1=1;}

	// upper triangle, YX order: (0,0)->(0,1)->(1,1)
	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6
	float x1 = x0 - i1 + G2;

	// Ofsets for middle corner in (x,y) unskewed coords
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2;

	// Ofsets for last corner in (x,y) unskewed coords
	float y2 = y0 - 1.0f + 2.0f * G2;

	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = perm[ii+perm[jj]]			% 12;
	int gi1 = perm[ii+i1+perm[jj+j1]]	% 12;
	int gi2 = perm[ii+1+perm[jj+1]]		% 12;

	// Calculate the contribution from the three corners
	float t0 = 0.5f - x0*x0-y0*y0;
	if(t0<0) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
	}

	float t1 = 0.5f - x1*x1-y1*y1;
	if(t1<0) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}

	float t2 = 0.5f - x2*x2-y2*y2;
	if(t2<0) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 72.0f * (n0 + n1 + n2);
}

float SimplexNoise::noise3D2(float xin, float yin, float zin) 
{
	//for(int i=0; i<512; i++) perm[i]=p[i & 255]; //Remove this comment to slow down code by a margin of 6-7x.

	float n0, n1, n2, n3; // Noise contributions from the four corners

	// Skew the input space to determine which simplex cell we're in
	float F3 = 0.33333333f; //1.0f / 3.0f

	float s = (xin+yin+zin)*F3; // Very nice and simple skew factor for 3D
	int i = fastfloor(xin+s);
	int j = fastfloor(yin+s);
	int k = fastfloor(zin+s);

	float G3 = 0.16666666666; // Very nice and simple unskew factor, too. 1.0f / 6.0f

	float t = (i+j+k)*G3;

	float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
	float Y0 = j-t;
	float Z0 = k-t;

	float x0 = xin-X0; // The x,y,z distances from the cell origin
	float y0 = yin-Y0;
	float z0 = zin-Z0;

	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int i1, j1, k1; // Ofsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Ofsets for third corner of simplex in (i,j,k) coords

	if(x0>=y0) {
		if(y0>=z0)
		{ i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
		else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
		else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
	}
	else { // x0<y0
		if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
		else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
		else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	float x1 = x0 - i1 + G3; // Ofsets for second corner in (x,y,z) coords
	float y1 = y0 - j1 + G3;
	float z1 = z0 - k1 + G3;
	float x2 = x0 - i2 + 2.0f*G3; // Ofsets for third corner in (x,y,z) coords
	float y2 = y0 - j2 + 2.0f*G3;
	float z2 = z0 - k2 + 2.0f*G3;
	float x3 = x0 - 1.0f + 3.0f*G3; // Ofsets for last corner in (x,y,z) coords
	float y3 = y0 - 1.0f + 3.0f*G3;
	float z3 = z0 - 1.0f + 3.0f*G3;

	// Work out the hashed gradient indices of the four simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;

	int gi0 = perm[ii+perm[jj+perm[kk]]]			% 12;
	int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]]	% 12;
	int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]]	% 12;
	int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]]		% 12;

	// Calculate the contribution from the four corners
	float t0 = 0.5f - x0*x0 - y0*y0 - z0*z0;
	if(t0<0) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
		//n0 = t0 * t0 * dot(grad3,gi0, x0, y0, z0);
	}

	float t1 = 0.5f - x1*x1 - y1*y1 - z1*z1;
	if(t1<0) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
		//n1 = t1 * t1 * dot(grad3,gi1, x1, y1, z1);
	}

	float t2 = 0.5f - x2*x2 - y2*y2 - z2*z2;
	if(t2<0) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
		//n2 = t2 * t2 * dot(grad3,gi2, x2, y2, z2);
	}

	float t3 = 0.5f - x3*x3 - y3*y3 - z3*z3;
	if(t3<0) n3 = 0.0f;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
		//n3 = t3 * t3 * dot(grad3,gi3, x3, y3, z3);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 72.0f*(n0 + n1 + n2 + n3);
}

/*
float3 fade(float3 t)

{

return t * t * t * (t * (t * 6 - 15) + 10); // new curve

//  return t * t * (3 - 2 * t); // old curve

}



float perm(float x)

{

return tex1D(permSampler, x / 256.0f) * 256;

}



float grad(float x, float3 p)

{

return dot(tex1D(gradSampler, x), p);

}



// 3D version

float inoise(float3 p)

{

float3 P = fmod(floor(p), 256.0f);

p -= floor(p);

float3 f = fade(p);



// HASH COORDINATES FOR 6 OF THE 8 CUBE CORNERS



float A = perm(P.x) + P.y;

float AA = perm(A) + P.z;

float AB = perm(A + 1) + P.z;

float B =  perm(P.x + 1) + P.y;

float BA = perm(B) + P.z;

float BB = perm(B + 1) + P.z;



// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE

return lerp(

lerp(lerp(grad(perm(AA), p),

grad(perm(BA), p + float3(-1, 0, 0)), f.x),

lerp(grad(perm(AB), p + float3(0, -1, 0)),

grad(perm(BB), p + float3(-1, -1, 0)), f.x), f.y),

lerp(lerp(grad(perm(AA + 1), p + float3(0, 0, -1)),

grad(perm(BA + 1), p + float3(-1, 0, -1)), f.x),

lerp(grad(perm(AB + 1), p + float3(0, -1, -1)),

grad(perm(BB + 1), p + float3(-1, -1, -1)), f.x), f.y),

f.z);

}
*/