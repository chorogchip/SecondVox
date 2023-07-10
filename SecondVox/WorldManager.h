#pragma once
#include "UtilsRandom.h"

namespace vox::wrd
{

void WMInit();
void WMClear();
void WMGoThroughPortalAndPrepareChangeMap(int portal_num_of_map);
void WMCheckToChangeMap();

}

