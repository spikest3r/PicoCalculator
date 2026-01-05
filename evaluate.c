#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int isNum(char* str) {
    int len = strlen(str);
    for(int i = 0; i < len; i++) {
        if(isdigit(str[i]) || (str[i] == '-' && i == 0 && len != 1) || str[i] == '.') continue;
        else return 0;
    }
    return 1;
}

int isop(const char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

char* strip(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *res = malloc(len + 1);
    if (!res) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++)
        if (s[i] != '.') res[j++] = s[i];

    res[j] = '\0';
    return res;
}

static inline int get_prec(const char c) {
    switch(c) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
    }
    return 0;
}

void* tokenize(const char* expr, const int size, int* retSize) {
    char (*tokens)[16] = malloc(24 * sizeof(*tokens));
    int tokenIndex = 0;

    char num[16] = "";
    int numIndex = 0;

    for(int i = 0; i < size; i++) {
        char ch = expr[i];
        if (ch == '-' && (i == 0 || isop(expr[i-1]) || expr[i-1] == '(')) {
            num[numIndex++] = ch;
            continue;
        }
        if(isdigit(ch) || ch == '.') {
            num[numIndex++] = ch;
        } else {
            if(numIndex > 0) {
                num[numIndex] = '\0';
                strncpy(tokens[tokenIndex++], num, sizeof(tokens[0]));
                memset(num,0,16);
                numIndex = 0;
            }
            if(isop(ch)) {
                tokens[tokenIndex][0] = ch;
                tokens[tokenIndex][1] = '\0';
                tokenIndex++;
            }
        }
    }

    if(numIndex > 0) {
        num[numIndex] = '\0';
        strncpy(tokens[tokenIndex++], num, sizeof(tokens[0]));
    }

    *retSize = tokenIndex;
    return tokens;
}

void* shunting_yard(void* tokens, int size, int* retSize) {
    char (*a)[16] = tokens;
    char (*out)[16] = malloc(24 * sizeof(*out));
    int outIndex = 0;
    char ops[16];
    int opsIndex = 0;

    for(int i = 0; i < size; i++) {
        char t[24];
        strncpy(t, a[i], sizeof(t));
        t[sizeof(t)-1] = '\0';

        char* stripped = strip(t);
        if(isNum(stripped)) {
            strncpy(out[outIndex++], t, sizeof(out[0]));
        } else {
            while(opsIndex > 0 && get_prec(ops[opsIndex - 1]) >= get_prec(t[0])) {
                char temp[2] = {ops[--opsIndex], '\0'};
                strncpy(out[outIndex++], temp, sizeof(out[0]));
            }
            ops[opsIndex++] = t[0];
        }
        free(stripped);
    }

    while(opsIndex > 0) {
        char temp[2] = {ops[--opsIndex], '\0'};
        strncpy(out[outIndex++], temp, sizeof(out[0]));
    }

    free(tokens);
    *retSize = outIndex;
    return out;
}

float eval_rpn(void* rpn, const int size, int* error) {
    char (*a)[16] = rpn;
    float stack[32];
    int stackIndex = 0;

    for(int i = 0; i < size; i++) {
        char t[24];
        strncpy(t, a[i], sizeof(t));
        t[sizeof(t)-1] = '\0';

        char* stripped = strip(t);
        if(isNum(stripped)) {
            stack[stackIndex++] = strtof(t, NULL);
        } else {
            if(stackIndex < 2) { free(rpn); free(stripped); *error = 1; return 0; }
            float b = stack[--stackIndex];
            float a_val = stack[--stackIndex];

            switch(t[0]) {
                case '+': stack[stackIndex++] = a_val + b; break;
                case '-': stack[stackIndex++] = a_val - b; break;
                case '*': stack[stackIndex++] = a_val * b; break;
                case '/': stack[stackIndex++] = a_val / b; break;
            }
        }
        free(stripped);
    }

    float result = stack[0];
    free(rpn);
    return result;
}

float eval(const char* expr, const int size, int* error) {
    int outSize = 0;
    int outSize2 = 0;
    void* tokens = tokenize(expr, size, &outSize);
    void* rpn = shunting_yard(tokens, outSize, &outSize2);
    return eval_rpn(rpn, outSize2, error);
}
