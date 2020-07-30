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
		return "invalid address format for some \"ip =\" variable";
	case NETGRAPH_ERR_BAD_MASK_FORMAT:
		return "invalid mask format after ip address";
	case NETGRAPH_ERR_BAD_MAC_FORMAT:
		return "invalid address format for some \"mac =\" variable";
	case NETGRAPH_ERR_TRAILING_VALUE:
		return "trailing value after \"ip =\" value";
	case NETGRAPH_ERR_MISSING_NAME_VARIABLE:
		return "not \"name =\" variable found";
	case NETGRAPH_ERR_NET_WITHOUT_IP:
		return "network entry lacks an IP address";
	}
	return conf_strerror(i);
}

static int
netgraph_add_net_ip(struct netgraph_net *net, struct conf_section *section)
{
	char const *s;
	size_t i = 0;

	s = conf_next_value(section, &i, "ip");
	if (s == NULL)
		return -NETGRAPH_ERR_NET_WITHOUT_IP;

	s = ip_parse_addr(s, net->ip);
	if (s == NULL)
		return -NETGRAPH_ERR_BAD_ADDR_FORMAT;

	s = ip_parse_mask(s, ip_version(net->ip), &net->mask);
	if (s == NULL)
		return -NETGRAPH_ERR_BAD_MASK_FORMAT;
	if (*s != '\0')
		return -NETGRAPH_ERR_TRAILING_VALUE;

	return 0;
}

static int
netgraph_add_host_ips(struct netgraph_host *host, struct conf_section *section)
{
	char const *s;
	size_t i = 0;

	while ((s = conf_next_value(section, &i, "ip"))) {
		uint8_t ip[16] = {0};

		s = ip_parse_addr(s, ip);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_ADDR_FORMAT;
		if (*s != '\0')
			return -NETGRAPH_ERR_TRAILING_VALUE;

		if (array_append(&host->ips, ip) < 0)
			return -NETGRAPH_ERR_SYSTEM;
	}
	return 0;
}

static int
netgraph_add_host_macs(struct netgraph_host *host, struct conf_section *section)
{
	char const *s;
	size_t i = 0;

	while ((s = conf_next_value(section, &i, "mac"))) {
		uint8_t mac[6] = {0};

		s = ip_parse_mac(s, mac);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_MAC_FORMAT;
		if (*s != '\0')
			return -NETGRAPH_ERR_TRAILING_VALUE;

		if (array_append(&host->macs, mac) < 0)
			return -NETGRAPH_ERR_SYSTEM;
	}
	return 0;
}

static int
netgraph_add_host(struct array *array, struct conf_section *section)
{
	struct netgraph_host host = {0};
	size_t i;
	int err;

	if (array_init(&host.ips, 16, array->pool) < 0)
		return -NETGRAPH_ERR_SYSTEM;

	if (array_init(&host.macs, 6, array->pool) < 0)
		return -NETGRAPH_ERR_SYSTEM;

	i = 0;
	host.name = conf_next_value(section, &i, "name");
	if (host.name == NULL)
		return -NETGRAPH_ERR_MISSING_NAME_VARIABLE;

	host.section = section;

	err = netgraph_add_host_ips(&host, section);
	if (err < 0)
		return err;

	err = netgraph_add_host_macs(&host, section);
	if (err < 0)
		return err;

	err = array_append(array, &host);
	if (err < 0)
		return -NETGRAPH_ERR_SYSTEM;

	return 0;
}

static int
netgraph_add_net(struct array *nets, struct conf_section *section)
{
	struct netgraph_net net = {0};
	size_t i;
	int err;

	net.section = section;

	i = 0;
	net.name = conf_next_value(section, &i, "name");
	if (net.name == NULL)
		return -NETGRAPH_ERR_MISSING_NAME_VARIABLE;

	err = netgraph_add_net_ip(&net, section);
	if (err < 0)
		return err;

	err = array_append(nets, &net);
	if (err < 0)
		return -NETGRAPH_ERR_SYSTEM;

	return 0;
}

int
netgraph_add_conf(struct array *nets, struct array *hosts, char *path, size_t *ln,
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
