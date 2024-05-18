#include "operator.h"

int bin_pow(int a, int p)
{
    if (p == 1)
        return a;
    if (p % 2 == 0) {
        int res = bin_pow(a, p / 2);
        return res * res;
    }
    return a * bin_pow(a, p - 1);
}

int perform_binary_operation(int a, int b, char op)
{
    switch(op) {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        return a / b;
    case '%':
        return a % b;
    case '^':
        return bin_pow(a, b);
    }
    return 0;
}

int is_unary_operation(char op)
{
    switch(op) {
    case '-':
    case '+':
        return 1;
    }
    return 0;
}

int perform_unary_operation(int a, char op)
{
    switch(op) {
    case '-':
        return -a;
    case '+':
        return a;
    }
    return 0;
}

int operator_priority(char op)
{
    switch (op) {
    case ')':
        return 1;

    case '+':
    case '-':
        return 2;

    case '*':
    case '/':
    case '%':
        return 3;

    case '^':
        return 4;

    case '(':
        return 5;
    }

    return 0;
}

void perform_operation(char op, int_stack_t *st)
{
    int a, b, res;
    if (st->size < 2) {
        /*
        printf("int_stack_perform_op('%c'): too few operands on stack.", op);
        */
        return;
    }
    b = int_stack_top(st);
    int_stack_pop(st);
    a = int_stack_top(st);
    int_stack_pop(st);
    res = perform_binary_operation(a, b, op);
    /*
    printf("%d %c %d = %d\n", a, op, b, res);
    */
    int_stack_push(res, st);
}

void handle_stack_operators(char_stack_t *op_st, int_stack_t *num_st)
{
    while (!char_stack_empty(op_st)) {
        char op;
        op = char_stack_top(op_st);
        char_stack_pop(op_st);
        if (op == '(') {
            break;
        } else {
            perform_operation(op, num_st);
        }
    }
}

void sift_operator(char op, char_stack_t *op_st, int_stack_t *num_st)
{
    while (!char_stack_empty(op_st)) {
        char top_op;
        top_op = char_stack_top(op_st);
        if (top_op == '(') {
            if (op == ')')
                char_stack_pop(op_st);
            else
                char_stack_push(op, op_st);
            return;
        } else if (operator_priority(op) <= operator_priority(top_op)) {
            char_stack_pop(op_st);
            perform_operation(top_op, num_st);
        } else {
            char_stack_push(op, op_st);
            return;
        }
    }
}