#pragma once

#include "DXMathHeaders.h"

namespace vox::ett
{

void CMInit();
void CMClear();
DirectX::XMVECTOR CMGetCamPos();
DirectX::XMMATRIX CMGetCamViewMat();
void CMTempMoveCameraFront(float dist);
void CMTempRotateCameraXCCW(float rot);
void CMTempRotateCameraYCCW(float rot);

}