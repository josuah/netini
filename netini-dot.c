#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "ip.h"
#include "log.h"
#include "mem.h"
#include "netini.h"

static char const *style_node_net = "color=red shape=ellipse";
static char const *style_node_host = "shape=rectangle";
static char const *style_edge_l1l2 = "color=grey,weight=2";
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

	fprintf(stdout, "\t{ \"%s\" [%s,label=\"%s\\n", s, style, s);

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

void
add_conf_to_graph(struct netini_graph *graph, char *path, struct mem_pool *pool)
{
	size_t ln = 0;
	int err;

	err = netini_add_conf(graph, path, &ln, pool);
	if (err < 0)
		die("msg=%s path=%s line=%d",
		  netini_strerror(err), path, ln);
}

int
main(int argc, char **argv)
{
	struct mem_pool pool = {0};
	struct netini_graph graph = {0};
	size_t i1, i2, i3;
	int err;

	arg0 = *argv++;
	argc--;

	err = netini_init_graph(&graph, &pool);
	if (err < 0)
		die("msg=%s", "initializing data");

	if (argc == 0) {
		add_conf_to_graph(&graph, "/dev/stdin", &pool);
	} else for (; *argv != NULL; argv++, argc--) {
		char *path = (strcmp(*argv, "-") == 0) ? "/dev/stdin" : *argv;
		add_conf_to_graph(&graph, path, &pool);
	}

	draw_beg();

	/* graph nodes */

	for (i1 = 0; i1 < array_length(&graph.nets); i1++) {
		struct netini_net *net = array_i(&graph.nets, i1);

		draw_node(net->name, net->section, style_node_net);
	}

	for (i1 = 0; i1 < array_length(&graph.hosts); i1++) {
		struct netini_host *host = array_i(&graph.hosts, i1);

		draw_node(host->name, host->section, style_node_host);
	}

	/* graph links: layer 3 topology */

	for (i1 = 0; i1 < array_length(&graph.nets); i1++) {
		struct netini_net *net = array_i(&graph.nets, i1);

		for (i2 = 0; i2 < array_length(&graph.hosts); i2++) {
			struct netini_host *host = array_i(&graph.hosts, i2);

			for (i3 = 0; i3 < array_length(&host->ips); i3++) {
				uint8_t *ip = array_i(&host->ips, i3);

				if (ip_match(ip, net->ip, net->mask))
					draw_edge(net->name, host->name, style_edge_l2l3);
			}
		}
	}

	/* graph links: layer 2 topology */

	for (i1 = 0; i1 < array_length(&graph.hosts); i1++) {
		struct netini_host *this = array_i(&graph.hosts, i1);

		for (i2 = 0; i2 < array_length(&this->links); i2++) {
			struct netini_link *link = array_i(&this->links, i2);
			struct netini_host *other;

			i3 = 0;
			while ((other = netini_next_linked(&graph.hosts, link, &i3)))
				draw_edge(this->name, other->name, style_edge_l1l2);
		}
	}

	/* graph links: IPsec VPNs */

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
