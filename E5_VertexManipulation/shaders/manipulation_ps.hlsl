// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);


Texture2D depthMapTexture : register(t1);
Texture2D depthMapTexture2 : register(t2);


SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float3 lightDirection;
    float padding;
    float4 ambient2;
    float4 diffuse2;
    float3 direction2;
    float padding2;
};

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
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos : TEXCOORD3;
    float4 lightViewPos2 : TEXCOORD4;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirectionCall, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirectionCall));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}




bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float4 linearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;

    return (2.0 * near * far) / (far + near - z * (far - near) );

}

bool isInSpotShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
 // float depthValue = sMap.Sample(shadowSampler, uv).r;
    float depthValue = linearizeDepth(sMap.Sample(shadowSampler, uv).r, 0.1, 200);
    depthValue = depthValue / 200;
	// Calculate the depth from the light.-
   float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}


//1
float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}




float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
    float4 lightColour2;

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    textureColour = texture0.Sample(sampler0, input.tex);
    lightColour = calculateLighting(-lightDirection, input.normal, diffuseColour);
    lightColour2 = calculateLighting(-direction2, input.normal, diffuse2);
    
    
    
    float3 lightVector = normalize(spot_lightPosition - input.worldPosition);
    
	//lightColour = calculateLighting(lightVector, input.normal, diffuseColour, ambientColour);	
    float ndotl = dot(lightVector, input.normal);
    float coneLighting = acos(dot(normalize(-spot_lightDirection) , normalize(lightVector)));
    float lightAngle = spot_cutoff;
    float intensity = clamp(((lightAngle - coneLighting) / lightAngle), 0, 2);
    intensity = intensity * ndotl;
    
    float shadowMapBias = 0.008f;
    //0.005
    //0.08
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 colour2 = float4(0.f, 0.f, 0.f, 1.f);
  
	// Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.lightViewPos);
    float2 pTexCoord2 = getProjectiveCoords(input.lightViewPos2);
	
    // Shadow test. Is or isn't in shadow
    if (hasDepthData(pTexCoord))
    {
        // Has depth map data
        if (!isInShadow(depthMapTexture, pTexCoord, input.lightViewPos, shadowMapBias))
        {
            // is NOT in shadow, therefore light
            colour = lightColour;
        }
    }
    if (hasDepthData(pTexCoord2))
    {
        // Has depth map data
        if (!isInSpotShadow(depthMapTexture2, pTexCoord2, input.lightViewPos2, shadowMapBias))
        {
            // is NOT in shadow, therefore light
           // colour2 = lightColour2;
            colour2 = (intensity * spot_diffuseColour);

            
        }
    }
    
    float cValue = colour.x + colour.y + colour.z;
    float cValue2 = colour2.x + colour2.y + colour2.z;
 
    //if (cValue == 0 || cValue2 == 0)
    //{

    //    colour = colour / 3;
    //    colour2 = colour2 / 3;
     
    //    colour = 0;
    //    colour2 = 0;
    //}
    
  //  colour = colour + colour2;
  
    
    colour = saturate(colour);
    float4 colourWithText = colour * textureColour;

    float4 newShadow = float4(0,0,0,1);
    
    float thresholdShadow = 0.f;
    
    if ((colour2.y <= thresholdShadow && intensity > 0))
    {
        newShadow = float4(1, 1, 1, 1) * intensity;
    }
    else if (colourWithText.y <= thresholdShadow && colourWithText.x <= thresholdShadow && colourWithText.z <= thresholdShadow)
    {
        newShadow = float4(1, 1, 1, 1);

    }
    
     
  //  colour = colour - newShadow;
   // return newShadow;
    //return colour * textureColour + (intensity * spot_diffuseColour);
    return ((lightColour * textureColour) + (intensity * spot_diffuseColour)) - newShadow;
   // return newShadow;
  
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

}



