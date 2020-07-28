#include "ip.h"
#include "conf.h"
#include "mem.h"
#include "log.h"

int
main(void)
{
	struct conf conf = {0};
	struct conf_section *section;
	struct mem_pool pool = {0};
	char *path = "example/host.ini";
	size_t ln = 0, i;
	int err;

	err = conf_parse_file(&conf, path, &ln, &pool);
	if (err < 0)
		die("msg=%s path=%s line=%d", conf_strerror(err), path, ln);

	i = 0;
	while ((section = conf_next_section(&conf, &si, NULL)) != NULL) {
		debug("section=%s", section->name);
	}

	mem_free(&pool);
	return 0;
}
