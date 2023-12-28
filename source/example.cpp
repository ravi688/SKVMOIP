#include <SKVMOIP/example.hpp>
#include <iostream>

BEGIN_CPP_COMPATIBLE

SKVMOIP_API void FunctionFromCPP(const char* str)
{
	std::cout << "FunctionFromCPP: " << str << std::endl;
}

END_CPP_COMPATIBLE
