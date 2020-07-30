#include "netgraph.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#include "ip.h"
#include "conf.h"
#include "mem.h"

char const *
netgraph_strerror(int i)
{
	enum netgraph_errno netgraph_err = (i < 0) ? -i : i;

	switch (netgraph_err) {
	case NETGRAPH_ERR_SYSTEM:
		return "system error";
	case NETGRAPH_ERR_BAD_ADDR_FORMAT:
		return "invalid ip format for some \"ip =\" variable";
	case NETGRAPH_ERR_TRAILING_VALUE:
		return "trailing value after \"ip =\" value";
	case NETGRAPH_ERR_BAD_MASK_FORMAT:
		return "invalid mask format after ip address";
	case NETGRAPH_ERR_MISSING_NAME_VARIABLE:
		return "not \"name =\" variable found";
	}
	return conf_strerror(i);
}

static int
netgraph_add_host(struct arr *arr, struct conf_section *section)
{
	struct netgraph_host host = {0};
	char const *s;
	size_t i;
	int err;

	if (arr_init(&host.ips, 16, arr->pool) < 0)
		return -NETGRAPH_ERR_SYSTEM;

	i = 0;
	host.name = conf_next_value(section, &i, "name");
	if (host.name == NULL)
		return -NETGRAPH_ERR_MISSING_NAME_VARIABLE;

	host.section = section;

	i = 0;
	while ((s = conf_next_value(section, &i, "ip"))) {
		uint8_t ip[16] = {0};

		s = ip_parse_addr(s, ip);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_ADDR_FORMAT;
		if (*s != '\0')
			return -NETGRAPH_ERR_TRAILING_VALUE;

		if (arr_append(&host.ips, ip) < 0)
			return -NETGRAPH_ERR_SYSTEM;
	}

	err = arr_append(arr, &host);
	if (err < 0)
		return -NETGRAPH_ERR_SYSTEM;

	return 0;
}

static int
netgraph_add_net(struct arr *nets, struct conf_section *section)
{
	struct netgraph_net net = {0};
	char const *s;
	size_t i;
	int err;

	net.section = section;

	i = 0;
	net.name = conf_next_value(section, &i, "name");
	if (net.name == NULL)
		return -NETGRAPH_ERR_MISSING_NAME_VARIABLE;

	i = 0;
	while ((s = conf_next_value(section, &i, "ip"))) {
		s = ip_parse_addr(s, net.ip);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_ADDR_FORMAT;

		s = ip_parse_mask(s, ip_version(net.ip), &net.mask);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_MASK_FORMAT;
		if (*s != '\0')
			return -NETGRAPH_ERR_TRAILING_VALUE;
	}

	err = arr_append(nets, &net);
	if (err < 0)
		return -NETGRAPH_ERR_SYSTEM;

	return 0;
}

int
netgraph_add_conf(struct arr *nets, struct arr *hosts, char *path, size_t *ln,
	struct mem_pool *pool)
{
	struct conf conf = {0};
	struct conf_section *section;
	size_t i;
	int err;

	err = conf_parse_file(&conf, path, ln, pool);
	if (err < 0)
		return err;

	i = 0;
	while ((section = conf_next_section(&conf, &i, "net"))) {
		err = netgraph_add_net(nets, section);
		if (err < 0)
			return err;
	}
	i = 0;
	while ((section = conf_next_section(&conf, &i, "host"))) {
		err = netgraph_add_host(hosts, section);
		if (err < 0)
			return err;
	}
	return 0;
}
