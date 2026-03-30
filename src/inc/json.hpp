#pragma once

#include "commondefs.hpp"
#include "emhash/hash_table8.hpp"
#include "emhash/hash_set8.hpp"
#include "fsm_tokenizer.hpp"
#include "serialization.hpp"
#include "str_util.hpp"
#include "base64.hpp"

namespace scfx {

    using std::any;
    using std::any_cast;

    DEFINE_RUNTIME_ERROR_CLASS(json_error)

    namespace detail {

        static std::wstring wescape(std::wstring const &ws) {
            std::wstringstream res{};
            for(std::wstring::size_type i = 0; i < ws.size(); i++) {
                wchar_t c = ws[i];
                switch (c) {
                    case '\0': res << "\\u0000"; break;
                    case '\"': case '\\': res << '\\' << c; break;
                    case '\a': res << '\\' << 'a'; break;
                    case '\b': res << '\\' << 'b'; break;
                    case '\f': res << '\\' << 'f'; break;
                    case '\n': res << '\\' << 'n'; break;
                    case '\r': res << '\\' << 'r'; break;
                    case '\t': res << '\\' << 't'; break;
                    case '\v': res << '\\' << 'v'; break;
                    default: res << c; break;
                }
            }
            return res.str();
        }

        static std::string escape(std::string const &s) {
            return str_util::to_utf8(wescape(str_util::from_utf8(s)));
        }

        static std::wstring wunescape(std::wstring const &ws) {
            std::wstringstream res{};
            bool escp{false};
            bool rn_escaped{false};
            bool unicode4_escape{false};
            bool unicode8_escape{false};
            std::wstring uni_buff{};
            for(std::size_t i = 0; i < ws.size(); ++i) {
                wchar_t c = ws[i];
                if(rn_escaped && (c == L'\n' || c == L'\r')) {
                    rn_escaped = false;
                    continue;
                }
                if(escp || unicode4_escape || unicode8_escape) {
                    if(unicode4_escape) {
                        if(str_util::ishex(c)) {
                            uni_buff += c;
                        } else {
                            throw json_error{"invalid string escape sequence"};
                        }
                        if(uni_buff.size() == 4) {
                            res << (wchar_t)str_util::atoui(str_util::to_utf8(uni_buff), 16);
                            uni_buff.clear();
                            unicode4_escape = false;
                        }
                    } else if(unicode8_escape) {
                        if(str_util::ishex(c)) {
                            uni_buff += c;
                        } else {
                            throw json_error{"invalid string escape sequence"};
                        }
                        if(uni_buff.size() == 8) {
                            res << (wchar_t)str_util::atoui(str_util::to_utf8(uni_buff), 16);
                            uni_buff.clear();
                            unicode8_escape = false;
                        }
                    } else {
                        switch(c) {
                            case L'0': res << L'\0'; break;
                            case L'a': res << L'\a'; break;
                            case L'b': res << L'\b'; break;
                            case L'f': res << L'\f'; break;
                            case L'n': res << L'\n'; break;
                            case L'r': res << L'\r'; break;
                            case L't': res << L'\t'; break;
                            case L'v': res << L'\v'; break;
                            case L'u': unicode4_escape = true; break;
                            case L'U': unicode8_escape = true; break;
                            case L'\n': case L'\r': rn_escaped = true; break;
                            default: res << c; break;
                        }
                        escp = false;
                    }
                } else {
                    if(c == L'\\') {
                        escp = true;
                    } else {
                        res << c;
                    }
                }
            }
            if(escp || unicode4_escape || unicode8_escape) {
                throw json_error{"invalid string escape sequence"};
            }
            return res.str();
        }

        static bool is_ident(std::wstring const &v) {
            if(v.size() == 0) { return false; }
            auto c0{v[0]};
            if(!(c0 == '_' || c0 == '$' || scfx::str_util::fltr<std::wstring>::isalpha(c0))) { return false; }
            for(auto &&c: v) { if(!(c == '_' || c == '$' || scfx::str_util::fltr<std::wstring>::isalnum(c))) { return false; } }
            static emhash8::HashSet<std::wstring> const rsvd{
                L"false",      L"true",     L"null",
                L"break",      L"do",       L"instanceof", L"typeof",
                L"case",       L"else",     L"new",        L"var",
                L"catch",      L"finally",  L"return",     L"void",
                L"continue",   L"for",      L"switch",     L"while",
                L"debugger",   L"function", L"this",       L"with",
                L"default",    L"if",       L"throw",
                L"delete",     L"in",       L"try",
                L"class",      L"enum",     L"extends",    L"super",
                L"const",      L"export",   L"import",
                L"implements", L"let",      L"private",    L"public", L"yield",
                L"interface",  L"package",  L"protected",  L"static",
            };
            return rsvd.find(v) == rsvd.end();
        }

        static bool is_ident(std::string const &v) {
            return is_ident(str_util::from_utf8(v));
        }

        template<typename JSON>
        class json_deserializer final {
        public:
            json_deserializer() {
                reset();
            }

            void reset() {
                stack_.clear();
                stack_.push_back({});
                what_next_ = expected::listed;
                nxt_lst_ = &next_list_initial_;
            }

            void num_expect() {
                if(!sgn_que_.empty()) {
                    throw json_error{"json error: number expected after sign"};
                }
            }

            auto harvest_result() {
                num_expect();
                if(stack_.size() > 1) {
                    throw json_error{"json error: unexpected end of input"};
                }
                return std::move(top().obj);
            }

            void accept_token(scfx::fsm_tokenizer<std::wstring>::token const &o) {
                auto t = o.type;
                if(t != L"spc") {
                    auto k = o.value;
                    if(t == L"str" || t == L"sts") {
                        k = k.substr(1, k.size() - 2);
                    }
                    consume_token(k, t, o.line, o.column);
                }
            }

