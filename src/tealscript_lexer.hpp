#pragma once

#include "inc/commondefs.hpp"
#include "inc/file_util.hpp"
#include "inc/str_util.hpp"

#include "tealscript_util.hpp"
#include "tealscript_token.hpp"
#include "tealscript_util.hpp"

namespace teal {

    class lexer {
        static inline std::int64_t constexpr NEWL{'\n'};
        static inline std::int64_t constexpr CRET{'\r'};
        static inline std::int64_t constexpr HTAB{'\t'};
        static inline std::int64_t constexpr BACK{'\b'};
        static inline std::int64_t constexpr ALRT{'\a'};
        static inline std::int64_t constexpr FMFD{'\f'};
        static inline std::int64_t constexpr VTAB{'\v'};
        static inline std::int64_t constexpr APOS{'\''};
        static inline std::int64_t constexpr DQUO{'\"'};
        static inline std::int64_t constexpr BKSL{'\\'};
        static inline std::int64_t constexpr ESCP{0x1b};

    public:
        void consume_char(std::int64_t c) {
            if(nl_buff_ == CRET) {
                if(c == NEWL) {
                    nl_buff_ = -1;
                    process_input_char(NEWL);
                } else if(c == CRET) {
                    process_input_char(NEWL);
                } else {
                    nl_buff_ = -1;
                    process_input_char(NEWL);
                    process_input_char(c);
                }
            } else {
                if(c == CRET) {
                    nl_buff_ = CRET;
                } else {
                    process_input_char(c);
                }
            }
        }

        void consume_eof() {
            consume_char(-1);
            reset();
        }

        void reset() {
            buff_.clear();
            state_.clear();
            pos_ = 0;
            row_ = 0;
            col_ = 0;
            token_pos_ = 0;
            token_row_ = 0;
            token_col_ = 0;
            float_phase_.clear();
            space_with_new_line_ = false;
            nl_buff_ = -1;
            string_buff_present_ = false;
            string_buff_.clear();
            str_token_row_ = -1;
            str_token_col_ = -1;
            stop_escaping();
        }

        void set_callback(std::function<void(token const &, bool)> &&on_token) {
            on_token_ = std::move(on_token);
        }

        void set_callback(std::function<void(token const &, bool)> const &on_token) {
            on_token_ = on_token;
        }

