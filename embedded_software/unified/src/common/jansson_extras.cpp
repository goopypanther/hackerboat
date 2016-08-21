
extern "C" {
#include <jansson.h>
#include <stdarg.h>
}
#include "hackerboatRoot.hpp"

int json_object_add(json_t *obj, const char *types, ...)
{
	va_list ap;

	va_start(ap, types);
	while (*types) {
		char tp = *types++;
		if (tp == ' ' || tp == ',')
			continue;
		const char *key = va_arg(ap, const char *);
		if (!key)
			return -1;

		json_t *value;
		switch(tp) {
		case 'o':
		case 'O':
			value = va_arg(ap, json_t *);
			if (tp == 'O')
				json_incref(value);
			break;
		case 's':
			value = json_string(va_arg(ap, const char *));
			break;
		case 'i':
			value = json_integer(va_arg(ap, int));
			break;
		case 'I':
			value = json_integer(va_arg(ap, json_int_t));
			break;
		case 'f':
			value = json_real(va_arg(ap, double));
			break;
		default:
			return -1;
		}

		int errk = json_object_set_new(obj, key, value);
		if (errk)
			return errk;
	}
	va_end(ap);

	return 0;
}


