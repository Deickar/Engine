#pragma once
#include <math.h>
#include "StructsAndEnums.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <random>

// The gradients are the midpoints of the vertices of a hypercube.
static const int grad4[32][4]= {
	{0,1,1,1},  {0,1,1,-1},  {0,1,-1,1},  {0,1,-1,-1},
	{0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
	{1,0,1,1},  {1,0,1,-1},  {1,0,-1,1},  {1,0,-1,-1},
	{-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
	{1,1,0,1},  {1,1,0,-1},  {1,-1,0,1},  {1,-1,0,-1},
	{-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
	{1,1,1,0},  {1,1,-1,0},  {1,-1,1,0},  {1,-1,-1,0},
	{-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}
};

// A lookup table to traverse the simplex around a given point in 4D.
static const int simplex[64][4] = {
	{0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
	{0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
	{1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
	{2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
};

const static Grads grad3 [] = 
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

struct DoubleXYZCoords
{
	int x1, y1, z1;
	int x2, y2, z2;

	DoubleXYZCoords(int x, int y, int z, int xx, int yy, int zz)
	{
		x1 = x;		y1 = y;		z1 = z;
		x2 = xx;	y2 = yy;	z2 = zz;
	}
};

static DoubleXYZCoords coords [] =
{
	DoubleXYZCoords(1, 0, 0, 1, 1, 0),
	DoubleXYZCoords(1, 0, 0, 1, 0, 1),
	DoubleXYZCoords(0, 0, 1, 1, 0, 1),

	DoubleXYZCoords(0, 0, 1, 0, 1, 1),
	DoubleXYZCoords(0, 1, 0, 0, 1, 1),
	DoubleXYZCoords(0, 1, 0, 1, 1, 0),
};

// Skew the input space to determine which simplex cell we're in
static const float F3 = 1.0f/3.0f; //1.0f / 3.0f
static const float G3 = 1.0f/6.0f; // Very nice and simple unskew factor, too. 1.0f / 6.0f
static const float G3TimesTwo = (1.0f/6.0f)*2.0f;
static const float G3TimesThree = (3.0f*(1.0f/6.0f));

// The skewing and unskewing factors are hairy again for the 4D case
static const float F4 = (sqrtf(5.0f)-1.0f)/4.0f;
static const float G4 = (5.0f-sqrtf(5.0f))/20.0f;

class NoiseClass
{
public:
	NoiseClass();
	~NoiseClass();

	float Noise2D(float xin, float yin);
	
	//float SimplexNoise3D(float xin, float yin, float zin);
	inline float SimplexNoise3D(float xin, float yin, float zin) 
	{
		float n0, n1, n2, n3; // Noise contributions from the four corners

		const float s = (xin+yin+zin)*F3; // Very nice and simple skew factor for 3D
		const int i = Fastfloor(xin+s);
		const int j = Fastfloor(yin+s);
		const int k = Fastfloor(zin+s);

		const float t = (i+j+k)*G3;

		//float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
		//float Y0 = j-t;
		//float Z0 = k-t;

		const float x0 = xin-(i-t); // The x,y,z distances from the cell origin
		const float y0 = yin-(j-t);
		const float z0 = zin-(k-t);

		// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
		// Determine which simplex we are in.
		DoubleXYZCoords* tempCoords;

		if(x0>=y0) 
		{
			if(y0>=z0){ tempCoords = &coords[0]; } // X Y Z order
			else if(x0>=z0) { tempCoords = &coords[1]; } // X Z Y order
			else { tempCoords = &coords[2]; } // Z X Y order
		}
		else 
		{ // x0<y0
			if(y0<z0) { tempCoords = &coords[3]; } // Z Y X order
			else if (x0<z0) { tempCoords = &coords[4]; } // Y Z X order
			else { tempCoords = &coords[5]; } // Y X Z order
		}

		// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
		// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
		// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
		// c = 1/6.
		const float x1 = x0 - tempCoords->x1 + G3; // Ofsets for second corner in (x,y,z) coords
		const float y1 = y0 - tempCoords->y1 + G3;
		const float z1 = z0 - tempCoords->z1 + G3;
		const float x2 = x0 - tempCoords->x2 + G3TimesTwo; // Offsets for third corner in (x,y,z) coords
		const float y2 = y0 - tempCoords->y2 + G3TimesTwo;
		const float z2 = z0 - tempCoords->z2 + G3TimesTwo;
		const float x3 = x0 - 1.0f + G3TimesThree; // Offsets for last corner in (x,y,z) coords
		const float y3 = y0 - 1.0f + G3TimesThree;
		const float z3 = z0 - 1.0f + G3TimesThree;

		// Work out the hashed gradient indices of the four simplex corners
		const int ii = i & 255;
		const int jj = j & 255;
		const int kk = k & 255;

		const int gi0 = perm[ii+perm[jj+perm[kk]]]			% 12;
		const int gi1 = perm[ii+tempCoords->x1+perm[jj+tempCoords->y1+perm[kk+tempCoords->z1]]]	% 12;
		const int gi2 = perm[ii+tempCoords->x2+perm[jj+tempCoords->y2+perm[kk+tempCoords->z2]]]	% 12;
		const int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]]	% 12;

		n0 = n1 = n2 = n3 = 0.0f;

		// Calculate the contribution from the four corners
		float t0 = 0.5f - x0*x0 - y0*y0 - z0*z0;
		if(t0 >= 0)
		{
			t0 *= t0;
			n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
		}

		float t1 = 0.5f - x1*x1 - y1*y1 - z1*z1;
		if(t1 >= 0)
		{
			t1 *= t1;
			n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
		}

		float t2 = 0.5f - x2*x2 - y2*y2 - z2*z2;
		if(t2 >= 0)
		{
			t2 *= t2;
			n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
		}

		float t3 = 0.5f - x3*x3 - y3*y3 - z3*z3;
		if(t3 >= 0)
		{
			t3 *= t3;
			n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
		}

		// Add contributions from each corner to get the final noise value.
		// The result is scaled to stay just inside [-1,1]

		//return (36.0f*(n0 + n1 + n2 + n3)+1)*0.5f;// 0 -> 1
		return (32.0f*(n0 + n1 + n2 + n3)); //-1 -> 1
	}
	
	
	inline float SimplexNoise3DZeroToOne(float xin, float yin, float zin)
	{
		return (1.0f+SimplexNoise3D(xin, yin, zin))*0.5f;
	}
	
	
	//float SimplexNoise4D(float x, float y, float z, float w);
	inline float SimplexNoise4D( float x, float y, float z, float w )
	{
		float n0, n1, n2, n3, n4; // Noise contributions from the five corners

		// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
		const float s = (x + y + z + w) * F4; // Factor for 4D skewing
		const int i = Fastfloor(x + s);
		const int j = Fastfloor(y + s);
		const int k = Fastfloor(z + s);
		const int l = Fastfloor(w + s);
		
		const float t = (i + j + k + l) * G4; // Factor for 4D unskewing
		const float X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
		const float Y0 = j - t;
		const float Z0 = k - t;
		const float W0 = l - t;

		const float x0 = x - X0; // The x,y,z,w distances from the cell origin
		const float y0 = y - Y0;
		const float z0 = z - Z0;
		const float w0 = w - W0;

		// For the 4D case, the simplex is a 4D shape I won't even try to describe.
		// To find out which of the 24 possible simplices we're in, we need to
		// determine the magnitude ordering of x0, y0, z0 and w0.
		// The method below is a good way of finding the ordering of x,y,z,w and
		// then find the correct traversal order for the simplex we're in.
		// First, six pair-wise comparisons are performed between each possible pair
		// of the four coordinates, and the results are used to add up binary bits
		// for an integer index.
		const int c1 = (x0 > y0) ? 32 : 0;
		const int c2 = (x0 > z0) ? 16 : 0;
		const int c3 = (y0 > z0) ? 8 : 0;
		const int c4 = (x0 > w0) ? 4 : 0;
		const int c5 = (y0 > w0) ? 2 : 0;
		const int c6 = (z0 > w0) ? 1 : 0;
		const int c = c1 + c2 + c3 + c4 + c5 + c6;

		//int i1, j1, k1, l1; // The integer offsets for the second simplex corner
		//int i2, j2, k2, l2; // The integer offsets for the third simplex corner
		//int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner

		// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
		// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
		// impossible. Only the 24 indices which have non-zero entries make any sense.
		// We use a thresholding to set the coordinates in turn from the largest magnitude.
		// The number 3 in the "simplex" array is at the position of the largest coordinate.
		const int i1 = simplex[c][0]>=3 ? 1 : 0;
		const int j1 = simplex[c][1]>=3 ? 1 : 0;
		const int k1 = simplex[c][2]>=3 ? 1 : 0;
		const int l1 = simplex[c][3]>=3 ? 1 : 0;
		// The number 2 in the "simplex" array is at the second largest coordinate.
		const int i2 = simplex[c][0]>=2 ? 1 : 0;
		const int j2 = simplex[c][1]>=2 ? 1 : 0;
		const int k2 = simplex[c][2]>=2 ? 1 : 0;
		const int l2 = simplex[c][3]>=2 ? 1 : 0;
		// The number 1 in the "simplex" array is at the second smallest coordinate.
		const int i3 = simplex[c][0]>=1 ? 1 : 0;
		const int j3 = simplex[c][1]>=1 ? 1 : 0;
		const int k3 = simplex[c][2]>=1 ? 1 : 0;
		const int l3 = simplex[c][3]>=1 ? 1 : 0;
		// The fifth corner has all coordinate offsets = 1, so no need to look that up.

		const float x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
		const float y1 = y0 - j1 + G4;
		const float z1 = z0 - k1 + G4;
		const float w1 = w0 - l1 + G4;
		const float x2 = x0 - i2 + 2.0f*G4; // Offsets for third corner in (x,y,z,w) coords
		const float y2 = y0 - j2 + 2.0f*G4;
		const float z2 = z0 - k2 + 2.0f*G4;
		const float w2 = w0 - l2 + 2.0f*G4;
		const float x3 = x0 - i3 + 3.0f*G4; // Offsets for fourth corner in (x,y,z,w) coords
		const float y3 = y0 - j3 + 3.0f*G4;
		const float z3 = z0 - k3 + 3.0f*G4;
		const float w3 = w0 - l3 + 3.0f*G4;
		const float x4 = x0 - 1.0f + 4.0f*G4; // Offsets for last corner in (x,y,z,w) coords
		const float y4 = y0 - 1.0f + 4.0f*G4;
		const float z4 = z0 - 1.0f + 4.0f*G4;
		const float w4 = w0 - 1.0f + 4.0f*G4;

		// Work out the hashed gradient indices of the five simplex corners
		const int ii = i & 255;
		const int jj = j & 255;
		const int kk = k & 255;
		const int ll = l & 255;
		const int gi0 = perm[ii+perm[jj+perm[kk+perm[ll]]]] % 32;
		const int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1+perm[ll+l1]]]] % 32;
		const int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2+perm[ll+l2]]]] % 32;
		const int gi3 = perm[ii+i3+perm[jj+j3+perm[kk+k3+perm[ll+l3]]]] % 32;
		const int gi4 = perm[ii+1+perm[jj+1+perm[kk+1+perm[ll+1]]]] % 32;

		// Calculate the contribution from the five corners
		float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0 - w0*w0;
		if(t0<0) n0 = 0.0f;
		else {
			t0 *= t0;
			n0 = t0 * t0 * dot(grad4[gi0], x0, y0, z0, w0);
		}

		float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1 - w1*w1;
		if(t1<0) n1 = 0.0f;
		else {
			t1 *= t1;
			n1 = t1 * t1 * dot(grad4[gi1], x1, y1, z1, w1);
		}

		float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2 - w2*w2;
		if(t2<0) n2 = 0.0f;
		else {
			t2 *= t2;
			n2 = t2 * t2 * dot(grad4[gi2], x2, y2, z2, w2);
		}

		float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3 - w3*w3;
		if(t3<0) n3 = 0.0f;
		else {
			t3 *= t3;
			n3 = t3 * t3 * dot(grad4[gi3], x3, y3, z3, w3);
		}

		float t4 = 0.6f - x4*x4 - y4*y4 - z4*z4 - w4*w4;
		if(t4<0) n4 = 0.0f;
		else {
			t4 *= t4;
			n4 = t4 * t4 * dot(grad4[gi4], x4, y4, z4, w4);
		}

		// Sum up and scale the result to cover the range [-1,1]
		return 27.0f * (n0 + n1 + n2 + n3 + n4);
	}

	float PerlinNoise3D(float x, float y,  float z);

	void ReseedRandom();

	int perm[512];
	int p[260];
	float grads[512];


