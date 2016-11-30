
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
		char* output = json_dumps(json, 0);
		stream << output;
		json_decref(json);
		free(output);
	} else {
		stream << "{}";
	}
	return stream;
}
