
#pragma once

#include <common/defines.h>
#include <SKVMOIP/api_defines.h>

#if !defined(SKVMOIP_RELEASE) && !defined(SKVMOIP_DEBUG)
#   warning "None of SKVMOIP_RELEASE && SKVMOIP_DEBUG is defined; using SKVMOIP_DEBUG"
#   define SKVMOIP_DEBUG
#endif