    private:
        void process_input_char(std::int64_t c, bool encountered = true) {
            check();
            if(state_.empty()) {
                float_phase_.clear();
                space_with_new_line_ = false;
                buff_.clear();
                set_tok_pos_to_current();
                if(teal::str_util::fltr<std::wstring>::isdigit(c)) { buff_ += c; state_ = "int";
                } else if(teal::str_util::fltr<std::wstring>::isspace(c)) { buff_ += c; state_ = "space";
                } else if(c == '.') { buff_ += c; state_ = "dot";
                } else if(is_ident_start(c)) { buff_ += c; state_ = "iden";
                } else if(c == DQUO) {
                    state_ = "str";
                    if(!string_buff_present_) {
                        str_token_row_ = row_;
                        str_token_col_ = col_;
                    }
                } else if(c == APOS) { state_ = "sc_str"; str_token_row_ = row_; str_token_col_ = col_;
                } else if(c == '=') { buff_ += c; state_ = "=";
                } else if(c == '*') { buff_ += c; state_ = "*";
                } else if(c == '/') { buff_ += c; state_ = "/";
                } else if(c == '%') { buff_ += c; state_ = "%";
                } else if(c == '+') { buff_ += c; state_ = "+";
                } else if(c == '-') { buff_ += c; state_ = "-";
                } else if(c == '&') { buff_ += c; state_ = "&";
                } else if(c == '|') { buff_ += c; state_ = "|";
                } else if(c == '^') { buff_ += c; state_ = "^";
                } else if(c == '<') { buff_ += c; state_ = "<";
                } else if(c == '>') { buff_ += c; state_ = ">";
                } else if(c == '!') { buff_ += c; state_ = "!";
                } else if(c == '~') { buff_ += c; state_ = "~"; instant_single_char();
                } else if(c == ';') { buff_ += c; state_ = ";"; instant_single_char();
                } else if(c == '?') { buff_ += c; state_ = "?"; instant_single_char();
                } else if(c == ':') { buff_ += c; state_ = ":"; instant_single_char();
                } else if(c == ',') { buff_ += c; state_ = ","; instant_single_char();
                } else if(c == '[') { buff_ += c; state_ = "["; instant_single_char();
                } else if(c == ']') { buff_ += c; state_ = "]"; instant_single_char();
                } else if(c == '(') { buff_ += c; state_ = "("; instant_single_char();
                } else if(c == ')') { buff_ += c; state_ = ")"; instant_single_char();
                } else if(c == '{') { buff_ += c; state_ = "{"; instant_single_char();
                } else if(c == '}') { buff_ += c; state_ = "}"; instant_single_char();
                } else {
                    if(c >= 0) {
                        throw compilation_error{row_, col_, "invalid character"};
                    } else {
                        reset();
                    }
                }
            } else if(state_ == "int") { num(c);
            } else if(state_ == "hex") { hex(c);
            } else if(state_ == "oct") { oct(c);
            } else if(state_ == "bin") { bin(c);
            } else if(state_ == "float") { flt(c);
            } else if(state_ == "dot") { dot(c);
            } else if(state_ == "iden") { ident(c);
            } else if(state_ == "str") { dq_str(c);
            } else if(state_ == "sc_str") { sc_str(c);
            } else if(state_ == "space") { space(c);
            } else if(state_ == "=") { eq(c);
            } else if(state_ == "*") { star(c);
            } else if(state_ == "/") { slash(c);
            } else if(state_ == "%") { percent(c);
            } else if(state_ == "+") { plus(c);
            } else if(state_ == "-") { minus(c);
            } else if(state_ == "&") { amp(c);
            } else if(state_ == "&&") { ampamp(c);
            } else if(state_ == "!") { excl(c);
            } else if(state_ == "|") { bar(c);
            } else if(state_ == "||") { barbar(c);
            } else if(state_ == "^") { circ(c);
            } else if(state_ == "<") { lt(c);
            } else if(state_ == "<=") { leq(c);
            } else if(state_ == "<<") { lshift(c);
            } else if(state_ == ">") { gt(c);
            } else if(state_ == ">>") { rshift(c);
            } else if(state_ == "slc") { slc(c);
            } else if(state_ == "mlc") { mlc(c);
            } else if(state_ == "mlc_star") { mlc_star(c);
            } else {
                throw compilation_error{row_, col_, "invalid lexer state"};
            }
            if(encountered) {
                count(c);
            }
        }

        void instant_single_char() {
            static str_map_t<token::type> const singles{
                {"(", token::type::LPAREN},
                {")", token::type::RPAREN},
                {"[", token::type::LBRACKET},
                {"]", token::type::RBRACKET},
                {"{", token::type::LCURLY},
                {"}", token::type::RCURLY},
                {",", token::type::COMMA},
                {"~", token::type::BITNOT},
                {"?", token::type::QUESTION},
                {":", token::type::COLON},
                {";", token::type::SEMICOLON},
            };
            report_token(singles.at(state_));
        }

        void count(std::int64_t c) {
            ++pos_;
            ++col_;
            if(c == NEWL) {
                ++row_;
                col_ = 0;
                space_with_new_line_ = true;
            }
        }

        void check() {
            if(str_escape_ && state_ != "str" && state_ != "sc_str") {
                throw compilation_error{row_, col_, "invalid string escape"};
            }
        }

        bool is_ident_start(std::int64_t c) {
            return teal::str_util::fltr<std::wstring>::isalpha(c) || c == '_' || c == '$';
        }

        bool is_ident_tail(std::int64_t c) {
            return is_ident_start(c) || teal::str_util::fltr<std::wstring>::isdigit(c);
        }

