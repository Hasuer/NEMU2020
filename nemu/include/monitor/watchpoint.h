#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */


} WP;
WP* new_wp (char* args);
void free_wp(int n);
//bool check_wp();
void del_wp(int n);
void print_wp();
#endif
