/**
 * 26.12.2020 | Piotr Wyrwas
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MATH_C_VERSION 2.0	// Current version

#define IS_LETTER(x) ((x  >= 'a' && x <= 'z') || x == '_')
#define IS_DIGIT(x) (x >= '0' && x <= '9')

float last_result = 0.0f;	// Holds the last result
float fPass = 0.0f;			// This value gets passed to the functions
char *input;				// Input to parse
char *last_input; 			// Last input
int pos = 0;				// Position of the lexer
int should_output = 1;		// 1 if the float output should be printed (0 for variable/function declarations)
int should_exit = 0;		// 1 if the program should exit on the next iteration

typedef struct VARIABLE VARIABLE;

struct VARIABLE {
	char *identifier;
	float value;
	int constant;
};

int variables_count = 0;
VARIABLE *variables[300];

int variable_exists(char *identifier) {
	for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
        if (variables[i] == NULL)
            continue;
		VARIABLE *var = variables[i];
		if (!strcmp(var->identifier, identifier)) {
			return 1;
        }
	}
    return 0;
}

float get_variable(char *identifier) {
    for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
        if (variables[i] == NULL)
            continue;
        VARIABLE *var = variables[i];
        if (!strcmp(var->identifier, identifier)) {
            return var->value;
        }
    }
    return 0;
}

VARIABLE *create_variable(char *identifier, float value, int constant) {
	VARIABLE *var = (VARIABLE*) malloc(sizeof(VARIABLE));
	var->identifier = identifier;
	var->value = value;
	var->constant = constant;
	return var;
}

void add_variable(char *id, float val, int constant) {
	if (!variable_exists(id)) {
    	variables[++ variables_count] = create_variable(id, val, constant);
		return;
	}
	for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
		if (variables[i] == NULL)
			continue;
		VARIABLE *var = variables[i];
		if (!strcmp(var->identifier, id)) {
			if (var->constant) {
				printf("Unable to modify the variable: It is marked as constant.\n");
				return;
			}
			var->constant = constant;
			var->value = val;
			break;
		}
	}
}
typedef struct FUNCTION FUNCTION;

struct FUNCTION {
	char *identifier;
	char *snippet;
	int constant;
};

int functions_count = 0;
FUNCTION *functions[300];


FUNCTION *create_function(char *identifier, char *snippet, int constant) {
	FUNCTION *fun = (FUNCTION*) malloc(sizeof(FUNCTION));
	fun->snippet = (char*) calloc(100, sizeof(char));
	fun->identifier = (char*) calloc(100, sizeof(char));
	fun->constant = constant;
	strcpy(fun->snippet, snippet);
	strcpy(fun->identifier, identifier);
	return fun;
}

int function_exists(char*);

void add_function(char *identifier, char *snippet, int constant) {
	if (!function_exists(identifier)) {
		functions[++ functions_count] = create_function(identifier, snippet, constant);
		return;
	}
	for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
		if (functions[i] == NULL) {
			continue;
		}
		FUNCTION *fun = functions[i];
		if (!strcmp(fun->identifier, identifier)) {
			if (fun->constant) {
				printf("Unable to modify the function: It is marked as constant.\n");
				return;
			}
			fun->snippet = snippet;
			fun->constant = constant;
			return;
		}
	}
	printf("Failed to modify function '%s'.\n", identifier);
}

int function_exists(char *identifier) {
	for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
		if (functions[i] == NULL) {
			continue;
		}
		FUNCTION *fun = functions[i];
		if (!strcmp(fun->identifier, identifier)) {
			return 1;
		}
	}
	return 0;
}

FUNCTION *get_function(char *identifier) {
	for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
		if (functions[i] == NULL) {
			continue;
		}
		FUNCTION *fun = functions[i];
		if (!strcmp(fun->identifier, identifier)) {
			return fun;
		}
	}
	return NULL;
}

char current();

char last() {
	if (pos > 0) {
		pos --;
	} else {
		return current();
	}
	return input[pos];
}

char next() {
	if (pos < strlen(input)) {
		pos ++;
	} else {
		return '\0';
	}
	return input[pos];
}

char current() {
	return input[pos];
}

float term();

float expression() {
	float t = term();
	while (current() == '+' || current() == '-') {
		if (current() == '+') {
			next();
			t += term();
		} else {
			next();
			t -= term();
		}
	}
	return t;
}

void strncatchar(char *str, char c) {
	str[strlen(str)] = c;
	str[strlen(str)] = '\0';
}

char *identifier() {
	char *iden = (char*) calloc(100, sizeof(char));
	while (IS_LETTER(current())) {
		strncatchar(iden, current());
		next();
		if (!IS_LETTER(current())) {
			last();
			break;
		}
	}
	return iden;
}

float parse();

float number() {
	float number = 0.0;
	if (current() == '$') {
		next();
		return last_result;
	}
	if (current() == '?') {
		next();
		return fPass;
	}
    if (IS_LETTER(current())) {
		char *iden = identifier();
		next();
		if (current() == '[') {
			next();
			float fexpr = expression();
			fPass = fexpr;
			if (current() != ']') {
				printf("Expected ']' after expression of function '%s' but got '%c' on column %d\n", iden, current(), pos + 1);
				return 0;
			}
			next();
			if (!strcmp(iden, "sin")) {
				return sinf(fPass);
			}
			if (!strcmp(iden, "cos")) {
				return cosf(fPass);
			}
			if (!strcmp(iden, "inv")) {
				return fPass * (-1);
			}
			if (!strcmp(iden, "sq")) {
				return fPass * fPass;
			}
			if (!strcmp(iden, "sqrt")) {
				return sqrtf(fPass);
			}
			if (!strcmp(iden, "tan")) {
				return tanf(fPass);
			}
			if (!function_exists(iden)) {
				printf("Unknown function '%s' on column %d.\n", iden, pos);
				return 0;
			}
			FUNCTION *fun = get_function(iden);
			int oldPos = pos;
			char *oldInput = input;
			pos = 0;
			input = fun->snippet;
			float fExpr = parse();
			pos = oldPos;
			input = oldInput;
			return fExpr;
		}
        if (!variable_exists(iden)) {
			last();
            printf("Unknown variable '%s' on column %d. Assuming 0.\n", iden, pos + 1);
            number = 0;
        } else {
			last();
            number = get_variable(iden);
        }
        next();
        return number;
    }
	int ap = 0;
	int i = 0;
	while (1) {
		i = ++i;
		char c = current();
		if (c == ' ' || c == '\n' || c == '\t' || c == '\0' || !(c >= '0' && c <= '9' || c == '.')) {
			break;
		}
		if (c == '-' && i == 0) {
			number *= -1;
			next();
			continue;
		}
		if (c >= '0' && c <= '9') {
			if (ap == 0) {
				number = number * 10.0 + (c + 0.0) - '0';
			} else {
				number = number * pow(10.0, ap);
				number = number + c - '0';
				number = number / pow(10.0, ap);
				ap ++;
			}
			next();
			continue;
		}
		if (c == '.') {
			if (i <= 0 && ap != 0) {
				return -1.0; // Error
			}
			ap ++;
			next();
			continue;
		}
	}
	return number;
}

float factor() {
	if (IS_LETTER(current()) || IS_DIGIT(current()) || current() == '$' || current() == '?') {
		return number();
	}
	if (current() == '-') {
		next(); // Skip -
		return -factor();
	}
	while (current() == '(') {
		next(); // Skip (
		int expr = expression();
		next(); // Skip )
		next();
		return expr;
	}
}

float term() {
	float fac = factor();
	while (current() == '*' || current() == '/') {
		if (current() == '*') {
			next();
			fac *= factor();
		} else {
			next();
			fac /= factor();
		}
	}
	return fac;
}

// @ a = 1+5
float variable() {
	char *iden = (char*) calloc(100, sizeof(char));
	float expr;
	int is_const = 0;
	if (current() != '@') {
		printf("Expected expression or '@' on column %d.\n", pos);
		return 0;
	}
	next();
	if (!IS_LETTER(current())) {
		printf("Expected identifier after '@' on column %d.\n", pos);
		return 0;
	}
	iden = identifier();
	next();
	if (current() == '=') {
		goto _not_const;
	} else if (current() != '(') {
		printf("Expected '=' or '(const)' after identifier on column %d.\n", pos);
		return 0;
	}
	next();
	if (!IS_LETTER(current())) {
		printf("Expected 'const' after '(' on column %d.\n", pos);
		return 0;
	}
	char *const_iden = identifier();
	next();
	if (current() != ')') {
		printf("Expected ')' after 'const' on column %d.\n", pos);
		return 0;
	}
	if (strcmp(const_iden, "const")) {
		printf("Expected '=' or '(const)' after identifier on column %d.\n", pos);
		return 0;
	}
	is_const = 1;

	next();
	if (current() != '=') {
		printf("Expected '=' after '(const)' on column %d.\n", pos);
		return 0;
	}

	_not_const:
	next();
	expr = expression();
	
	add_variable(iden, expr, is_const);
	return 0;
}

// #identifier {? - 1}
float function() {
	int is_constant = 0;
	char *iden;
	char *snippet = (char*) calloc(100, sizeof(char));
	if (current() != '#') {
		printf("Expected '#' on column %d.\n", pos);
		return 0;
	}
	next();
	if (!IS_LETTER(current())) {
		printf("Expected identifier after '#' on column %d.\n", pos);
		return 0;
	}
	iden = identifier();
	next();
	if (current() != '(') {
		goto fun_not_constant;
	}
	next();
	if (!IS_LETTER(current())) {
		printf("Expected '(const)' or '{' after identifier on column %d.\n", pos);
		return 0;
	}
	char *const_iden = identifier();
	next();
	if (current() != ')') {
		printf("Expected ')' after 'const' on column %d.\n", pos);
		return 0;
	}
	if (strcmp(const_iden, "const")) {
		printf("Expected '(const)' or '{' after identifier on column %d.\n", pos);
		return 0;
	}
	is_constant = 1;
	next();
	fun_not_constant:
	if (current() != '{') {
		printf("Expected '{' after identifier on column %d.\n", pos);
		return 0;
	}
	next();
	while (current() != '}' && current() != '\0') {
		strncatchar(snippet, current());
		next();
	}
	if (current() == '\0') {
		printf("Expected '}' after function body on column %d.\n", pos);
		return 0;
	}
	add_function(iden, snippet, is_constant);
	next();
}

float parse() {
	if (current() == '@') {
		should_output = 0;
		return variable();
	}
	if (current() == '#') {
		should_output = 0;
		return function();
	}
	if (IS_DIGIT(current()) || IS_LETTER(current()) || current() == '-' || current() == '$' || current() == '?') {
		return expression();
	}
	printf("Unknown expression starting with '%c'.\n", current());
	should_output = 0;
	return 0;
}

int countinstr(char *str, char c) {
	int found = 0;
	for (int i = 0; i < strlen(str); i ++) {
		if (str[i] == c)
			found ++;
	}
	return found;
}

char *clean(char *input) {
	char *output = (char*) calloc(100, sizeof(char));
	for (int i = 0; i < strlen(input); i ++) {
		char c = input[i];
		if (c == ' ' || c == '\t' || c == '\n') {
			continue;
		}
		strncatchar(output, c);
	}
	return output;
}

void environment_command() {
	if (!strcmp(input, ".help")) {
		printf(".help		- Show this help page\n.clear		- Clear the console\n.last		- Repeat last input\n.exit		- Exit the program\n.quit		- Exit the program\n.lvar		- List variables\n.lfun		- List functions\n");
		return;
	} else if (!strcmp(input, ".exit") || !strcmp(input, ".quit")) {
		should_exit = 1;
		return;
	} else if (!strcmp(input, ".clear")) {
		for (int i = 0; i < 100; i ++) {printf("\n");}
		printf("Type math expressions here (ex. 1 + 2 * 2)\nUse .help for a list of commands.\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
		return;
	} else if (!strcmp(input, ".last")) {
		if (!strcmp(last_input, "")) {
			printf("There is no last expression.\n");
			return;
		}
		strcpy(input, last_input);
	} else if (!strcmp(input, ".lvar")) {
		printf("---------------------------------------------\n");
		for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
			if (variables[i] == NULL) {
				continue;
			}
			VARIABLE *var = variables[i];
			if (!var->constant) {
				printf("%s\n .. %f\n", var->identifier, var->value);
			} else {
				printf("%s [constant]\n .. %f\n", var->identifier, var->value);
			}
		}
		printf("------------------------------------------\n");
		return;
	} else if (!strcmp(input, ".lfun")) {
		printf("--------------------------------------------\n");
		for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
			if (functions[i] == NULL) {
				continue;
			}
			FUNCTION *fun = functions[i];
			if (!fun->constant) {
				printf("%s\n .. %s\n", fun->identifier, fun->snippet);
			} else {
				printf("%s [constant]\n .. %s\n", fun->identifier, fun->snippet);
			}
		}
		printf("------------------------------------------\n");
		return;
	} else  {
		printf("Unknown command. Use .help for help.\n");
		return;
	}
}

int main(void) {
	last_input = (char*) calloc(100, sizeof(char));
	input = (char*) calloc(100, sizeof(char));
    add_variable("math_c_version", MATH_C_VERSION, 1);
	add_function("sin", "sin[?]", 1);
	add_function("cos", "cos[?]", 1);
	add_function("tan", "tan[?]", 1);
	add_function("inv", "inv[?]", 1);
	add_function("sq", "sq[?]", 1);
	add_function("sqrt", "sqrt[?]", 1);

	printf("Type math expressions here (ex. 1 + 2 * 2)\nUse .help for a list of commands.\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	while (1) {
		if (should_exit) {
			return 0;
		}
		should_output = 1;
		pos = 0;

		printf(">>> ");
		fgets(input, 100, stdin);
		input = clean(input);

		if (input[0] == '.') {
			should_output = 0;
			environment_command();
			continue;
		}

		if (countinstr(input, '(') != countinstr(input, ')')) {
			printf("Unbalanced open and closed parantheses.\n");
			continue;	
		}
		if (countinstr(input, '{') != countinstr(input, '}')) {
			printf("Unbalanced open and closed curly brackets.\n");
			continue;	
		}
		if (countinstr(input, '[') != countinstr(input, ']')) {
			printf("Unbalanced open and closed square brackets.\n");
			continue;	
		}

		float expr = parse();
		if (should_output) {
			printf("= %f\n", expr); 
			last_result = expr;
		}
		strcpy(last_input, input);
	}
	free(input);
}