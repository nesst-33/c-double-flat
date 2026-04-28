#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <variant>

struct Position
{
    int line{};
    int column{};
    int offset{}; // NOTE: this is a logical character offset; NOT A BYTE OFFSET!
};

enum class TokenType
{
    // Identifiers and r-values
    IDENTIFIER_T,
    INT_VALUE_T,
    STR_VALUE_T,
    FLP_VALUE_T,

    PLUS_T,             // + (both unary and binary)
    MINUS_T,            // - (same here)
    MULT_T,             // *
    DIV_T,              // /
    MOD_T,              // %

    CONCAT_T,           // ~
    CONJUN_T,           // &
    SPLIT_T,            // :
    CARDINALITY_T,      // !
    APPEND_T,           // <<
    EXTRACT_T,          // >>

    GREATER_T,          // >
    LESSER_T,           // <
    EQ_T,               // ==
    NOT_EQ_T,           // !=
    GREATER_EQ_T,       // >=
    LESSER_EQ_T,        // <=

    ASSIGN_T,           // =
    ADD_ASSIGN_T,       // +=
    SUB_ASSIGN_T,       // -=
    MULT_ASSIGN_T,      // *=
    DIV_ASSIGN_T,       // /=
    MOD_ASSIGN_T,       // %=
    CONCAT_ASSIGN_T,    // ~=

    // Keywords (most are self-explanatory)
    IF_T,
    ELSE_T,
    WHILE_T,
    CONST_T,
    AS_T,               // Used for type casting (ex. 3 as str)
    AND_T,
    OR_T,
    NOT_T,
    VOID_T,
    INT_T,
    FLP_T,
    STR_T,
    ARR_T,
    RETURN_T,

    COMMA_T,            // ,
    L_BRACKET_T,        // (
    R_BRACKET_T,        // )
    L_BRACE_T,          // {
    R_BRACE_T,          // }
    L_SQUARE_T,         // [
    R_SQUARE_T,         // ]

    NEWLINE_T,          // \n - End of a statement
    COMMENT_T,
    EOT,                // No more chars in stream 
    UNKNOWN             // Unknown token
};

struct Token
{
    TokenType type;
    std::variant<std::string, int, double> value; 
    Position position; 
};

#endif
