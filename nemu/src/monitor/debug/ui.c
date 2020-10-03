#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_Sstep(char* args){
	char* token = strtok(args, " ");
	if (token == NULL){
		cpu_exec(1);
		return 0;
	}
	int num ;
	sscanf(args, "%d", &num);
	//	printf("num = %d\n", num);
	if (num < 0){
		printf("N should be positive\n");
		return 0;
	}
	int i = 0;
	for(;i < num; i ++){
		cpu_exec(1);
	}
	return 0;
}

static int cmd_info(char* args){
	char* token = strtok(args," ");
	if (token == NULL){
		printf("just one parameter. Mismatch the format [info SUNSMD]\n");
		return 0;
	}
	if (strtok(NULL, " ") != NULL){
		printf("too many parameters. Mismatch the format\n");
		return 0;
	}	
	if (strcmp(token,"r") == 0){
		int i = 0;
		printf("name          value\n");
		printf("----------------------\n");
		for(; i < 8; i ++){
			printf("%s        0x%x\n", regsl[i], cpu.gpr[i]._32);
		}
		printf("eip        0x%x\n",  cpu.eip);
	}
	else{
		printf("wrong parameter!\n");
		return 0;
	}
	return 0;
}

static int cmd_x(char* args){
	char* arg = strtok(args," ");
	if(arg == NULL){
		//test if there is only one parameter
		printf("only one parameter. mismatch the format! [x N EXPR] \n");
		return 1;
	}
	char* EXPR = strtok(NULL, " "); // get EXPR
	if(EXPR == NULL){
		// test if there is only two parameter
		printf("only two parameter. mismatch the format! [X N EXPR]");
		return 0;
	}
	if(strtok(NULL," ")!=NULL){
		//test if there are more than three  parameter
		printf("to many parameters. mismatch the format! [x N EXPR] \n");
		return 0;
	}
	int n;
	swaddr_t address; 
	sscanf(args, "%d", &n); //get n
	sscanf(EXPR, "%d", &address);//get start address
	int i = 0;
	for (;i<=n;i++)
	{
		printf ("0x%08x ",swaddr_read (address,4));
		address+=4;
	}
	printf ("\n");
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	/* TODO: Add more commands */
	{"si", "Single Step. si [N] for exectuating N steps, the default N is 1.", cmd_Sstep},
	{"info", "[info r] to print the register status, [info w] to print the watchpoint info", cmd_info},
	{"x", "[x N EXPR] to scan the memory", cmd_x},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
