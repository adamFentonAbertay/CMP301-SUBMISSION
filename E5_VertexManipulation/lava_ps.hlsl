// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer LightSpotBuffer : register(b1)
{
    float4 spot_diffuseColour;
    float3 spot_lightDirection;
    float spot_padding;
    float4 spot_ambientColour;
    float3 spot_lightPosition;
    float spot_cutoff;
    //float spot_specularPower;
    //float3 spot_padding2;
    //float4 spot_specular;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
    //calc for spot light
    float3 lightVector = normalize(spot_lightPosition - input.worldPosition);
    float ndotl = dot(lightVector, input.normal);
    float coneLighting = acos(dot(normalize(-spot_lightDirection), normalize(lightVector)));
    float lightAngle = spot_cutoff;
    float intensity = clamp(((lightAngle - coneLighting) / lightAngle), 0, 2);
    intensity = intensity * ndotl;
    
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 lavaLight = { 0.7, 0.2, 0.2, 1.f };
    float4 textureColor = texture0.Sample(Sampler0, input.tex);
   
    
    return textureColor + lavaLight + (intensity * spot_diffuseColour) ;
}