            void consume_token(std::wstring const &tk, std::wstring const &tt, int line, int col) {
                if(tt == L"cmt") {
                    return;
                }
                if(
                    what_next_ == expected::none
                    || (what_next_ == expected::listed && !nxt_lst_->contains(scfx::str_util::to_utf8(tt)))
                    || (stack_.size() == 1 && top().closed)
                ) {
                    static const emhash8::HashMap<std::string, std::string> exp_map {
                        {"str" , "<string>" },
                        {"sts" , "<single-quoted string>" },
                        {"sgn" , "<sign>" },
                        {"int" , "<int>" },
                        {"hex" , "<hex>" },
                        {"fpl" , "<number>" },
                        {"inf" , "<inf>" },
                        {"nan" , "<nan>" },
                        {"idr" , "<identifier>" },
                        {"tru" , "\"true\"" },
                        {"fal" , "\"false\"" },
                        {"nul" , "\"null\"" },
                        {"["   , "\"[\"" },
                        {"]"   , "\"]\"" },
                        {"{"   , "\"{\"" },
                        {"}"   , "\"}\"" },
                        {","   , "\",\"" },
                        {":"   , "\":\"" },
                    };
                    std::stringstream ss{};
                    ss << "json error: at <" << line << ":" << col << "> expected: ";
                    if(what_next_ == expected::listed) {
                        std::string sep{};
                        for(auto&& s: *nxt_lst_) {
                            ss << sep << exp_map.at(s);
                            sep = " or ";
                        }
                    } else if(what_next_ == expected::none) {
                        ss << "Nothing";
                    } else if(what_next_ == expected::any) {
                        ss << "Anything";
                    } else {
                        ss << "<Unknown>";
                    }
                    ss << ", got: \"" << scfx::str_util::to_utf8(tk) << "\" (" << exp_map.at(scfx::str_util::to_utf8(tt)) << ")";
                    throw json_error{ss.str()};
                }

                if(tt == L"str" || tt == L"sts") {
                    num_expect();
                    if(top().obj.is_array()) {
                        top().obj.push_back(detail::wunescape(tk));
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_str_a_;
                    } else if(top().obj.is_object()) {
                        push();
                        top().name = scfx::str_util::to_utf8(detail::wunescape(tk));
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_str_o_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = scfx::str_util::to_utf8(detail::wunescape(tk));
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_str_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = (detail::wunescape(tk));
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"idr") {
                    num_expect();
                    push();
                    top().name = scfx::str_util::to_utf8(detail::wunescape(tk));
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    nxt_lst_ = &next_list_str_o_;
                } else if(tt == L"int") {
                    std::int64_t v{scfx::str_util::atoi(tk)};
                    if(neg_accordingly_sgn_stack()) { v = -v; }
                    if(top().obj.is_array()) {
                        top().obj.push_back(v);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_int_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = v;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_int_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = v;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"hex") {
                    std::int64_t v{scfx::str_util::atoi(tk.substr(2), 16)};
                    if(neg_accordingly_sgn_stack()) { v = -v; }
                    if(top().obj.is_array()) {
                        top().obj.push_back(v);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_int_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = v;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_int_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = v;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"fpl") {
                    std::wistringstream os(tk); long double ld; os >> ld;
                    if(neg_accordingly_sgn_stack()) { ld = -ld; }
                    if(top().obj.is_array()) {
                        top().obj.push_back(ld);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = ld;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = ld;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"inf") {
                    long double ld{std::numeric_limits<long double>::infinity()};
                    if(neg_accordingly_sgn_stack()) { ld = -ld; }
                    if(top().obj.is_array()) {
                        top().obj.push_back(ld);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = ld;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = ld;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"nan") {
                    long double ld{std::numeric_limits<long double>::quiet_NaN()};
                    if(neg_accordingly_sgn_stack()) { ld = -ld; }
                    if(top().obj.is_array()) {
                        top().obj.push_back(ld);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = ld;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_fpl_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = ld;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"nul") {
                    num_expect();
                    if(top().obj.is_array()) {
                        top().obj.push_back({});
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_nul_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = {};
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_nul_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = {};
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"tru" || tt == L"fal") {
                    num_expect();
                    if(top().obj.is_array()) {
                        top().obj.push_back(tk.size() == 4);
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_tf_a_;
                    } else if(top().obj_itm_phase == 2) {
                        top().obj = tk.size() == 4;
                        ++top().obj_itm_phase;
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_tf_ip2_;
                        top().closed = true;
                    } else {
                        top().obj = tk.size() == 4;
                        what_next_ = expected::none;
                        top().closed = true;
                    }
                } else if(tt == L"{") {
                    num_expect();
                    if(top().obj_itm_phase == 2) {
                        ++top().obj_itm_phase;
                        top().closed = true;
                    }
                    push();
                    top().obj.become_object();
                    what_next_ = expected::listed;
                    nxt_lst_ = &next_list_ocb_;
                } else if(tt == L"}") {
                    num_expect();
                    if(!top().obj.is_object()) {
                        throw json_error{"json error: \"}\" - invalid token."};
                    }
                    top().closed = true;
                    if(stack_.size() == 2 && top().closed) {
                        what_next_ = expected::none;
                    } else {
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_ccb_;
                    }
                }
                else if(tt == L"[") {
                    num_expect();
                    if(top().obj_itm_phase == 2) {
                        ++top().obj_itm_phase;
                        top().closed = true;
                    }
                    push();
                    top().obj.become_array();
                    what_next_ = expected::listed;
                    nxt_lst_ = &next_list_osb_;
                } else if(tt == L"]") {
                    num_expect();
                    if(!top().obj.is_array()) {
                        throw json_error{"json error: \"]\" - invalid token."};
                    }
                    top().closed = true;

                    if(stack_.size() == 2 && top().closed) {
                        what_next_ = expected::none;
                    } else {
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_csb_;
                    }
                } else if(tt == L":") {
                    num_expect();
                    if(top().obj_itm_phase != 1) {
                        throw json_error{"json error: \":\" - invalid token, name expected."};
                    }
                    ++top().obj_itm_phase;
                    what_next_ = expected::listed;
                    nxt_lst_ = &next_list_col_;
                } else if(tt == L",") {
                    num_expect();
                    if(!top().obj.is_object() && !top().obj.is_array()) {
                        throw json_error{"json error: \":\" - invalid token, name expected."};
                    }
                    if(top().obj.is_object()) {
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_com_o_;
                    } else if(top().obj.is_array()) {
                        what_next_ = expected::listed;
                        nxt_lst_ = &next_list_com_a_;
                    }
                } else if(tt == L"sgn") {
                    sgn_que_.push_back(tk);
                    what_next_ = expected::listed;
                    nxt_lst_ = &next_list_sgn_;
                } else {
                    throw json_error{std::string{"json error: invalid token: \""} + scfx::str_util::to_utf8(tk) + "\""};
                }
                collapse_top_down();
            }

        private:
            bool neg_accordingly_sgn_stack() {
                if(!sgn_que_.empty()) {
                    int minus_cnt{0};
                    for(auto &&c: sgn_que_) {
                        minus_cnt += c == L"-" ? 1 : 0;
                    }
                    sgn_que_.clear();
                    return minus_cnt % 2 == 1;
                }
                return false;
            }

        private:
            std::list<std::wstring> sgn_que_{};
            struct stack_item {
                JSON obj{};
                std::string name{};
                int obj_itm_phase{0};
                bool closed{false};
            };

            std::list<stack_item> stack_{};

            enum class expected {
                any,
                none,
                listed
            };

            expected what_next_{expected::any};
            static inline std::unordered_set<std::string> const next_list_initial_{
                "str", "sts", "sgn", "hex", "int", "fpl", "inf", "nan", "tru", "fal", "nul", "[", "{"
            };
            static inline std::unordered_set<std::string> const next_list_str_a_{",", "]"};
            static inline std::unordered_set<std::string> const next_list_str_o_{":"};
            static inline std::unordered_set<std::string> const next_list_str_ip2_{"}", "]", ","};
            static inline std::unordered_set<std::string> const next_list_int_a_{",", "]"};
            static inline std::unordered_set<std::string> const next_list_int_ip2_{"}", "]", ","};
            static inline std::unordered_set<std::string> const next_list_fpl_a_{",", "]"};
            static inline std::unordered_set<std::string> const next_list_fpl_ip2_{"}", "]", ","};
            static inline std::unordered_set<std::string> const next_list_nul_a_{",", "]"};
            static inline std::unordered_set<std::string> const next_list_nul_ip2_{"}", "]", ","};
            static inline std::unordered_set<std::string> const next_list_tf_a_{",", "]"};
            static inline std::unordered_set<std::string> const next_list_tf_ip2_{"}", "]", ","};
            static inline std::unordered_set<std::string> const next_list_ocb_{"idr", "str", "sts", "}"};
            static inline std::unordered_set<std::string> const next_list_ccb_{"]", "}", ","};
            static inline std::unordered_set<std::string> const next_list_osb_{
                "str", "sts", "sgn", "hex", "int", "fpl", "inf", "nan", "tru", "fal", "nul", "[", "]", "{"
            };
            static inline std::unordered_set<std::string> const next_list_csb_{"]", "}", ","};
            static inline std::unordered_set<std::string> const next_list_col_{
                "str", "sts", "sgn", "hex", "int", "fpl", "inf", "nan", "tru", "fal", "nul", "[", "{"
            };
            static inline std::unordered_set<std::string> const next_list_com_o_{"idr", "str", "sts", "}"};
            static inline std::unordered_set<std::string> const next_list_com_a_{
                "str", "sts", "sgn", "hex", "int", "fpl", "inf", "nan", "tru", "fal", "nul", "[", "]", "{"
            };
            static inline std::unordered_set<std::string> const next_list_sgn_{
                "int", "hex", "fpl", "inf", "nan", "sgn"
            };
            std::unordered_set<std::string> const *nxt_lst_{&next_list_initial_};

        private:
            stack_item &top() {
                return stack_.back();
            }

            stack_item &pre_top() {
                auto it{stack_.end()};
                --it; --it;
                return *it;
            }

            void pop() {
                stack_.pop_back();
            }

            void push() {
                stack_.push_back({ {}, {}, 0, false });
            }

            void collapse_top_down() {
                while(stack_.size() > 1 && top().closed) {
                    if(pre_top().obj.is_object() && top().obj_itm_phase == 3) {
                        JSON o{std::move(top().obj)};
                        std::string n{ std::move(top().name) };
                        pop();
                        top().obj[n] = std::move(o);
                    } else if(pre_top().obj.is_array()) {
                        JSON o{std::move(top().obj)};
                        std::string n{ std::move(top().name) };
                        pop();
                        top().obj.push_back(std::move(o));
                    } else {
                        pre_top().obj = std::move(top().obj);
                        pop();
                    }
                    if(stack_.size() == 1) {
                        top().closed = true;
                    }
                }
            }
        };

    }

    class json {
    public:
        using size_type = std::uint64_t;
        using ssize_type = std::int64_t;

    private:
#ifdef SCFX_DEBUGGING
        using o_t = std::map<std::string, json>;
#else
        using o_t = emhash8::HashMap<std::string, json>;
        // using o_t = std::unordered_map<std::string, json>;
#endif
        using a_t = std::vector<json>;

    public:
        json() = default;
        ~json() { cleanup(); }
        json(json const &that): v_{that.v_}, t_{that.t_} {}
        json(json &&that) noexcept: v_{std::move(that.v_)}, t_{that.t_} { that.t_ = jo_null; }
        json &operator=(const json &that) { if(this != &that) { v_ = that.v_; t_ = that.t_; } return *this; }
        json &operator=(json &&that) noexcept { if(this != &that) { std::swap(v_, that.v_); std::swap(t_, that.t_); } return *this; }

        json(char const *v): v_{std::string{v}}, t_{jo_string} {}
        json(std::string const &v): v_{(v)}, t_{jo_string} {}
        json(std::string &&v) noexcept(std::is_nothrow_move_constructible<std::string>::value): v_{std::move(v)}, t_{jo_string} {}
        json(std::string_view v): v_{(std::string{v})}, t_{jo_string} {}
        json(wchar_t const *v): v_{scfx::str_util::to_utf8(v)}, t_{jo_string} {}
        json(std::wstring const &v): v_{scfx::str_util::to_utf8(v)}, t_{jo_string} {}
        json(std::wstring_view v): v_{scfx::str_util::to_utf8((std::wstring{v}))}, t_{jo_string} {}
        json(std::int64_t v): v_{v}, t_{jo_int} {}
        json(std::uint64_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::int32_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::uint32_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::int16_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::uint16_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::int8_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(std::uint8_t v): v_{static_cast<std::int64_t>(v)}, t_{jo_int} {}
        json(bool v): v_{v}, t_{jo_bool} {}
        json(long double v): v_{v}, t_{jo_flt} {}
        json(float v): v_{static_cast<long double>(v)}, t_{jo_flt} {}
        json(double v): v_{static_cast<long double>(v)}, t_{jo_flt} {}
        json(std::vector<std::uint8_t> const &v): v_{std::string{v.begin(), v.end()}}, t_{jo_string} {}
        template<std::size_t N>
        json(std::array<std::uint8_t, N> const &v): v_{std::string{v.begin(), v.end()}}, t_{jo_string} {}

        json &become_object() {
            if(t_ != jo_object) {
                v_ = o_t{};
                t_ = jo_object;
            }
            return *this;
        }

        json &become_array() {
            if(t_ != jo_array) {
                v_ = a_t{};
                t_ = jo_array;
            }
            return *this;
        }

        json &become_number() {
            if(t_ != jo_int) {
                v_ = {};
                t_ = jo_int;
            }
            return *this;
        }

        json &become_number(std::int64_t val) {
            if(t_ != jo_int) {
                v_ = val;
                t_ = jo_int;
            } else {
                v_ = val;
            }
            return *this;
        }

        json &become_real() {
            if(t_ != jo_flt) {
                v_ = {};
                t_ = jo_flt;
            }
            return *this;
        }

        json &become_real(long double val) {
            if(t_ != jo_flt) {
                v_ = val;
                t_ = jo_flt;
            } else {
                v_ = val;
            }
            return *this;
        }

        json &become_floating_point_number() {
            if(t_ != jo_flt) {
                v_ = {};
                t_ = jo_flt;
            }
            return *this;
        }

        json &become_floating_point_number(long double val) {
            if(t_ != jo_flt) {
                v_ = val;
                t_ = jo_flt;
            } else {
                v_ = val;
            }
            return *this;
        }

        json &become_string() {
            if(t_ != jo_string) {
                v_ = {};
                t_ = jo_string;
            }
            return *this;
        }

        json &become_string(std::string const &val) {
            if(t_ != jo_string) {
                v_ = val;
                t_ = jo_string;
            } else {
                v_ = val;
            }
            return *this;
        }

        json &become_string(std::string &&val) {
            if(t_ != jo_string) {
                v_ = std::move(val);
                t_ = jo_string;
            } else {
                v_ = std::move(val);
            }
            return *this;
        }

        json &become_boolean() {
            if(t_ != jo_bool) {
                v_ = false;
                t_ = jo_bool;
            }
            return *this;
        }

        json &become_boolean(bool val) {
            if(t_ != jo_bool) {
                v_ = val;
                t_ = jo_bool;
            } else {
                v_ = val;
            }
            return *this;
        }

        json &become_null() {
            if(t_ != jo_null) {
                v_ = any{};
                t_ = jo_null;
            }
            return *this;
        }

        void reset() {
            become_null();
        }

        bool is_object() const noexcept {
            return t_ == jo_object;
        }

        bool is_array() const noexcept {
            return t_ == jo_array;
        }

        bool is_string() const noexcept {
            return t_ == jo_string;
        }

        bool is_bool() const noexcept {
            return t_ == jo_bool;
        }

        bool is_number() const noexcept {
            return t_ == jo_int;
        }

        bool is_float() const noexcept {
            return t_ == jo_flt;
        }

        bool is_real() const noexcept {
            return t_ == jo_flt;
        }

        bool is_null() const noexcept {
            return t_ == jo_null;
        }

        scfx::bytevec as_bytevec() const {
            if(is_string()) {
                return scfx::base64_str_to_data(as<std::string>());
            } else {
                throw json_error{"json error: invalid source type: string needed"};
            }
        }

        std::int64_t as_number() const {
            if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_flt) {
                return as<long double>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        std::int64_t as_number(std::int64_t default_value) const {
            if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_flt) {
                return as<long double>();
            } else {
                return default_value;
            }
        }

        int as_int() const {
            if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_flt) {
                return as<long double>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        int as_int(int default_value) const {
            if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_flt) {
                return as<long double>();
            } else {
                return default_value;
            }
        }

        size_type as_size_t() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        size_type as_size_t(size_type default_value) const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                return default_value;
            }
        }

        float as_float() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        float as_float(float default_value) const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                return default_value;
            }
        }

        double as_double() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        double as_double(double default_value) const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                return default_value;
            }
        }

        long double as_longdouble() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                throw json_error{"json error: wrong number"};
            }
        }

