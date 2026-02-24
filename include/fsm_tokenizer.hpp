#pragma once

#include "commondefs.hpp"
#ifdef DEBUG_FSM_TOKENIZER
#include <map>
#else
#include <unordered_map>
#endif
#include <string>
#include <functional>
#include "str_util.hpp"

namespace scfx {

    namespace detail {

        template<typename STR_T>
        class unescaper {
        public:
            unescaper(const STR_T &str = {}) {
                unescape(str);
            }

            unescaper &unescape(const STR_T &str) {
                res.clear();
                esc = escape::no;
                for(auto c: str) {
                    unescape_char(c);
                }
                finalize();
                return *this;
            }

            operator STR_T const &() const {
                return res;
            }

            STR_T const &val() const {
                return res;
            }

        private:
            STR_T res{};
            enum class escape { no, start, oct, hex, uni4, uni8 };
            escape esc{escape::no};
            std::string cur_char_num{};

            void finalize() {
                switch(esc) {
                    case escape::no:
                        break;
                    case escape::start:
                        cur_char_num.clear();
                        throw std::runtime_error("escape sequence not finished");
                        break;
                    case escape::oct:
                        if(cur_char_num.size()) {
                            res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 8);
                            cur_char_num.clear();
                            esc = escape::no;
                        } else {
                            throw std::runtime_error("octal sequence started");
                        }
                        break;
                    case escape::hex:
                        if(cur_char_num.size()) {
                            res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                            cur_char_num.clear();
                            esc = escape::no;
                        } else {
                            throw std::runtime_error("hexadecimal sequence started");
                        }
                        break;
                    case escape::uni4:
                        if(cur_char_num.size()) {
                            res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                            cur_char_num.clear();
                            esc = escape::no;
                        } else {
                            throw std::runtime_error("unicode character sequence started");
                        }
                        break;
                    case escape::uni8:
                        if(cur_char_num.size() == 0) {
                            throw std::runtime_error("unicode character sequence is started");
                        } else if(cur_char_num.size() != 8) {
                            throw std::runtime_error("unicode character sequence is not completed");
                        }
                        res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                        cur_char_num.clear();
                        esc = escape::no;
                        break;
                    default:
                        cur_char_num.clear();
                        break;
                }
            }

