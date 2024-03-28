#pragma once

class Bitstream
{
public:
  Bitstream(void* data, size_t size)
	{
    this->size = size;
    buff = (char*)data;
	}

	template <typename T>
	void write(const T& new_data)
	{
		memcpy(buff, &new_data, sizeof(T));
		size -= sizeof(T);
    buff += sizeof(T);
	}

	template <typename T>
	void read(T& val)
	{
		memcpy(&val, buff, sizeof(T));
    size -= sizeof(T);
		buff += sizeof(T);
	}

	template <typename T>
	void read()
	{
    size -= sizeof(T);
		buff += sizeof(T);
	}

private:
	char* buff;
	size_t size;
};