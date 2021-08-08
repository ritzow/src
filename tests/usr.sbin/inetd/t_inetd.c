#include <atf-c.h>
#include <spawn.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>

#define CHECK_ERROR(expr) ATF_REQUIRE_MSG((expr) != -1,\
    "Unexpected failure: %s", strerror(errno))

pid_t run(const char *, char **);
char	*concat(const char * restrict, const char * restrict);
void waitfor(pid_t, const char *);
bool run_udp_client(const char *, const char *, time_t);

ATF_TC(test_ip_ratelimit_wait);

ATF_TC_HEAD(test_ip_ratelimit_wait, tc) {
	atf_tc_set_md_var(tc, "descr", "Test IP rate limiting cases");
	atf_tc_set_md_var(tc, "require.user", "root");
	atf_tc_set_md_var(tc, "require.progs", "inetd");
	atf_tc_set_md_var(tc, "require.progs", "gcc");
}

ATF_TC_BODY(test_ip_ratelimit_wait, tc) {
	pid_t proc = run("gcc", (char*[]) {
		"gcc", concat(atf_tc_get_config_var(tc, "srcdir"), "/udpserver.c"),
		"-o", "udpserver",
		NULL
	});

	waitfor(proc, "Test server compilation");

	proc = run("inetd", (char*[]) {
		"inetd", "-d",
		concat(atf_tc_get_config_var(tc, "srcdir"), "/inetd_ratelimit.conf"),
		NULL
	});

	/* Wait for inetd to load the server */
	CHECK_ERROR(sleep(1));

	/* ip_max of 3, should receive these 3 responses */
	for(int i = 0; i < 3; i++) {
		ATF_REQUIRE(run_udp_client("127.0.0.1", "5432", 1));
	}
	
	/* Rate limiting should prevent a response to this request */
	ATF_REQUIRE(!run_udp_client("127.0.0.1", "5432", 1));

	CHECK_ERROR(kill(proc, SIGTERM));

	waitfor(proc, "Inetd execution");
}

ATF_TP_ADD_TCS(tp) {
	ATF_TP_ADD_TC(tp, test_ip_ratelimit_wait);
}

#define UDP 17

/* Return: true if successfully received message */
bool run_udp_client(const char *address, const char *port, time_t timeout_sec) {
	struct addrinfo hints = {
		.ai_flags = AI_NUMERICHOST,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = UDP
	};
	struct addrinfo * res;
	int error;

	ATF_REQUIRE_EQ_MSG(error = getaddrinfo(address, port, &hints, &res), 0, 
	    "%s", gai_strerror(error));

	ATF_REQUIRE(res->ai_next == NULL);

	char buffer[] = "test";

	int udp;
	CHECK_ERROR(udp = socket(res->ai_family, 
	    res->ai_socktype, res->ai_protocol));
	struct timeval timeout = { timeout_sec };
	setsockopt(udp, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	CHECK_ERROR(sendto(udp, buffer, sizeof(buffer), 0, 
	    res->ai_addr, res->ai_addrlen));

	struct iovec iov = {
		.iov_base = buffer,
		.iov_len = sizeof(buffer)
	};

	struct msghdr msg = {
		.msg_name = res->ai_addr,
		.msg_namelen = res->ai_addrlen,
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	struct mmsghdr msglist = {
		.msg_hdr = &msg
	};
	
	ssize_t count = recvmsg(udp, &msg, 0);
	freeaddrinfo(res);
	return count > 0;
}

pid_t run(const char * prog, char **args) {
	pid_t proc;
	extern char **environ;
	ATF_REQUIRE_EQ(posix_spawnp(&proc, prog, 
	    NULL, NULL, args, environ), 0);
	return proc;
}

void waitfor(pid_t pid, const char * taskname) {
	int status;
	CHECK_ERROR(waitpid(pid, &status, WALLSIG) == pid);

	ATF_REQUIRE_EQ_MSG(WEXITSTATUS(status), EXIT_SUCCESS, 
	    "%s failed with "
	    "exit status %d", taskname, WEXITSTATUS(status));
}

char * concat(const char * restrict left, const char * restrict right) {
	size_t len = strlen(left);
	char * storage = malloc(len + strlen(right) + 1);
	strcpy(storage, left);
	strcpy(storage + len, right);
	return storage;
}