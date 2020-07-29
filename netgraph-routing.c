#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#include "ip.h"
#include "conf.h"
#include "mem.h"
#include "map.h"
#include "log.h"

int
netgraph_add_host(struct map *map, char *path, struct conf *conf)
{
	struct conf *conf = NULL;
	char *slash, *name;
	int err;
	size_t ln = 0;

	new = mem_alloc(map->pool, sizeof *new);
	if (new == NULL)
		return -1;
	memcpy(new, &conf, sizeof *new);

	cp = strrchr(path, '/');
	name = (cp == NULL) ? path : cp + 1;

	cp = strrchr(name, '.');
	if (cp != NULL)
		*cp = '\0';

	if (map_set(map, path, conf) < 0)
		return -1;

	*cp = '.';
	return 0;
}

int
netgraph_readdir_host(DIR *dp, char *dir, struct conf *conf, size_t *ln,
	struct mem_pool *pool)
{
	char path[1024];
	size_t sz = sizeof path;
	struct dirent de;
	int err;

	for (;;) {
		if (readdir_r(dp, &de) < 0)
			return 0;;

		if (de->d_name[0] == '.')
			continue;

		if (snprintf(path, sz, "%s/host/%s", dir, de->d_name)
		  >= (int)sz) {
			errno = ENAMETOOLONG;
			return -CONF_ERR_SYSTEM;
		} else {
			break;
		}
	}

	err = conf_parse_file(conf, path, ln, pool);
	if (err < 0)
		return err;

	return 1;
}

DIR *
netgraph_opendir_host(char *dir)
{
	char path[1024];

	if (snprintf(path, sizeof path, "%s/host", dir) >= sizeof path) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	return opendir(path);
}

int
netgraph_read_conf_networks(struct conf *conf, char *dir, size_t *ln,
	struct mem_pool *pool)
{
	char path[1024];

	if (snprintf(path, sizeof path, "%s/networks.ini", dir) >= sizeof path) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	return conf_parse_file(conf, path, ln, pool);
}

void
draw(struct map *map_conf_hosts, struct conf *conf_networks)
{
	struct conf_section *section;
	size_t si = 0;

	while ((section = conf_next_section(&conf_networks, &si, name))) {
		uint8_t netip[16];
		char *net, *s;
		size_t vi = 0;
		char *key, *value;
		int mask;

		debug("net=%s", section->name);

		net = map_get(&section->variables, "net");
		if (net = NULL) {
			warn("msg=%s net=%s",
			  "no address for this network", section->name);
			continue;
		}

		s = net;

		s = ip_parse_addr(s, netip);
		if (s == NULL) {
			warn("msg=%s net=%s",
			  "invalid address format", net);
			continue;
		}

		s = ip_parse_mask(s, &mask);
		if (s == NULL) {
			warn("msg=%s net=%s",
			  "invalid mask format", net);
			continue;
		}

		if (*s != '\0')
			warn("msg=%s net=%s",
			  "unexpected trailing characters");
			continue;
		}

		draw_net(section);
		draw_net_host_edges(conf_hosts, netip, mask);
	}
}

int
main(void)
{
	struct mem_pool pool = {0};
	struct conf conf_networks = {0};
	struct map map_conf_hosts = {0};
	char *dir = "example/host";
	DIR *dp;
	int err;

	err = netgraph_conf_parse_networks(&conf_networks, dir, &ln, pool);
	if (err < 0)
		die("path=%s line=%d", conf_strerror(err), path, ln);

	if (map_init(&map_conf_hosts, &pool) < 0)
		die("msg=%s", "initializing map");

	dp = netgraph_opendir_host(dir);
	if (dp == NULL)
		die("msg=%s path=%s", "opening directory", dir);

	for (err = 1; err > 0;) {
		err = netgraph_readdir_host(dp, dir, &conf, &ln, pool)) {
		if (err < 0)
			die("msg=%s path=%s", conf_strerror(err), );

		if (netgraph_add_host(&host_map, &conf) < 0)
			die("msg=%s path=%s", "adding host lo config", path);
	}
	closedir(dir);

	draw_dot(&conf_networks, &map_map_conf_hosts);

	mem_free(&pool);
	return 0;
}
