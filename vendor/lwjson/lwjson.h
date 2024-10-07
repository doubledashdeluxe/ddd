/**
 * \file            lwjson.h
 * \brief           LwJSON - Lightweight JSON format parser
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwJSON - Lightweight JSON format parser.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.7.0
 */
#ifndef LWJSON_HDR_H
#define LWJSON_HDR_H

#include <stdint.h>
#include <string.h>
#include "lwjson/lwjson_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWJSON Lightweight JSON format parser
 * \brief           LwJSON - Lightweight JSON format parser
 * \{
 */

/**
 * \brief           Get size of statically allocated array
 * \param[in]       x: Object to get array size of
 * \return          Number of elements in array
 */
#define LWJSON_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           List of supported JSON types
 */
typedef enum {
    LWJSON_TYPE_STRING,   /*!< String/Text format. Everything that has beginning and ending quote character */
    LWJSON_TYPE_NUM_INT,  /*!< Number type for integer */
    LWJSON_TYPE_NUM_REAL, /*!< Number type for real number */
    LWJSON_TYPE_OBJECT,   /*!< Object data type */
    LWJSON_TYPE_ARRAY,    /*!< Array data type */
    LWJSON_TYPE_TRUE,     /*!< True boolean value */
    LWJSON_TYPE_FALSE,    /*!< False boolean value */
    LWJSON_TYPE_NULL,     /*!< Null value */
} lwjson_type_t;

/**
 * \brief           Real data type
 */
typedef LWJSON_CFG_REAL_TYPE lwjson_real_t;

/**
 * \brief           Integer data type
 */
typedef LWJSON_CFG_INT_TYPE lwjson_int_t;

/**
 * \brief           JSON token
 */
typedef struct lwjson_token {
    struct lwjson_token* next; /*!< Next token on a list */
    lwjson_type_t type;        /*!< Token type */
    const char* token_name;    /*!< Token name (if exists) */
    size_t token_name_len;     /*!< Length of token name (this is needed to support const input strings to parse) */

    union {
        struct {
            const char* token_value; /*!< Pointer to the beginning of the string */
            size_t
                token_value_len; /*!< Length of token value (this is needed to support const input strings to parse) */
        } str;                   /*!< String data */

        lwjson_real_t num_real;           /*!< Real number format */
        lwjson_int_t num_int;             /*!< Int number format */
        struct lwjson_token* first_child; /*!< First children object for object or array type */
    } u;                                  /*!< Union with different data types */
} lwjson_token_t;

/**
 * \brief           JSON result enumeration
 */
typedef enum {
    lwjsonOK = 0x00, /*!< Function returns successfully */
    lwjsonERR,       /*!< Generic error message */
    lwjsonERRJSON,   /*!< Error JSON format */
    lwjsonERRMEM,    /*!< Memory error */
    lwjsonERRPAR,    /*!< Parameter error */

    lwjsonSTREAMWAITFIRSTCHAR, /*!< Streaming parser did not yet receive first valid character
                                    indicating start of JSON sequence */
    lwjsonSTREAMDONE,          /*!< Streaming parser is done,
                                    closing character matched the stream opening one */
    lwjsonSTREAMINPROG,        /*!< Stream parsing is still in progress */
} lwjsonr_t;

/**
 * \brief           LwJSON instance
 */
typedef struct {
    lwjson_token_t* tokens;     /*!< Pointer to array of tokens */
    size_t tokens_len;          /*!< Size of all tokens */
    size_t next_free_token_pos; /*!< Position of next free token instance */
    lwjson_token_t first_token; /*!< First token on a list */

    struct {
        uint8_t parsed : 1; /*!< Flag indicating JSON parsing has finished successfully */
    } flags;                /*!< List of flags */
} lwjson_t;

lwjsonr_t lwjson_init(lwjson_t* lwobj, lwjson_token_t* tokens, size_t tokens_len);
lwjsonr_t lwjson_parse_ex(lwjson_t* lwobj, const void* json_data, size_t len);
lwjsonr_t lwjson_parse(lwjson_t* lwobj, const char* json_str);
const lwjson_token_t* lwjson_find(lwjson_t* lwobj, const char* path);
const lwjson_token_t* lwjson_find_ex(lwjson_t* lwobj, const lwjson_token_t* token, const char* path);
lwjsonr_t lwjson_free(lwjson_t* lwobj);

void lwjson_print_token(const lwjson_token_t* token);
void lwjson_print_json(const lwjson_t* lwobj);

/**
 * \brief           Object type for streaming parser
 */
typedef enum {
    LWJSON_STREAM_TYPE_NONE,       /*!< No entry - not used */
    LWJSON_STREAM_TYPE_OBJECT,     /*!< Object indication */
    LWJSON_STREAM_TYPE_OBJECT_END, /*!< Object end indication */
    LWJSON_STREAM_TYPE_ARRAY,      /*!< Array indication */
    LWJSON_STREAM_TYPE_ARRAY_END,  /*!< Array end indication */
    LWJSON_STREAM_TYPE_KEY,        /*!< Key string */
    LWJSON_STREAM_TYPE_STRING,     /*!< Strin type */
    LWJSON_STREAM_TYPE_TRUE,       /*!< True primitive */
    LWJSON_STREAM_TYPE_FALSE,      /*!< False primitive */
    LWJSON_STREAM_TYPE_NULL,       /*!< Null primitive */
    LWJSON_STREAM_TYPE_NUMBER,     /*!< Generic number */
} lwjson_stream_type_t;

/**
 * \brief           Stream parsing stack object
 */
typedef struct {
    lwjson_stream_type_t type; /*!< Streaming type - current value */

    union {
        char name[LWJSON_CFG_STREAM_KEY_MAX_LEN
                  + 1]; /*!< Last known key name, used only for \ref LWJSON_STREAM_TYPE_KEY type */
        uint16_t index; /*!< Current index when type is an array */
    } meta;             /*!< Meta information */
} lwjson_stream_stack_t;

typedef enum {
    LWJSON_STREAM_STATE_WAITINGFIRSTCHAR = 0x00, /*!< State to wait for very first opening character */
    LWJSON_STREAM_STATE_PARSING,        /*!< In parsing of the first char state - detecting next character state */
    LWJSON_STREAM_STATE_PARSING_STRING, /*!< Parse string primitive */
    LWJSON_STREAM_STATE_PARSING_PRIMITIVE, /*!< Parse any primitive that is non-string, either "true", "false", "null" or a number */
    LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END, /*!< Expecting ',', '}' or ']' */
    LWJSON_STREAM_STATE_EXPECTING_COLON,        /*!< Expecting ':' */
} lwjson_stream_state_t;

/* Forward declaration */
struct lwjson_stream_parser;

/**
 * \brief           Callback function for various events
 * 
 */
typedef void (*lwjson_stream_parser_callback_fn)(struct lwjson_stream_parser* jsp, lwjson_stream_type_t type);

/**
 * \brief           LwJSON streaming structure
 */
typedef struct lwjson_stream_parser {
    lwjson_stream_stack_t
        stack[LWJSON_CFG_STREAM_STACK_SIZE]; /*!< Stack used for parsing. TODO: Add conditional compilation flag */
    size_t stack_pos;                        /*!< Current stack position */

    lwjson_stream_state_t parse_state; /*!< Parser state */

    lwjson_stream_parser_callback_fn evt_fn; /*!< Event function for user */

    void* user_data; /*!< User data for callback function */

    /* State */
    union {
        struct {
            char buff[LWJSON_CFG_STREAM_STRING_MAX_LEN
                      + 1];        /*!< Buffer to write temporary data. TODO: Size to be variable with define */
            size_t buff_pos;       /*!< Buffer position for next write (length of bytes in buffer) */
            size_t buff_total_pos; /*!< Total buffer position used up to now (in several data chunks) */
            uint8_t is_last;       /*!< Status indicates if this is the last part of the string */
        } str;                     /*!< String structure. It is only used for keys and string objects.
                                        Use primitive part for all other options */

        struct {
            char buff[LWJSON_CFG_STREAM_PRIMITIVE_MAX_LEN + 1]; /*!< Temporary write buffer */
            size_t buff_pos;                                    /*!< Buffer position for next write */
        } prim; /*!< Primitive object. Used for all types, except key or string */

        /* Todo: Add other types */
    } data; /*!< Data union used to parse various */

    char prev_c; /*!< History of characters */
} lwjson_stream_parser_t;

lwjsonr_t lwjson_stream_init(lwjson_stream_parser_t* jsp, lwjson_stream_parser_callback_fn evt_fn);
lwjsonr_t lwjson_stream_set_user_data(lwjson_stream_parser_t* jsp, void* user_data);
void* lwjson_stream_get_user_data(lwjson_stream_parser_t* jsp);
lwjsonr_t lwjson_stream_reset(lwjson_stream_parser_t* jsp);
lwjsonr_t lwjson_stream_parse(lwjson_stream_parser_t* jsp, char c);

/**
 * \brief           Get number of tokens used to parse JSON
 * \param[in]       lwobj: Pointer to LwJSON instance
 * \return          Number of tokens used to parse JSON
 */
#define lwjson_get_tokens_used(lwobj) (((lwobj) != NULL) ? ((lwobj)->next_free_token_pos + 1) : 0)

/**
 * \brief           Get very first token of LwJSON instance
 * \param[in]       lwobj: Pointer to LwJSON instance
 * \return          Pointer to first token
 */
#define lwjson_get_first_token(lwobj) (((lwobj) != NULL) ? (&(lwobj)->first_token) : NULL)

/**
 * \brief           Get token value for \ref LWJSON_TYPE_NUM_INT type
 * \param[in]       token: token with integer type
 * \return          Int number if type is integer, `0` otherwise
 */
#define lwjson_get_val_int(token)                                                                                      \
    ((lwjson_int_t)(((token) != NULL && (token)->type == LWJSON_TYPE_NUM_INT) ? (token)->u.num_int : 0))

/**
 * \brief           Get token value for \ref LWJSON_TYPE_NUM_REAL type
 * \param[in]       token: token with real type
 * \return          Real numbeer if type is real, `0` otherwise
 */
#define lwjson_get_val_real(token)                                                                                     \
    ((lwjson_real_t)(((token) != NULL && (token)->type == LWJSON_TYPE_NUM_REAL) ? (token)->u.num_real : 0))

/**
 * \brief           Get first child token for \ref LWJSON_TYPE_OBJECT or \ref LWJSON_TYPE_ARRAY types
 * \param[in]       token: token with integer type
 * \return          Pointer to first child or `NULL` if parent token is not object or array
 */
#define lwjson_get_first_child(token)                                                                                  \
    (const void*)(((token) != NULL && ((token)->type == LWJSON_TYPE_OBJECT || (token)->type == LWJSON_TYPE_ARRAY))     \
                      ? (token)->u.first_child                                                                         \
                      : NULL)

/**
 * \brief           Get string value from JSON token
 * \param[in]       token: Token with string type
 * \param[out]      str_len: Pointer to variable holding length of string.
 *                      Set to `NULL` if not used
 * \return          Pointer to string or `NULL` if invalid token type
 */
static inline const char*
lwjson_get_val_string(const lwjson_token_t* token, size_t* str_len) {
    if (token != NULL && token->type == LWJSON_TYPE_STRING) {
        if (str_len != NULL) {
            *str_len = token->u.str.token_value_len;
        }
        return token->u.str.token_value;
    }
    return NULL;
}

/**
 * \brief           Get length of string for \ref LWJSON_TYPE_STRING token type
 * \param[in]       token: token with string type
 * \return          Length of string in units of bytes
 */
#define lwjson_get_val_string_length(token)                                                                            \
    ((size_t)(((token) != NULL && (token)->type == LWJSON_TYPE_STRING) ? (token)->u.str.token_value_len : 0))

/**
 * \brief           Compare string token with user input string for a case-sensitive match
 * \param[in]       token: Token with string type
 * \param[in]       str: NULL-terminated string to compare
 * \return          `1` if equal, `0` otherwise
 */
static inline uint8_t
lwjson_string_compare(const lwjson_token_t* token, const char* str) {
    if (token != NULL && token->type == LWJSON_TYPE_STRING) {
        return strncmp(token->u.str.token_value, str, token->u.str.token_value_len) == 0;
    }
    return 0;
}

/**
 * \brief           Compare string token with user input string for a case-sensitive match
 * \param[in]       token: Token with string type
 * \param[in]       str: NULL-terminated string to compare
 * \param[in]       len: Length of the string in bytes
 * \return          `1` if equal, `0` otherwise
 */
static inline uint8_t
lwjson_string_compare_n(const lwjson_token_t* token, const char* str, size_t len) {
    if (token != NULL && token->type == LWJSON_TYPE_STRING && len <= token->u.str.token_value_len) {
        return strncmp(token->u.str.token_value, str, len) == 0;
    }
    return 0;
}

/**
 * \name            LWJSON_STREAM_SEQ
 * \brief           Helper functions for stack analysis in a callback function
 * \note            Useful exclusively for streaming functions
 * \{
 */

/**
 * \brief           Check the sequence of JSON stack, starting from start_number index
 * \note            This applies only to one sequence element. Other macros, starting with
 *                      `lwjson_stack_seq_X` (where X is the sequence length), provide
 *                      more parameters for longer sequences.
 * 
 * \param[in]       jsp: LwJSON stream instance
 * \param[in]       start_num: Start number in the stack. Typically starts with `0`, but user may choose another
 *                      number, if intention is to check partial sequence only
 * \param[in]       sp0: Stream stack type. Value of \ref lwjson_stream_type_t, but only last part of the enum.
 *                      If user is interested in the \ref LWJSON_STREAM_TYPE_OBJECT,
 *                      you should only write `OBJECT` as parameter.
 *                      Idea behind is to make code smaller and easier to read, especially when
 *                      using other sequence values with more parameters.
 * \return          `0` if sequence doesn't match, non-zero otherwise
 */
#define lwjson_stack_seq_1(jsp, start_num, sp0) ((jsp)->stack[(start_num)].type == LWJSON_STREAM_TYPE_##sp0)
#define lwjson_stack_seq_2(jsp, start_num, sp0, sp1)                                                                   \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0) && lwjson_stack_seq_1((jsp), (start_num) + 1, sp1))
#define lwjson_stack_seq_3(jsp, start_num, sp0, sp1, sp2)                                                              \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0) && lwjson_stack_seq_2((jsp), (start_num) + 1, sp1, sp2))
#define lwjson_stack_seq_4(jsp, start_num, sp0, sp1, sp2, sp3)                                                         \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0) && lwjson_stack_seq_3((jsp), (start_num) + 1, sp1, sp2, sp3))
#define lwjson_stack_seq_5(jsp, start_num, sp0, sp1, sp2, sp3, sp4)                                                    \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0) && lwjson_stack_seq_4((jsp), (start_num) + 1, sp1, sp2, sp3, sp4))
#define lwjson_stack_seq_6(jsp, start_num, sp0, sp1, sp2, sp3, sp4, sp5)                                               \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0)                                                                   \
     && lwjson_stack_seq_5((jsp), (start_num) + 1, sp1, sp2, sp3, sp4, sp5))
#define lwjson_stack_seq_7(jsp, start_num, sp0, sp1, sp2, sp3, sp4, sp5, sp6)                                          \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0)                                                                   \
     && lwjson_stack_seq_6((jsp), (start_num) + 1, sp1, sp2, sp3, sp4, sp5, sp6))
#define lwjson_stack_seq_8(jsp, start_num, sp0, sp1, sp2, sp3, sp4, sp5, sp6, sp7)                                     \
    (lwjson_stack_seq_1((jsp), (start_num) + 0, sp0)                                                                   \
     && lwjson_stack_seq_7((jsp), (start_num) + 1, sp1, sp2, sp3, sp4, sp5, sp6, sp7))

/**
 * \}
 */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWJSON_HDR_H */
