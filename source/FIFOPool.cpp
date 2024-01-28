#include <SKVMOIP/FIFOPool.hpp>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	DataBuffer::DataBuffer(u32 capacity) : m_isValid(false)
	{
		m_buffer = BUFnew(sizeof(u8), capacity, 0);
		buf_clear(m_buffer, NULL);
		m_isValid = true;
	}

	void DataBuffer::destroy()
	{
		_assert(m_isValid);
		m_isValid = false;
		buf_free(m_buffer);
		m_buffer = NULL;
	}
	
	const u8* DataBuffer::getPtr() const
	{
		return m_isValid ? buf_get_ptr_typeof(const_cast<buffer_t*>(m_buffer), u8) : NULL;
	}
	
	u8* DataBuffer::getPtr()
	{
		return m_isValid ? buf_get_ptr_typeof(m_buffer, u8) : NULL;
	}
	
	u32 DataBuffer::getSize() const
	{
		return m_isValid ? static_cast<u32>(buf_get_capacity(const_cast<buffer_t*>(m_buffer))) : 0;
	}
}