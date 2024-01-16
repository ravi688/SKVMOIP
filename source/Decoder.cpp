#include <SKVMOIP/Decoder.hpp>
#include <SKVMOIP/debug.h>

namespace SKVMOIP
{
	Decoder::Decoder() : m_cudaContext(NULL), m_isValid(false)
	{
		int iGpu = 0;
     	ck(cuInit(0));
        int nGpu = 0;
        ck(cuDeviceGetCount(&nGpu));
        if (iGpu < 0 || iGpu >= nGpu)
        {
        	debug_log_error("GPU ordinal out of range. Should be within [%d, %d]", 0, nGpu - 1);
        	return;
        }

        createCudaContext(&m_cudaContext, iGpu, 0);

		m_nvDecoder = std::move(std::unique_ptr<NvDecoder>(new NvDecoder(m_cudaContext, false, cudaVideoCodec_H264, true, false)));

        m_isValid = true;
	}

	Decoder::Decoder(Decoder&& decoder) : m_cudaContext(decoder.m_cudaContext), 
										  m_nvDecoder(std::move(decoder.m_nvDecoder)),
										  m_isValid(decoder.m_isValid)
	{
		decoder.m_cudaContext = NULL;
		decoder.m_isValid = false;
	}

	Decoder::~Decoder()
	{
	}

	bool Decoder::decode(u8* const data, u32 dataSize, u32& decodedFrameCount)
	{
		if(!m_isValid)
			return false;
		int iDecodedFrameCount = m_nvDecoder->Decode(data, dataSize);
		if(iDecodedFrameCount > 0)
			decodedFrameCount = iDecodedFrameCount;
		else
		{
			decodedFrameCount = 0;
			return false;
		}
		return true;
	}
	u8* Decoder::getFrame()
	{
		return m_nvDecoder->GetFrame();
	}

	u32 Decoder::getFrameSize()
	{
		return m_nvDecoder->GetFrameSize();
	}
}