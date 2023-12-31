#pragma once

#include <SKVMOIP/defines.h>

template<typename T> T& null_reference() { return *reinterpret_cast<T*>(NULL); }