        long double as_longdouble(long double default_value) const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else {
                return default_value;
            }
        }

        bool as_boolean() const {
            if(is_bool()) {
                return as<bool>();
            } else if(is_null()) {
                return false;
            } else if(is_number()) {
                return as<std::int64_t>() != 0;
            } else if(is_real()) {
                return as<long double>() != 0.0L;
            } else if(is_object()) {
                return as<o_t>().size() != 0;
            } else if(is_array()) {
                return as<a_t>().size() != 0;
            } else if(is_string()) {
                try {
                    long double flt{try_as_long_double()};
                    return flt != 0.0L;
                } catch(...) {
                }
                std::string s{as<std::string>()};
                if(scfx::str_util::fltr<std::string>::strtolower(s) == "true") {
                    return true;
                } else if(scfx::str_util::fltr<std::string>::strtolower(s) == "false") {
                    return false;
                } else {
                    return s.size() != 0;
                }
            } else {
                throw json_error{"json error: invalid json state"};
            }
        }

        std::string as_string() const {
            if(is_null()) {
                return std::string{};
            } else if(is_number()) {
                return str_util::itoa<std::string>(as_number());
            } else if(is_real()) {
                return str_util::ftoa(as_longdouble(), std::numeric_limits<long double>::digits10);
            } else if(is_bool()) {
                return as_boolean() ? "true" : "false";
            } else if(is_object()) {
                return serialize();
            } else if(is_array()) {
                return serialize();
            } else if(is_string()) {
                return string_ref();
            } else {
                throw json_error{"json error: invalid json state"};
            }
        }

