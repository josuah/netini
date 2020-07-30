#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "ip.h"
#include "log.h"
#include "mem.h"
#include "netgraph.h"

static char *style_net = "shape=ellipse";
static char *style_host = "shape=rectangle";

void
draw_beg(void)
{
	fprintf(stdout, "graph G {\n");
}

void
draw_end(void)
{
	fprintf(stdout, "}\n");
}

void
draw_edge(char *left, char *right)
{
	fprintf(stdout, "\t\"%s\" -- \"%s\";\n", left, right);
}

void
draw_node(char *s, struct conf_section *section, char *style)
{
	struct conf_variable *var;
	size_t i;

	fprintf(stdout, "\t{ \"%s\" [%s; label=\"%s\\n", s, style, s);

	i = 0;
	while ((var = conf_next_variable(section, &i, NULL))) {
		if (strcmp(var->key, "name") == 0)
			continue;
		fprintf(stdout, "%s: %s\\n", var->key, var->value);
	}
	fprintf(stdout, "\"] }\n");
}

int
main(int argc, char **argv)
{
	struct mem_pool pool = {0};
	struct arr hosts = {0}, nets = {0};
	size_t i1, i2, i3;
	int err;

	arg0 = *argv++;
	argc--;

	if (arr_init(&hosts, sizeof (struct netgraph_host), &pool) < 0
	 || arr_init(&nets, sizeof (struct netgraph_net), &pool) < 0)
		die("msg=%s", "initializing arrays");

	for (; *argv != NULL; argv++) {
		size_t ln = 0;

		err = netgraph_add_conf(&nets, &hosts, *argv, &ln, &pool);
		if (err < 0)
			die("msg=%s path=%s line=%d",
			  netgraph_strerror(err), *argv, ln);
	}

	draw_beg();

	for (i1 = 0; i1 < arr_length(&nets); i1++) {
		struct netgraph_net *net = arr_i(&nets, i1);

		draw_node(net->name, net->section, style_net);
	}

	for (i1 = 0; i1 < arr_length(&hosts); i1++) {
		struct netgraph_host *host = arr_i(&hosts, i1);

		draw_node(host->name, host->section, style_host);
	}

	for (i1 = 0; i1 < arr_length(&nets); i1++) {
		struct netgraph_net *net = arr_i(&nets, i1);

		for (i2 = 0; i2 < arr_length(&hosts); i2++) {
			struct netgraph_host *host = arr_i(&hosts, i2);

			for (i3 = 0; i3 < arr_length(&host->ips); i3++) {
				uint8_t *ip = arr_i(&host->ips, i3);

				if (ip_match(ip, net->ip, net->mask))
					draw_edge(net->name, host->name);
			}
		}
	}

	draw_end();

	mem_free(&pool);
	return 0;
}
