//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;
float gBrightness;
float gContrast;
float gHue;
float gSaturation;

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
float3x3 QuaternionToMatrix(float4 quat)
{
    float3 cross = quat.yzx * quat.zxy;
    float3 square= quat.xyz * quat.xyz;
    float3 wimag = quat.w * quat.xyz;

    square = square.xyz + square.yzx;

    float3 diag = 0.5 - square;
    float3 a = (cross + wimag);
    float3 b = (cross - wimag);

    return float3x3(
    2.0 * float3(diag.x, b.z, a.y),
    2.0 * float3(a.z, diag.y, b.x),
    2.0 * float3(b.y, a.x, diag.z));
}

const float3 lumCoeff = float3(0.2125, 0.7154, 0.0721);

float4 PS(PS_INPUT input): SV_Target
{
    //initialcolor
    float4 outputColor = gTexture.Sample(samPoint, input.TexCoord);
           
    float3 root3 = float3(0.57735, 0.57735, 0.57735);
    float half_angle = 0.5 * radians(gHue); // Hue is radians of 0 tp 360 degree
    float4 rot_quat = float4( (root3 * sin(half_angle)), cos(half_angle));
    float3x3 rot_Matrix = QuaternionToMatrix(rot_quat);     
    outputColor.rgb = mul(rot_Matrix, outputColor.rgb);
    outputColor.rgb = (outputColor.rgb - 0.5) *(gContrast + 1.0) + 0.5;  
    outputColor.rgb = (outputColor.rgb + outputColor.rgb * gBrightness);
    float3 intensity = float(dot(outputColor.rgb, lumCoeff));
    outputColor.rgb = lerp(intensity, outputColor.rgb, gSaturation );    
    return outputColor;
}

//TECHNIQUE
//---------
technique11 FiltersTechnique
{
    pass P0
    {
		// Set states...
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(CullBack);
        SetDepthStencilState(DepthWrite,0);
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader( NULL );
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}