#include "netgraph.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "ip.h"
#include "mac.h"
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

		s = mac_parse_addr(s, mac);
		if (s == NULL)
			return -NETGRAPH_ERR_BAD_MAC_FORMAT;
		if (*s != '\0')
			return -NETGRAPH_ERR_TRAILING_VALUE;

		if (array_append(&host->macs, mac) < 0)
			return -NETGRAPH_ERR_SYSTEM;
	}
	return 0;
}

static void
netgraph_parse_link(struct netgraph_link *link, char const *s)
{
	char const *cp;

	link->type = NETGRAPH_T_IP;
	cp = ip_parse_addr(s, link->u.ip);
	if (cp != NULL && *cp == '\0')
		return;

	link->type = NETGRAPH_T_MAC;
	cp = mac_parse_addr(s, link->u.mac);
	if (cp != NULL && *cp == '\0')
		return;

	link->type = NETGRAPH_T_NAME;
	link->u.name = s;
}

static int
netgraph_add_host_links(struct netgraph_host *host, struct conf_section *section)
{
	char const *s;
	size_t i = 0;

	while ((s = conf_next_value(section, &i, "link"))) {
		struct netgraph_link link = {0};

		netgraph_parse_link(&link, s);
		if (array_append(&host->links, &link) < 0)
			return -NETGRAPH_ERR_SYSTEM;
	}
	return 0;
}

static int
netgraph_add_host(struct array *array, struct conf_section *section)
{
	struct netgraph_host host = {0};
	size_t i, sz;
	int err;

	if (array_init(&host.ips, 16, array->pool) < 0)
		return -NETGRAPH_ERR_SYSTEM;

	if (array_init(&host.macs, 6, array->pool) < 0)
		return -NETGRAPH_ERR_SYSTEM;

	sz = sizeof(struct netgraph_link);
	if (array_init(&host.links, sz, array->pool) < 0)
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

	err = netgraph_add_host_links(&host, section);
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

struct netgraph_host *
netgraph_next_linked(struct array *hosts, struct netgraph_link *link, size_t *i)
{
	for (; *i < array_length(hosts); (*i)++) {
		struct netgraph_host *host = array_i(hosts, *i);
		struct array *arr;
		size_t sz;

		switch (link->type) {
		case NETGRAPH_T_IP:
			arr = &host->ips;
			for (size_t i2 = 0; i2 < array_length(arr); i2++) {
				uint8_t *ip = array_i(arr, i2);

				sz = sizeof link->u.ip;
				if (memcmp(link->u.ip, ip, sz) == 0) {
					return host;
				}
			}
			break;
		case NETGRAPH_T_MAC:
			arr = &host->macs;
			for (size_t i2 = 0; i2 < array_length(arr); i2++) {
				uint8_t *mac = array_i(arr, i2);

				sz = sizeof link->u.mac;
				if (memcmp(link->u.mac, mac, sz) == 0) {
					(*i)++;
					return host;
				}
			}
			break;
		case NETGRAPH_T_NAME:
			assert(host->name != NULL);
			assert(link->u.name != NULL);

			if (strcmp(link->u.name, host->name) == 0) {
				(*i)++;
				return host;
			}
			break;
		}
	}
	return NULL;
}
