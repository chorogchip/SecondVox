

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include "WinApiAndDXHeaders.h"

#include <iostream>

#include "D3DClass.h"
#include "Timer.h"

#include "WorldManager.h"
#include "MemoryManager.h"
#include "GlobalMemories.h"
#include "BlockListManager.h"

static bool Init();
static bool InitWindow();
static bool OnResize();
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

static HWND hWnd_;
static HINSTANCE hInst_;
static const WCHAR *window_title_ = L"Title";
static const WCHAR *window_class_ = L"WindowClass";
static uint64_t game_states_ = 0ULL;

static vox::utils::Timer timer{};

static void FilterError( HRESULT hr )
{
    if ( FAILED( hr ) )
    {
        std::cout << "hr : " << hr << std::endl;
        if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
        {
            std::cout << "dx device lost : " << std::endl;
        }
    }
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPWSTR    lpCmdLine,
                       _In_ int       nCmdShow)
{
    hInst_ = hInstance;
    
    vox::mem::MMInit();
    vox::mem::GMInit();

    if ( !Init() ) return 0;
    if ( !OnResize() ) return 0;

    vox::ren::BMInit();
    vox::wrd::WMInit();

    MSG msg{};
    timer.Start();

    while(WM_QUIT != msg.message)
    {
        if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            constexpr static int MAX_FRAME_SKIP = 10;
            for (int i = 0; i < MAX_FRAME_SKIP; ++i)
            {
                constexpr static int TPS = 60;
                constexpr static auto MICROSEC_PER_TICK =
                    std::chrono::microseconds( 1000LL * 1000LL / TPS );
                if ( timer.GetElapsedMicroSec() >= MICROSEC_PER_TICK )
                {
                    timer.AddTimeMicroSec( MICROSEC_PER_TICK );

                    // update

                    vox::wrd::WMCheckToChangeMap();
                    vox::ren::BMUpdate();
                }
                else goto RENDER_FRAME;
            }
            {
                // when the frame skip is too much, which means game is laggy
                timer.Start();  // reset the timer to slow game
            }
            RENDER_FRAME:
            // render
            const float delta_time = (float)timer.GetElapsedMicroSec().count() / 100000.0f;

            // present
            vox::ren::DCDraw();
        }
    }

    // clear
    vox::wrd::WMClear();
    vox::ren::BMClear();
    vox::ren::DCClear();
    vox::mem::GMClear();
    vox::mem::MMClear();

    return (int) msg.wParam;
}

static bool Init()
{
    if (!InitWindow())
    {
        return false;
    }

    if (!vox::ren::DCInit(hWnd_, hInst_))
    {
        return false;
    }

    vox::ren::DCOnResize();

    return true;
}
static bool InitWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst_;
    wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
    wc.hCursor = LoadCursor( 0, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = window_class_;

    if ( !RegisterClass( &wc ) )
    {
        MessageBox(0, L"Register Class Failed.", 0, 0);
        return false;
    }

    hWnd_ = CreateWindowW( window_class_, window_title_,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        nullptr, nullptr, hInst_, nullptr );
    if( !hWnd_ )
    {
        MessageBox(0, L"Create Window Failed.", 0, 0);
        return false;
    }

    ShowWindow( hWnd_, SW_SHOW );
    UpdateWindow( hWnd_ );

    return true;
}

static bool OnResize()
{
    vox::ren::DCOnResize();
    return true;
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool is_user_resizing = false;

    switch (message)
    {
    case WM_ACTIVATE:
    {
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            vox::ren::DCSetPaused(true);
            timer.Stop();
        }
        else
        {
            vox::ren::DCSetPaused(false);
            timer.Start();
        }
        return 0;
    }
    case WM_SIZE:
    {   
        const int32_t width  = LOWORD(lParam);
        const int32_t height = HIWORD(lParam);
        vox::ren::DCSetWidthAndHeight(width, height);
        if (vox::ren::DCIsDeviceExist())
        {
            if (wParam == SIZE_RESTORED && !is_user_resizing)
            {
                vox::ren::DCOnResize();
            }
        }
        return 0;
    }
    case WM_ENTERSIZEMOVE:
        is_user_resizing = true;
        return 0;

    case WM_EXITSIZEMOVE:
        is_user_resizing = false;
        vox::ren::DCOnResize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    /*
    case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        else if((int)wParam == VK_F2)
            Set4xMsaaState(!m4xMsaaState);
    */
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
