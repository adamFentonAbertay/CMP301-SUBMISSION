// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)
//https://learnopengl.com/Advanced-Lighting/Bloom
//https://www.youtube.com/watch?v=tI70-HIc5ro
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer bloomBuffer : register(b0)
{
    float cutoff;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
};



float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 cull = { 0,0,0,1.f};
    float cullCutoff = cutoff;

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    textureColour = texture0.Sample(sampler0, input.tex);

    //https://contrastchecker.online/color-relative-luminance-calculator
    
    //other methods
    
   //float percievedColour = sqrt(.241 * pow(textureColour.x, 2) + .691 * pow(textureColour.y, 2) + .068 * pow(textureColour.z, 2));
    
   //float percievedColour = ((textureColour.x * 299) + (textureColour.y * 587) + (textureColour.z * 114)) / 1000;
    
    //percieved colour for sRGB in relative luminosity.
    float percievedColour = (0.2126 * textureColour.x + 0.7152 * textureColour.y + 0.0722 * textureColour.z);
    
    //simple mask if its not bright enough
    if (percievedColour > cullCutoff)
    {
        return textureColour;
    }
    else
    {
        return cull;
    }

       
}



