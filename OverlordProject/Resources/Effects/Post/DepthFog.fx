//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gDepthSRV;
Texture2D gColorSRV;
float3 gFogColor = float3(1,0,0);
float gFogStrength = 1.5f;
float gFogFalloff = 3;
float4x4 gMatrixProjInverse;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState DepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

/// Create Rasterizer State (Backface culling) 
RasterizerState CullBack
{
    CullMode = BACK;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
    output.Position = float4(input.Position, 1);
	// Set the TexCoord
    output.TexCoord = input.TexCoord;
	return output;
}


//PIXEL SHADER
//------------

float4 PS(PS_INPUT input): SV_Target
{
    float nonLinearDepth = (gDepthSRV.Sample(samPoint, input.TexCoord));
    float4 ndcCoords = float4(0, 0, nonLinearDepth, 1.0f);
    float4 viewCoords = mul(ndcCoords, gMatrixProjInverse);
    float linearDepth = viewCoords.z / viewCoords.w;
    linearDepth = clamp((linearDepth / (gFogFalloff)), 0, 1);
    
    float3 ogColor = (gColorSRV.Sample(samPoint, input.TexCoord)).rgb;
    return float4(ogColor * (1 - linearDepth) + (linearDepth * gFogColor * gFogStrength) * (linearDepth), 1);
}

BlendState gBS_NoBlending
{
    BlendEnable[0] = FALSE;
    RenderTargetWriteMask[0] = 0x0f;
};

//TECHNIQUE
//---------
technique11 DepthFog
{
    pass P0
    {
		// Set states...
        SetBlendState(gBS_NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(CullBack);
        SetDepthStencilState(DepthWrite,0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader( NULL );
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}