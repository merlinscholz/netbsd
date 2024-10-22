/*	$NetBSD: msg_353.c,v 1.2 2023/07/03 09:33:07 rillig Exp $	*/
# 3 "msg_353.c"

// Test for message 353: empty initializer braces require C23 or later [353]
//
// See also:
//	c23.c

/* lint1-extra-flags: -Ac11 */

void
c23(void)
{
	struct s {
		int member;
	} s;

	/* expect+1: error: empty initializer braces require C23 or later [353] */
	s = (struct s){};
	s = (struct s){0};
}
