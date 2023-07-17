#pragma once

namespace vox::ren
{

void MRInit();
void MRClear();
void MROnResize();
void MRUpdate();
void MRRender(float delta_time);

}