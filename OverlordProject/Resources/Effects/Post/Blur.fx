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
	// Step 1: find the dimensions of the texture (the texture has a method for that)	
    int w, h;
    gTexture.GetDimensions(w, h);
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
    float dx = 1.0f / w;
    float dy = 1.0f / h;
	// Step 3: Create a double for loop (5 iterations each)
    
    float4 finalColor = (float4)0;
    
    for (int i = -2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
	        //  Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
            float2 offset = float2(i * dx, j * dy);
            //	Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
            finalColor += gTexture.Sample(samPoint, input.TexCoord + offset);
        }
    }
	// Step 4: Divide the final color by the number of passes (in this case 5*5)
    finalColor /= 25.0f;
	// Step 5: return the final color
    return float4(finalColor.rgb, 1);
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
        SetRasterizerState(CullBack);
        SetDepthStencilState(DepthWrite,0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}