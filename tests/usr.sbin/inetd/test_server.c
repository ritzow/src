#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>

#define CHECK(expr) do {\
	if ((expr) == -1) {\
		syslog(LOG_ERR, "Error at %s:%d: %s", \
		    __FILE__, __LINE__, \
		    strerror(errno));\
		exit(1);\
	}\
} while(0);

static void stream_nowait_service(void);
static void stream_wait_service(void);
static void dgram_wait_service(void);

int
main(int argc, char **argv)
{
	openlog("inetd_test_server", LOG_PID | LOG_NOWAIT, LOG_DAEMON);

	if (argc < 3) {
		syslog(LOG_ERR, "Invalid arg count");
	}

	/* Run the correct service according to the args */
	if (strcmp(argv[1], "dgram") == 0) {
		if (strcmp(argv[2], "wait") == 0) {
			dgram_wait_service();
		} else {
			syslog(LOG_ERR, "Invalid arg %s", argv[2]);
		}
	} else if (strcmp(argv[1], "stream") == 0) {
		if (strcmp(argv[2], "wait") == 0) {
			stream_wait_service();
		} else if (strcmp(argv[2], "nowait") == 0) { 
			stream_nowait_service();
		} else {
			syslog(LOG_ERR, "Invalid arg %s", argv[2]);
		}
	} else {
		syslog(LOG_ERR, "Invalid args %s %s", argv[1], argv[2]);
	}
	return 0;
}

static void
stream_nowait_service()
{
	ssize_t count;
	char buffer[10];
	CHECK(count = recv(0, buffer, sizeof(buffer), 0));
	CHECK(send(1, buffer, count, 0));
}

static void
stream_wait_service()
{
	struct sockaddr_storage addr;
	ssize_t count;
	int fd;
	socklen_t addr_len;
	char buffer[10];

	CHECK(fd = accept(0, (struct sockaddr*)&addr, &addr_len));
	CHECK(count = recv(fd, buffer, sizeof(buffer), 0));
	CHECK(send(fd, buffer, count, 0));
	CHECK(shutdown(fd, SHUT_RDWR));
	CHECK(close(fd));
}

static void
dgram_wait_service()
{
	char buffer[256];
	char name[NI_MAXHOST];
	socklen_t source_size;
	struct sockaddr_storage addr;

	struct iovec store = {
		.iov_base = &buffer,
		.iov_len = sizeof(buffer)
	};
	struct msghdr header = {
		.msg_name = &addr,
		.msg_namelen = sizeof(struct sockaddr_storage),
		.msg_iov = &store,
		.msg_iovlen = 1
		/* scatter/gather and control info is null */
	};
	int count;

	/* Peek so service can still get the packet */
	CHECK(count = recvmsg(0, &header, 0));

	CHECK(sendto(1, buffer, count, 0, 
	    (struct sockaddr*)(&addr), 
	    addr.ss_len));
	
	int error = getnameinfo((struct sockaddr*)&addr, 
	    addr.ss_len, name, NI_MAXHOST,
	    NULL, 0, NI_NUMERICHOST);
	
	if (error) {
		syslog(LOG_ERR, "getnameinfo error: %s\n", gai_strerror(error));
		return;
	}
	//syslog(LOG_ERR, "Received message \"%s\" from %s\n", buffer, name);
}
