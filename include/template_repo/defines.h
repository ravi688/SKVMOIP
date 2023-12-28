
#pragma once

#include <common/defines.h>

#ifdef TEMPLATE_REPO_STATIC_LIBRARY
#	define TEMPLATE_REPO_API
#elif TEMPLATE_REPO_DYNAMIC_LIBRARY
#	define TEMPLATE_REPO_API __declspec(dllimport)
#elif BUILD_DYNAMIC_LIBRARY
#	define TEMPLATE_REPO_API __declspec(dllexport)
#else
#	define TEMPLATE_REPO_API
#endif
