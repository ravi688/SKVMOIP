#pragma once

#include <SKVMOIP/defines.hpp>

#pragma push_macro("assert")
#pragma push_macro("_assert")
#undef assert
#undef _assert
#include <SKVMOIP/third_party/NvDecoder.hpp>
#include <SKVMOIP/third_party/AppDecUtils.hpp>
#pragma pop_macro("assert")
#pragma pop_macro("_assert")

#include <memory>

namespace SKVMOIP
{
	class Decoder
	{
	private:
		CUcontext m_cudaContext;
		std::unique_ptr<NvDecoder> m_nvDecoder;
		bool m_isValid;

	public:
		Decoder();
		Decoder(Decoder&& decoder);
		Decoder(Decoder& decoder) = delete;
		Decoder& operator=(Decoder& decoder) = delete;
		~Decoder();
		bool decode(u8* const data, u32 dataSize, u32& decodedFrameCount);
		u8* getFrame();
		u32 getFrameSize();
	};
}