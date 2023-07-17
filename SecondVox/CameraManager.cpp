#include "CameraManager.h"

#include <cmath>

namespace vox::ett
{

DirectX::XMVECTOR cam_pos_;
float rot_ccw_[3];

void CMInit()
{
    cam_pos_ = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
}

void CMClear()
{

}

DirectX::XMVECTOR CMGetCamPos()
{
    return cam_pos_;
}

DirectX::XMMATRIX CMGetCamViewMat()
{
    // temp code

    const DirectX::XMVECTOR target =
        DirectX::XMVectorAdd(cam_pos_, DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
    const DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    const DirectX::XMMATRIX view_mat = DirectX::XMMatrixLookAtLH(cam_pos_, target, up);
    return view_mat;
}
void CMTempMoveCameraFront(float dist)
{
    cam_pos_.m128_f32[2] += dist;
}

void CMTempRotateCameraXCCW(float rot)
{
    rot_ccw_[0] += rot;
}

void CMTempRotateCameraYCCW(float rot)
{
    rot_ccw_[1] += rot;
}

}