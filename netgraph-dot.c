#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "ip.h"
#include "log.h"
#include "mem.h"
#include "netgraph.h"

static char const *style_node_net = "color=red shape=ellipse";
static char const *style_node_host = "shape=rectangle";
static char const *style_edge_l1l2 = "color=grey";
static char const *style_edge_l2l3 = "color=red";

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
draw_edge(char const *left, char const *right, char const *style)
{
	fprintf(stdout, "\t\"%s\" -- \"%s\" [%s];\n", left, right, style);
}

void
draw_node(char *s, struct conf_section *section, char const *style)
{
	struct conf_variable *var;
	size_t i;

	fprintf(stdout, "\t{ \"%s\" [%s; label=\"%s\\n", s, style, s);

	i = 0;
	while ((var = conf_next_variable(section, &i, NULL))) {
		if (strcmp(var->key, "name") == 0)
			continue;
		if (strcmp(var->key, "link") == 0)
			continue;
		fprintf(stdout, "%s %s\\n", var->key, var->value);
	}
	fprintf(stdout, "\"] }\n");
}

int
main(int argc, char **argv)
{
	struct mem_pool pool = {0};
	struct array hosts = {0}, nets = {0};
	size_t i1, i2, i3;
	int err;

	arg0 = *argv++;
	argc--;

	if (array_init(&hosts, sizeof (struct netgraph_host), &pool) < 0
	 || array_init(&nets, sizeof (struct netgraph_net), &pool) < 0)
		die("msg=%s", "initializing arrayays");

	for (; *argv != NULL; argv++) {
		size_t ln = 0;

		err = netgraph_add_conf(&nets, &hosts, *argv, &ln, &pool);
		if (err < 0)
			die("msg=%s path=%s line=%d",
			  netgraph_strerror(err), *argv, ln);
	}

	draw_beg();

	for (i1 = 0; i1 < array_length(&nets); i1++) {
		struct netgraph_net *net = array_i(&nets, i1);

		draw_node(net->name, net->section, style_node_net);
	}

	for (i1 = 0; i1 < array_length(&hosts); i1++) {
		struct netgraph_host *host = array_i(&hosts, i1);

		draw_node(host->name, host->section, style_node_host);
	}

	/* layer 3 topology */

	for (i1 = 0; i1 < array_length(&nets); i1++) {
		struct netgraph_net *net = array_i(&nets, i1);

		for (i2 = 0; i2 < array_length(&hosts); i2++) {
			struct netgraph_host *host = array_i(&hosts, i2);

			for (i3 = 0; i3 < array_length(&host->ips); i3++) {
				uint8_t *ip = array_i(&host->ips, i3);

				if (ip_match(ip, net->ip, net->mask))
					draw_edge(net->name, host->name, style_edge_l2l3);
			}
		}
	}

	/* layer 2 topology */

	for (i1 = 0; i1 < array_length(&hosts); i1++) {
		struct netgraph_host *this = array_i(&hosts, i1);

		for (i2 = 0; i2 < array_length(&this->links); i2++) {
			struct netgraph_link *link = array_i(&this->links, i2);
			struct netgraph_host *other;

			i3 = 0;
			while ((other = netgraph_next_linked(&hosts, link, &i3)))
				draw_edge(this->name, other->name, style_edge_l1l2);
		}
	}

	draw_end();

	mem_free(&pool);
	return 0;
}
