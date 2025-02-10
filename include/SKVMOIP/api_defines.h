#pragma once

#if (defined _WIN32 || defined __CYGWIN__) && defined(__GNUC__)
#	define SKVMOIP_IMPORT_API __declspec(dllimport)
#	define SKVMOIP_EXPORT_API __declspec(dllexport)
#else
#	define SKVMOIP_IMPORT_API __attribute__((visibility("default")))
#	define SKVMOIP_EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef SKVMOIP_BUILD_STATIC_LIBRARY
#	define SKVMOIP_API
#elif defined(SKVMOIP_BUILD_DYNAMIC_LIBRARY)
#	define SKVMOIP_API SKVMOIP_EXPORT_API
#elif defined(SKVMOIP_USE_DYNAMIC_LIBRARY)
#	define SKVMOIP_API SKVMOIP_IMPORT_API
#elif defined(SKVMOIP_USE_STATIC_LIBRARY)
#	define SKVMOIP_API
#else
#	define SKVMOIP_API
#endif
