#include "MapRenderer.h"

#include <memory>

#include "WinApiAndDXHeaders.h"
#include "WindowUtils.h"
#include "UtilsMath.h"
#include "UtilsLinearAlgebra.h"
#include "UploadBuffer.h"

#include "D3DClass.h"
#include "CameraManager.h"

namespace vox::ren
{

struct ConstBuffer
{
    DirectX::XMFLOAT4X4 mat_view_proj;
    DirectX::XMFLOAT4 cam_pos;
};
static ConstBuffer const_buffer_;
static DirectX::XMFLOAT4X4 mat_view_;
static DirectX::XMFLOAT4X4 mat_proj_;

static ComPtr<ID3D12RootSignature> root_signature_;
static ComPtr<ID3D12DescriptorHeap> cbv_heap_;
static std::unique_ptr<UploadBuffer<ConstBuffer>> const_buffer_upload_ = nullptr;
static ComPtr<ID3DBlob> vertex_shader_blob_;
static ComPtr<ID3DBlob> geometry_shader_blob_;
static ComPtr<ID3DBlob> pixel_shader_blob_;
static ComPtr<ID3D12PipelineState> pipeline_state_object_;
static D3D12_INPUT_ELEMENT_DESC input_layouts_[2];

static void BuildDescriptorHeaps();
static void BuildConstantBuffers();
static void BuildRootSignature();
static void BuilsShadersAndInputLayout();
static void BuildPSO();

void MRInit()
{
    const_buffer_.mat_view_proj = utils::LAIdentity();
    mat_view_ = utils::LAIdentity();
    mat_proj_ = utils::LAIdentity();

    BuildDescriptorHeaps();
    BuildConstantBuffers();
    BuildRootSignature();
    BuilsShadersAndInputLayout();
    BuildGeometry();
    BuildPSO();

    DCExecuteMainCommandList();
}

void MRClear()
{

}

void MROnResize()
{
    DirectX::XMMATRIX proj_mat = DirectX::XMMatrixPerspectiveFovLH(
        0.25f * utils::UM_PI, DCGetAspectRatio(), 1.0f, 1000.0f);
    DirectX::XMStoreFloat4x4(&mat_proj_, proj_mat);
}

void MRUpdate()
{
    DirectX::XMMATRIX view_mat = ett::CMGetCamViewMat();
    DirectX::XMMATRIX proj_mat = DirectX::XMLoadFloat4x4(&mat_proj_);
    DirectX::XMMATRIX view_proj_mat = view_mat * proj_mat;

    DirectX::XMStoreFloat4x4(&const_buffer_.mat_view_proj,
                             DirectX::XMMatrixTranspose(view_proj_mat));
    DirectX::XMStoreFloat4(&const_buffer_.cam_pos, ett::CMGetCamPos());
    const_buffer_upload_->CopyData(0, const_buffer_);
}

void MRRender(float delta_time)
{
    DCResetMainCommandList(pipeline_state_object_.Get());
    
}

static void BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc;
    cbv_heap_desc.NumDescriptors = 1;
    cbv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbv_heap_desc.NodeMask = 0;
    ThrowIfFailed(DCGetD3DDevice().CreateDescriptorHeap(
        &cbv_heap_desc, IID_PPV_ARGS(&cbv_heap_)));
}

static void BuildConstantBuffers()
{
    const_buffer_upload_ = std::make_unique<UploadBuffer<ConstBuffer>>(
        DCGetD3DDevice(), 1, true);
    const UINT const_buffer_size = (sizeof(ConstBuffer) + 255) & ~255;
    D3D12_GPU_VIRTUAL_ADDRESS const_buffer_address =
        const_buffer_upload_->Resource()->GetGPUVirtualAddress();
    
    int box_const_buffer_index = 0;
    const_buffer_address += box_const_buffer_index * const_buffer_size;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
    cbv_desc.BufferLocation = const_buffer_address;
    cbv_desc.SizeInBytes = const_buffer_size;

    DCGetD3DDevice().CreateConstantBufferView(
        &cbv_desc, cbv_heap_->GetCPUDescriptorHandleForHeapStart());

}

static void BuildRootSignature()
{
    CD3DX12_ROOT_PARAMETER slot_root_parameter[1];
    CD3DX12_DESCRIPTOR_RANGE cbv_table;
    cbv_table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slot_root_parameter[0].InitAsDescriptorTable(1, &cbv_table);

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc(
        1, slot_root_parameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serialized_root_signature = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(
        &root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
        serialized_root_signature.GetAddressOf(), error_blob.GetAddressOf());
    if (error_blob != nullptr)
    {
        OutputDebugStringA((char*)error_blob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(DCGetD3DDevice().CreateRootSignature(
        0,
        serialized_root_signature->GetBufferPointer(),
        serialized_root_signature->GetBufferSize(),
        IID_PPV_ARGS(&root_signature_)));
}

static void BuilsShadersAndInputLayout()
{
    HRESULT hr = S_OK;

    vertex_shader_blob_ = d3dUtil::CompileShader(
        L"GameData\\Shaders\\basic_pass_VS.hlsl", nullptr, "VS", "vs_5_0");
    geometry_shader_blob_ = d3dUtil::CompileShader(
        L"GameData\\Shaders\\basic_pass_GS.hlsl", nullptr, "GS", "gs_5_0");
    pixel_shader_blob_ = d3dUtil::CompileShader(
        L"GameData\\Shaders\\basic_pass_PS.hlsl", nullptr, "PS", "ps_5_0");

    input_layouts_[0] = { "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, 0,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    input_layouts_[1] = { "MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, 4,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

static void BuildGeometry()
{

}

static void BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
    ZeroMemory(&pso_desc, sizeof(pso_desc));
    pso_desc.InputLayout = { input_layouts_, ARRAYSIZE(input_layouts_) };
    pso_desc.pRootSignature = root_signature_.Get();
    pso_desc.VS =
    {
        reinterpret_cast<BYTE*>(vertex_shader_blob_->GetBufferPointer()),
        vertex_shader_blob_->GetBufferSize()
    };
    pso_desc.GS =
    {
        reinterpret_cast<BYTE*>(geometry_shader_blob_->GetBufferPointer()),
        geometry_shader_blob_->GetBufferSize()
    };
    pso_desc.PS =
    {
        reinterpret_cast<BYTE*>(pixel_shader_blob_->GetBufferPointer()),
        pixel_shader_blob_->GetBufferSize()
    };
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso_desc.SampleDesc.Count = DCGetTo4xMsaaState() ? 4 : 1;
    pso_desc.SampleDesc.Quality = DCGetTo4xMsaaState() ? (DCGet4xMsaaQuality() - 1) : 0;
    pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ThrowIfFailed(DCGetD3DDevice().CreateGraphicsPipelineState(
        &pso_desc, IID_PPV_ARGS(&pipeline_state_object_)));
}


}