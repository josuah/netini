#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "ip.h"
#include "log.h"
#include "mem.h"

int
main(int argc, char **argv)
{
	struct mem_pool pool = {0};
	size_t i1, i2, i3;
	int err;

	arg0 = *argv++;
	argc--;

	for (i1 = 0; i1 < array_length(&graph.ipsecs); i1++) {
		struct conf_section *section = array_i(&graph.ipsecs, i1);
		char *h1;

		i2 = 0;
		while ((h1 = conf_next_value(section, &i2, "host"))) {
			char *h2;

			i3 = i2;
			while ((h2 = conf_next_value(section, &i3, "host")))
				if (strcmp(h1, h2) != 0)
					draw_edge(h1, h2, style_edge_l1l2);
		}
	}

	draw_end();

	mem_free(&pool);
	return 0;
}
