Texture2D normalSceneTexture : register(t0);
Texture2D blurSceneTexture : register(t1);
Texture2D depthSceneTexture : register(t2);

SamplerState Sampler0 : register(s0);

cbuffer DepthBuffer : register(b0)
{
    float distance; 
    float nearValue;
    float farValue;
    float offset;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 colour = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float4 normalScene = normalSceneTexture.Sample(Sampler0, input.tex); // unblurred scene (firstPass)
    float4 blurScene = blurSceneTexture.Sample(Sampler0, input.tex); // blurred scene (taken after hBlur and vBlur) 
    float depthValue = depthSceneTexture.Sample(Sampler0, input.tex).r; // second depth pass (povDepthPass)
    float centerTexel = depthSceneTexture.Sample(Sampler0, float2(0.5f, 0.5f)).r; // midpoint of the scene 

    float coc = abs(depthValue - centerTexel) * distance; // circle of confusion formula to determine outward blur

    colour = lerp(normalScene, blurScene, coc); 
    return colour;

}