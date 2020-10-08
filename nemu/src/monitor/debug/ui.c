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
		return 1;
	}
	if (strtok(NULL, " ") != NULL){
		printf("Too many parameters!\n");
		return 1;
	}
	int num ;
	sscanf(args, "%d", &num);
	//	printf("num = %d\n", num);
	if (num < 0){
		printf("N should be positive\n");
		return 1;
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
		printf("Just one parameter. Mismatch the format [info SUNSMD]\n");
		return 0;
	}
	if (strtok(NULL, " ") != NULL){
		printf("Too many parameters. Mismatch the format\n");
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
	else if (strcmp(token, "w") == 0){
		print_wp();
	}
	else{
		printf("Wrong parameter!\n");
		return 0;
	}
	return 0;
}

static int cmd_x(char* args){
	char* arg = strtok(args," ");
	if(arg == NULL){
		//test if there is only one parameter
		printf("Only one parameter. mismatch the format! [x N EXPR] \n");
		return 1;
	}
	char* EXPR = strtok(NULL, " "); // get EXPR
	if(EXPR == NULL){
		// test if there is only two parameter
		printf("Only two parameter. mismatch the format! [X N EXPR]\n");
		return 0;
	}
	if(strtok(NULL," ")!=NULL){
		//test if there are more than three  parameter
		printf("Too many parameters. mismatch the format! [x N EXPR] \n");
		return 0;
	}
	int n;
	swaddr_t address; 
	sscanf(args, "%d", &n); //get n
	sscanf(EXPR, "%x", &address);//get start address
	int i = 0;
	for (;i < n; i ++)
	{
		printf ("0x%08x ",swaddr_read (address,4));
		address+=4;
	}
	printf ("\n");
	return 0;
}

static int cmd_p(char* args){
	char* token = strtok(args, " ");
	if(token == NULL){
		printf("Just one parameter.Mismatch the format [p EXPR]\n");
		return 0;
	}
	if(strtok(NULL, " ") != NULL){
		printf("Too many parameters.Mismatvh the foramt [p EXPR\n]");
		return 0;
	}
	bool success = true;
	uint32_t result = expr(args, &success);
	if(!success)
		printf("Invalid ecpression!\n");
	else 
		printf("Answer is %d\n", result);
	return 0;
}

static int cmd_setwp(char* args){
	char* token = strtok(args, " ");
	if(token == NULL){
		printf("Just one parameter.Mismatch the format [w EXPR]\n");
		return 0;
	}
	if(strtok(NULL, " ") != NULL){
		printf("Too many parameters.Mismatvh the foramt [w EXPR]\n");
		return 0;
	}
	// begin set new watchpoint
	WP* temp = new_wp();
	bool suc;
//	printf ("Watchpoint %d: %s\n",temp->NO,args);
	temp->oldVal = expr (args,&suc);
//	strcpy (temp->expr,args);
	if (!suc)
		Assert (0,"Set watchpoint failed\n");
	printf ("Watchpoint set! EXPR: %s, NO: %d, Value : %d\n", args, temp->NO, temp->oldVal);
	return 0;
}

static int cmd_d(char* args){
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
	{"p", "[p EXPR] to calculate the expression.\n", cmd_p},
	{"w", "[w EXPR] to set the watchpoint\n", cmd_setwp},
	{"d", "[d N] to delete the watchc point\n", cmd_d},
	//	{"bt", "print the stack", cmd_bt},
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