        void num(std::int64_t c) {
            if(teal::str_util::fltr<std::wstring>::isdigit(c)) {
                buff_ += c;
            } else if(c == '\'') {
                if(!buff_.empty() && buff_[buff_.size() - 1] != '\'') {
                    buff_ += c;
                } else {
                    throw compilation_error{row_, col_, "wrong placed separator in numeric literal "};
                }
            } else if(c == '.') {
                if(buff_.find('\'') != std::wstring::npos) {
                    str_util::atoui(buff_, 10, true);
                    buff_ = str_util::remove_char(buff_, '\'');
                }
                float_phase_ = ".";
                buff_ += c;
                state_ = "float";
            } else if(c == 'e' || c == 'E') {
                if(buff_.find('\'') != std::wstring::npos && buff_.find('.') == std::wstring::npos) {
                    str_util::atoui(buff_, 10, true);
                    buff_ = str_util::remove_char(buff_, '\'');
                }
                float_phase_ = "e";
                buff_ += c;
                state_ = "float";
            } else if(c == 'x' || c == 'X') {
                if(buff_.size() == 1 && buff_[0] == '0') {
                    state_ = "hex";
                    buff_.clear();
                } else {
                    throw compilation_error{row_, col_, "invalid hexadecimal integer literal"};
                }
            } else if(c == 'o' || c == 'O') {
                if(buff_.size() == 1 && buff_[0] == '0') {
                    state_ = "oct";
                    buff_.clear();
                } else {
                    throw compilation_error{row_, col_, "invalid octal integer literal"};
                }
            } else if(c == 'b' || c == 'B') {
                if(buff_.size() == 1 && buff_[0] == '0') {
                    state_ = "bin";
                    buff_.clear();
                } else {
                    throw compilation_error{row_, col_, "invalid binary integer literal"};
                }
            } else {
                report_token(token::type::INT_LITERAL, c);
            }
        }

        void hex(std::int64_t c) {
            if(teal::str_util::ishex(c)) {
                buff_ += c;
            } else if(c == '\'') {
                buff_ += c;
            } else {
                report_token(token::type::HEX_LITERAL, c);
            }
        }

        void oct(std::int64_t c) {
            if(c >= '0' && c < '8') {
                buff_ += c;
            } else if(c == '\'') {
                buff_ += c;
            } else {
                report_token(token::type::OCT_LITERAL, c);
            }
        }

        void bin(std::int64_t c) {
            if(c == '0' || c == '1') {
                buff_ += c;
            } else if(c == '\'') {
                buff_ += c;
            } else {
                report_token(token::type::BIN_LITERAL, c);
            }
        }

        void dot(std::int64_t c) {
            if(teal::str_util::fltr<std::wstring>::isdigit(c)) {
                state_ = "float";
                float_phase_ = ".";
                buff_ += c;
            } else {
                report_token(token::type::DOT, c);
            }
        }

        void flt(std::int64_t c) {
            if(teal::str_util::fltr<std::wstring>::isdigit(c)) {
                buff_ += c;
                if(float_phase_ == "e" || float_phase_ == "es") {
                    float_phase_ = "ed";
                }
            } else if(teal::str_util::fltr<std::wstring>::tolower(c) == 'e') {
                if(float_phase_ != ".") {
                    throw compilation_error{row_, col_, "floating point number syntax error"};
                } else {
                    float_phase_ = "e";
                    buff_ += c;
                }
            } else if(c == '-' || c == '+') {
                if(float_phase_ == "e") {
                    float_phase_ = "es";
                    buff_ += c;
                } else if(float_phase_ == "." || float_phase_ == "ed") {
                    report_token(float_phase_ == "." ? token::type::FP_LITERAL : token::type::FP_EXP_LITERAL, c);
                } else {
                    throw compilation_error{row_, col_, "floating point number syntax error"};
                }
            } else if(c == -1) {
                if(float_phase_ == "." || float_phase_ == "ed") {
                    report_token(float_phase_ == "." ? token::type::FP_LITERAL : token::type::FP_EXP_LITERAL);
                } else {
                    throw compilation_error{row_, col_, "floating point number syntax error"};
                }
            } else {
                if(float_phase_ == "." || float_phase_ == "ed") {
                    report_token(float_phase_ == "." ? token::type::FP_LITERAL : token::type::FP_EXP_LITERAL, c);
                } else {
                    throw compilation_error{row_, col_, "floating point number syntax error"};
                }
            }
        }

        void ident(std::int64_t c) {
            if(is_ident_tail(c)) {
                buff_ += c;
            } else {
                report_token(token::type::IDENTIFIER, c);
            }
        }

