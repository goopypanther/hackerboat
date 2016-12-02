
#include "json_utilities.hpp"
#include "hackerboatRoot.hpp"

int jansson_ostream(const char *buffer, size_t size, void *ctxt)
{
	std::ostream *os = static_cast<std::ostream *>(ctxt);
	os->write(buffer, size);
	return 0;
}

std::ostream& operator<< (std::ostream& stream, const HackerboatState& state) {
	json_t* json;
	json = state.pack();
	if (json) {
		stream << json;
		json_decref(json);
	} else {
		stream << "{}";
	}
	return stream;
}
