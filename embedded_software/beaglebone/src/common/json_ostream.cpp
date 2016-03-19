
#include "json_utilities.hpp"

int jansson_ostream(const char *buffer, size_t size, void *ctxt)
{
	std::ostream *os = static_cast<std::ostream *>(ctxt);
	os->write(buffer, size);
	return 0;
}


