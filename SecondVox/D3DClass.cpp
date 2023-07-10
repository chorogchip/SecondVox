#include "D3DClass.h"


#include "WinApiAndDXHeaders.h"
#include "WindowUtils.h"

namespace vox::ren
{

static bool to_4x_MSAA_state = false;
static UINT num_4x_MSAA_quality = 0;

static bool is_paused_ = false;

static HWND hWnd_;
static HINSTANCE hInst_;

static Microsoft::WRL::ComPtr<IDXGIFactory4> dxgi_factory_;
static Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
static Microsoft::WRL::ComPtr<ID3D12Device> d3d_device_;

static Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
static UINT64 current_fence_ = 0;

static Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_;
static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> direct_cmdlist_alloc_;
static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list_;

static constexpr int SWAPCHAIN_BUFFER_COUNT = 2;
static int curr_back_buffer_ = 0;
static Microsoft::WRL::ComPtr<ID3D12Resource> swap_chain_buffer_[SWAPCHAIN_BUFFER_COUNT];
static Microsoft::WRL::ComPtr<ID3D12Resource> depth_stencil_buffer_;

static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTV_Heap_;
static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSV_Heap_;
static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SRV_Heap_;

static Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;

static Microsoft::WRL::ComPtr<ID3DBlob> geometry_shader_, vertex_shader_, pixel_shader_;
static Microsoft::WRL::ComPtr<ID3D12PipelineState> PSO_;

static D3D12_INPUT_ELEMENT_DESC input_layout_[] =
{
    { "POSITION", 0, DXGI_FORMAT_R16_UINT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "POSITION", 1, DXGI_FORMAT_R16_UINT, 0, 2, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "SIZE_MATERIAL", 0, DXGI_FORMAT_R16_UINT, 0, 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};


static D3D12_VIEWPORT screen_viewport_; 
static D3D12_RECT scissor_rect_;

static UINT RTV_descriptor_size_ = 0;
static UINT DSV_descriptor_size_ = 0;
static UINT CBV_SRV_UAV_descriptor_size_ = 0;

// Derived class should set these in derived constructor to customize starting values.
static constexpr D3D_DRIVER_TYPE d3d_driver_type_ = D3D_DRIVER_TYPE_HARDWARE;
static constexpr DXGI_FORMAT back_buffer_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
static constexpr DXGI_FORMAT depth_stencil_format_ = DXGI_FORMAT_D24_UNORM_S8_UINT;
static int32_t client_width_;
static int32_t client_height_;

static void LogAdapters();
static void CreateCommandObjects();
static void CreateSwapChain();
static void CreateRTVandDSVDecriptorHeaps();
static void FlushCommandQueue();

static ID3D12Resource* GetCurrentBackBuffer();
static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView();
static D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();



bool DCIsDeviceExist()
{
    return d3d_device_ != nullptr;
}
float DCGetAspectRatio()
{
    return (float)client_width_ / (float)client_height_;
}
void DCGetWidthAndHeight(int32_t* wh_dest)
{
    wh_dest[0] = client_width_;
    wh_dest[1] = client_height_;
}
void DCSetWidthAndHeight(int32_t width, int32_t height)
{
    client_width_ = width;
    client_height_ = height;
}
bool DCGetTo4xMsaaState()
{
    return to_4x_MSAA_state;
}
void DCSetTo4xMsaaState(bool value)
{
    if (to_4x_MSAA_state != value)
    {
        to_4x_MSAA_state = value;
        CreateSwapChain();
        DCOnResize();
    }
}

bool DCIsPaused()
{
    return is_paused_;
}
void DCSetPaused(bool value)
{
    is_paused_ = value;
}

bool DCInit(HWND hWnd, HINSTANCE hInstance)
{
    hWnd_ = hWnd;
    hInst_ = hInstance;

#ifdef M_DEBUG
    {
        ComPtr<ID3D12Debug> debug_controller;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
        debug_controller->EnableDebugLayer();
    }
#endif

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory_)));

    HRESULT hardware_result = D3D12CreateDevice(
        nullptr, D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&d3d_device_));

    if (FAILED(hardware_result))
    {
        ComPtr<IDXGIAdapter> warp_adapter;
        ThrowIfFailed(dxgi_factory_->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));
        ThrowIfFailed(D3D12CreateDevice(
            warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&d3d_device_)));
    }

    ThrowIfFailed(d3d_device_->CreateFence(
        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

    RTV_descriptor_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    DSV_descriptor_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CBV_SRV_UAV_descriptor_size_ = d3d_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
    ms_quality_levels.Format = back_buffer_format_;
    ms_quality_levels.SampleCount = 4;
    ms_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    ms_quality_levels.NumQualityLevels = 0;
    ThrowIfFailed(d3d_device_->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &ms_quality_levels, sizeof(ms_quality_levels)));

    num_4x_MSAA_quality = ms_quality_levels.NumQualityLevels;
    assert(num_4x_MSAA_quality > 0 && "Unexpected MSAA quality level.");

#ifdef M_DEBUG
    LogAdapters();
#endif

    CreateCommandObjects();
    CreateSwapChain();
    CreateRTVandDSVDecriptorHeaps();

    return true;
}