        std::string const &string_ref() const {
            if(is_string()) {
                return as<std::string>();
            } else {
                throw json_error{"json error: not a string"};
            }
        }

        std::string &string_ref() {
            if(is_string()) {
                return as<std::string>();
            } else {
                throw json_error{"json error: not a string"};
            }
        }

        std::string const &direct_string() const {
            if(is_string()) {
                return as<std::string>();
            } else {
                throw json_error{"json error: not a string value"};
            }
        }

        std::string &direct_string() {
            if(is_string()) {
                return as<std::string>();
            } else {
                throw json_error{"json error: not a string value"};
            }
        }

        std::wstring as_wstring() const {
            return scfx::str_util::from_utf8(as_string());
        }

        size_type size() const {
            if(is_null()) {
                return 0;
            } else if(is_bool()) {
                return 1;
            } else if(is_string()) {
                return string_ref().size();
            } else if(is_number()) {
                return 1;
            } else if(is_real()) {
                return 1;
            } else if(is_object()) {
                return as<o_t>().size();
            } else if(is_array()) {
                return as<a_t>().size();
            }
            throw json_error{"json error: invalid json state"};
        }

        float try_as_float() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_bool) {
                return as<bool>() ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return scfx::str_util::atof(as<std::string>());
            }
            throw json_error{"json error: invalid json state"};
        }

        long double try_as_long_double() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_bool) {
                return as<bool>() ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return scfx::str_util::atof(as<std::string>());
            }
            throw json_error{"json error: invalid json state"};
        }

        double try_as_double() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_bool) {
                return as<bool>() ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return scfx::str_util::atof(as<std::string>());
            }
            throw json_error{"json error: invalid json state"};
        }

        std::int64_t try_as_number() const {
            if(t_ == jo_flt) {
                return as<long double>();
            } else if(t_ == jo_int) {
                return as<std::int64_t>();
            } else if(t_ == jo_bool) {
                return as<bool>() ? 1 : 0;
            } else if(t_ == jo_null) {
                return 0;
            } else if(t_ == jo_array || t_ == jo_object) {
                return size();
            } else if(t_ == jo_string) {
                return try_as_long_double();
            }
            throw json_error{"json error: invalid json state"};
        }

        template<typename T>
        std::vector<T> extract_array() const {
            if(!is_array()) {
                throw json_error{"json error: not an array"};
            }
            size_type sz{size()};
            std::vector<T> result{};
            for(size_type i{0}; i < sz; i++) {
                json const &v{(*this)[i]};
                if(v.is_bool()) {
                    result.push_back((T)v.as_boolean());
                } else if(v.is_null()) {
                    result.push_back(T{});
                } else if(v.is_number() || v.is_real()) {
                    result.push_back((T)v.as_number());
                } else if(v.is_string()) {
                    result.push_back(v.as_string());
                } else {
                    throw json_error{"json error: invalid conversion"};
                }
            }
            return result;
        }

        std::vector<std::string> extract_strarray() const {
            if(!is_array()) {
                throw json_error("json error: not an array");
            }
            size_type sz{size()};
            std::vector<std::string> result{};
            for(size_type i{0}; i < sz; i++) {
                result.push_back((*this)[i].as_string());
            }
            return result;
        }

        json &operator[](std::string const &name) {
            become_object();
            return as<o_t>()[name];
        }

        json const &operator[](std::string const &name) const {
            if(!is_object()) {
                throw json_error{"json error: not an object"};
            }
            o_t const &o{as<o_t>()};
            auto it{o.find(name)};
            if(it == o.end()) {
                throw json_error{std::string{"json error: \""} + name + "\" - field not exist"};
            }
            return it->second;
        }

        json &operator[](std::wstring const &name) {
            become_object();
            return as<o_t>()[str_util::to_utf8(name)];
        }

        json const &operator[](std::wstring const &name) const {
            if(!is_object()) {
                throw json_error{"json error: not an object"};
            }
            o_t const &o{as<o_t>()};
            auto it{o.find(str_util::to_utf8(name))};
            if(it == o.end()) {
                throw json_error{std::string{"json error: \""} + str_util::to_utf8(name) + "\" - field not exist"};
            }
            return it->second;
        }

        void traverse_object(std::function<void(std::string const &key, json const &val)> const &func) const {
            if(is_object()) {
                o_t const &o{as<o_t>()};
                for(auto &&p: o) {
                    func(p.first, p.second);
                }
            } else {
                throw json_error{"json error: not an object"};
            }
        }

        void traverse_object(std::function<void(std::string const &key, json &val)> const &func) {
            if(is_object()) {
                o_t &o{as<o_t>()};
                for(auto &&p: o) {
                    func(p.first, p.second);
                }
            } else {
                throw json_error{"json error: not an object"};
            }
        }

        void traverse_array(std::function<void(std::size_t index, json const &val)> const &func) const {
            if(is_array()) {
                a_t const &a{as<a_t>()};
                for(std::size_t i{}; i < a.size(); ++i) {
                    func(i, a[i]);
                }
            } else {
                throw json_error{"json error: not an array"};
            }
        }

        void traverse_array(std::function<void(std::size_t index, json &val)> const &func) {
            if(is_array()) {
                a_t &a{as<a_t>()};
                for(std::size_t i{}; i < a.size(); ++i) {
                    func(i, a[i]);
                }
            } else {
                throw json_error{"json error: not an array"};
            }
        }

        json &operator[](size_type index) {
            if(is_object()) {
                o_t &o{as<o_t>()};
                o_t::iterator oi{o.begin()};
                for(size_type i{0}; i < o.size(); ++i) {
                    if(i == index) {
                        return oi->second;
                    }
                    ++oi;
                }
                throw json_error{"json error: index out of range"};
            }
            become_array();
            a_t &a{as<a_t>()};
            if(index >= a.size()) {
                a.resize(index + 1);
            }
            return a[index];
        }

        json const &operator[](size_type index) const {
            if(is_object()) {
                o_t const &o{as<o_t>()};
                o_t::const_iterator oi{o.cbegin()};
                for(size_type i{0}; i < o.size(); ++i) {
                    if(i == index) {
                        return oi->second;
                    }
                    ++oi;
                }
                throw json_error{"json error: index out of range"};
            } else if(is_array()) {
                a_t const &a{as<a_t>()};
                if(index >= a.size()) {
                    throw json_error{"json error: index out of range"};
                }
                return a.at(index);
            }
            throw json_error{"json error: numeric indexing is for objects and arrays only"};
        }

        void clear() {
            become_null();
        }

        void push_back(json const &v) {
            become_array()[size()] = v;
        }

        void push_back(json &&v) {
            become_array()[size()] = std::move(v);
        }

        void reserve(size_type n) {
            if(is_array()) {
                a_t &arr{as<a_t>()};
                arr.reserve(n);
            }
        }

        void array_sort(std::function<bool(json const &, json const &)> cmp) {
            a_t &arr{as<a_t>()};
            std::sort(arr.begin(), arr.end(), cmp);
        }

        static json null_value() {
            return json{};
        }

        bool key_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{as<o_t>()};
                return o.find(key_name) != o.end();
            } catch(...) {
            }
            return false;
        }

        bool string_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{as<o_t>()};
                return o.find(key_name) != o.end() && o.at(key_name).is_string();
            } catch(...) {
            }
            return false;
        }

        bool num_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) { return false; }
                o_t const &o{as<o_t>()};
                return o.find(key_name) != o.end() && o.at(key_name).is_number();
            } catch(...) {
            }
            return false;
        }

        bool bool_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) {
                    return false;
                }
                o_t const &o{as<o_t>()};
                return o.find(key_name) != o.end() && o.at(key_name).is_bool();
            } catch(...) {
            }
            return false;
        }

        bool float_field_exists(std::string const &key_name) const noexcept {
            try {
                if(!is_object()) {
                    return false;
                }
                o_t const &o{as<o_t>()};
                return o.find(key_name) != o.end() && o.at(key_name).is_real();
            } catch(...) {
            }
            return false;
        }

        std::vector<std::string> key_list() const {
            std::vector<std::string> res{};
            if(!is_object()) {
                return res;
            }
            for(auto &&p: as<o_t>()) {
                res.push_back(p.first);
            }
            return res;
        }

        std::string key(size_type index) const {
            if(!is_object()) {
                throw json_error{"json error: not an object"};
            }
            o_t const &o{as<o_t>()};
            o_t::const_iterator oi{o.begin()};
            for(size_type i{0}; i < o.size(); i++) {
                if(i == index) {
                    return oi->first;
                }
                ++oi;
            }
            throw json_error{"json error: index out of range"};
        }

        void key_remove(size_type index) {
            if(!is_object()) {
                throw json_error{"json error: not an object"};
            }
            std::string name{key(index)};
            o_t &o{as<o_t>()};
            o_t::iterator oi{o.find(name)};
            if(oi != o.end()) {
                o.erase(oi);
            }
        }

        void key_remove(std::string const &name) {
            if(!is_object()) {
                throw json_error{"json error: not an object"};
            }
            o_t &o{as<o_t>()};
            o_t::iterator oi{o.find(name)};
            if(oi != o.end()) {
                o.erase(oi);
            }
        }

        void array_item_remove(size_type index) {
            if(!is_array()) {
                throw json_error{"json error: not an array"};
            }
            a_t &a{as<a_t>()};
            if(index >= a.size()) {
                throw json_error{"json error: index out of range"};
            }
            a.erase(std::next(a.begin(), index));
        }

        void erase(size_type index) {
            if(is_array()) {
                array_item_remove(index);
            } else if(is_object()) {
                key_remove(index);
            } else {
                throw json_error{"json error: invalid conversion"};
            }
        }

        void erase(std::string const &key) {
            if(is_object()) {
                key_remove(key);
            } else {
                throw json_error{"json error: invalid conversion"};
            }
        }

        json &parse(std::string const &s) {
            *this = deserialize(s);
            return *this;
        }

        json &parse(std::string_view s) {
            *this = deserialize(s);
            return *this;
        }

        json &parse(const char *str, std::size_t siz) {
            *this = deserialize(std::string_view{str, siz});
            return *this;
        }

        static json deserialize(scfx::bytevec const &s) {
            return deserialize(std::string{s.begin(), s.end()});
        }

        static json deserialize(std::string_view s) {
            return deserialize(std::string{s});
        }

        static json deserialize(std::string const &s) {
            detail::json_deserializer<json> d{};
            scfx::fsm_tokenizer<std::wstring> t{
                parse_rules_5,
                [&](scfx::fsm_tokenizer<std::wstring>::token const &a) { d.accept_token(a); }
            };
            for(size_t pos{0}; pos < s.size();) {
                int increment{};
                std::int64_t uc{scfx::str_util::utf8_to_ucs(s.data() + pos, &increment)};
                if(uc < 0) {
                    ++pos;
                    uc = '?';
                } else {
                    pos += increment;
                }
                t.accept_char(uc);
            }
            t.finalize();
            return d.harvest_result();
        }

        static json deserialize(std::wstring const &s) {
            detail::json_deserializer<json> d{};
            scfx::fsm_tokenizer<std::wstring> t{
                parse_rules_5,
                [&](scfx::fsm_tokenizer<std::wstring>::token const &a) { d.accept_token(a); }
            };
            t.accept_string(s.data(), s.size());
            t.finalize();
            return d.harvest_result();
        }

        static json deserialize(char const *s) {
            return deserialize(std::string_view{s, std::strlen(s)});
        }

        std::string serialize(int indent_size = -1) const {
            bool use_formatting{indent_size >= 0};
            std::string indent{};
            std::string newline{};
            std::string space{};
            if(indent_size >= 0) {
                for(int i{0}; i < indent_size; ++i) { indent += " "; }
                newline = "\n";
                space = " ";
            }
            return serialize_actual(use_formatting, indent, newline, space, 0);
        }

        std::string serialize5(int indent_size = -1) const {
            bool use_formatting{indent_size >= 0};
            std::string indent{};
            std::string newline{};
            std::string space{};
            if(indent_size >= 0) {
                for(int i{0}; i < indent_size; ++i) { indent += " "; }
                newline = "\n";
                space = " ";
            }
            // return serialize_actual5(use_formatting, indent, newline, space, 0);
            return serialize_actual5_nr(this, use_formatting, indent, newline, space, 0);
        }

        std::vector<std::uint8_t> bserialize() const {
            scfx::serializer ser{};
            bserialize_actual(ser);
            return ser.take_vec();
        }

        static json bdeserialize(std::vector<std::uint8_t> const &vk) {
            scfx::serial_reader sr{vk.data(), vk.size()};
            auto iter{sr.begin()};
            return bdeserialize_actual(iter);
        }

        friend std::ostream &operator<<(std::ostream &os, json const &o) {
            os << o.serialize(4);
            return os;
        }

        json &load_from_file(std::string const &file_name) {
            become_null();
            std::ifstream ifs;
            ifs.open(file_name.c_str());
            if(ifs.good()) {
                ifs.seekg(0, std::ios_base::end);
                std::int64_t fsize{(std::int64_t)ifs.tellg()};
                if(fsize > 0) {
                    ifs.seekg(0);
                    std::vector<std::uint8_t> buff{};
                    buff.resize(fsize);
                    std::int64_t actual_read{0};
                    bool read_failed{false};
                    while(actual_read < fsize) {
                        auto rd_cnt{ifs.readsome((std::ifstream::char_type *)&buff[actual_read], fsize - actual_read)};
                        if(rd_cnt > 0) {
                            actual_read += rd_cnt;
                        } else {
                            read_failed = true;
                            break;
                        }
                    }
                    if(!read_failed) {
                        parse((const char *)buff.data(), buff.size());
                    }
                }
            }
            return *this;
        }

        json const &save_to_file(std::string const &file_name, std::size_t indent_size = 0) const {
            std::ofstream ofs;
            ofs.open(file_name.c_str());
            if(ofs.good()) {
                ofs << serialize(indent_size);
                ofs.close();
            }
            return *this;
        }

        json const &save_to_file5(std::string const &file_name, std::size_t indent_size = 0) const {
            std::ofstream ofs;
            ofs.open(file_name.c_str());
            if(ofs.good()) {
                ofs << serialize5(indent_size);
                ofs.close();
            }
            return *this;
        }

        json &operator+=(json const &r) {
            if(is_array()) {
                if(r.is_array()) {
                    for(size_type i = 0; i < r.size(); ++i) {
                        push_back(r[i]);
                    }
                } else {
                    push_back(r);
                }
            } else if(r.is_array()) {
                json newval{};
                newval.become_array();
                newval.push_back(std::move(*this));
                for(size_type i = 0; i < r.size(); ++i) {
                    newval.push_back(r[i]);
                }
                *this = std::move(newval);
            } else if(is_object() && r.is_object()) {
                for(size_type i = 0; i < r.size(); ++i) {
                    if(key_exists(r.key(i))) {
                        (*this)[r.key(i)] += r[r.key(i)];
                    } else {
                        (*this)[r.key(i)] = r[i];
                    }
                }
            } else {
                json newval{};
                newval.become_array();
                newval.push_back(std::move(*this));
                newval.push_back(r);
                *this = std::move(newval);
            }
            return *this;
        }

        json &operator+=(json &&r) {
            if(is_array()) {
                if(r.is_array()) {
                    for(size_type i = 0; i < r.size(); ++i) {
                        push_back(std::move(r[i]));
                    }
                } else {
                    push_back(std::move(r));
                }
            } else if(r.is_array()) {
                json newval{};
                newval.become_array();
                newval.push_back(std::move(*this));
                for(size_type i = 0; i < r.size(); ++i) {
                    newval.push_back(std::move(r[i]));
                }
                *this = std::move(newval);
            } else if(is_object() && r.is_object()) {
                for(size_type i = 0; i < r.size(); ++i) {
                    if(key_exists(r.key(i))) {
                        (*this)[r.key(i)] += std::move(r[r.key(i)]);
                    } else {
                        (*this)[r.key(i)] = std::move(r[i]);
                    }
                }
            } else {
                json newval{};
                newval.become_array();
                newval.push_back(std::move(*this));
                newval.push_back(std::move(r));
                *this = std::move(newval);
            }
            return *this;
        }

        friend json operator+(json const &l, json const &r) {
            return json{l} += r;
        }

        friend json operator+(json &&l, json const &r) {
            return json{std::move(l)} += r;
        }

        friend json operator+(json const &l, json &&r) {
            return json{l} += std::move(r);
        }

        friend json operator+(json &&l, json &&r) {
            return json{std::move(l)} += std::move(r);
        }

    private:
        std::int64_t max_array_index() const {
            return ((std::int64_t)as<a_t>().size()) - 1;
        }

        std::string serialize_actual(bool use_formatting, std::string indent, std::string newline, std::string space, int level) const {
            std::string prefix{};
            if(use_formatting) {
                for(int l{0}; l < level; ++l) { prefix += indent; }
            }
            if(t_ == jo_null) {
                return "null";
            } else if(t_ == jo_int) {
                return str_util::itoa<std::string>(as<std::int64_t>());
            } else if(t_ == jo_flt) {
                return str_util::ftoa(as<long double>(), std::numeric_limits<long double>::digits10);
            } else if(t_ == jo_bool) {
                return as<bool>() ? "true" : "false";
            } else if(t_ == jo_string) {
                return std::string{"\""} + detail::escape(as<std::string>()) + std::string{"\""};
            } else if(t_ == jo_object) {
                std::string res{std::string{"{"} + (as<o_t>().size() ? newline : "")};
                std::string comma{","};
                std::size_t i{0};
                for(auto &&p: as<o_t const &>()) {
                    if(++i >= as<o_t const &>().size()) { comma = ""; }
                    res += prefix + indent + std::string{"\""} + p.first + "\"";
                    res += std::string() + ":" + space;
                    res += p.second.serialize_actual(use_formatting, indent, newline, space, level + 1);
                    res += comma + (i < as<o_t>().size() ? newline : "");
                }
                res += (i > 0 ? newline + prefix : "") + "}";
                return res;
            } else if(t_ == jo_array) {
                a_t const &a{as<a_t>()};
                std::string res{std::string{"["} + (a.size() ? newline : "")};
                std::string comma{","};
                std::int64_t max_ind{max_array_index()};
                std::int64_t i{0};
                for(; i <= max_ind; ++i) {
                    if(i >= max_ind) { comma = ""; }
                    res += prefix + indent;
                    res += a.at(i).serialize_actual(use_formatting, indent, newline, space, level + 1);
                    res += comma + (i - 1 < max_ind ? newline : "");
                }
                res += (i > 0 ? prefix : "") + "]";
                return res;
            } else {
                throw json_error{"json error: invalid json"};
            }
        }

        std::string serialize_actual5(bool use_formatting, std::string indent, std::string newline, std::string space, int level) const {
            std::string prefix{};
            if(use_formatting) {
                for(int l{0}; l < level; ++l) { prefix += indent; }
            }
            if(t_ == jo_null) {
                return "null";
            } else if(t_ == jo_int) {
                return str_util::itoa<std::string>(as<std::int64_t>());
            } else if(t_ == jo_flt) {
                return str_util::ftoa(as<long double>(), std::numeric_limits<long double>::digits10);
            } else if(t_ == jo_bool) {
                return as<bool>() ? "true" : "false";
            } else if(t_ == jo_string) {
                return std::string{"\""} + detail::escape(as<std::string>()) + std::string{"\""};
            } else if(t_ == jo_object) {
                o_t const &o{as<o_t>()};
                std::string res{std::string{"{"} + (as<o_t>().size() ? newline : "")};
                std::string comma{","};
                std::size_t i{0};
                for(auto &&p: o) {
                    ++i;
                    if(!detail::is_ident(p.first)) {
                        res += prefix + indent + std::string{"\""} + p.first + "\"";
                    } else {
                        res += prefix + indent + p.first;
                    }
                    res += std::string() + ":" + space;
                    res += p.second.serialize_actual5(use_formatting, indent, newline, space, level + 1);
                    res += comma + (i < as<o_t>().size() ? newline : "");
                }
                res += (i > 0 ? newline + prefix : "") + "}";
                return res;
            } else if(t_ == jo_array) {
                a_t const &a{as<a_t>()};
                std::string res{std::string{"["} + (a.size() ? newline : "")};
                std::string comma{","};
                std::int64_t max_ind{max_array_index()};
                std::int64_t i{0};
                for(; i <= max_ind; ++i) {
                    res += prefix + indent;
                    res += a.at(i).serialize_actual5(use_formatting, indent, newline, space, level + 1);
                    res += comma + (i - 1 < max_ind ? newline : "");
                }
                res += (i > 0 ? prefix : "") + "]";
                return res;
            } else {
                throw json_error{"json error: invalid json"};
            }
        }

        static std::string serialize_actual5_nr(json const *this_, bool use_formatting, std::string indent, std::string newline, std::string space, int level) {
            auto prefix{[use_formatting, indent](int lvl) {
                std::string prfx{};
                if(use_formatting) {
                    for(int l{0}; l < lvl; ++l) { prfx += indent; }
                }
                return prfx;
            }};
            struct frame {
                json const *this_{nullptr};
                std::stringstream result_{};
                int curr_level_{0};
                size_t i_{0};
                size_t phase_{0};
                std::size_t maxind_{0};
            };
            frame stack_res{};
            std::deque<frame> stack{};
            stack.emplace_back(this_, std::stringstream{}, level, 0, 0, 0);
            while(!stack.empty()) {
                if(stack.back().this_->t_ == jo_null) {
                    stack.back().result_ << "null";
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                } else if(stack.back().this_->t_ == jo_int) {
                    stack.back().result_ << str_util::itoa<std::string>(stack.back().this_->as<std::int64_t>());
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                } else if(stack.back().this_->t_ == jo_flt) {
                    stack.back().result_ << str_util::ftoa(stack.back().this_->as<long double>(), std::numeric_limits<long double>::digits10);
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                } else if(stack.back().this_->t_ == jo_bool) {
                    stack.back().result_ << (stack.back().this_->as<bool>() ? "true" : "false");
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                } else if(stack.back().this_->t_ == jo_string) {
                    stack.back().result_ << std::string{"\""} + detail::escape(stack.back().this_->as<std::string>()) + std::string{"\""};
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                } else if(stack.back().this_->t_ == jo_object) {
                    if(stack.back().phase_ == 0) {
                        stack.back().result_ << std::string{"{"} + (stack.back().this_->as<o_t>().size() ? newline : "");
                        stack.back().i_ = 0;
                        stack.back().phase_ = 1;
                        continue;
                    } else if(stack.back().phase_ == 1) {
                        if(stack.back().i_ < stack.back().this_->as<o_t>().size()) {
                            auto p{stack.back().this_->as<o_t>().begin()};
                            for(size_t i{}; i < stack.back().i_; ++i) { ++p; }
                            ++stack.back().i_;

                            if(!detail::is_ident(p->first)) {
                                stack.back().result_ << prefix(stack.back().curr_level_) + indent + std::string{"\""} + p->first + "\"";
                            } else {
                                stack.back().result_ << prefix(stack.back().curr_level_) + indent + p->first;
                            }
                            stack.back().result_ << std::string{} + ":" + space;
                            stack.back().phase_ = 2;
                            stack.emplace_back(&p->second, std::stringstream{}, stack.back().curr_level_ + 1, 0, 0, 0);
                            continue;
                        } else {
                            stack.back().result_ << (stack.back().i_ > 0 ? newline + prefix(stack.back().curr_level_) : "") + "}";
                            stack_res = std::move(stack.back());
                            stack.pop_back();
                        }
                    } else if(stack.back().phase_ == 2) {
                        stack.back().result_ << stack_res.result_.str();
                        stack.back().result_ << "," + (stack.back().i_ < stack.back().this_->as<o_t>().size() ? newline : "");
                        stack.back().phase_ = 1;
                        continue;
                    }
                } else if(stack.back().this_->t_ == jo_array) {
                    if(stack.back().phase_ == 0) {
                        stack.back().result_ << std::string{"["} + (stack.back().this_->as<a_t>().size() ? newline : "");
                        stack.back().maxind_ = stack.back().this_->max_array_index();
                        stack.back().i_ = 0;
                        stack.back().phase_ = 1;
                        continue;
                    } else if(stack.back().phase_ == 1) {
                        if(stack.back().i_ <= stack.back().maxind_) {
                            stack.back().result_ << prefix(stack.back().curr_level_) + indent;
                            stack.back().phase_ = 2;
                            stack.emplace_back(&stack.back().this_->as<a_t>().at(stack.back().i_), std::stringstream{}, stack.back().curr_level_ + 1, 0, 0, 0);
                            continue;
                        } else {
                            stack.back().result_ << (stack.back().i_ > 0 ? newline + prefix(stack.back().curr_level_) : "") + "]";
                            stack_res = std::move(stack.back());
                            stack.pop_back();
                        }
                    } else if(stack.back().phase_ == 2) {
                        ++stack.back().i_;
                        stack.back().result_ << stack_res.result_.str();
                        stack.back().result_ << "," + (stack.back().i_ < stack.back().this_->as<a_t>().size() ? newline : "");
                        stack.back().phase_ = 1;
                        continue;
                    }
                } else {
                    throw json_error{"json error: invalid json"};
                }
            }
            return stack_res.result_.str();
        }

        void bserialize_actual(scfx::serializer &ser) const {
            if(!type_valid()) {
                throw json_error{"json error: invalid state"};
            }
            ser << (std::uint8_t)t_;
            if(t_ == jo_null) {
            } else if(t_ == jo_int) {
                ser << str_util::itoa<std::string>(as<std::int64_t>());
            } else if(t_ == jo_flt) {
                ser << str_util::ftoa(as<long double>(), std::numeric_limits<long double>::digits10);
            } else if(t_ == jo_bool) {
                ser << (as<bool>() ? (char)1 : (char)0);
            } else if(t_ == jo_string) {
                ser << as<std::string>();
            } else if(t_ == jo_object) {
                ser << str_util::utoa<std::string>(size());
                for(auto &&p: as<o_t const &>()) {
                    ser << p.first;
                    p.second.bserialize_actual(ser);
                }
            } else if(t_ == jo_array) {
                a_t const &a{as<a_t>()};
                ser << str_util::utoa<std::string>(size());
                std::int64_t max_ind{max_array_index()};
                for(std::int64_t i{0}; i <= max_ind; ++i) {
                    a.at(i).bserialize_actual(ser);
                }
            }
        }

        static json bdeserialize_actual(scfx::serial_reader::const_iterator &iter) {
            json res{};
            type t{(type)iter->as_unumber()};
            if(!type_valid(t)) {
                throw json_error{"json error: invalid type"};
            }
            ++iter;
            if(t == jo_null) {
            } else if(t == jo_int) {
                res = str_util::atoi(iter->as_string());
                ++iter;
            } else if(t == jo_flt) {
                res = str_util::atof(iter->as_string());
                ++iter;
            } else if(t == jo_bool) {
                res = (iter->as_unumber() ? true : false);
                ++iter;
            } else if(t == jo_string) {
                res = iter->as_string();
                ++iter;
            } else if(t == jo_object) {
                res.become_object();
                size_type obj_size{str_util::atoui(iter->as_string())};
                ++iter;
                for(size_type i{0}; i < obj_size; ++i) {
                    std::string k{iter->as_string()};
                    ++iter;
                    res[k] = bdeserialize_actual(iter);
                }
            } else if(t == jo_array) {
                res.become_array();
                size_type arr_size{str_util::atoui(iter->as_string())};
                ++iter;
                res.reserve(arr_size);
                for(size_type i{0}; i < arr_size; ++i) {
                    res.push_back(bdeserialize_actual(iter));
                }
            } else {
                throw json_error{"json error: invalid json"};
            }
            return res;
        }

    private:
        void cleanup() {
            struct frame {
                json *j_{nullptr};
                bool cleaned_{false};
            };
            frame stack_res{};
            std::deque<frame> stack{};
            stack.emplace_back(this);
            while(!stack.empty()) {
                if(stack.back().j_->is_object()) {
                    o_t &o{stack.back().j_->as<o_t>()};
                    if(!o.empty()) {
                        auto it{o.begin()};
                        if(stack_res.cleaned_) {
                            stack_res.cleaned_ = false;
                            o.erase(it);
                        } else {
                            if(it->second.is_object() || it->second.is_array()) {
                                stack.emplace_back(&it->second);
                            } else {
                                o.erase(it);
                            }
                        }
                    } else {
                        stack.back().cleaned_ = true;
                        stack_res = std::move(stack.back());
                        stack.pop_back();
                    }
                } else if(stack.back().j_->is_array()) {
                    a_t &a{stack.back().j_->as<a_t>()};
                    if(!a.empty()) {
                        auto it{a.begin()};
                        if(stack_res.cleaned_) {
                            stack_res.cleaned_ = false;
                            a.erase(it);
                        } else {
                            if(it->is_object() || it->is_array()) {
                                stack.emplace_back(&(*it));
                            } else {
                                a.erase(it);
                            }
                        }
                    } else {
                        stack.back().cleaned_ = true;
                        stack_res = std::move(stack.back());
                        stack.pop_back();
                    }
                } else {
                    stack.back().cleaned_ = true;
                    stack_res = std::move(stack.back());
                    stack.pop_back();
                }
            }
        }

    private:
        template<typename T> const T &as() const & {
            return any_cast<T const &>(v_);
        }

        template<typename T> T &as() & {
            return any_cast<T &>(v_);
        }

        enum type {
            jo_null,
            jo_object,
            jo_array,
            jo_string,
            jo_int,
            jo_flt,
            jo_bool
        };

        any v_{};
        type t_{jo_null};

        static inline std::wstring const parse_rules_5{
LR"(#rd|'\"'>>str;str|'\"'>><str|'\\'>>stre|$df>>str;stre|$df>>str;
#rd|'\''>>sts;sts|$df>>sts|'\''>><sts|'\\'>>stse;stse|$df>>sts;

#rd|[$_:alpha:]>>id;#rd|'\\'>>id_ue;id_ue|'u'>>id_u;id_u|[:hex:]>>id_u1;id_u1|[:hex:]>>id_u2;id_u2|[:hex:]>>id_u3;
id_u3|[:hex:]>>id;id|'\\'>>id_ue|[$_:alnum:]>>id|$df>>^idr;id_ue|'u'>>id_u;id_u|[:hex:]>>id_u1;id_u1|[:hex:]>>id_u2;
id_u2|[:hex:]>>id_u3;id_u3|[:hex:]>>id;

#rd|'/'>>cst;cst|'*'>>mlc|'/'>>slc;mlc|'*'>>mlcs|$df>>mlc;slc|[\r\n]>>^cmt|$df>>slc|$ef>>^cmt;
mlcs|'*'>>mlcs|'/'>><cmt|$df>>mlc;

#rd|' '>>spc|'\t'>>spc|'\r'>>spc|'\n'>>spc|'\f'>>spc|'\v'>>spc|[:space:]>>spc|[:cntrl:]>>spc;
spc|' '>>spc|'\t'>>spc|'\r'>>spc|'\n'>>spc|'\f'>>spc|'\v'>>spc|[:space:]>>spc|[:cntrl:]>>spc|$df>>^spc;

#rd|[+-]>>^sgn;

#rd|[123456789]>>int;int|[:digit:]>>int|'.'>>fpd|[:punct::space::cntrl:]>>^int|[eE]>>fpe1|$ef>>^int;
#rd|'0'>>mbhex;mbhex|[xX]>>hex|'.'>>fpd|[eE]>>fpe1|[:digit:]>>int|$df>>^int|$ef>>^int;
hex|[:hex:]>>hex|$df>>^hex|$ef>>^hex;#rd|'.'>>mbfp;mbfp|[:digit:]>>fp;fpd|[:digit:]>>fp|[eE]>>fpe1|$df>>^fpl;
fp|[:punct::space::cntrl:]>>^fpl|[:digit:]>>fp|[eE]>>fpe1|$ef>>^fpl;fpe1|[-+]>>fpes|[:digit:]>>fpe2;
fpes|[:digit:]>>fpe2|'.'>>fpesd;fpesd|[:digit:]>>fpesd|[:punct::space::cntrl:]>>^fpl|$ef>>^fpl;
fpe2|[:digit:]>>fpe2|'.'>>fpe2d|[:punct::space:]>>^fpl|$ef>>^fpl;
fpe2d|[:digit:]>>fpe2d|[:punct::space::cntrl:]>>^fpl|$ef>>^fpl;

#rd|'I'>>I;I|'n'>>In|[$_:alnum:]>>id|$ef>>^id;In|'f'>>Inf|[$_:alnum:]>>id|$ef>>^id;Inf|'i'>>Infi|[$_:alnum:]>>id|$ef>>^id;
Infi|'n'>>Infin|[$_:alnum:]>>id|$ef>>^id;Infin|'i'>>Infini|[$_:alnum:]>>id|$ef>>^id;Infini|'t'>>Infinit|[$_:alnum:]>>id|$ef>>^id;
Infinit|'y'>>Infinity|[$_:alnum:]>>id|$ef>>^id;Infinity|[$_:alnum:]>>id|$df>>^inf|$ef>>^inf;

#rd|'N'>>N;N|'a'>>Na|[$_:alnum:]>>id|$ef>>^id;Na|'N'>>NaN|[$_:alnum:]>>id|$ef>>^id;NaN|$df>>^nan|[$_:alnum:]>>id|$ef>>^nan;

#rd|'n'>>n;n|'u'>>nu|[$_:alnum:]>>id|$ef>>^id;nu|'l'>>nul|[$_:alnum:]>>id|$ef>>^id;nul|'l'>>null|[$_:alnum:]>>id|$ef>>^id;
null|$df>>^nul|[$_:alnum:]>>id|$ef>>^nul;

#rd|'t'>>t;t|'r'>>tr|[$_:alnum:]>>id|$ef>>^id;tr|'u'>>tru|[$_:alnum:]>>id|$ef>>^id;tru|'e'>>true|[$_:alnum:]>>id|$ef>>^id;
true|$df>>^tru|[$_:alnum:]>>id|$ef>>^tru;

#rd|'f'>>f;f|'a'>>fa|[$_:alnum:]>>id|$ef>>^id;fa|'l'>>fal|[$_:alnum:]>>id|$ef>>^id;fal|'s'>>fals|[$_:alnum:]>>id|$ef>>^id;
fals|'e'>>false|[$_:alnum:]>>id|$ef>>^id;false|$df>>^fal|[$_:alnum:]>>id|$ef>>^fal;

#rd|','>><,;#rd|':'>><:;#rd|'['>><[;#rd|']'>><];#rd|'{'>><{;#rd|'}'>><};)"
        };

        bool type_valid() const {
            return
                static_cast<int>(t_) >= static_cast<int>(jo_null)
                &&
                static_cast<int>(t_) <= static_cast<int>(jo_bool);
        }

        static bool type_valid(type t) {
            return
                static_cast<int>(t) >= static_cast<int>(jo_null)
                &&
                static_cast<int>(t) <= static_cast<int>(jo_bool);
        }
    };

}