        void dq_str(std::int64_t c) {
            if(str_escape_) {
                process_str_esc_input(c, row_, col_);
            } else if(c == NEWL || c == CRET) {
                throw compilation_error{row_, col_, std::string{"unexpected new line in string"}};
            } else if(c == BKSL) {
                str_escape_ = true;
                esc_row_ = row_;
                esc_col_ = col_;
            } else if(c == DQUO) {
                if(str_escape_) {
                    throw compilation_error{esc_row_, esc_col_, "invalid string escape"};
                }
                report_token(token::type::STRING_LITERAL);
            } else if(c == -1) {
                throw compilation_error{str_token_row_, str_token_col_, "unexpected end of input"};
            } else {
                string_buff_ += c;
            }
        }

        void sc_str(std::int64_t c) {
            if(str_escape_) {
                process_str_esc_input(c, row_, col_);
            } else if(c == NEWL || c == CRET) {
                throw compilation_error{row_, col_, std::string{"unexpected new line in character literal"}};
            } else if(c == BKSL) {
                str_escape_ = true;
                esc_row_ = row_;
                esc_col_ = col_;
            } else if(c == APOS) {
                if(str_escape_) {
                    throw compilation_error{esc_row_, esc_col_, "invalid character literal escape"};
                }
                if(string_buff_.size() < 1) {
                    throw compilation_error{str_token_row_, str_token_col_, "invalid character"};
                }
                report_token(token::type::CHAR_LITERAL);
            } else if(c == -1) {
                throw compilation_error{str_token_row_, str_token_col_, "unexpected end of input"};
            } else {
                string_buff_ += c;
            }
        }

        void process_str_esc_input(std::int64_t c, std::int64_t row, std::int64_t col) {
            if(is_str_num_escaping()) {
                esc_buff_ += c;
                bool num_parse_error{false};
                std::string err_msg{};
                try {
                    switch(str_num_escape_mode_) {
                        case str_num_escaping_t::oct: {
                                if(c < '0' || c > '7') {
                                    throw compilation_error{
                                        row,
                                        col,
                                        "invalid octal number for single-byte sequence"
                                    };
                                }
                                std::int64_t chr{
                                    (std::int64_t)teal::str_util::atoi(
                                        teal::str_util::to_utf8(esc_buff_),
                                        8
                                    )
                                };
                                if(esc_buff_.size() == 3) {
                                    string_buff_ += (decltype(string_buff_)::value_type)chr;
                                    stop_escaping();
                                }
                            }
                            break;
                        case str_num_escaping_t::hex: {
                                if(!teal::str_util::ishex(c)) {
                                    throw compilation_error{
                                        row,
                                        col,
                                        "invalid hex number for single-byte sequence"
                                    };
                                }
                                std::int64_t chr{
                                    (std::int64_t)teal::str_util::atoi(
                                        teal::str_util::to_utf8(esc_buff_),
                                        16
                                    )
                                };
                                if(esc_buff_.size() == 2) {
                                    string_buff_ += (decltype(string_buff_)::value_type)chr;
                                    stop_escaping();
                                }
                            }
                            break;
                        case str_num_escaping_t::u4: {
                                if(!teal::str_util::ishex(c)) {
                                    throw compilation_error{
                                        row,
                                        col,
                                        "invalid hex number for 2-byte sequence"
                                    };
                                }
                                std::int64_t chr{
                                    (std::int64_t)teal::str_util::atoi(
                                        teal::str_util::to_utf8(esc_buff_),
                                        16
                                    )
                                };
                                if(esc_buff_.size() == 4) {
                                    string_buff_ += (decltype(string_buff_)::value_type)chr;
                                    stop_escaping();
                                }
                            }
                            break;
                        case str_num_escaping_t::u8: {
                                if(!teal::str_util::ishex(c)) {
                                    throw compilation_error{row, col, "invalid hex number for 4-byte sequence"};
                                }
                                std::int64_t chr{
                                    (std::int64_t)teal::str_util::atoi(
                                        teal::str_util::to_utf8(esc_buff_),
                                        16
                                    )
                                };
                                if(esc_buff_.size() == 8) {
                                    string_buff_ += (decltype(string_buff_)::value_type)chr;
                                    stop_escaping();
                                }
                            }
                            break;
                        default:
                            break;
                    }
                } catch(std::exception const &e) {
                    err_msg = e.what();
                    num_parse_error = true;
                } catch(...) {
                    num_parse_error = true;
                }
                if(num_parse_error) {
                    if(err_msg.empty()) {
                        throw compilation_error{esc_row_, esc_col_,
                            std::string{"wrong numeric escape sequence"}
                        };
                    } else {
                        throw compilation_error{esc_row_, esc_col_,
                            std::string{"wrong numeric escape sequence: "} + err_msg
                        };
                    }
                }
            } else {
                if(c == 'n') { string_buff_ += NEWL; stop_escaping();
                } else if(c == 'r') { string_buff_ += CRET; stop_escaping();
                } else if(c == 't') { string_buff_ += HTAB; stop_escaping();
                } else if(c == 'b') { string_buff_ += BACK; stop_escaping();
                } else if(c == 'a') { string_buff_ += ALRT; stop_escaping();
                } else if(c == 'f') { string_buff_ += FMFD; stop_escaping();
                } else if(c == 'v') { string_buff_ += VTAB; stop_escaping();
                } else if(c == 'e') { string_buff_ += ESCP; stop_escaping();
                } else if(c >= '0' && c < '8') { esc_buff_ += c; str_num_escape_mode_ = str_num_escaping_t::oct;
                } else if(c == 'x') { str_num_escape_mode_ = str_num_escaping_t::hex;
                } else if(c == 'u') { str_num_escape_mode_ = str_num_escaping_t::u4;
                } else if(c == 'U') { str_num_escape_mode_ = str_num_escaping_t::u8;
                } else { string_buff_ += c; stop_escaping();
                }
            }
        }