void DCClear()
{
    FlushCommandQueue();
}

void DCOnResize()
{
    FlushCommandQueue();

    ThrowIfFailed(command_list_->Reset(direct_cmdlist_alloc_.Get(), nullptr));

    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i)
            swap_chain_buffer_[i].Reset();
    depth_stencil_buffer_.Reset();

    ThrowIfFailed(swap_chain_->ResizeBuffers(
        SWAPCHAIN_BUFFER_COUNT,
        client_width_, client_height_, back_buffer_format_,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    curr_back_buffer_ = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE RTV_heap_handle = RTV_Heap_->GetCPUDescriptorHandleForHeapStart();
    for (int i = 0; i < SWAPCHAIN_BUFFER_COUNT; ++i)
    {
        ThrowIfFailed(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&swap_chain_buffer_[i])));
        d3d_device_->CreateRenderTargetView(
            swap_chain_buffer_[i].Get(), nullptr, RTV_heap_handle);
        RTV_heap_handle.ptr += RTV_descriptor_size_;
    }

    D3D12_RESOURCE_DESC depth_stencil_desc;
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Alignment = 0;
    depth_stencil_desc.Width = client_width_;
    depth_stencil_desc.Height = client_height_;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;  // for create RTV, DSV
    depth_stencil_desc.SampleDesc.Count = to_4x_MSAA_state ? 4 : 1;
    depth_stencil_desc.SampleDesc.Quality = to_4x_MSAA_state ? (num_4x_MSAA_quality - 1) : 0;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heap_prop;
    heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    heap_prop.CPUPageProperty= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_prop.CreationNodeMask = 1;
    heap_prop.VisibleNodeMask = 1;

    D3D12_CLEAR_VALUE opt_clear;
    opt_clear.Format= depth_stencil_format_;
    opt_clear.DepthStencil.Depth = 1.0f;
    opt_clear.DepthStencil.Stencil = 0;

    ThrowIfFailed(d3d_device_->CreateCommittedResource(
        &heap_prop, D3D12_HEAP_FLAG_NONE,
        &depth_stencil_desc, D3D12_RESOURCE_STATE_COMMON,
        &opt_clear,
        IID_PPV_ARGS(depth_stencil_buffer_.GetAddressOf())
    ));

    D3D12_DEPTH_STENCIL_VIEW_DESC DSV_desc;
    DSV_desc.Flags = D3D12_DSV_FLAG_NONE;
    DSV_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    DSV_desc.Format = depth_stencil_format_;
    DSV_desc.Texture2D.MipSlice = 0;
    d3d_device_->CreateDepthStencilView(
        depth_stencil_buffer_.Get(), &DSV_desc,
        DSV_Heap_->GetCPUDescriptorHandleForHeapStart());
   
    D3D12_RESOURCE_BARRIER resource_barrier;
    ZeroMemory(&resource_barrier, sizeof(resource_barrier));
    resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barrier.Transition.pResource = depth_stencil_buffer_.Get();
    resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    command_list_->ResourceBarrier(1, &resource_barrier);

    ThrowIfFailed(command_list_->Close());
    ID3D12CommandList* cmd_lists[] = { command_list_.Get() };
    command_queue_->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

    FlushCommandQueue();

    screen_viewport_.TopLeftX = 0;
    screen_viewport_.TopLeftY = 0;
    screen_viewport_.Width = (float)client_width_;
    screen_viewport_.Height = (float)client_height_;
    screen_viewport_.MinDepth = 0.0f;
    screen_viewport_.MaxDepth = 1.0f;

    scissor_rect_ = { 0, 0, client_width_, client_height_ };
}


