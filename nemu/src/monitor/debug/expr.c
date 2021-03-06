#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, NEQ, AND, OR, NEGATIVE, POINTOR, NUMBER, HEX, REGISTER, 
	/* TODO: Add more token types */
};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"\\b[0-9]+\\b", NUMBER, 0},		// number
	{"\\b0[xX][0-9a-fA-F]+\\b", HEX, 0},	//hex
	{"\\$[a-zA-Z]+", REGISTER, 0},		// register

	{"\\+", '+', 4},	// plus
	{"-", '-', 4},		// sub
	{"\\*", '*', 5},	// mul
	{"/", '/', 5},		// div

	{"==", EQ, 3},		// equal
	{"!=", NEQ, 3},		// not equal	
	{"&&", AND, 2},		// and
	{"\\|\\|", OR, 1},	// or
	{"!", '!', 6},		// not

	{"\\t+", NOTYPE, 0},	// tabs
	{" +", NOTYPE, 0},	// spaces
	{"\\(", '(', 7},	// left bracket   
	{"\\)", ')', 7},	// right bracket 
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[128];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
//	printf("in make token\n");
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

//				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);


				//append begin
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				char* temp = e + position + 1;//for REGISTER use,because of removing the '$' ahead of regisiter 
				switch(rules[i].token_type){
					case 256:
						break;
					case REGISTER://remove the "$" ahead of the regisiter
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy (tokens[nr_token].str, temp, substr_len - 1);
						tokens[nr_token].str[substr_len - 1] = '\0';
						nr_token ++;
						break;
					default:
						tokens[nr_token].type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
						strncpy (tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token ++;
				}
				position += substr_len;
				break;
			}
		}
		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	return true;
}
//check the brackets
bool check_parentheses(int l ,int r){
//	printf("in check_paratheses, l = %d, n = %d\n", l , r);
	int i,tag = 0;
	if(tokens[l].type != '(' || tokens[r].type != ')') return false;  
	for(i = l ; i <= r ; i ++){    
		if(tokens[i].type == '(') tag++;
		else if(tokens[i].type == ')') tag--;
		if(tag == 0 && i < r) return false ;  
	}                              
	if( tag != 0 ) return false;   
	return true;                   
} 

//dominant_oper, return the position of the oper in tokens[]
int dominant_operator(int l, int r){
//	printf("in dominant_opre, l = %d, r = %d\n", l ,r);
	//the operator with the min number is the dominant_oper
	int i ;
	int min_priority = 10;
	int oper = l;
	int count = 0;
	for(i = l; i <= r; i ++){
		if (tokens[i].type == NUMBER || tokens[i].type == HEX || tokens[i].type == REGISTER){
			continue;
		}
		if(tokens[i].type == '('){
			count ++;
		}
		if(tokens[i].type == ')'){
			count --;
		}
		if(count != 0)// meaning still in a bracket
			continue;
		if(tokens[i].priority <= min_priority){
			min_priority = tokens[i].priority;
			oper = i;
		}
	}
	return oper;
}

uint32_t eval(int l, int r){
//	printf("in eval l = %d, r = %d\n", l ,r);
	if (l > r){
		Assert(0, "wrong border!\n");
		return 1;
	}
	else if(l == r){
		//	printf("in eval ==\n");
		//single token
		uint32_t num = 0;
		if(tokens[l].type == NUMBER){

			//	printf("in eval number\n");
			sscanf(tokens[l].str, "%d", &num);
		}
		else if(tokens[l].type == HEX){
			//	printf("in eval hex\n");
			sscanf(tokens[l].str, "%x", &num);
		}
		else if(tokens[l].type == REGISTER){
			if(strlen(tokens[l].str) == 3){

				//		printf("in eval 3R\n");
				//32bits register
				int i = 0;
				for(; i < 8; i ++){
					if (strcmp(tokens[l].str, regsl[i]) == 0)
						break;
				}
				if(i == 8){
					if(strcmp(tokens[l].str, "eip") == 0)
						num = cpu.eip;
					else 
						Assert(0, "register not exist!\n");
				}
				else
					num = reg_l(i);
			}
			else if (strlen (tokens[l].str) == 2) {

				//	printf("in eval 2R\n");
				if (tokens[l].str[1] == 'x' || tokens[l].str[1] == 'p' || tokens[l].str[1] == 'i') {
					int i;
					for (i = R_AX; i <= R_DI; i ++)
						if (strcmp (tokens[l].str,regsw[i]) == 0)break;
					num = reg_w(i);
				}
				else if (tokens[l].str[1] == 'l' || tokens[l].str[1] == 'h') {
					int i;
					for (i = R_AL; i <= R_BH; i ++)
						if (strcmp (tokens[l].str,regsb[i]) == 0)break;
					num = reg_b(i);
				}
				else
					Assert(0, "register not exist!\n");
			}
		}
		return num;
	}
	else if (check_parentheses (l, r) == true)
		return eval (l + 1, r - 1);
	else {

		//	printf("in eval l < r\n");
		int opre = dominant_operator (l,r);
		if (l == opre || tokens[opre].type == POINTOR || tokens[opre].type == NEGATIVE || tokens[opre].type == '!')
		{
			uint32_t val = eval(l + 1,r);
			switch (tokens[l].type)
			{
				case POINTOR:
					return swaddr_read (val,4);
				case NEGATIVE:
					return -val;
				case '!':
					return !val;
				default :
					Assert (0,"wrong expression in case 'l < r'\n");
			} 
		}

		uint32_t val1 = eval(l, opre - 1);
		uint32_t val2 = eval(opre + 1, r);
		switch (tokens[opre].type)
		{
			case '+':
				return val1 + val2;
			case '-':
				return val1 - val2;
			case '*':
				return val1 * val2;
			case '/':
				return val1 / val2;
			case EQ:
				return val1 == val2;
			case NEQ:
				return val1 != val2;
			case AND:
				return val1 && val2;
			case OR:
				return val1 || val2;
			default:
				break;
		}
	}
	return -1234234;
}
uint32_t expr(char *e, bool *success) {
//	printf("in expr\n");
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int i = 0;
	for (i = 0;i < nr_token; i ++) {
		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HEX && tokens[i - 1].type != REGISTER && tokens[i - 1].type !=')'))) {
			tokens[i].type = POINTOR;
			tokens[i].priority = 6;
		}
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUMBER && tokens[i - 1].type != HEX && tokens[i - 1].type != REGISTER && tokens[i - 1].type !=')'))) {
			tokens[i].type = NEGATIVE;
			tokens[i].priority = 6;
		}
	}
	return eval(0, nr_token - 1);
}
