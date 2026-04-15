#pragma once

#include "inc/commondefs.hpp"
#include "inc/file_util.hpp"
#include "inc/str_util.hpp"

#include "tealscript_util.hpp"

namespace teal {

    class token {
    public:
        enum class type {
            NONE,
            INT_LITERAL,
            HEX_LITERAL,
            OCT_LITERAL,
            BIN_LITERAL,
            FP_LITERAL,
            FP_EXP_LITERAL,
            STRING_LITERAL,
            CHAR_LITERAL,
            SPACE,
            IDENTIFIER,
            PLUS,                        // +
            MINUS,                       // -
            STAR,                        // *
            SLASH,                       // /
            MOD,                         // %
            AT,                          // @
            EQUAL,                       // ==
            NOT,                         // !
            NOTEQUAL,                    // !=
            LESSEQUAL,                   // <=
            LESS,                        // <
            GREATER,                     // >
            GREATEREQUAL,                // >=
            SPACESHIP,                   // <=>
            ASSIGN,                      // =
            ADDASSIGN,                   // +=
            SUBASSIGN,                   // -=
            MULASSIGN,                   // *=
            DIVASSIGN,                   // /=
            MODASSIGN,                   // %=
            BITANDASSIGN,                // &=
            ANDASSIGN,                   // &&=
            XORASSIGN,                   // ^=
            BITORASSIGN,                 // |=
            ORASSIGN,                    // ||=
            COLONCOLONASSIGN,            // ::=
            LSHIFTASSIGN,                // <<=
            RSHIFTASSIGN,                // >>=
            INCREMENT,                   // ++
            DECREMENT,                   // --
            LSHIFT,                      // <<
            RSHIFT,                      // >>
            DOTDOT,                      // ..
            STARSTAR,                    // **
            QUESTIONCOLON,               // ?:
            QUESTIONQUESTION,            // ??
            BITNOT,                      // ~
            XOR,                         // ^
            CIRCUMFLEXCIRCUMFLEX,        // ^^
            BITOR,                       // |
            OR,                          // ||
            BITAND,                      // &
            AND,                         // &&
            QUESTION,                    // ?
            COLON,                       // :
            COLONCOLON,                  // ::
            SEMICOLON,                   // ;
            LPAREN,                      // (
            RPAREN,                      // )
            LBRACKET,                    // [
            RBRACKET,                    // ]
            LCURLY,                      // {
            RCURLY,                      // }
            COMMA,                       // ,
            DOT,                         // .
            FUNCCALL,
            TYPECAST,
            ENDOFFILE,
        };

        static inline num_map_t<type, std::string> const type_names {
            {type::NONE, "NONE"},
            {type::INT_LITERAL, "INT_LITERAL"},
            {type::HEX_LITERAL, "HEX_LITERAL"},
            {type::OCT_LITERAL, "OCT_LITERAL"},
            {type::BIN_LITERAL, "BIN_LITERAL"},
            {type::FP_LITERAL, "FP_LITERAL"},
            {type::FP_EXP_LITERAL, "FP_EXP_LITERAL"},
            {type::STRING_LITERAL, "STRING_LITERAL"},
            {type::CHAR_LITERAL, "CHAR_LITERAL"},
            {type::SPACE, "SPACE"},
            {type::IDENTIFIER, "IDENTIFIER"},
            {type::PLUS, "PLUS"},
            {type::MINUS, "MINUS"},
            {type::STAR, "STAR"},
            {type::SLASH, "SLASH"},
            {type::MOD, "MOD"},
            {type::AT, "AT"},
            {type::EQUAL, "EQUAL"},
            {type::NOT, "NOT"},
            {type::NOTEQUAL, "NOTEQUAL"},
            {type::LESSEQUAL, "LESSEQUAL"},
            {type::LESS, "LESS"},
            {type::GREATER, "GREATER"},
            {type::GREATEREQUAL, "GREATEREQUAL"},
            {type::SPACESHIP, "SPACESHIP"},
            {type::ASSIGN, "ASSIGN"},
            {type::ADDASSIGN, "ADDASSIGN"},
            {type::SUBASSIGN, "SUBASSIGN"},
            {type::MULASSIGN, "MULASSIGN"},
            {type::DIVASSIGN, "DIVASSIGN"},
            {type::MODASSIGN, "MODASSIGN"},
            {type::BITANDASSIGN, "BITANDASSIGN"},
            {type::ANDASSIGN, "ANDASSIGN"},
            {type::XORASSIGN, "XORASSIGN"},
            {type::BITORASSIGN, "BITORASSIGN"},
            {type::ORASSIGN, "ORASSIGN"},
            {type::COLONCOLONASSIGN, "COLONCOLONASSIGN"},
            {type::LSHIFTASSIGN, "LSHIFTASSIGN"},
            {type::RSHIFTASSIGN, "RSHIFTASSIGN"},
            {type::INCREMENT, "INCREMENT"},
            {type::DECREMENT, "DECREMENT"},
            {type::LSHIFT, "LSHIFT"},
            {type::RSHIFT, "RSHIFT"},
            {type::DOTDOT, "DOTDOT"},
            {type::STARSTAR, "STARSTAR"},
            {type::QUESTIONCOLON, "QUESTIONCOLON"},
            {type::QUESTIONQUESTION, "QUESTIONQUESTION"},
            {type::BITNOT, "BITNOT"},
            {type::XOR, "XOR"},
            {type::CIRCUMFLEXCIRCUMFLEX, "CIRCUMFLEXCIRCUMFLEX"},
            {type::BITOR, "BITOR"},
            {type::OR, "OR"},
            {type::BITAND, "BITAND"},
            {type::AND, "AND"},
            {type::QUESTION, "QUESTION"},
            {type::COLON, "COLON"},
            {type::COLONCOLON, "COLONCOLON"},
            {type::SEMICOLON, "SEMICOLON"},
            {type::LPAREN, "LPAREN"},
            {type::RPAREN, "RPAREN"},
            {type::LBRACKET, "LBRACKET"},
            {type::RBRACKET, "RBRACKET"},
            {type::LCURLY, "LCURLY"},
            {type::RCURLY, "RCURLY"},
            {type::COMMA, "COMMA"},
            {type::DOT, "DOT"},
            {type::FUNCCALL, "FUNCCALL"},
            {type::ENDOFFILE, "ENDOFFILE"},
        };

        token() = default;
        token(std::int64_t row, std::int64_t col, type tkn_type, std::wstring text):
            type_{tkn_type},
            lexem_{text},
            row_{row},
            col_{col}
        {
        }

        type tktype() const { return type_; }
        std::string tktype_str() const { return type_names.at(type_); }
        bool type_is(type t) const { return type_ == t; }
        bool type_is_not(type t) const { return type_ != t; }
        bool is_id() const { return type_ == type::IDENTIFIER; }
        bool is_eof() const { return type_ == type::ENDOFFILE; }
        std::wstring lexem() const { return lexem_; }
        std::int64_t line() const { return row_; }
        std::int64_t col() const { return col_; }

    private:
        type type_{type::ENDOFFILE};
        std::wstring lexem_{};
        std::int64_t row_{0};
        std::int64_t col_{0};
    };

}
