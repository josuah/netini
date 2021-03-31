#include "netini.h"

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
netini_strerror(int i)
{
	enum netini_errno netini_err = (i < 0) ? -i : i;

	switch (netini_err) {
	case NETINI_ERR_SYSTEM:
		return "system error";
	case NETINI_ERR_BAD_ADDR_FORMAT:
		return "invalid address format for some ip= variable";
	case NETINI_ERR_BAD_MASK_FORMAT:
		return "invalid mask format after ip address";
	case NETINI_ERR_BAD_MAC_FORMAT:
		return "invalid address format for some mac= variable";
	case NETINI_ERR_TRAILING_VALUE:
		return "trailing value after ip= value";
	case NETINI_ERR_MISSING_NAME_VARIABLE:
		return "not name= variable found";
	case NETINI_ERR_NET_WITHOUT_IP:
		return "network entry lacks an IP address";
	case NETINI_ERR_MULTIPLE_NET_IP:
		return "network entry with multiple IP addresses";
	}
	return conf_strerror(i);
}

static int
netini_add_net_ip(struct netini_net *net, struct conf_section *section, size_t *ln)
{
	struct conf_variable *var;
	char const *s;
	size_t i = 0;

	*ln = section->ln;

	var = conf_next_variable(section, &i, "ip");
	if (var == NULL)
		return -NETINI_ERR_NET_WITHOUT_IP;

	*ln = var->ln;
	s = var->value;

	s = ip_parse_addr(s, net->ip);
	if (s == NULL)
		return -NETINI_ERR_BAD_ADDR_FORMAT;

	s = ip_parse_mask(s, ip_version(net->ip), &net->mask);
	if (s == NULL)
		return -NETINI_ERR_BAD_MASK_FORMAT;
	if (*s != '\0')
		return -NETINI_ERR_TRAILING_VALUE;

	var = conf_next_variable(section, &i, "ip");
	if (var != NULL) {
		*ln = var->ln;
		return -NETINI_ERR_MULTIPLE_NET_IP;
	}

	return 0;
}

static int
netini_add_host_ips(struct netini_host *host, struct conf_section *section, size_t *ln)
{
	struct conf_variable *var;
	char const *s;
	size_t i = 0;

	*ln = section->ln;

	while ((var = conf_next_variable(section, &i, "ip"))) {
		uint8_t ip[16] = {0};

		*ln = var->ln;
		s = var->value;

		s = ip_parse_addr(s, ip);
		if (s == NULL)
			return -NETINI_ERR_BAD_ADDR_FORMAT;
		if (*s != '\0')
			return -NETINI_ERR_TRAILING_VALUE;

		*ln = 0;

		if (array_append(&host->ips, ip) < 0)
			return -NETINI_ERR_SYSTEM;
	}
	return 0;
}

static int
netini_add_host_macs(struct netini_host *host, struct conf_section *section, size_t *ln)
{
	struct conf_variable *var;
	char const *s;
	size_t i = 0;

	*ln = section->ln;

	while ((var = conf_next_variable(section, &i, "mac"))) {
		uint8_t mac[6] = {0};

		*ln = var->ln;
		s = var->value;

		s = mac_parse_addr(s, mac);
		if (s == NULL)
			return -NETINI_ERR_BAD_MAC_FORMAT;
		if (*s != '\0')
			return -NETINI_ERR_TRAILING_VALUE;

		*ln = 0;

		if (array_append(&host->macs, mac) < 0)
			return -NETINI_ERR_SYSTEM;
	}
	return 0;
}

static void
netini_parse_link(struct netini_link *link, char const *s)
{
	char const *cp;

	link->type = NETINI_T_IP;
	cp = ip_parse_addr(s, link->u.ip);
	if (cp != NULL && *cp == '\0')
		return;

	link->type = NETINI_T_MAC;
	cp = mac_parse_addr(s, link->u.mac);
	if (cp != NULL && *cp == '\0')
		return;

	link->type = NETINI_T_NAME;
	link->u.name = s;
}

