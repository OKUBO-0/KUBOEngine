#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer PostEffectParameters : register(b0)
{
    float2 gRadialBlurCenter;
    float gRadialBlurWidth;
    float gRadialBlurSamples;
    float gVignettePower;
    float gVignetteMultiplier;
    float gBoxFilterStepScale;
    float gBoxFilterBlend;
    float gOutlineStrength;
    float gOutlineThreshold;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    // ---- 手打ち定数 ----
    float2 direction = gRadialBlurCenter - input.texcoord; // 中心からの方向
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    int sampleCount = max(1, (int)gRadialBlurSamples);

    // サンプルの合成
    for (int i = 0; i < sampleCount; ++i)
    {
        float2 offset = direction * gRadialBlurWidth * i;
        float2 sampleUV = input.texcoord + offset;
        outputColor += gTexture.Sample(gSampler, sampleUV).rgb;
    }

    // 平均化
    outputColor /= sampleCount;

    PixelShaderOutput output;
    output.color = float4(outputColor, 1.0f);
    return output;
}