            void unescape_char(typename STR_T::value_type cc) {
                if(cc == 0) {
                    finalize();
                    return;
                }
                switch(esc) {
                    case escape::no:
                        if(cc == '\\') {
                            esc = escape::start;
                            cur_char_num.clear();
                        } else if(cc != 0) {
                            res += cc;
                        }
                        break;
                    case escape::start:
                        if(cc == 'r') {
                            cc = '\r'; res += cc; esc = escape::no;
                        } else if(cc == 'n') {
                            cc = '\n'; res += cc; esc = escape::no;
                        } else if(cc == 't') {
                            cc = '\t'; res += cc; esc = escape::no;
                        } else if(cc == 'b') {
                            cc = '\b'; res += cc; esc = escape::no;
                        } else if(cc == 'a') {
                            cc = '\a'; res += cc; esc = escape::no;
                        } else if(cc == 'v') {
                            cc = '\v'; res += cc; esc = escape::no;
                        } else if(cc == 'f') {
                            cc = '\f'; res += cc; esc = escape::no;
                        } else if(cc == 'e') {
                            cc = 0x1b; res += cc; esc = escape::no;
                        } else if(cc >= '0' && cc <= '7') {
                            esc = escape::oct; cur_char_num += cc;
                        } else if(cc == 'o') {
                            esc = escape::oct;
                        } else if(cc == 'x') {
                            esc = escape::hex;
                        } else if(cc == 'u') {
                            esc = escape::uni4;
                        } else if(cc == 'U') {
                            esc = escape::uni8;
                        } else if(cc == '\\') {
                            res += cc; esc = escape::no;
                        } else {
                            cur_char_num.clear(); res += cc; esc = escape::no;
                        }
                        break;
                    case escape::oct:
                        if(cc >= '0' && cc <= '7') {
                            cur_char_num += cc;
                            if(cur_char_num.size() > 24) {
                                throw std::runtime_error{"octal buffer overflow"};
                            }
                        } else {
                            if(cur_char_num.size()) {
                                res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 8);
                                cur_char_num.clear();
                            } else {
                                throw std::runtime_error("octal sequence is started");
                            }
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::hex:
                        if((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F')) {
                            cur_char_num += cc;
                            if(cur_char_num.size() > 16) {
                                throw std::runtime_error{"hexadecimal buffer overflow"};
                            }
                        } else {
                            if(cur_char_num.size()) {
                                res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                                cur_char_num.clear();
                            } else {
                                throw std::runtime_error("hexadecimal sequence is started");
                            }
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::uni4:
                        if((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F')) {
                            cur_char_num += cc;
                            if(cur_char_num.size() > 16) {
                                throw std::runtime_error{"hexadecimal buffer overflow"};
                            }
                        } else {
                            if(cur_char_num.size()) {
                                res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                                cur_char_num.clear();
                            } else {
                                throw std::runtime_error("unicode sequence is started");
                            }
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                    case escape::uni8:
                        if((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F')) {
                            cur_char_num += cc;
                            if(cur_char_num.size() == 8) {
                                res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                                cur_char_num.clear();
                                esc = escape::no;
                            }
                        } else {
                            if(cur_char_num.size() != 8) {
                                throw std::runtime_error("unicode sequence is not completed");
                            }
                            res += (typename STR_T::value_type)str_util::atoui(cur_char_num, 16);
                            cur_char_num.clear();
                            esc = escape::no;
                            unescape_char(cc);
                        }
                        break;
                }
            }
        };

    }


    template<typename STR_T>
    class fsm_tokenizer {
        enum class parse_states { initial, rule, target };
        using CHAR_T = typename STR_T::value_type;

    public:
        struct token {
            STR_T value;
            STR_T type;
            int line;
            int column;
        };

    public:
        fsm_tokenizer() {
            reset();
        }

        fsm_tokenizer(std::function<void(token const &)> delegate): lsnr_{delegate} {
            reset();
        }

        fsm_tokenizer(const STR_T &rules) {
            reset();
            add_rules(rules);
        }

        fsm_tokenizer(const STR_T &rules, std::function<void(token const &)> delegate):
            lsnr_{delegate}
        {
            reset();
            add_rules(rules);
        }

        void reset() {
            rule_parse_state_ = parse_states::initial;
            buffer_.clear();
            state_current_ = STATE_INITIAL;
            rules_.clear();
            line_ = 0;
            colmn_ = 0;
            current_l_ = 0;
            current_c_ = 0;
            match_cache_.clear();
        }

        INLINE_GETTER_SETTER(CHAR_T, name_for_self_char_prefix, SELF_CHAR_PREFIX)
        INLINE_GETTER_SETTER(CHAR_T, name_for_outer_char_prefix, OUTER_CHAR_PREFIX)
        INLINE_GETTER_SETTER(CHAR_T, name_for_rules_separator, RULES_SEPARATOR)
        INLINE_GETTER_SETTER(CHAR_T, name_for_ruleset_finalizer, RULESET_FINALIZER)
        INLINE_GETTER_SETTER(STR_T, name_for_rule_to_target_transition, RULE_TO_TARGET_TRANSITION)
        INLINE_GETTER_SETTER(STR_T, name_for_state_initial, STATE_INITIAL)
        INLINE_GETTER_SETTER(STR_T, name_for_state_error, STATE_ERROR)
        INLINE_GETTER_SETTER(STR_T, name_for_class_default, CLASS_DEFAULT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_space, CATEGORY_SPACE)
        INLINE_GETTER_SETTER(STR_T, name_for_category_alpha, CATEGORY_ALPHA)
        INLINE_GETTER_SETTER(STR_T, name_for_category_alnum, CATEGORY_ALNUM)
        INLINE_GETTER_SETTER(STR_T, name_for_category_cntrl, CATEGORY_CNTRL)
        INLINE_GETTER_SETTER(STR_T, name_for_category_digit, CATEGORY_DIGIT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_punct, CATEGORY_PUNCT)
        INLINE_GETTER_SETTER(STR_T, name_for_category_hex, CATEGORY_HEX)
        INLINE_GETTER_SETTER(STR_T, name_for_category_oct, CATEGORY_OCT)

        void accept_char(CHAR_T sym) {
            accept(sym, false);
            current_c_++;
            if(sym == '\n') {
                current_l_++;
                current_c_ = 0;
            }
        }

        void accept_string(const STR_T &in_str) {
            for(size_t i{0}; i < in_str.size(); ++i) {
                accept(in_str[i], false);
                current_c_++;
                if(in_str[i] == '\n') {
                    current_l_++;
                    current_c_ = 0;
                }
            }
        }

        void accept_string(CHAR_T const *in_str, std::size_t len) {
            for(size_t i{0}; i < len; ++i) {
                accept(in_str[i], false);
                current_c_++;
                if(in_str[i] == '\n') {
                    current_l_++;
                    current_c_ = 0;
                }
            }
        }

        void finalize() {
            accept(0, true);
            rule_parse_state_ = parse_states::initial;
            buffer_.clear();
            state_current_ = STATE_INITIAL;
            line_ = 0;
            colmn_ = 0;
            current_l_ = 0;
            current_c_ = 0;
        }

        void set_listener_delegate(std::function<void(token const &)> lsnr) {
            lsnr_ = lsnr;
        }

        void add_rules(const STR_T &rules) {
            match_cache_.clear();
            rule_parse_state_ = parse_states::initial;
            buffer_.clear();
            state_current_ = STATE_INITIAL;
            line_ = 0;
            colmn_ = 0;
            current_l_ = 0;
            current_c_ = 0;

            STR_T state_name{};
            STR_T symbol_or_class{};
            STR_T next_state{};

            STR_T buf_scope{};

            bool escape{false};

            rule_parse_state_ = parse_states::initial;
            for(auto ch: rules) {
                switch(rule_parse_state_) {
                    case parse_states::initial:
                        if(escape) {
                            state_name += ch;
                            escape = false;
                        } else {
                            if(ch == RULES_SEPARATOR) {
                                rule_parse_state_ = parse_states::rule;
                                state_name = scfx::str_util::trim(state_name);
                                if(state_name.empty()) {
                                    throw std::runtime_error{"error in rules"};
                                }
                            } else if(ch == ESCAPE_CHAR) {
                                escape = true;
                            } else {
                                state_name += ch;
                            }
                        }
                        break;
                    case parse_states::rule:
                        if(buf_scope + ch == RULE_TO_TARGET_TRANSITION) {
                            buf_scope.clear();
                            rule_parse_state_ = parse_states::target;
                            if(symbol_or_class.size() > 2) {
                                symbol_or_class = symbol_or_class.substr(0, symbol_or_class.size() - RULE_TO_TARGET_TRANSITION.size() + 1);
                                symbol_or_class = scfx::str_util::trim(symbol_or_class);
                                if(symbol_or_class.empty()) {
                                    throw std::runtime_error{"error in rules"};
                                }
                            } else {
                                throw std::runtime_error{"error in rules"};
                            }
                        } else {
                            symbol_or_class += ch;
                            buf_scope += ch;
                            buf_scope = buf_scope.size() > RULE_TO_TARGET_TRANSITION.size() - 1 ? buf_scope.substr(buf_scope.size() - (RULE_TO_TARGET_TRANSITION.size() - 1)) : buf_scope;
                        }
                        break;
                    case parse_states::target:
                        if(escape) {
                            next_state += ch;
                            escape = false;
                        } else {
                            if(ch == RULES_SEPARATOR) {
                                next_state = scfx::str_util::trim(next_state);
                                if(next_state.empty()) {
                                    throw std::runtime_error{"error in rules"};
                                }
                                add_rule(
                                    scfx::str_util::trim(state_name),
                                    scfx::str_util::trim(symbol_or_class),
                                    scfx::str_util::trim(next_state)
                                );
                                symbol_or_class.clear();
                                next_state.clear();
                                rule_parse_state_ = parse_states::rule;
                            } else if(ch == RULESET_FINALIZER) {
                                rule_parse_state_ = parse_states::initial;
                                next_state = scfx::str_util::trim(next_state);
                                if(next_state.empty()) {
                                    throw std::runtime_error{"error in rules"};
                                }
                                add_rule(
                                    scfx::str_util::trim(state_name),
                                    scfx::str_util::trim(symbol_or_class),
                                    scfx::str_util::trim(next_state)
                                );
                                state_name.clear();
                                symbol_or_class.clear();
                                next_state.clear();
                            } else if(ch == ESCAPE_CHAR) {
                                escape = true;
                            } else {
                                next_state += ch;
                            }
                        }
                        break;
                }
            }
            if(rule_parse_state_ != parse_states::initial || escape) {
                throw std::runtime_error{"error in rules"};
            }
        }

    private:
#ifdef DEBUG_FSM_TOKENIZER
        std::map<STR_T, std::map<CHAR_T, bool>> match_cache_{};
#else
        std::unordered_map<STR_T, std::unordered_map<CHAR_T, bool>> match_cache_{};
#endif

        bool match(CHAR_T sym, bool eof, const STR_T &sym_or_class) {
            if(sym_or_class == CLASS_EOF) {
                return eof;
            }
            bool res{false};
            bool not_cond{false};
            if(sym_or_class[0] == '\'') {
                return sym == sym_or_class[1];
            } else {
                auto mc_entry_it{match_cache_.find(sym_or_class)};
                if(mc_entry_it != match_cache_.end()) {
                    auto c_entry_it{mc_entry_it->second.find(sym)};
                    if(c_entry_it != mc_entry_it->second.end()) {
                        return c_entry_it->second;
                    }
                }

                enum class local_states { none, class_body, end };
                local_states ls{local_states::none};
                bool spaces{false};
                bool alphas{false};
                bool digits{false};
                bool puncts{false};
                bool cntrl{false};
                bool hex{false};
                bool oct{false};

                bool not_spaces{false};
                bool not_alphas{false};
                bool not_digits{false};
                bool not_puncts{false};
                bool not_cntrl{false};
                bool not_hex{false};
                bool not_oct{false};

                STR_T set{};
                STR_T negative_set{};
                bool local_not_cond{false};
                for(std::size_t i{0}; i < sym_or_class.size(); ++i) {
                    if(ls == local_states::end) {
                        break;
                    }
                    CHAR_T cc{sym_or_class[i]};
                    switch (ls) {
                        case local_states::none:
                            if(cc == '!') {
                                not_cond = !not_cond;
                            } else if(cc == '[') {
                                ls = local_states::class_body;
                            }
                            break;
                        case local_states::class_body:
                            if(cc == '!') {
                                local_not_cond = true;
                                continue;
                            } else if(cc == ']') {
                                ls = local_states::end;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_SPACE)) {
                                if(local_not_cond) {
                                    not_spaces = true;
                                } else {
                                    spaces = true;
                                }
                                i += CATEGORY_SPACE.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_ALPHA)) {
                                if(local_not_cond) {
                                    not_alphas = true;
                                } else {
                                    alphas = true;
                                }
                                i += CATEGORY_ALPHA.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_ALNUM)) {
                                if(local_not_cond) not_digits = true; else digits = true;
                                if(local_not_cond) not_alphas = true; else alphas = true;
                                i += CATEGORY_ALNUM.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_CNTRL)) {
                                if(local_not_cond) not_cntrl = true; else cntrl = true;
                                i += CATEGORY_CNTRL.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_DIGIT)) {
                                if(local_not_cond) not_digits = true; else digits = true;
                                i += CATEGORY_DIGIT.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_PUNCT)) {
                                if(local_not_cond) not_puncts = true; else puncts = true;
                                i += CATEGORY_PUNCT.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_HEX)) {
                                if(local_not_cond) not_hex = true; else hex = true;
                                i += CATEGORY_HEX.size() - 1;
                                continue;
                            } else if(scfx::str_util::string_starts_from(sym_or_class.substr(i), CATEGORY_OCT)) {
                                if(local_not_cond) not_oct = true; else oct = true;
                                i += CATEGORY_OCT.size() - 1;
                                continue;
                            }
                            if(local_not_cond) {
                                negative_set += cc;
                            } else {
                                set += cc;
                            }
                            local_not_cond = false;
                            break;

                        default:
                            break;
                    }
                }

