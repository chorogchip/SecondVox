#include "WindowUtils.h"

namespace vox::wut
{

bool CheckTearingSupport()
{
    ComPtr<IDXGIFactory4> factory4;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory4));
    BOOL allow_tearing = false;
    if (SUCCEEDED(hr))
    {
        ComPtr<IDXGIFactory5> factory5;
        hr = factory4.As(&factory5);
        if(SUCCEEDED(hr))
        {
            hr = factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allow_tearing, sizeof(allow_tearing));
        }
    }
    return allow_tearing;
}
/*
    to save tearing code
    //--------------------------------------------------------------------------------------------------------
    // Set up swapchain properly
    //--------------------------------------------------------------------------------------------------------

    // It is recommended to always use the tearing flag when it is supported.
    swapChainDesc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    //--------------------------------------------------------------------------------------------------------
    // Present
    //--------------------------------------------------------------------------------------------------------

    UINT presentFlags = (m_tearingSupport && m_windowedMode) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(0, presentFlags))
*/

}

