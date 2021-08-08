.include <bsd.own.mk>

TESTSDIR=               ${TESTSBASE}/usr.sbin/inetd

#.for file in t_v1_syntax t_v2_syntax
#TESTS_SH+=              ${name}
#TESTS_SH_SRC_${name}=   h_common.sh ${name}.sh
#.endfor

TESTS_C += t_inetd

#TESTS_SH += t_ratelimit

FILESDIR= ${TESTSDIR}
FILES=udpserver.c inetd_ratelimit.conf

.include <bsd.test.mk>
