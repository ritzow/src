#ifndef _INETD_H
#define _INETD_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>

#include <arpa/inet.h>

#include <netdb.h>

#include "pathnames.h"

#ifdef IPSEC
#include <netipsec/ipsec.h>
#ifndef IPSEC_POLICY_IPSEC	/* no ipsec support on old ipsec */
#undef IPSEC
#endif
#include "ipsec.h"
#endif

#define NORM_TYPE	0
#define MUX_TYPE	1
#define MUXPLUS_TYPE	2
#define FAITH_TYPE	3
#define ISMUX(sep)	(((sep)->se_type == MUX_TYPE) || \
			 ((sep)->se_type == MUXPLUS_TYPE))
#define ISMUXPLUS(sep)	((sep)->se_type == MUXPLUS_TYPE)

#define	A_CNT(a)	(sizeof (a) / sizeof (a[0]))
#define	TOOMANY		40		/* don't start more than TOOMANY */

#define CONF_ERROR_FMT "%s line %zu: "

/* Log warning/error with 0 or variadic args with line number and file name */
#define IL0(prio, msg) syslog(prio, CONF_ERROR_FMT msg ".", \
	CONFIG, line_number)

#define ILV(prio, msg, args...) syslog(prio, CONF_ERROR_FMT msg ".", \
	CONFIG, line_number, args)

#define WRN0(msg) IL0(LOG_WARNING, msg)
#define ERR0(msg) IL0(LOG_ERR, msg)
#define WRN(msg, args...) ILV(LOG_WARNING, msg, args)
#define ERR(msg, args...) ILV(LOG_ERR, msg, args)

struct	servtab {
	char	*se_hostaddr;		/* host address to listen on */
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	int	se_family;		/* address family */
	char	*se_proto;		/* protocol used */
	int	se_sndbuf;		/* sndbuf size */
	int	se_rcvbuf;		/* rcvbuf size */
	int	se_rpcprog;		/* rpc program number */
	int	se_rpcversl;		/* rpc program lowest version */
	int	se_rpcversh;		/* rpc program highest version */
#define isrpcservice(sep)	((sep)->se_rpcversl != 0)
	pid_t	se_wait;		/* single threaded server */
	short	se_checked;		/* looked at during merge */
	char	*se_user;		/* user name to run as */
	char	*se_group;		/* group name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
#define	MAXARGV 64
	char	*se_argv[MAXARGV+1];	/* program arguments */
#ifdef IPSEC
	char	*se_policy;		/* IPsec poilcy string */
#endif
	struct accept_filter_arg se_accf; /* accept filter for stream service */
	int	se_fd;			/* open descriptor */
	int	se_type;		/* type */
	union {
		struct	sockaddr se_ctrladdr;
		struct	sockaddr_in se_ctrladdr_in;
		struct	sockaddr_in6 se_ctrladdr_in6; /* in6 is used by bind()/getaddrinfo */
		struct	sockaddr_un se_ctrladdr_un;
	};			/* bound address */
	int	se_ctrladdr_size;
	int	se_service_max;		/* max # of instances of this service */
	int	se_count;		/* number of instances of this service started since se_time */
	int	se_ip_max;  		/* max # of instances of this service per ip per minute */
	struct se_ip_list_node{
		char address[14];
		int count;		/* number of instances of this service started from
						this ip address since se_time */
		struct se_ip_list_node* next;
	} *se_ip_list_head; 		/* linked list of number of requests per ip */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
};

/* From inetd.c */
int 	parse_protocol(struct servtab *);
int 	parse_wait(struct servtab *, int wait);
int 	parse_server(struct servtab *, const char *);
void 	parse_socktype(char *, struct servtab *);
void 	parse_accept_filter(char *, struct servtab *);
char 	*nextline(FILE *);
char 	*newstr(const char *);
void	freeconfig(struct servtab *);

/* Global debug mode boolean, enabled with -d */
extern int debug;

/* Current config file path */
extern const char *CONFIG;

/* Open config file */
extern FILE	*fconfig;

/* Current line number in current config file */
extern size_t line_number;

/* Default listening hostname/IP for current config file */
extern char *defhost;

/* Default IPsec policy for current config file */
extern char *policy;

/* "Unspecified" indicator value for servtabs (mainly used by v2 syntax) */
#define SE_SERVICE_MAX_UNINIT -1
#define SE_IP_MAX_UNINIT -1
#define SE_WAIT_UNINIT -1
#define SE_SOCKTYPE_UNINIT -1

#endif
