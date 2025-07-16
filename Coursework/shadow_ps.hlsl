
Texture2D shaderTexture : register(t0);
Texture2D HeightMap : register(t1);
Texture2D depthMapTexture[2] : register(t2);
SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler[2] : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 ambient[2];
    float4 diffuse[2];
    float4 direction[2];
    float4 position[2];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float maxHeight : TEXCOORD1;
    float4 lightViewPos[2] : TEXCOORD2;
};

float getDisplacement(float2 uv, float height)
{
   
   float offset = HeightMap.SampleLevel(diffuseSampler, uv, 0).r;
   return offset;
    
        
   
}


float3 CalculateNormals(float2 uv, float maxHeight)
{
   
    float2 dimensions;
    HeightMap.GetDimensions(dimensions.x, dimensions.y);
    float uvOff = 1.0f / min(dimensions.x, dimensions.y);
    float hTop = getDisplacement(float2(uv.x, uv.y + uvOff), maxHeight);
    float hBottom = getDisplacement(float2(uv.x, uv.y - uvOff), maxHeight);
    float hRight = getDisplacement(float2(uv.x + uvOff, uv.y), maxHeight);
    float hLeft = getDisplacement(float2(uv.x - uvOff, uv.y), maxHeight);
    
    float step = 100.0f * uvOff;
    float3 tan = normalize(float3(2.0f * step, hRight - hLeft, 0));
    float3 bitan = normalize(float3(0, hTop - hBottom, 2.0f * step));
    return cross(bitan, tan);

}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    return saturate(diffuse * intensity); // return final colour
}

// Is the geometry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, SamplerState sSampler, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(sSampler, uv).r;
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
    float shadowMapBias = 0.05f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
    input.normal = CalculateNormals(input.tex, input.maxHeight);
    input.normal = normalize(input.normal);
   
    
    // Prepare light view positions and projective coordinates


    // Initialise shadow factor
    float shadowOutput = 1.f;


    for (int i = 0; i < 2; i++)
    {
        // Combine ambient light contributions
        colour += ambient[i];
        // get texture coordinates
        float2 pTexCoords = getProjectiveCoords(input.lightViewPos[i]);

        // Check if in shadow for this light
        if (hasDepthData(pTexCoords) && isInShadow(depthMapTexture[i], shadowSampler[i], pTexCoords, input.lightViewPos[i], shadowMapBias))
        {
            shadowOutput *= 0.f; // In shadow
        }
    }
    
    // If not in shadow, calculate lighting contribution for both lights
    if (shadowOutput > 0.f)
    {
        for (int i = 0; i < 2; i++)
        {
            colour += calculateLighting(-direction[i], input.normal, diffuse[i]);
        }
    }

    // Add combined ambient light and apply texture
   
    
   
    return saturate(colour) * textureColour;

}