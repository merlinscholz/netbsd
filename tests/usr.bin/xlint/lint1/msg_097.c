/*	$NetBSD: msg_097.c,v 1.5 2023/07/07 06:03:31 rillig Exp $	*/
# 3 "msg_097.c"

/* Test for message: suffix U is illegal in traditional C [97] */

/* lint1-flags: -gtw -X 191 */

void
example()
{
	int i = 1234567;
	unsigned u = 1234567;

	/* expect+1: warning: suffix U is illegal in traditional C [97] */
	unsigned u_upper = 1234567U;
	/* expect+1: warning: suffix U is illegal in traditional C [97] */
	unsigned u_lower = 1234567u;

	long l = 1234567L;
	/* expect+1: warning: suffix U is illegal in traditional C [97] */
	unsigned long ul = 1234567UL;

	long long ll = 1234567LL;
	/* expect+1: warning: suffix U is illegal in traditional C [97] */
	unsigned long long ull = 1234567ULL;
}