void DCDraw()
{
    ThrowIfFailed(direct_cmdlist_alloc_->Reset());
    ThrowIfFailed(command_list_->Reset(direct_cmdlist_alloc_.Get(), nullptr));

    D3D12_RESOURCE_BARRIER resource_barrier;
    ZeroMemory(&resource_barrier, sizeof(resource_barrier));
    resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barrier.Transition.pResource = GetCurrentBackBuffer();
    resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    command_list_->ResourceBarrier(1, &resource_barrier);

    command_list_->RSSetViewports(1, &screen_viewport_);
    command_list_->RSSetScissorRects(1,& scissor_rect_);

    static constexpr float colors[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
    command_list_->ClearRenderTargetView(
        GetCurrentBackBufferView(),
        colors, 0, nullptr);
    command_list_->ClearDepthStencilView(
        GetDepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr);

    const auto cbbv = GetCurrentBackBufferView();
    const auto dsv = GetDepthStencilView();
    command_list_->OMSetRenderTargets(
        1, &cbbv,
        true, &dsv);

    D3D12_RESOURCE_BARRIER resource_barrier2;
    ZeroMemory(&resource_barrier2, sizeof(resource_barrier2));
    resource_barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barrier2.Transition.pResource = GetCurrentBackBuffer();
    resource_barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    resource_barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    resource_barrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    command_list_->ResourceBarrier(1, &resource_barrier2);

    ThrowIfFailed(command_list_->Close());

    ID3D12CommandList* cmds_lists[] = { command_list_.Get() };
    command_queue_->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);

    ThrowIfFailed(swap_chain_->Present(0, 0));
    curr_back_buffer_ = (curr_back_buffer_ + 1) % SWAPCHAIN_BUFFER_COUNT;

    FlushCommandQueue();
}

static void LogAdapters()
{

}

static void CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queue_desc;
    ZeroMemory(&queue_desc, sizeof(queue_desc));
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(d3d_device_->CreateCommandQueue(
        &queue_desc, IID_PPV_ARGS(&command_queue_)));

    ThrowIfFailed(d3d_device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(direct_cmdlist_alloc_.GetAddressOf())));
    
    ThrowIfFailed(d3d_device_->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        direct_cmdlist_alloc_.Get(),
        nullptr,
        IID_PPV_ARGS(command_list_.GetAddressOf())));
    
    command_list_->Close();
}

static void CreateSwapChain()
{
    swap_chain_.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = client_width_;
    sd.BufferDesc.Height = client_height_;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = back_buffer_format_;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = to_4x_MSAA_state ? 4 : 1;
    sd.SampleDesc.Quality  = to_4x_MSAA_state ? (num_4x_MSAA_quality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SWAPCHAIN_BUFFER_COUNT;
    sd.OutputWindow = hWnd_;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(dxgi_factory_->CreateSwapChain(
        command_queue_.Get(), &sd, swap_chain_.GetAddressOf()));
}

static void CreateRTVandDSVDecriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC RTV_heap_desc;
    RTV_heap_desc.NumDescriptors = SWAPCHAIN_BUFFER_COUNT;
    RTV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    RTV_heap_desc.NodeMask = 0;
    ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
        &RTV_heap_desc, IID_PPV_ARGS(RTV_Heap_.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC DSV_heap_desc;
    DSV_heap_desc.NumDescriptors = 1;
    DSV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DSV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DSV_heap_desc.NodeMask = 0;
    ThrowIfFailed(d3d_device_->CreateDescriptorHeap(
        &DSV_heap_desc, IID_PPV_ARGS(DSV_Heap_.GetAddressOf())));

}
static void FlushCommandQueue()
{
    current_fence_++;
    ThrowIfFailed(command_queue_->Signal(fence_.Get(), current_fence_));
    if (fence_->GetCompletedValue() < current_fence_)
    {
        HANDLE eventHandle = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        ThrowIfFailed(fence_->SetEventOnCompletion(current_fence_, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

static ID3D12Resource* GetCurrentBackBuffer()
{
    return swap_chain_buffer_[curr_back_buffer_].Get();
}
static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle;
    cpu_desc_handle.ptr = RTV_Heap_->GetCPUDescriptorHandleForHeapStart().ptr
        + curr_back_buffer_ * RTV_descriptor_size_;

    return cpu_desc_handle;
}
static D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()
{
    return DSV_Heap_->GetCPUDescriptorHandleForHeapStart();
}

}