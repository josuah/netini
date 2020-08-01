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
static char const *style_edge_l1l2 = "color=grey";
static char const *style_edge_l2l3 = "color=red";

int
iplist_match_iplist(struct array *iplist, uint8_t *ip, size_t prefixlen)
{
	size_t len = array_length(iplist);

	for (size_t i = 0; i < len; i++)
		if (ip_match(ip, array_i(iplist, i, prefixlen)) == 0)
			return 1;
	return 0;
}

void
iplist_matches_ip(struct array *iplist1, struct array *iplist2)
{
	size_t len = array_length(iplist2);

	for (size_t i = 0; i < len; i++) {
		uint8_t *ip = array_i(iplist2, i);

		if (iplist_has_matching(iplist1, ip, 128)) {
			info("");
		}
	}
}

void
merge_hosts(struct array *hosts, size_t pos)
{
	for (size_t i1 = 0; i1 < pos; i1++) {
		for (size_t i2 = pos; i2 < array_length(hosts);) {
			merge_ips();
		}
	}
}

int
main(int argc, char **argv)
{
	struct mem_pool pool = {0};
	struct array hosts = {0}, nets = {0};
	size_t i1, i2, i3;
	int err;

	if (array_init(&hosts, sizeof (struct netini_host), &pool) < 0
	 || array_init(&nets, sizeof (struct netini_net), &pool) < 0)
		die("msg=%s", "initializing arrayays");

	for (arg0 = *argv++; *argv != NULL; argv++, argc--) {
		size_t ln = 0;

		err = netini_add_conf(nets, hosts, path, &ln, pool);
		if (err < 0)
			die("msg=%s path=%s line=%d",
			  netini_strerror(err), path, ln);

		merge_hosts();
		merge_nets();
	}

	conf_dump(&conf, stdout);

	mem_free(&pool);
	return 0;
}