        void space(std::int64_t c) {
            if(!teal::str_util::fltr<std::wstring>::isspace(c)) {
                buff_ += ' ';
                report_token(token::type::SPACE, c);
            }
        }

        void slc(std::int64_t c) {
            if(c == NEWL) {
                state_.clear();
            }
        }

        void mlc(std::int64_t c) {
            if(c == '*' ) {
                state_ = "mlc_star";
            } else if(c == -1) {
                throw compilation_error{row_, col_, "unexpected end of input on comment"};
            }
        }

        void mlc_star(std::int64_t c) {
            if(c == '/' ) {
                state_.clear();
                buff_.clear();
            } else if(c == -1) {
                throw compilation_error{row_, col_, "unexpected end of input on comment"};
            } else if(c == '*' ) {
                state_ = "mlc_star";
            } else {
                state_ = "mlc";
            }
        }

        void star(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::MULASSIGN);
            } else {
                report_token(token::type::STAR, c);
            }
        }

        void eq(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::EQUAL);
            } else {
                report_token(token::type::ASSIGN, c);
            }
        }

        void slash(std::int64_t c) {
            if(c == '/') {
                state_ = "slc";
            } else if(c == '*') {
                state_ = "mlc";
            } else {
                if(c == '=') {
                    buff_ += c;
                    report_token(token::type::DIVASSIGN);
                } else {
                    state_ = "div";
                    report_token(token::type::SLASH, c);
                }
            }
        }

        void percent(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::MODASSIGN);
            } else {
                report_token(token::type::MOD, c);
            }
        }

        void plus(std::int64_t c) {
            if(c == '+') {
                buff_ += c;
                report_token(token::type::INCREMENT);
            } else if(c == '=') {
                buff_ += c;
                report_token(token::type::ADDASSIGN);
            } else {
                report_token(token::type::PLUS, c);
            }
        }

        void minus(std::int64_t c) {
            if(c == '-') {
                buff_ += c;
                report_token(token::type::DECREMENT);
            } else if(c == '=') {
                buff_ += c;
                report_token(token::type::SUBASSIGN);
            } else {
                report_token(token::type::MINUS, c);
            }
        }

        void circ(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::XORASSIGN);
            } else {
                report_token(token::type::XOR, c);
            }
        }

        void lt(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                state_ = "<=";
            } else if(c == '<') {
                buff_ += c;
                state_ = "<<";
            } else {
                report_token(token::type::LESS, c);
            }
        }

        void leq(std::int64_t c) {
            if(c == '>') {
                buff_ += c;
                report_token(token::type::SPACESHIP);
            } else {
                report_token(token::type::LESSEQUAL, c);
            }
        }

        void lshift(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::LSHIFTASSIGN);
            } else {
                report_token(token::type::LSHIFT, c);
            }
        }

        void gt(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::GREATEREQUAL);
            } else if(c == '>') {
                buff_ += c;
                state_ = ">>";
            } else {
                report_token(token::type::GREATER, c);
            }
        }

        void rshift(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::RSHIFTASSIGN);
            } else {
                report_token(token::type::RSHIFT, c);
            }
        }

        void amp(std::int64_t c) {
            if(c == '&') {
                buff_ += c;
                state_ = "&&";
            } else if(c == '=') {
                buff_ += c;
                report_token(token::type::BITANDASSIGN);
            } else {
                report_token(token::type::BITAND, c);
            }
        }

        void ampamp(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::ANDASSIGN);
            } else {
                report_token(token::type::AND, c);
            }
        }

        void bar(std::int64_t c) {
            if(c == '|') {
                buff_ += c;
                state_ = "||";
            } else if(c == '=') {
                buff_ += c;
                report_token(token::type::BITORASSIGN);
            } else {
                report_token(token::type::BITOR, c);
            }
        }

        void barbar(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::ORASSIGN);
            } else {
                report_token(token::type::OR, c);
            }
        }

        void excl(std::int64_t c) {
            if(c == '=') {
                buff_ += c;
                report_token(token::type::NOTEQUAL);
            } else {
                report_token(token::type::NOT, c);
            }
        }

    private:
        void report_token(token::type tt) {
            if(on_token_) {
                if(tt == token::type::STRING_LITERAL) {
                    string_buff_present_ = true;
                } else if(tt == token::type::CHAR_LITERAL) {
                    on_token_(
                        token{
                            str_token_row_,
                            str_token_col_,
                            token::type::CHAR_LITERAL,
                            string_buff_
                        },
                        space_with_new_line_
                    );
                    string_buff_.clear();
                    str_token_row_ = -1;
                    str_token_col_ = -1;
                } else {
                    if(string_buff_present_) {
                        if(state_ != "space") {
                            on_token_(
                                token{
                                    str_token_row_,
                                    str_token_col_,
                                    token::type::STRING_LITERAL,
                                    string_buff_
                                },
                                space_with_new_line_
                            );
                            string_buff_.clear();
                            string_buff_present_ = false;
                            str_token_row_ = -1;
                            str_token_col_ = -1;
                        } else {
                            buff_.clear();
                            state_.clear();
                            return;
                        }
                    }
                    on_token_(
                        token{token_row_, token_col_, tt, buff_},
                        space_with_new_line_
                    );
                }
            } else {
                string_buff_.clear();
            }
            buff_.clear();
            state_.clear();
        }

        void report_token(token::type tt, std::int64_t c) {
            report_token(tt);
            process_input_char(c, false);
        }

        void set_tok_pos_to_current() {
            token_pos_ = pos_;
            token_row_ = row_;
            token_col_ = col_;
        }

    private:
        std::function<void(token const &, bool)> on_token_{nullptr};
        bool string_buff_present_{false};
        std::wstring string_buff_{};
        std::int64_t str_token_row_{0};
        std::int64_t str_token_col_{0};

        std::wstring buff_{};
        std::string state_{};
        std::int64_t pos_{0};
        std::int64_t row_{0};
        std::int64_t col_{0};

        std::int64_t token_pos_{0};
        std::int64_t token_row_{0};
        std::int64_t token_col_{0};

        std::string float_phase_{};
        bool space_with_new_line_{false};

        std::int64_t nl_buff_{-1};
        bool str_escape_{false};

        enum class str_num_escaping_t {none, oct, hex, u4, u8};
        str_num_escaping_t str_num_escape_mode_{str_num_escaping_t::none};
        std::wstring esc_buff_{};
        std::int64_t esc_row_{-1};
        std::int64_t esc_col_{-1};
        void stop_escaping() {
            str_num_escape_mode_ = str_num_escaping_t::none;
            str_escape_ = false;
            esc_buff_.clear();
            esc_row_ = -1;
            esc_col_ = -1;
        }
        bool is_str_num_escaping() const {
            return (std::int64_t)str_num_escape_mode_ > (std::int64_t)str_num_escaping_t::none &&
                   (std::int64_t)str_num_escape_mode_ <= (std::int64_t)str_num_escaping_t::u8;
        }
    };

}
