#include "stdafx.h"
#include "MaterialBase.h"

#include <dxcapi.h>
#include <d3d11shader.h>
#include <StringHelper.h>

// https://rtarun9.github.io/blogs/shader_reflection/#what-is-shader-reflection-

MaterialBase::MaterialBase()
{
    IDxcUtils* utils;
    IDxcCompiler3* compiler;
    IDxcIncludeHandler* includeHandler;
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    utils->CreateDefaultIncludeHandler(&includeHandler);

    std::vector<LPCWSTR> compilationArguments
    {
        L"-E",
        L"PSMain",
        L"-T",
        L"ps_5_0",
        DXC_ARG_PACK_MATRIX_ROW_MAJOR,
        DXC_ARG_WARNINGS_ARE_ERRORS,
        DXC_ARG_ALL_RESOURCES_BOUND,
        L"-Qstrip_debug",
        L"-Qstrip_reflect"
    };

    // Indicate that the shader should be in a debuggable state if in debug mode.
    // Else, set optimization level to 3.
#ifdef _DEBUG
    compilationArguments.push_back(DXC_ARG_DEBUG);
#else
    compilationArguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

    // Load the shader source file to a blob.
    IDxcBlobEncoding* sourceBlob{};
    utils->LoadFile(L"PosNormCol3D.hlsl", nullptr, &sourceBlob);

    DxcBuffer sourceBuffer{};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = 0u;

    // Compile the shader.
    IDxcResult* compiledShaderBuffer{};
    HRESULT hr = compiler->Compile(&sourceBuffer, compilationArguments.data(), compilationArguments.size(), includeHandler, IID_PPV_ARGS(&compiledShaderBuffer));
    Logger::LogHResult(hr, L"Failed To Compile Shader");

    IDxcBlobUtf8* errors{};
    hr = compiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    Logger::LogHResult(hr, L"Failed To Get DXC_OUT_ERRORS");

    if (errors && errors->GetStringLength() > 0)
    {
        const LPCSTR errorMessage = errors->GetStringPointer();
        Logger::LogError(StringHelpers::StringToWString(errorMessage));
    }

    // Get shader reflection data.
    IDxcBlob* reflectionBlob{};
    hr = compiledShaderBuffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
    Logger::LogHResult(hr, L"Failed To Get DXC_OUT_REFLECTION");

    DxcBuffer reflectionBuffer{};
    reflectionBuffer.Ptr = reflectionBlob->GetBufferPointer();
    reflectionBuffer.Size = reflectionBlob->GetBufferSize();
    reflectionBuffer.Encoding = 0u;

    ID3D11ShaderReflection* shaderReflection{};
    utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
    D3D11_SHADER_DESC shaderDesc{};
    shaderReflection->GetDesc(&shaderDesc);

    // only for vertex shader
    std::vector<LPCSTR> inputElementSemanticNames;
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;

    inputElementSemanticNames.reserve(shaderDesc.InputParameters);
    inputElementDescs.reserve(shaderDesc.InputParameters);
    for (int i = 0; i < shaderDesc.InputParameters; i++)
    {
        D3D11_SIGNATURE_PARAMETER_DESC signatureParameterDesc{};
        shaderReflection->GetInputParameterDesc(i, &signatureParameterDesc);
        inputElementSemanticNames.emplace_back(signatureParameterDesc.SemanticName);

        D3D11_INPUT_ELEMENT_DESC inputEl{};
        inputEl.SemanticName = inputElementSemanticNames.back();
        inputEl.SemanticIndex = signatureParameterDesc.SemanticIndex;
        inputEl.Format = signatureParameterDesc.Mask;
        
        /*
        NEEDS CONVERSION : This Mask represents the number of components the element is using in binary form.
        In other words, a float3 would be represented by 7 (b'111), 
        a float2 by 3 (b'11) and so on.
        */

        inputEl.InputSlot = 0u; // used if there are mulitple vertexbuffers
        inputEl.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        inputEl.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputEl.InstanceDataStepRate = 0u;
        
        inputElementDescs.push_back(inputEl);
    }

    ID3D11Device* pDevice; //temp device, needs to be replaced by gamecontext, renderer dvice
    // then all this needs to be behind a GA:: layer

    // vulkan implementation would transpile to spir-v

    pDevice->CreateInputLayout(
        inputElementDescs.data(),
        inputElementDescs.size(),
        sourceBlob->GetBufferPointer(),
        sourceBlob->GetBufferSize(),
        &m_pInputLayout);
}

MaterialBase::~MaterialBase()
{

}
