#ifndef NETINI_H
#define NETINI_H

#include <stdint.h>

#include "conf.h"

enum netini_errno {
	NETINI_ERR_SYSTEM = CONF_ERR_ENUM_END,
	NETINI_ERR_BAD_MASK_FORMAT,
	NETINI_ERR_BAD_ADDR_FORMAT,
	NETINI_ERR_BAD_MAC_FORMAT,
	NETINI_ERR_TRAILING_VALUE,
	NETINI_ERR_MISSING_NAME_VARIABLE,
	NETINI_ERR_NET_WITHOUT_IP,
	NETINI_ERR_MULTIPLE_NET_IP,
};

struct netini_net {
	char *name;
	uint8_t ip[16];
	int mask;
	struct conf_section *section;
};

struct netini_host {
	char *name;
	struct array ips; /* uint8_t[16] */
	struct array macs; /* uint8_t[6] */
	struct array links; /* struct netini_link */
	struct conf_section *section;
};

struct netini_graph {
	int init;
	struct array nets; /* struct netini_host */
	struct array hosts; /* struct netini_host */
	struct array ipsecs; /* struct conf_section */
};

enum netini_type {
	NETINI_T_IP,
	NETINI_T_MAC,
	NETINI_T_NAME,
};

struct netini_link {
	enum netini_type type;
	union {
		uint8_t mac[6];
		uint8_t ip[16];
		char const *name;
	} u;
};

/** src/netini.c **/
char const * netini_strerror(int i);
int netini_add_conf(struct netini_graph *graph, char *path, size_t *ln, struct mem_pool *pool);
int netini_init_graph(struct netini_graph *graph, struct mem_pool *pool);
struct netini_host * netini_next_linked(struct array *hosts, struct netini_link *link, size_t *i);

#endif