                bool neg_found{false};
                if(not_spaces) { if(scfx::str_util::fltr<STR_T>::isspace(sym)) { neg_found = true; } }
                if(not_alphas) { if(scfx::str_util::fltr<STR_T>::isalpha(sym)) { neg_found = true; } }
                if(not_digits) { if(scfx::str_util::fltr<STR_T>::isdigit(sym)) { neg_found = true; } }
                if(not_puncts) { if(scfx::str_util::fltr<STR_T>::ispunct(sym)) { neg_found = true; } }
                if(not_cntrl) { if(scfx::str_util::fltr<STR_T>::iscntrl(sym)) { neg_found = true; } }
                if(not_hex) { if(scfx::str_util::fltr<STR_T>::ishexdigit(sym)) { neg_found = true; } }
                if(not_oct) { if(scfx::str_util::fltr<STR_T>::isoctdigit(sym)) { neg_found = true; } }
                for(auto c: negative_set) {
                    if(sym == c) {
                        neg_found = true;
                        break;
                    }
                }

                bool pos_given{false};
                bool pos_found{false};
                if(spaces) { pos_given = true; if(scfx::str_util::fltr<STR_T>::isspace(sym)) { pos_found = true; } }
                if(alphas) { pos_given = true; if(scfx::str_util::fltr<STR_T>::isalpha(sym)) { pos_found = true; } }
                if(digits) { pos_given = true; if(scfx::str_util::fltr<STR_T>::isdigit(sym)) { pos_found = true; } }
                if(puncts) { pos_given = true; if(scfx::str_util::fltr<STR_T>::ispunct(sym)) { pos_found = true; } }
                if(cntrl) { pos_given = true; if(scfx::str_util::fltr<STR_T>::iscntrl(sym)) { pos_found = true; } }
                if(hex) { pos_given = true; if(scfx::str_util::fltr<STR_T>::ishexdigit(sym)) { pos_found = true; } }
                if(oct) { pos_given = true; if(scfx::str_util::fltr<STR_T>::isoctdigit(sym)) { pos_found = true; } }
                for(auto const &c: set) {
                    pos_given = true;
                    if(sym == c) {
                        pos_found = true;
                        break;
                    }
                }

