#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* args){
	 if (free_ == NULL) {
		     printf("there are no free watchpoints.\n");
		         return NULL;
			   }
	   WP *p = free_;
	     free_ = free_->next;

	       p->next = head;
	         head = p;

		   return p;
}
