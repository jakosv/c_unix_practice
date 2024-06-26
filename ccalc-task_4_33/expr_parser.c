#include "expr_parser.h"
#include "syscall.h"
#include "operator.h"
#include "history_buffer.h"
#include "stack.h"
#include "util.h"
#include "variable.h"

enum { max_parser_err_msg_len = 256 };

char parser_err_msg[max_parser_err_msg_len];

static const char unknown_symbol_err_msg[] = "Parse error: unknown symbol '";

enum parser_states { pst_read, pst_num_read, pst_var_read };

typedef struct parser {
    char ch;
    int number;
    enum parser_states state;
    enum parser_status status;
} parser_t;

static void make_unknown_symbol_err_msg(char ch)
{
    int len;
    char symbol_str[2];
    symbol_str[0] = ch;
    symbol_str[1] = '\0';
    len = str_concat(unknown_symbol_err_msg, symbol_str, parser_err_msg, 
                     max_parser_err_msg_len);
    if (len < max_parser_err_msg_len - 1) {
        parser_err_msg[len] = '\'';    
        parser_err_msg[len+1] = '\0';
    }
}

static int is_space_symbol(char ch)
{
    return ch == ' ' || ch == '\t';
}

static void parser_handle_unknown_symbol(parser_t *parser)
{
    char ch = parser->ch;
    if (is_space_symbol(ch)) {
        parser->state = pst_read;
        parser->status = ps_ok;
        return;
    }
    make_unknown_symbol_err_msg(ch);
    parser->status = ps_err;
}

static void parser_handle_num_read_end(parser_t *parser, expression_t *expr)
{
    switch (parser->state) {
    case pst_num_read:
        expression_add_number(parser->number, expr);
        break;
    case pst_var_read:
        expression_add_variable(parser->number, expr);
        break;
    }

    if (is_operator(parser->ch)) {
        expression_add_op(parser->ch, expr);
    } else {
        parser_handle_unknown_symbol(parser);
        return;
    }

    parser->state = pst_read;
}

static void parser_handle_symbol(char ch, parser_t *parser,
                                 expression_t *expr)
{
    parser->ch = ch;
    switch (parser->state) {
    case pst_read:
        if (is_operator(ch)) {
            expression_add_op(ch, expr);
            break;
        } else if (ch == var_char) {
            parser->number = 0;
            parser->state = pst_var_read;
            break;
        } else if (!is_digit(ch)) {
            parser_handle_unknown_symbol(parser);
            break;
        } 
        parser->number = 0;
        parser->state = pst_num_read;
        /* go to pst_num_read state handler */

    case pst_num_read:
    case pst_var_read:
        if (!is_digit(ch)) {
            parser_handle_num_read_end(parser, expr);
            break;
        }
        append_char_to_number(ch, &parser->number);
        break;
        
    default:
        break;
    }
}

enum parser_status parse_expression(char *buf_str, int buflen,
                                    expression_t *expr)
{
    parser_t parser;
    int i;

    parser.status = ps_ok;
    parser.state = pst_read;

    for (i = 0; i < buflen; i++)
        parser_handle_symbol(buf_str[i], &parser, expr);

    if (parser.status == ps_ok)
        if (parser.state == pst_num_read)
            expression_add_number(parser.number, expr);
        else if (parser.state == pst_var_read)
            expression_add_variable(parser.number, expr);

    return parser.status;
}
