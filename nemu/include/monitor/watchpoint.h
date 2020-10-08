#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	uint32_t oldVal;
	char expr[32];
	/* TODO: Add more members if necessary */


} WP;
WP* new_wp();
void free_wp(int n);
bool check_wp();
void del_wp(int n);
void print_wp();
#endif
