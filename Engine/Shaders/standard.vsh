////////////////////////////////////////////////////////////////////////////////
// Filename: standard.vs
////////////////////////////////////////////////////////////////////////////////

/*
* Pretty much just a standard vertex shader when you don't really need any special operations.
*/

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
		matrix worldViewProjection;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
		float4 position : POSITION;
		float2 tex : TEXCOORD0;
};

struct PixelInputType
{
		float4 position : SV_POSITION;
		float2 tex : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType StandardVertexShader(VertexInputType input)
{
		PixelInputType output;	

		// Change the position vector to be 4 units for proper matrix calculations.
		input.position.w = 1.0f;

		// Calculate the position of the vertex against the world, view, and projection matrices.
		output.position = mul(input.position, worldViewProjection);

		// Store the texture coordinates for the pixel shader.
		output.tex = input.tex;
		
		return output;
}