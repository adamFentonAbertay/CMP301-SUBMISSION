// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
    matrix lightViewMatrix2;
    matrix lightProjectionMatrix2;
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float camerapadding;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float3 padding;
}

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos : TEXCOORD3;
    float4 lightViewPos2 : TEXCOORD4;
};

float getHeight(float2 uv)
{
    float4 sampleColour;
    sampleColour = texture0.SampleLevel(sampler0, uv, 0);
	
	return (sampleColour.x + sampleColour.y + sampleColour.z) / 3;
    
}

float3 normalCalc(float2 uv, float height)
{
	//distance to next vertex (from plane source code)
	float worldStep = 1;
	//size of plane 0-100, size of texture 0-1, 100th of the jump to next texure location
	float textureStep =  worldStep / 100;
	
	//neighbouring points
	float east = getHeight(float2(uv.x + textureStep, uv.y)) * 30;
	
	float west = getHeight(float2(uv.x - textureStep, uv.y)) * 30;
	
	float north = getHeight(float2(uv.x, uv.y - textureStep)) * 30;
	
	float south = getHeight(float2(uv.x, uv.y + textureStep)) * 30;
	
	//vectors to neighbouring points and normalized
	float3 centerToNorth = normalize(float3(0, north - height, worldStep));
	float3 centerToSouth = normalize(float3(0, south - height, -worldStep));
	float3 centerToEast = normalize(float3(worldStep, east - height, 0));
	float3 centerToWest = normalize(float3(-worldStep, west - height, 0));
	
	//cross producting and adding to get averaged normal from surface
	return (cross(centerToNorth, centerToEast) + cross(centerToEast, centerToSouth) + cross(centerToSouth, centerToWest) + cross(centerToWest, centerToNorth)) / 4;

}

OutputType main(InputType input)
{
	OutputType output;
	
 

	float averagedHeightValue = getHeight(input.tex) * 30;
	
    input.position.y += averagedHeightValue;
	
	input.normal = normalCalc(input.tex, averagedHeightValue);
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
    output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	
	
    output.lightViewPos = mul(input.position, worldMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);
    
    output.lightViewPos2 = mul(input.position, worldMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, lightViewMatrix2);
    output.lightViewPos2 = mul(output.lightViewPos2, lightProjectionMatrix2);


	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	
    float4 worldPosition2 = mul(input.position, worldMatrix);
    output.viewVector = cameraPosition.xyz - worldPosition2.xyz;
    output.viewVector = normalize(output.viewVector);

	return output;
}