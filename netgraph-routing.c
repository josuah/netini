#include "ip.h"
#include "conf.h"
#include "mem.h"
#include "log.h"

void
()
{
	struct conf_section *section;
	size_t si = 0;

	while ((section = conf_next_section(&conf, &si, "ip"))) {
		size_t vi = 0;
		char *key, *value;

		debug("section=%s", section->name);
		while (conf_next_key_value(section, &vi, &key, &value)) {
			debug("key=%s val=%s", key, value);
		}
	}
}

int
main(void)
{
	struct conf conf = {0};
	struct mem_pool pool = {0};
	char *path = "example/host.ini";
	size_t ln = 0;
	int err;

	err = conf_parse_file(&conf, path, &ln, &pool);
	if (err < 0)
		die("msg=%s path=%s line=%d", conf_strerror(err), path, ln);

	mem_free(&pool);
	return 0;
}
