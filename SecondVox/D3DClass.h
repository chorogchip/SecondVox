#pragma once

#include <cstdint>

#include "WinApiAndDXHeaders.h"

namespace vox::ren
{
bool DCIsDeviceExist();
float DCGetAspectRatio();
void DCGetWidthAndHeight(int32_t* wh_dest);
void DCSetWidthAndHeight(int32_t width, int32_t height);
bool DCGetTo4xMsaaState();
void DCSetTo4xMsaaState(bool value);
UINT DCGet4xMsaaQuality();
bool DCIsPaused();
void DCSetPaused(bool value);

ID3D12Device &DCGetD3DDevice();
ID3D12GraphicsCommandList &DCGetMainCommandList();
void DCExecuteMainCommandList();
void DCResetMainCommandList(ID3D12PipelineState* p_PSO)

bool DCInit(HWND hWnd, HINSTANCE hInstance);
void DCClear();
void DCOnResize();
void DCDraw();


}