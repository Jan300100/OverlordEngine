//GLOBAL Variables
//***************
// The World View Projection Matrix
float4x4 gMatrixWVP : WORLDVIEWPROJECTION;
// The ViewInverse Matrix - the third row contains the camera position!
float4x4 gMatrixViewInverse : VIEWINVERSE;
// The World Matrix
float4x4 gMatrixWorld : WORLD;
// PI constant
float gPi = 3.14159265f;

float gWorldUVScale = 0.05f;
Texture2D gNoiseTexture;
float gNoiseHeight = 10.0f;
float gNoiseUVScale = 0.01f;

//STATES
//******
BlendState gBS_EnableBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
};

RasterizerState gRS_Filled 
{ 
	CullMode = NONE; 
};

RasterizerState gRS_WireFrame
{
    CullMode = NONE;
    FillMode = WIREFRAME;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

DepthStencilState DisableDepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};


//SAMPLER STATES
//**************
SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
 	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

//LIGHT
//*****
float3 gLightDirection : DIRECTION = float3(0.577f, -0.577f, 0.577f);
float gLightIntensity = 5.0f;

//ALBEDO
//*******
bool gUseAlbedoTexture;
Texture2D gAlbedoTexture;
float3 gAlbedoColor = float3(1, 1, 1);



//ROUGH-DISP-AO-METAL packed RDAM
Texture2D gRDAM;
//ROUGHNESS
//*********
bool gUseRoughnessMap;
float gRoughness;
//DISPLACEMENT
//************
float gMaxTessDistance = 100;
float gMaxTessFactor = 3;
//AO
//**
bool gUseAO = false;
//METAL
//*****
bool gIsMetal = false;
bool gUseMetalnessMap = false;


float gDisplacementAmount = 1;

//NORMAL MAPPING
//**************
bool gUseNormalMap = false;
bool gFlipGreenChannel = false;
Texture2D gNormalMap;

//ENVIRONMENT MAPPING
//*******************

bool gUseTextureEnvironment = false;

TextureCube gCubeEnvironment;

float gRefractionIndex = 0.3f;




//ENVIRONMENT MAPPING FUNCTION
float3 CalculateEnvironment(float3 viewDirection, float3 normal)
{
    float3 environmentColor = float3(0, 0, 0);
	
    if (gUseTextureEnvironment)
    {
		//reflected
        float3 reflected = reflect(viewDirection, normal);
        reflected = gCubeEnvironment.Sample(gTextureSampler, reflected).rgb;
        reflected *= 1;
		
		//refracted
        float3 refracted = refract(viewDirection, normal, gRefractionIndex);
        refracted = gCubeEnvironment.Sample(gTextureSampler, refracted).rgb;
        refracted *= 0;
		
        return reflected + refracted;
    }
		
    return environmentColor;
	
}

//STRUCTS
//***********
struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
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

struct DomainOut
{
    float4 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_POSITION;
};

