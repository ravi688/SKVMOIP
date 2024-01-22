#include <SKVMOIP/FIFOPool.hpp>

namespace SKVMOIP
{
	DataBuffer::DataBuffer(u32 capacity)
	{
		m_buffer = buf_create(sizeof(u8), capacity, 0);
		buf_clear(&m_buffer, NULL);
	}
	DataBuffer::DataBuffer(DataBuffer&& data) : m_isValid(data.m_isValid)
	{
		buf_move(&data.m_buffer, &m_buffer);
		data.m_isValid = false;
	}
	DataBuffer& DataBuffer::operator=(DataBuffer&& data)
	{
		m_isValid = data.m_isValid;
		buf_move(&data.m_buffer, &m_buffer);
		data.m_isValid = false;
		return *this;
	}
	DataBuffer::~DataBuffer()
	{
		if(!m_isValid) return;
		buf_free(&m_buffer);
	}
	
	const u8* DataBuffer::getPtr() const
	{
		return m_isValid ? buf_get_ptr_typeof(const_cast<buffer_t*>(&m_buffer), u8) : NULL;
	}
	
	u8* DataBuffer::getPtr()
	{
		return m_isValid ? buf_get_ptr_typeof(&m_buffer, u8) : NULL;
	}
	
	u32 DataBuffer::getSize() const
	{
		return m_isValid ? static_cast<u32>(buf_get_capacity(const_cast<buffer_t*>(&m_buffer))) : 0;
	}
}