#pragma once

#include <SKVMOIP/defines.hpp>


enum class Message : u8
{
	/* Format: | u8 (Start) | u8 (device ID) | */
	Start,
	/* Format: | u8 (Stop) | */
	Stop
};

enum class SocketType : u8
{
	/* Control Socket, carries signalling messages */
	Control,
	/* Data Socket, carriers video stream data which should be decoded at the client computer 
	 * Format: | u32 (client id) | */
	Stream
};
