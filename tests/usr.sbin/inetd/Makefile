.include <bsd.own.mk>

TESTSDIR=${TESTSBASE}/usr.sbin/inetd

TESTS_C += t_inetd

#inetd service, supports dgram and stream via args
MKMAN = no
PROGS += test_server
BINDIR=${TESTSDIR}

#Other files that should be copied to /usr/tests
FILESDIR=${TESTSDIR}
FILES=test_server inetd_ratelimit.conf

.include <bsd.test.mk>
