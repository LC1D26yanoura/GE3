#include "Object3d.hlsl"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 world;

};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct Material
{
    float4 color;
    int enableLighting;
};


VertexShanderOutput main(VertexShaderInput input)
{
  
    VertexShanderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.world));
    
    return output;
};