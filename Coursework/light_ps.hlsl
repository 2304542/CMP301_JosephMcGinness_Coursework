// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

#define MAXLIGHTS 2

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour[MAXLIGHTS];
    float4 ambientColour[MAXLIGHTS];
    float4 lightDirection[MAXLIGHTS];
    float4 lightPosition[MAXLIGHTS];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
    
    
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    for (int i = 0; i < MAXLIGHTS; i++)
    {
        
        lightColour += calculateLighting(-lightDirection[i], input.normal, diffuseColour[i]);
    }
    
    textureColour = texture0.Sample(sampler0, input.tex);
	
    return lightColour * textureColour;
}