static int
netini_add_host_links(struct netini_host *host, struct conf_section *section, size_t *ln)
{
	char const *s;
	size_t i = 0;

	while ((s = conf_next_value(section, &i, "link"))) {
		struct netini_link link = {0};

		*ln = 0;

		netini_parse_link(&link, s);
		if (array_append(&host->links, &link) < 0)
			return -NETINI_ERR_SYSTEM;
	}
	return 0;
}

static int
netini_add_host(struct array *array, struct conf_section *section, size_t *ln)
{
	struct netini_host host = {0};
	struct conf_variable *var;
	size_t i, sz;
	int err;

	*ln = 0;

	if (array_init(&host.ips, 16, array->pool) < 0)
		return -NETINI_ERR_SYSTEM;

	if (array_init(&host.macs, 6, array->pool) < 0)
		return -NETINI_ERR_SYSTEM;

	sz = sizeof(struct netini_link);
	if (array_init(&host.links, sz, array->pool) < 0)
		return -NETINI_ERR_SYSTEM;

	i = 0;
	var = conf_next_variable(section, &i, "name");
	*ln = var->ln;
	host.name = var->value;
	if (host.name == NULL)
		return -NETINI_ERR_MISSING_NAME_VARIABLE;

	host.section = section;

	err = netini_add_host_ips(&host, section, ln);
	if (err < 0)
		return err;

	err = netini_add_host_macs(&host, section, ln);
	if (err < 0)
		return err;

	err = netini_add_host_links(&host, section, ln);
	if (err < 0)
		return err;

	err = array_append(array, &host);
	if (err < 0)
		return -NETINI_ERR_SYSTEM;

	return 0;
}

static int
netini_add_net(struct array *nets, struct conf_section *section, size_t *ln)
{
	struct netini_net net = {0};
	size_t i;
	int err;

	net.section = section;

	i = 0;
	net.name = conf_next_value(section, &i, "name");
	if (net.name == NULL)
		return -NETINI_ERR_MISSING_NAME_VARIABLE;

	err = netini_add_net_ip(&net, section, ln);
	if (err < 0)
		return err;

	err = array_append(nets, &net);
	if (err < 0)
		return -NETINI_ERR_SYSTEM;

	return 0;
}

static int
netini_add_ipsec(struct array *ipsec, struct conf_section *section, size_t *ln)
{
	*ln = 0;
	if (array_append(ipsec, section) < 0)
		return -NETINI_ERR_SYSTEM;
	return 0;
}

int
netini_add_conf(struct netini_graph *graph, char *path, size_t *ln,
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
		err = netini_add_net(&graph->nets, section, ln);
		if (err < 0)
			return err;
	}

	i = 0;
	while ((section = conf_next_section(&conf, &i, "host"))) {
		err = netini_add_host(&graph->hosts, section, ln);
		if (err < 0)
			return err;
	}

	i = 0;
	while ((section = conf_next_section(&conf, &i, "ipsec"))) {
		err = netini_add_ipsec(&graph->ipsecs, section, ln);
		if (err < 0)
			return err;
	}

	return 0;
}

int
netini_init_graph(struct netini_graph *graph, struct mem_pool *pool)
{
	assert(graph->init == 0);

        if (array_init(&graph->hosts, sizeof(struct netini_host), pool) < 0
         || array_init(&graph->nets, sizeof(struct netini_net), pool) < 0
	 || array_init(&graph->ipsecs, sizeof(struct conf_section), pool) < 0)
                return -1;
	graph->init = 1;
	return 0;
}

struct netini_host *
netini_next_linked(struct array *hosts, struct netini_link *link, size_t *i)
{
	for (; *i < array_length(hosts); (*i)++) {
		struct netini_host *host = array_i(hosts, *i);
		struct array *arr;
		size_t sz;

		switch (link->type) {
		case NETINI_T_IP:
			arr = &host->ips;
			for (size_t i2 = 0; i2 < array_length(arr); i2++) {
				uint8_t *ip = array_i(arr, i2);

				sz = sizeof link->u.ip;
				if (memcmp(link->u.ip, ip, sz) == 0) {
					return host;
				}
			}
			break;
		case NETINI_T_MAC:
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
		case NETINI_T_NAME:
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
