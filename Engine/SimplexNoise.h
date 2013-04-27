#pragma once
#include <math.h>
#include "structs.h"
#include <stdlib.h>
#include <stdio.h>

class SimplexNoise
{
public:
	SimplexNoise();
	~SimplexNoise();

	float noise2D(float xin, float yin);
	float noise3D2(float xin, float yin, float zin);

	void ReseedRandom();

	short perm[512];
	short p[256];

	
private:
	inline int fastfloor(float x) 
	{
		int xi = (int)x;
		return x < xi ? xi-1 : xi;
	}
		
/*
	
	float dot(int g[], float x, float y) {return g[0]*x + g[1]*y; }

	float dot(int g[], float x, float y, float z) {return g[0]*x + g[1]*y + g[2]*z; }

	float dot(int g[], float x, float y, float z, float w) {return g[0]*x + g[1]*y + g[2]*z + g[3]*w;}

	float dot(int g[],int i, float x, float y) {return g[i]*x + g[i+1]*y; }

	float dot(int g[],int i, float x, float y, float z) {return g[i]*x + g[i+1]*y + g[i+2]*z; }

	float dot(int g[],int i, float x, float y, float z, float w) {return g[i]*x + g[i+1]*y + g[i+2]*z + g[i+3]*w;}
*/

	

static float dot(int g[], float x, float y) {
	return g[0]*x + g[1]*y; }

static float dot(int g[], float x, float y, float z) {
	return g[0]*x + g[1]*y + g[2]*z; }

static float dot(int g[], float x, float y, float z, float w) {
	return g[0]*x + g[1]*y + g[2]*z + g[3]*w; }


static float dot(Grads g, float x, float y) {
	return x*g.x + y*g.y; }

static float dot(Grads g, float x, float y, float z) {
	return x*g.x + y*g.y + z*g.z; }

};