                res = !neg_found && (pos_found || !pos_given);
            }
            bool fin_res{not_cond ? !res : res};

            match_cache_[sym_or_class][sym] = fin_res;

            return fin_res;
        }

        inline void add_rule(STR_T const &state_name, STR_T const &symbol_or_class, STR_T const &next_state) {
            STR_T sym_or_class_unescaped{detail::unescaper<STR_T>{symbol_or_class}.val()};
            if(sym_or_class_unescaped.size() < 3) {
                throw std::runtime_error{"invalid matching criteria"};
            }
            rules_[state_name][sym_or_class_unescaped] = next_state;
        }

        static inline bool state_valid(STR_T const &s) {
            return !s.empty();
        }

        inline bool state_final(STR_T const &s) const {
            return s[0] == OUTER_CHAR_PREFIX || s[0] == SELF_CHAR_PREFIX;
        }

        inline bool state_initial(STR_T const &s) const {
            return s == STATE_INITIAL;
        }

        bool is_current_state_initial() const {
            return state_current_ == STATE_INITIAL;
        }

        void set_state(STR_T const &s) {
            if(state_initial(s)) {
                line_ = current_l_;
                colmn_ = current_c_;
            }
            state_current_ = s;
        }

        token create_token(STR_T const &type) {
            token t{};
            t.value = buffer_;
            t.type = type;
            t.line = line_ + 1;
            t.column = colmn_ + 1;
            return t;
        }

        void accept(CHAR_T sym, bool eof, bool repeat = false) {
            if(eof && is_current_state_initial()) {
                return;
            }
            auto state_map_it{rules_.find(state_current_)};
            if(state_map_it == rules_.end()) {
                std::stringstream ss{};
                ss << "at line " << current_l_ + 1 << ", col " << current_c_ + 1 << ": no rules for transition from state \"" << scfx::str_util::fltr<STR_T>::utf8(state_current_) << "\"";
                throw std::runtime_error{ss.str()};
            }
            STR_T match_state{};
            auto const &m{state_map_it->second};
            STR_T sym_str{}; sym_str += '\''; sym_str += ' '; sym_str += '\'';
            if(eof) {
                sym_str = CLASS_EOF;
            } else {
                sym_str[1] = (CHAR_T)sym;
            }
            auto state_char_it{m.find(sym_str)};
            if(state_char_it == m.end()) {
                for(auto const &p: m) {
                    auto const &sym_or_class{p.first};
                    auto const &next_state{p.second};
                    if(CLASS_DEFAULT != sym_or_class && match(sym, eof, sym_or_class)) {
                        match_state = next_state;
                        break;
                    }
                }
            } else {
                match_state = state_char_it->second;
            }

            if(match_state == STATE_ERROR) {
                std::stringstream ss{};
                ss << "invalid character at line " << current_l_ + 1 << ", col " << current_c_ + 1;
                throw std::runtime_error{ss.str()};
            }

            if(!match_state.empty()) {
                if(state_final(match_state)) {
                    if(is_current_state_initial()) {
                        buffer_.clear();
                        buffer_ += sym;
                        if(lsnr_) {
                            lsnr_(create_token(match_state.substr(1)));
                        }
                        buffer_.clear();
                        set_state(STATE_INITIAL);
                    } else {
                        bool sym_belongs_current{match_state[0] == SELF_CHAR_PREFIX};
                        if(sym_belongs_current) {
                            buffer_ += sym;
                            if(lsnr_) {
                                lsnr_(create_token(match_state.substr(1)));
                            }
                            buffer_.clear();
                            set_state(STATE_INITIAL);
                        } else {
                            if(lsnr_) {
                                lsnr_(create_token(match_state.substr(1)));
                            }
                            buffer_.clear();
                            set_state(STATE_INITIAL);
                            if(!repeat) {
                                accept(sym, eof, true);
                            }
                        }
                    }
                } else {
                    buffer_ += sym;
                    set_state(match_state);
                }
            } else {
                STR_T default_state{};
                auto def_it{m.find(CLASS_DEFAULT)};
                if(def_it != m.end()) {
                    default_state = def_it->second;
                }
                if(default_state.empty()) {
                    std::stringstream ss{};
                    ss << "invalid character at line " << current_l_ + 1 << ", col " << current_c_ + 1;
                    throw std::runtime_error{ss.str()};
                }
                if(state_final(default_state)) {
                    if(is_current_state_initial()) {
                        buffer_.clear();
                        buffer_ += sym;
                        if(lsnr_) {
                            lsnr_(create_token(default_state.substr(1)));
                        }
                        buffer_.clear();
                        set_state(STATE_INITIAL);
                    } else {
                        bool sym_belongs_current{default_state[0] == SELF_CHAR_PREFIX};
                        if(sym_belongs_current) {
                            buffer_ += sym;
                        }
                        if(lsnr_) {
                            lsnr_(create_token(default_state.substr(1)));
                        }
                        buffer_.clear();
                        set_state(STATE_INITIAL);
                        if(!sym_belongs_current && !repeat) {
                            accept(sym, eof, true);
                        }
                    }
                } else {
                    set_state(default_state);
                    buffer_ += sym;
                }
            }
        }

        STR_T RULE_TO_TARGET_TRANSITION{scfx::str_util::fltr<STR_T>::cast(">>")};
        STR_T STATE_INITIAL{scfx::str_util::fltr<STR_T>::cast("#rd")};
        STR_T STATE_ERROR{scfx::str_util::fltr<STR_T>::cast("#er")};
        STR_T CLASS_DEFAULT{scfx::str_util::fltr<STR_T>::cast("$df")};
        STR_T CLASS_EOF{scfx::str_util::fltr<STR_T>::cast("$ef")};
        STR_T CATEGORY_SPACE{scfx::str_util::fltr<STR_T>::cast(":space:")};
        STR_T CATEGORY_ALPHA{scfx::str_util::fltr<STR_T>::cast(":alpha:")};
        STR_T CATEGORY_ALNUM{scfx::str_util::fltr<STR_T>::cast(":alnum:")};
        STR_T CATEGORY_CNTRL{scfx::str_util::fltr<STR_T>::cast(":cntrl:")};
        STR_T CATEGORY_DIGIT{scfx::str_util::fltr<STR_T>::cast(":digit:")};
        STR_T CATEGORY_PUNCT{scfx::str_util::fltr<STR_T>::cast(":punct:")};
        STR_T CATEGORY_HEX{scfx::str_util::fltr<STR_T>::cast(":hex:")};
        STR_T CATEGORY_OCT{scfx::str_util::fltr<STR_T>::cast(":oct:")};
        CHAR_T ESCAPE_CHAR{'\\'};
        CHAR_T SELF_CHAR_PREFIX{'<'};
        CHAR_T OUTER_CHAR_PREFIX{'^'};
        CHAR_T RULES_SEPARATOR{'|'};
        CHAR_T RULESET_FINALIZER{';'};

        parse_states rule_parse_state_{parse_states::initial};
        STR_T buffer_{};
        STR_T state_current_{};
#ifdef DEBUG_FSM_TOKENIZER
        std::map<STR_T, std::map<STR_T, STR_T>> rules_{};
#else
        std::unordered_map<STR_T, std::unordered_map<STR_T, STR_T>> rules_{};
#endif

        int line_{0};
        int colmn_{0};
        int current_l_{0};
        int current_c_{0};

        std::function<void(token const &)> lsnr_{nullptr};
    };

}

#undef USE_CUSTOM_HASHER