private:
	inline int MFloatToInt(float x)
	{
		x -= 0.5f;
		int t;
		__asm fld x;
		__asm fistp t;

		return t;
	}

	inline int Fastfloor(float x) 
	{
		int xi = (int)x;
		return x < xi ? xi-1 : xi;
	}

	inline float Fade(float t) 
		{ return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

	inline float Lerp(float t, float a, float b) 
		{ return a + t * (b - a); }

	inline float Grad(int hash, float x, float y, float z) 
	{
		int h = hash & 15;                     // CONVERT LO 4 BITS OF HASH CODE
		float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h==12||h==14 ? x : z;
		return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
	}

	inline float dot(int g[], float x, float y) 
	{
		return g[0]*x + g[1]*y; 
	}

	inline float dot(int g[], float x, float y, float z) 
	{
		return g[0]*x + g[1]*y + g[2]*z; 
	}

	inline float dot(int g[], float x, float y, float z, float w) 
	{
		return g[0]*x + g[1]*y + g[2]*z + g[3]*w; 
	}

	inline float dot(const Grads& g, float x, float y) 
	{
		return x*g.x + y*g.y; 
	}

	inline float dot(const Grads& g, float x, float y, float z)
	{
		return x*g.x + y*g.y + z*g.z; 
	}

	inline float dot(const int* g, const float x, const float y, const float z, const float w)
	{
		return g[0]*x + g[1]*y + g[2]*z + g[3]*w;
	}
};