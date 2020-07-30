#ifndef NETGRAPH_H
#define NETGRAPH_H

#include <stdint.h>

#include "conf.h"

enum netgraph_errno {
	NETGRAPH_ERR_SYSTEM = CONF_ERR_ENUM_END,
	NETGRAPH_ERR_BAD_MASK_FORMAT,
	NETGRAPH_ERR_BAD_ADDR_FORMAT,
	NETGRAPH_ERR_BAD_MAC_FORMAT,
	NETGRAPH_ERR_TRAILING_VALUE,
	NETGRAPH_ERR_MISSING_NAME_VARIABLE,
	NETGRAPH_ERR_NET_WITHOUT_IP,
};

struct netgraph_net {
	char *name;
	uint8_t ip[16];
	int mask;
	struct conf_section *section;
};

struct netgraph_host {
	char *name;
	struct array ips; /* uint8_t[16] */
	struct array macs; /* uint8_t[6] */
	struct array links; /* struct netgraph_link */
	struct conf_section *section;
};

enum netgraph_type {
	NETGRAPH_T_IP,
	NETGRAPH_T_MAC,
	NETGRAPH_T_NAME,
};

struct netgraph_link {
	enum netgraph_type type;
	union {
		uint8_t mac[6];
		uint8_t ip[16];
		char const *name;
	} u;
};

/** src/netgraph.c **/
char const * netgraph_strerror(int i);
int netgraph_add_conf(struct array *nets, struct array *hosts, char *path, size_t *ln, struct mem_pool *pool);
struct netgraph_host * netgraph_next_linked(struct array *hosts, struct netgraph_link *link, size_t *i);

#endif
