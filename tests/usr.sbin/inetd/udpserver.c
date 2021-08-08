#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#define UDP 17	

int main() {
	openlog("udpserver", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
	char buffer[65535];
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
	count = recvmsg(0, &header, 0);
	if (count == -1) {
		syslog(LOG_ERR,
			"failed to get dgram source address: %s",
			strerror(errno));
			return 1;
	}

	sendto(0, buffer, count, 0, 
		(struct sockaddr*)(&addr), 
		addr.ss_len);

	int error = getnameinfo((struct sockaddr*)&addr, 
		addr.ss_len, name, NI_MAXHOST,
		NULL, 0, NI_NUMERICHOST);
	
	if (error) {
		syslog(LOG_ERR, "Error: %s\n", gai_strerror(error));
		return 1;
	} else {
		syslog(LOG_ERR, "Received message \"%s\" from %s\n", buffer, name);
	}
	return 0;
}
