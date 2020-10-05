#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, NEQ, GE, LE,  AND, OR, MINUS, POINTOR, NUMBER, HEX, REGISTER, MARK,
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
	{"\\b[0-9]+\\b",NUMBER,0},			// number
	{"\\b0[xX][0-9a-fA-F]+\\b",HEX,0},//hex
	{"\\$[a-zA-Z]+",REGISTER,0},		// register
	{"\\b[a-zA-Z_0-9]+" , MARK, 0},		// mark
	{"!=",NEQ,3},						// not equal	
	{"!",'!',6},						// not
	{"\\*",'*',5},						// mul
	{"/",'/',5},						// div
	{"\\t+",NOTYPE,0},					// tabs
	{"-",'-',4},						// sub
	{"&&",AND,2},						// and
	{">", '>', 3},      				// greater
	{"<", '<', 3}, 						// lower
	{">=", GE, 3},						// greater or equal
	{"<=", LE, 3},						// lower or equal
	{"\\|\\|",OR,1},					// or
	{"\\(",'(',7},                      // left bracket   
	{"\\)",')',7},                      // right bracket 
	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ}						// equal
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
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
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

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);


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
	//the operator with the min number is the dominant_oper
	int i ;
	int min_priority = 10;
	int oper = l;
	int count = 0;
	for(i = l; i <= r; i ++){
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
	return 0;
}

uint32_t eval(int l, int r){
	if (l > r){
		Assert(l>r, "wrong border!\n");
		return 1;
	}
	else if(l == r){
		//single token
		uint32_t num = 0;
		if(tokens[l].type == NUMBER){
			sscanf(tokens[l].str, "%d", &num);
		}
		else if(tokens[l].type == HEX){
			sscanf(tokens[l].str, "%x", &num);
		}
		else if(tokens[l].type == REGISTER){
			if(strlen(tokens[l].str) == 3){
				//32bits register
				int i = 0;
				for(; i < 8; i ++){
					if (strcmp(tokens[l].str, regsl[i]) == 0)
						break;
				}

			}
		}
	}
	return 0;
}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}
