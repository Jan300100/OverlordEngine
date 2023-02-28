#include "PbrBase.fx"

float gWorldUVScale = 0.05f;
Texture2D gNoiseTexture;
float gNoiseHeight = 10.0f;
float gNoiseUVScale = 0.01f;

//DISPLACEMENT
//************
float gTessFactor = 3;
float gDisplacementAmount = 1;

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct VS_Instance
{
    float2 WorldPositionXZ : INSTANCE2F;
};

//rasterizerstates
RasterizerState gRS_Filled 
{ 
	CullMode = NONE; 
};

RasterizerState gRS_WireFrame
{
    CullMode = NONE;
    FillMode = WIREFRAME;
};

struct VS_Output
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    float Tesselation : TESS;
};

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

struct HullOut
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
};


//MAIN VERTEX SHADER
//******************
VS_Output MainVS(VS_Input input, VS_Instance instance) {
	
	VS_Output output = (VS_Output)0;
	
    //put the groundTile in the world
    input.Position.x += instance.WorldPositionXZ.x;
    input.Position.z += instance.WorldPositionXZ.y;
    
    output.Position = float4(input.Position, 1.0);
    float4 worldPosition = mul(float4(input.Position, 1.0), gMatrixWorld);
    output.Normal = input.Normal;
    output.Tangent = mul(input.Tangent, (float3x3)gMatrixWorld);
	output.TexCoord = input.TexCoord;
    output.Tesselation = gTessFactor;
	return output;
}

//HULL SHADER
//***********
PatchTess PatchConstFunc(InputPatch<VS_Output, 3> patch,
                  uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    // Average tess factors along edges, 
    pt.EdgeTess[0] = 0.5f * (patch[1].Tesselation + patch[2].Tesselation);
    pt.EdgeTess[1] = 0.5f * (patch[2].Tesselation + patch[0].Tesselation);
    pt.EdgeTess[2] = 0.5f * (patch[0].Tesselation + patch[1].Tesselation);
	//pick an edge tess factor for the interior tessellation.
    pt.InsideTess = pt.EdgeTess[0];

    return pt;
}

[domain("tri")]
[partitioning("pow2")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstFunc")]
HullOut MainHS(InputPatch<VS_Output, 3> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HullOut output;
	
	// Pass through shader.
    output.Position = p[i].Position;
    output.Normal = p[i].Normal;
    output.Tangent = p[i].Tangent;
    output.TexCoord = p[i].TexCoord;
	
    return output;
}


//DOMAIN SHADER
//*************


// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
PS_Input MainDS(PatchTess patchTess,
             float3 bary : SV_DomainLocation,
             const OutputPatch<HullOut, 3> tri)
{
    PS_Input output;

    // Interpolate patch attributes to generated vertices.
    output.Position = bary.x * tri[0].Position + bary.y * tri[1].Position + bary.z * tri[2].Position;
    output.Normal = bary.x * tri[0].Normal + bary.y * tri[1].Normal + bary.z * tri[2].Normal;
    output.Tangent = bary.x * tri[0].Tangent + bary.y * tri[1].Tangent + bary.z * tri[2].Tangent;
    output.TexCoord = bary.x * tri[0].TexCoord + bary.y * tri[1].TexCoord + bary.z * tri[2].TexCoord;
    
    // DISPLACEMENT MAPPING
    output.Normal = normalize(output.Normal);

    //calculate world space position in order to find new texturecoordinates
    output.WorldPosition = mul(float4(output.Position), gMatrixWorld);
    
    //update texcoords based on worldspace
    output.TexCoord = float2(output.WorldPosition.x / gWorldUVScale, output.WorldPosition.z / gWorldUVScale);

    //sample from displacementmap
    float h = gRDAM.SampleLevel(gTextureSampler, output.TexCoord, 0).g;

    //sample from noise map for terrainheight
    h += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(output.WorldPosition.x / gNoiseUVScale, output.WorldPosition.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    output.Position += float4((gDisplacementAmount * (h)) * output.Normal, 0);

    //put positions in the corrects space
    output.WorldPosition = mul(float4(output.Position), gMatrixWorld);
    output.Position = mul(output.Position, gMatrixWVP);
    output.Normal = mul(output.Normal, (float3x3) gMatrixWorld);

    return output;
}

// Default Technique
technique11 tDefault {
	pass p0 {
        //SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
		SetRasterizerState(gRS_Filled);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(CompileShader(hs_5_0, MainHS()));
        SetDomainShader(CompileShader(ds_5_0, MainDS()));
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}

technique11 tWireframe
{
    pass p0
    {
        //SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_WireFrame);
        SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(CompileShader(hs_5_0, MainHS()));
        SetDomainShader(CompileShader(ds_5_0, MainDS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}