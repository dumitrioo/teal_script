#pragma once

#include "../commondefs.hpp"
#include "../str_util.hpp"
#include "../json.hpp"

namespace teal {

    class url final {
    public:
        url() = default;

        url(std::string const &s) {
            parse(s);
        }

        url(std::string_view s) {
            parse(std::string{s});
        }

        url(char const * s) {
            parse(std::string{s});
        }

        void parse(const std::string &url_in) {
            clear();
            std::string urll{decode(url_in)};
            // scheme
            auto pos = urll.find(':');
            if(pos == std::string::npos) return;
            std::string scheme = str_util::fltr<std::string>::strtolower(urll.substr(0, pos));
            if(scheme.empty()) return;
            scheme_ = scheme;
            // "//" authority?
            size_t i = pos+1;
            if(urll.size() > i + 1 && urll[i] == '/' && urll[i+1] == '/') {
                i += 2;
                // authority is up to next '/' or '?' or '#'
                size_t auth_end = urll.find_first_of("/?#", i);
                std::string authority = (auth_end==std::string::npos) ? urll.substr(i) : urll.substr(i, auth_end - i);
                // split userinfo@host:port
                size_t at = authority.find('@');
                std::string hostport = authority;
                if(at != std::string::npos) {
                    userinfo_ = authority.substr(0, at);
                    hostport = authority.substr(at+1);
                }
                // IPv6 literal [::1]
                if(!hostport.empty() && hostport.front() == '[') {
                    size_t rb = hostport.find(']');
                    if(rb==std::string::npos) return;
                    host_ = hostport.substr(0, rb+1); // include brackets
                    if(rb + 1 < hostport.size() && hostport[rb+1] == ':') {
                        std::string portstr = hostport.substr(rb+2);
                        try{
                            port_ = str_util::atoui(portstr);
                        } catch(...) {
                        }
                    }
                } else {
                    // split by last ':' to separate port
                    size_t colon = hostport.rfind(':');
                    if(colon != std::string::npos && hostport.find(':') == colon) { // only one ':'
                        host_ = hostport.substr(0, colon);
                        std::string portstr = hostport.substr(colon+1);
                        try {
                            port_ = str_util::atoui(portstr);
                        } catch(...) { host_ = hostport; }
                    } else {
                        host_ = hostport;
                    }
                }
                i = (auth_end == std::string::npos) ? urll.size() : auth_end;
            }
            // path
            size_t path_start = i;
            size_t qpos = urll.find('?', path_start);
            size_t hpos = urll.find('#', path_start);
            size_t path_end = std::min(qpos == std::string::npos ? urll.size() : qpos, hpos == std::string::npos ? urll.size() : hpos);
            if(path_end > path_start) {
                path_ = urll.substr(path_start, path_end - path_start);

            } else {
                path_.clear();
            }
            // query
            if(qpos!=std::string::npos) {
                size_t qend = (hpos == std::string::npos) ? urll.size() : hpos;
                if(qend > qpos + 1) {
                    query_ = urll.substr(qpos + 1, qend - qpos - 1);
                } else {
                    query_ = std::string{};
                }
            }
            // fragment
            if(hpos != std::string::npos) {
                if(hpos + 1 < urll.size()) {
                    fragment_ = urll.substr(hpos+1);
                } else {
                    fragment_ = std::string{};
                }
            }
            valid_ = true;
        }

        void clear() {
            scheme_.clear();
            userinfo_.clear();
            host_.clear();
            port_ = {};
            path_.clear();
            query_.clear();
            fragment_.clear();
            valid_ = false;
        }

        std::string scheme() const { return scheme_; }
        std::pair<std::string, std::string> userinfo() const {
            if(!userinfo_.empty()) {
                auto creds{str_util::str_tok<std::string>(userinfo_, ":")};
                if(creds.size() == 2) {
                    return std::pair<std::string, std::string>{creds[0], creds[1]};
                }
                return std::pair<std::string, std::string>{creds[0], {}};
            }
            return {};
        }
        std::string username() const { return userinfo().first; }
        std::string userpasswd() const { return userinfo().second; }
        std::string host() const { return host_; }
        std::optional<int> port() const { return port_; }
        std::string path() const { return path_; }

        std::map<std::string, std::string> query_map() const {
            std::map<std::string, std::string> res{};
            if(valid_ && !query_.empty()) {
                auto qtr{str_util::str_tok<std::string>(query_, "&")};
                for(auto &&q: qtr) {
                    auto qkv{str_util::str_tok<std::string>(q, "=")};
                    if(qkv.size() == 2) {
                        res[str_util::trim(qkv[0])] = str_util::trim(qkv[1]);
                    }
                }
            }
            return res;
        }

        json query_json() const {
            json res{};
            res.become_object();
            std::map<std::string, std::string> m{query_map()};
            for(auto &&p: m) {
                res[p.first] = p.second;
            }
            return res;
        }

        std::optional<std::string> fragment() const {
            return fragment_;
        }

        bool valid() const {
            return valid_;
        }

        static std::string encode(const std::string &s) {
            std::ostringstream out{};
            out << std::hex << std::uppercase;
            for(unsigned char c: s) {
                if(is_unreserved((char)c)) {
                    out << c;
                } else {
                    out << '%' << std::setw(2) << std::setfill('0') << (int)c;
                }
            }
            return out.str();
        }

        static std::string decode(const std::string &s) {
            std::string out{};
            out.reserve(s.size());
            for(size_t i = 0; i < s.size(); ++i) {
                char c = s[i];
                if(c == '%') {
                    if(i + 2 >= s.size()) throw std::invalid_argument("Incomplete percent-encoding");
                    int hi = hex_value(s[i+1]);
                    int lo = hex_value(s[i+2]);
                    if(hi == -1 || lo == -1) throw std::invalid_argument("Invalid hex digits in percent-encoding");
                    unsigned char byte = (unsigned char)((hi << 4) | lo);
                    out.push_back((char)byte);
                    i += 2;
                } else {
                    out.push_back(c);
                }
            }
            return out;
        }

    private:
        // "unreserved" RFC 3986: ALPHA / DIGIT / "-" / "." / "_" / "~"
        static inline bool is_unreserved(char c) {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '-' || c == '.' || c == '_' || c == '~';
        }

        static inline int hex_value(char c) {
            static std::array<int, 256> constexpr hex_do_digit{
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
            };
            return hex_do_digit[static_cast<std::uint8_t>(c)];
        }

    private:
        std::string scheme_{};
        std::string userinfo_{};
        std::string host_{};
        std::optional<int> port_{};
        std::string path_{};
        std::string query_{};
        std::string fragment_{};
        bool valid_{false};
    };

}
