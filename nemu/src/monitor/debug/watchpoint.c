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

WP* new_wp(){
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
	if(n >= 0 && n < NR_WP){
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
			for(p = head; p->next != NULL; p = p->next){
				if(p->next->NO == n){
					WP* q = p->next;
					p->next = p->next->next;
					q->next = free_;
					free_ = q;
					found = true;
					break;
				}
			}

		}
		if(!found){
			printf("no watchpoint with index '%d' is used\n", n);
		}
	}
	else{

		printf("Index %d out of range!(0<=index<%d)\n", n, NR_WP);
	}
}


void print_wp()
{
	WP *f;
	f=head;
	while (f!=NULL)
	{
		printf ("Watchpoint %d:, EXPR = %s, oldVal = %d \n",f->NO, f->expr, f->oldVal);
		f = f->next;
	}
}

bool check_wp(){
	printf("in check_wp\n");
	WP* p = head;
	bool success = true;
	for(p = head; p != NULL; p = p->next){
		uint32_t temp = expr(p->expr, &success);
		if(!success){
			Assert(0,"wrong in check_wp()\n");
		}
		if(temp != p->oldVal){
			printf ("Value of watchpoint %d: %s has changed!\n", p->NO, p->expr);
			printf ("Old value = %d has changed to %d\n", p->oldVal, temp);
			p->oldVal = temp;
			return true;
		}
	}
	return false;
}

