/**
 * 26.12.2020 | Piotr Wyrwas
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float last_result = 0.0f;	// Holds the last result
char *input;				// Input to parse
int pos = 0;				// Position of the lexer
int should_output = 1;		// 1 if the float output should be printed (0 for variable/function declarations)
float fPass = 0.0f;			// This value gets passed to the functions

typedef struct VARIABLE VARIABLE;

struct VARIABLE {
	char *identifier;
	float value;
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

VARIABLE *create_variable(char *identifier, float value) {
	VARIABLE *var = (VARIABLE*) malloc(sizeof(VARIABLE));
	var->identifier = identifier;
	var->value = value;
	return var;
}

void add_variable(char *id, float val) {
    variables[++ variables_count] = create_variable(id, val); 
}

void mod_variable(char *id, float nVal) {
	if (variable_exists(id)) {
		for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
			if (variables[i] == NULL)
				continue;
			VARIABLE *var = variables[i];
			if (!strcmp(var->identifier, id)) {
				var->value = nVal;
				break;
			}
		}
	}
}

typedef struct FUNCTION FUNCTION;

struct FUNCTION {
	char *identifier;
	char *snippet;
};

int functions_count = 0;
FUNCTION *functions[300];


FUNCTION *create_function(char *identifier, char *snippet) {
	FUNCTION *fun = (FUNCTION*) malloc(sizeof(FUNCTION));
	fun->snippet = (char*) calloc(100, sizeof(char));
	fun->identifier = (char*) calloc(100, sizeof(char));
	strcpy(fun->snippet, snippet);
	strcpy(fun->identifier, identifier);
	return fun;
}

int function_exists(char*);

void add_function(char *identifier, char *snippet) {
	if (!function_exists(identifier)) {
		functions[++ functions_count] = create_function(identifier, snippet);
		return;
	}
	printf("Function '%s' already exists. Modifying ..\n", identifier);
	for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
		if (functions[i] == NULL) {
			continue;
		}
		FUNCTION *fun = functions[i];
		if (!strcmp(fun->identifier, identifier)) {
			fun->snippet = snippet;
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
	while ((current() >= 'a' && current() <= 'z')) {
		strncatchar(iden, current());
		next();
		if (!(current() >= 'a' && current() <= 'z')) {
			last();
			break;
		}
	}
	return iden;
}

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
    if (current() >= 'a' && current() <= 'z') {
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
			float fExpr = expression();
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
	if ((current() >= 'a' && current() <= 'z') || (current() >= '0' && current() <= '9') || current() == '$' || current() == '?') {
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
	if (current() != '@') {
		printf("Expected expression or '@' on column %d.\n", pos);
		return 0;
	}
	next();
	if (!(current() >= 'a' && current() <= 'z')) {
		printf("Expected identifier after '@' on column %d.\n", pos);
		return 0;
	}
	iden = identifier();
	next();
	if (current() != '=') {
		printf("Expected '=' after identifier on column %d.\n", pos);
		return 0;
	}
	next();
	expr = expression();
	
	if (variable_exists(iden)) {
		printf("Variable '%s' already exists. Modifying ...\n", iden);
		mod_variable(iden, expr);
		return 0;
	}
	add_variable(iden, expr);
	return 0;
}

// #identifier {? - 1}
float function() {
	char *iden;
	char *snippet = (char*) calloc(100, sizeof(char));
	if (current() != '#') {
		printf("Expected '#' on column %d.\n", pos);
		return 0;
	}
	next();
	if (!(current() >= 'a' && current() <= 'z')) {
		printf("Expected identifier after '#' on column %d.\n", pos);
		return 0;
	}
	iden = identifier();
	next();
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
	add_function(iden, snippet);
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
	if (current() >= '0' && current() <= '9' || current() == '-' || current() >= 'a' && current() <= 'z' || current() == '$' || current() == '?') {
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

int main(void) {
	char *last_input = (char*) calloc(100, sizeof(char));
	input = (char*) calloc(100, sizeof(char));
    add_variable("x", 1.5f);

	printf("Type math expressions here (ex. 1 + 2 * 2)\nUse .help for a list of commands.\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	while (1) {
		// fPass = 0.0f;
		should_output = 1;
		printf(">>> ");
		fgets(input, 100, stdin);
		input = clean(input);
		pos = 0;
		if (!strcmp(input, ".help")) {
			printf(".help		- Show this help page\n.last		- Repeat last input\n.exit		- Exit the program\n.quit		- Exit the program\n.lvar		- List variables\n.lfun		- List functions\n");
			continue;
		} else if (!strcmp(input, ".exit") || !strcmp(input, ".quit")) {
			return 0;
		} else if (!strcmp(input, ".last")) {
			if (!strcmp(last_input, "")) {
				printf("There is no last expression.\n");
				continue;
			}
			strcpy(input, last_input);
		} else if (!strcmp(input, ".lvar")) {
			printf("Listing variables ..\n");
			for (int i = 0; i < sizeof(variables) / sizeof(VARIABLE); i ++) {
				if (variables[i] == NULL) {
					continue;
				}
				VARIABLE *var = variables[i];
				printf("%s\n .. %f\n", var->identifier, var->value);
			}
			continue;
		} else if (!strcmp(input, ".lfun")) {
			printf("Listing functions ..\n");
			printf("sin\n .. (built in)\ncos\n .. (built in)\ntan\n .. (built in)\ninv\n .. (built in)\nsq\n .. (built in)\nsqrt\n .. (built in)\n");
			for (int i = 0; i < sizeof(functions) / sizeof(FUNCTION); i ++) {
				if (functions[i] == NULL) {
					continue;
				}
				FUNCTION *fun = functions[i];
				printf("%s\n .. %s\n", fun->identifier, fun->snippet);
			}
			continue;
		} else if (input[0] == '.') {
			printf("Unknown command. Use .help for help.\n");
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