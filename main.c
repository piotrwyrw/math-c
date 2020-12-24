#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float last_result = 0.0f;

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


char *input;
int pos = 0;

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

// float strtofloat(char *str) {
// 	int ap = 0; // Digits after point
// 	float rslt = 0.0f;
// 	for (int i = 0; i < strlen(str); i ++) {
// 		char c = str[i];
// 		if (c == '-' && i == 0) {
// 			rslt *= -1;
// 			continue;
// 		}
// 		if (c >= '0' && c <= '9') {
// 			if (ap == 0) {
// 				rslt = rslt * 10.0 + (c + 0.0) - '0';
// 			} else {
// 				rslt = rslt * pow(10.0, ap);
// 				rslt = rslt + c - '0';
// 				rslt = rslt / pow(10.0, ap);
// 				ap ++;
// 			}
// 			continue;
// 		}
// 		if (c == '.') {
// 			if (i <= 0 && ap != 0)
// 				return 0.0; // Error
// 			ap ++;
// 			continue;
// 		}
// 	}
// 	return rslt;
// }

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
    if (current() >= 'a' && current() <= 'z') {
		char *iden = identifier();
		next();
		if (current() == '[') {
			next();
			float fexpr = expression();
			if (current() != ']') {
				printf("Expected ']' after expression of function '%s' but got '%c' on column %d\n", iden, current(), pos + 1);
				return 0;
			}
			next();
			if (!strcmp(iden, "sin")) {
				return sinf(fexpr);
			}
			if (!strcmp(iden, "cos")) {
				return cosf(fexpr);
			}
			if (!strcmp(iden, "inv")) {
				return fexpr * (-1);
			}
			if (!strcmp(iden, "sq")) {
				return fexpr * fexpr;
			}
			if (!strcmp(iden, "sqrt")) {
				return sqrtf(fexpr);
			}
			printf("Unknown function '%s' on column %d.\n", iden, pos + 1);
			return 0;
		}
        if (!variable_exists(iden)) {
            printf("Unknown variable '%s' on column %d. Assuming 0.\n", iden, pos + 1);
            number = 0;
        } else {
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
	if ((current() >= 'a' && current() <= 'z') || (current() >= '0' && current() <= '9') || current() == '$') {
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

	printf("Type math expressions here (ex. 1 + 2 * 2)\nUse .help for help, exit with .exit or .quit\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");

	while (1) {
		printf(">>> ");
		fgets(input, 100, stdin);
		input = clean(input);
		pos = 0;
		if (!strcmp(input, ".exit") || !strcmp(input, ".quit")) {
			return 0;
		}
		if (!strcmp(input, ".help")) {
			printf("_______________________________________________________________________________");
			printf("\nType in a valid mathematical expression to evaluate it. Example:\n>>> 1 + 2 * (1 + 5)\n= 13.000000\n");
			printf("\nYou can use functions in your calculations. The available functions are (inv, sin, cos, sq, sqrt). Examples:\n>>> inv[ sq[8] ]\n= -64.00000\n>>> inv[1 + 2]\n= -3.00000\n");
			printf("\nThe environment commands are (.exit, .quit, .last, .help).\n.exit       - Exits the program\n.quit       - Exits the program\n.last       - Evaluates the last expression\n.help       - Shows this help page\n");
			printf("\nVariables are still in developement at the moment. To insert the\nresult of the last calculation into your expression, use $. Example:\n>>> $+1\n= 1.00000\n>>> .last\n= 2.00000\n>>> .last\n= 3.00000\n...\n");
			printf("_______________________________________________________________________________\n");
			continue;
		}
		if (!strcmp(input, ".last")) {
			if (!strcmp(last_input, "")) {
				printf("There is no last expression.\n");
				continue;
			}
			strcpy(input, last_input);
		}
		if (countinstr(input, '(') != countinstr(input, ')')) {
			printf("Unbalanced open and closed parantheses.\n");
			continue;	
		}
		if (countinstr(input, '[') != countinstr(input, ']')) {
			printf("Unbalanced open and closed square brackets.\n");
			continue;	
		}
		float expr = expression();
		printf("= %f\n", expr); 
		last_result = expr;
		strcpy(last_input, input);
	}
	free(input);
}