//MAIN VERTEX SHADER
//******************
VS_Output MainVS(VS_Input input) {
	
	VS_Output output = (VS_Output)0;
	
    output.Position = float4(input.Position, 1.0);
    float4 worldPosition = mul(float4(input.Position, 1.0), gMatrixWorld);
    output.Normal = input.Normal;
    output.Tangent = mul(input.Tangent, (float3x3)gMatrixWorld);

	output.TexCoord = input.TexCoord;
	
	//TESSELATION FACTOR PER VERTEX
	//calculate TesselationValue based on distance to the camera
    float d = distance(worldPosition.xyz, gMatrixViewInverse[3].xyz);
	
	// Normalized tessellation factor. 
    float tess = saturate(d / gMaxTessDistance);
    tess = 1 - tess;

    // Rescale [0,1] --> [gMinTessFactor = max/2, gMaxTessFactor].
    output.Tesselation = (gMaxTessFactor / 2.0f) + tess * (gMaxTessFactor - (gMaxTessFactor / 2.0f));
	
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
DomainOut MainDS(PatchTess patchTess,
             float3 bary : SV_DomainLocation,
             const OutputPatch<HullOut, 3> tri)
{
    DomainOut output;

    // Interpolate patch attributes to generated vertices.
    output.Position = bary.x * tri[0].Position + bary.y * tri[1].Position + bary.z * tri[2].Position;
    output.Normal = bary.x * tri[0].Normal + bary.y * tri[1].Normal + bary.z * tri[2].Normal;
    output.Tangent = bary.x * tri[0].Tangent + bary.y * tri[1].Tangent + bary.z * tri[2].Tangent;
    output.TexCoord = bary.x * tri[0].TexCoord + bary.y * tri[1].TexCoord + bary.z * tri[2].TexCoord;
    
    // DISPLACEMENT MAPPING
    output.Normal = normalize(output.Normal);

    //Put Positions in the right space
    output.WorldPosition = mul(float4(output.Position), gMatrixWorld);
    
    //update texcoords
    output.TexCoord = float2(output.WorldPosition.x / gWorldUVScale, output.WorldPosition.z / gWorldUVScale);
    float h = gRDAM.SampleLevel(gTextureSampler, output.TexCoord, 0).g;
    // Offset vertex along normal.
    h += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(output.WorldPosition.x / gNoiseUVScale, output.WorldPosition.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    
    output.Position += float4((gDisplacementAmount * (h)) * output.Normal, 0);

    output.WorldPosition = mul(float4(output.Position), gMatrixWorld);
    output.Position = mul(output.Position, gMatrixWVP);
    output.Normal = mul(output.Normal, (float3x3) gMatrixWorld);
    


    return output;
}


//MAIN PIXEL SHADER
//*****************
float4 MainPS(DomainOut input) : SV_TARGET 
{
    
    float3 viewDirection = normalize(input.WorldPosition.xyz - gMatrixViewInverse[3].xyz);
    float3 lDir = normalize(gLightDirection);
	
	//NORMAL
    float3 newNormal = normalize(input.Normal);
    if (gUseNormalMap)
    {
        float3 newTangent = normalize(input.Tangent);
        float3 binormal = normalize(cross(newTangent, newNormal));
        if (gFlipGreenChannel)
        {
            binormal = -binormal;
        }
        float3x3 localAxis = float3x3(newTangent, binormal, newNormal);
		
        if (gUseNormalMap)
            newNormal = gNormalMap.Sample(gTextureSampler, input.TexCoord).xyz;
        newNormal = 2.0f * newNormal - 1.0f; //remap to [-1,1]
 
        newNormal = mul(newNormal, localAxis);
    }
	//DIFFUSE
    float4 diffColor = float4(gAlbedoColor,1);
    if (gUseAlbedoTexture)
    {
        diffColor = (gAlbedoTexture.Sample(gTextureSampler, input.TexCoord));
    }
    
    diffColor.rgb /= gPi;
	//SPECULAR
	
	//Determine F0 value
    float3 f0 = float3(0.04, 0.04, 0.04);
    float metal = gIsMetal;
    if (gIsMetal)
    {
        if (gUseMetalnessMap)
        {
			//sample from metalnessmap
            metal = gRDAM.Sample(gTextureSampler, input.TexCoord).a;
            if (metal == 1.0f)
            {
                if (gUseAlbedoTexture)
                {               
				//sample from albedomap if metal, else stay 0.04 (non metal value);
                f0 = gAlbedoTexture.Sample(gTextureSampler, input.TexCoord).rgb;
                }
                else
                {
                    f0 = gAlbedoColor;
                }
            }
        }
        else
        {
            if (gUseAlbedoTexture)
            {
				//sample from albedomap if metal, else stay 0.04 (non metal value);
                f0 = gAlbedoTexture.Sample(gTextureSampler, input.TexCoord).rgb;
            }
            else
            {
                f0 = gAlbedoColor;
            }
        }
    }
	//calc halfvector between viewDir and lightDir
    float3 hv = (viewDirection + lDir) / length(viewDirection + lDir);
	//calculate Fresnel (F) (Schlick)
    float3 fresnel = f0 + ((float3) 1 - f0) * pow(1 - dot(hv, viewDirection), 5); //5 magic number
	
	//Calculate Normal Distribution (D)
    float roughness = gRoughness;
    if (gUseRoughnessMap)
    {
        roughness = gRDAM.Sample(gTextureSampler, input.TexCoord).r;
    }
	//trowbridge_reitz_ggx 
    float rPow2 = pow(roughness, 2);
    float rPow4 = pow(rPow2, 2);
    float distribution = saturate(rPow4 / (gPi * pow(pow(dot(-newNormal, hv), 2) * (rPow4 - 1) + 1, 2)));
	
	
	//Calculate Geometry //Smith_schlick
    float geometry = dot(newNormal, viewDirection) * dot(newNormal, lDir);
	
	//cook-torrance
    float3 specular = (fresnel * geometry * distribution) / (4 * dot(-viewDirection, newNormal) * dot(-lDir, newNormal));
	
	//ENVIRONMENT MAPPING
    float3 environmentColor = CalculateEnvironment(viewDirection, newNormal);
  
    //this is probably wrong
    specular += (environmentColor * fresnel * (1-roughness)) / gPi;
	
	//Ambient occlusion
    float aoValue = 1;
    if (gUseAO)
        aoValue = gRDAM.Sample(gTextureSampler, input.TexCoord).b;
	
    float lambertDot = saturate(dot(-newNormal, lDir));
	
	//FINAL COLOR CALCULATION
    float3 finalColor = specular;
    if (metal == 0.0f) //magic metal number
    {
		//not a metal
        float diffuseR = 1 - fresnel.r;
        finalColor = ((diffColor.rgb * diffuseR) + specular);
    }
    finalColor *= aoValue * lambertDot * gLightIntensity;
	//OPACITY
    float opacity = diffColor.a;
	
    return float4(finalColor, opacity);
}

// Default Technique
technique11 tNoBlending {
	pass p0 {
        SetDepthStencilState(EnableDepth, 0);
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
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_WireFrame);
        SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(CompileShader(hs_5_0, MainHS()));
        SetDomainShader(CompileShader(ds_5_0, MainDS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}