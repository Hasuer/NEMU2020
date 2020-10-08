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

void free_wp(int n){
	WP* p;
	p = head;
	bool found = false;
	if (head->NO == n){
		head = head->next;
		p->next = free_;
		free_ = p;
		found = true;
	}
	else{
		for(p = head; p != NULL; p = p->next){
			if(p->NO == n){
				WP* q = p;
				p = p->next;
				q->next = free_;
				free_ = q;
				found = true;
				break;
			}
		}

	}
	if(!found){
		printf("no watchpoint with index '%d' is used", n);
	}
}

void del_wp(int n) {
	if(n >= 0 && n < NR_WP)
		free_wp(n);
	else
		printf("Index %d out of range!(0<=index<%d)\n", n, NR_WP);
}

void print_wp()
{
	WP *f;
	f=head;
	while (f!=NULL)
	{
		printf ("Watchpoint %d:",f->NO);
		f = f->next;
	}
}
