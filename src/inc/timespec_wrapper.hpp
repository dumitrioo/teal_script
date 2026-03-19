#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#include "containers/static_buffer.hpp"
#include <cstdint>
#include <ctime>

namespace scfx {

    class timespec_wrapper final {
    public:
        timespec_wrapper() = default;

        timespec_wrapper(const timespec_wrapper &that) noexcept = default;

        timespec_wrapper &operator=(const timespec_wrapper &that) noexcept = default;

        timespec_wrapper(timespec_wrapper &&that) noexcept = default;

        timespec_wrapper &operator=(timespec_wrapper &&that) noexcept = default;

        ~timespec_wrapper() noexcept = default;

        constexpr timespec_wrapper(time_t sec, long int nsec) noexcept: t_{sec, nsec} {
        }

        constexpr timespec_wrapper(const struct timespec &that) noexcept: t_(that) {
        }

        explicit timespec_wrapper(std::string const &mysql_date) noexcept {
            from_iso_8601(mysql_date);
        }

        explicit timespec_wrapper(char const *mysql_date) {
            from_iso_8601(std::string{mysql_date});
        }

        explicit timespec_wrapper(std::wstring const &mysql_date) noexcept {
            from_iso_8601(mysql_date);
        }

        explicit timespec_wrapper(wchar_t const *mysql_date) {
            from_iso_8601(std::wstring{mysql_date});
        }

        explicit timespec_wrapper(std::int64_t msec) noexcept {
            t_.tv_sec = msec / 1'000LL;
            t_.tv_nsec = (msec % 1'000LL) * 1'000'000LL;
        }

        explicit timespec_wrapper(long double flt_val) noexcept {
            t_.tv_sec = flt_val;
            t_.tv_nsec = (flt_val - t_.tv_sec) * 1'000'000'000LL;
        }

        explicit timespec_wrapper(double flt_val) noexcept {
            t_.tv_sec = flt_val;
            t_.tv_nsec = (flt_val - t_.tv_sec) * 1'000'000'000LL;
        }

        explicit timespec_wrapper(float flt_val) noexcept {
            t_.tv_sec = flt_val;
            t_.tv_nsec = (flt_val - t_.tv_sec) * 1'000'000'000LL;
        }

        std::int64_t seconds() const noexcept {
            return t_.tv_sec;
        }

        std::int64_t milliseconds() const noexcept {
            return static_cast<std::int64_t>(t_.tv_sec) * 1'000LL +
                   static_cast<std::int64_t>(t_.tv_nsec) / 1'000'000LL;
        }

        std::int64_t useconds() const noexcept {
            return static_cast<std::int64_t>(t_.tv_sec) * 1'000'000LL +
                   ((std::int64_t)t_.tv_nsec) / 1'000LL;
        }

        std::int64_t nseconds() const noexcept {
            return static_cast<std::int64_t>(t_.tv_sec) * 1'000'000'000LL +
                   (static_cast<std::int64_t>(t_.tv_nsec));
        }

        long double fseconds() const noexcept {
            return static_cast<long double>(t_.tv_sec) + static_cast<long double>(t_.tv_nsec) / 1e9L;
        }

        long double fsubseconds() const noexcept {
            return static_cast<long double>(t_.tv_nsec) / 1e9L;
        }

        static inline const std::array<std::string, 7> weekdays_shr2 = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
        static inline const std::array<std::string, 7> weekdays_shrt = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        static inline const std::array<std::string, 7> weekdays_full = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        static inline const std::array<std::string, 12> month_full   = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        static inline const std::array<std::string, 12> month_shrt   = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        static inline const std::array<std::string, 12> month_shrt2  = {"Ja", "Fe", "Mr", "Ap", "My", "Jn", "Jl", "Au", "Se", "Oc", "Nv", "De"};

        std::string as_rfc_1123_str(bool gmt = true) const {
            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << weekdays_shrt[tmp->tm_wday] << ", "
               << std::setfill('0') << std::setw(2) << tmp->tm_mday << ' '
               << std::setfill('0') << std::setw(2) << month_shrt[tmp->tm_mon] << ' '
               << tmp->tm_year + 1900 << ' '
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec) << ' ';
            if(gmt) { ss << "GMT"; }

            return ss.str();
        }

        std::string as_rfc_1036_str(bool gmt = true) const {
            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << weekdays_full[tmp->tm_wday] << ", "
               << std::setfill('0') << std::setw(2) << tmp->tm_mday << '-'
               << std::setfill('0') << std::setw(2) << month_shrt[tmp->tm_mon] << '-'
               << (tmp->tm_year % 100) << ' '
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec) << ' ';
            if(gmt) { ss << "GMT"; }

            return ss.str();
        }

        std::string as_iso_8601_str(std::size_t prec = 9) const {
            std::stringstream ss{};
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
               << 'T'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if(prec > 0) {
                if(prec > 9) { prec = 9; }
                ss << subseconds_to_str(fsubseconds(), prec);
            }

            std::int64_t time_offs{date_gmtoffset(*this).seconds()};
            std::int64_t hours_offs{time_offs / 3600};
            std::int64_t minutes_offs{(time_offs % 3600) / 60};
            if(hours_offs== 0) {
                ss << 'Z';
            } else {
                ss << (hours_offs < 0 ? "-" : "+")
                << std::setfill('0') << std::setw(2) << std::abs(hours_offs) << ":"
                << std::setfill('0') << std::setw(2) << std::abs(minutes_offs);
            }

            return ss.str();
        }

        std::string as_gmt_iso_8601_str(std::size_t prec = 9) const {
            long double fsecs{fseconds()};
            std::int64_t secs{static_cast<std::int64_t>(fsecs)};
            long double subseconds{fsecs - secs};

            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
               << 'T'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if(prec > 0) {
                if(prec > 9) { prec = 9; }
                ss << subseconds_to_str(subseconds, prec);
            }

            ss << 'Z';

            return ss.str();
        }

        int year() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_year + 1900; }
        int month() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_mon + 1; }
        int day() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_mday; }
        int hour() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_hour; }
        int min() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_min; }
        int sec() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_sec; }
        long double sec_with_subsec() const { return (long double )sec() + std::fmod(fseconds(), static_cast<long double>(1)); }

        int weekday() const { struct tm *tmp{::localtime(&(t_.tv_sec))}; return tmp->tm_wday; }

        std::string as_any_tz_iso_8601_str(std::size_t prec = 9, long double hours_offset = 0) const {
            if(std::abs(hours_offset) > 12.0L) { return {}; }

            timespec_wrapper dst_tsw{*this};
            dst_tsw.to_gmt();
            dst_tsw += timespec_wrapper{hours_offset * 3600.0L};

            long double fsecs{fseconds()};
            std::int64_t secs{static_cast<std::int64_t>(fsecs)};
            long double subseconds{fsecs - secs};

            std::stringstream ss;
            struct tm *tmp{::localtime(&(dst_tsw.t_.tv_sec))};
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
               << 'T'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if(prec > 0) {
                if(prec > 9) { prec = 9; }
                ss << subseconds_to_str(subseconds, prec);
            }

            std::int64_t seconds_offs{static_cast<std::int64_t>(hours_offset * 3600.0L)};
            std::int64_t hours_offs{seconds_offs / 3600};
            std::int64_t minutes_offs{(seconds_offs % 3600) / 60};
            if(seconds_offs == 0) {
                ss << 'Z';
            } else {
                ss << (hours_offs < 0 ? "-" : "+")
                << std::setfill('0') << std::setw(2) << std::abs(hours_offs) << ":"
                << std::setfill('0') << std::setw(2) << std::abs(minutes_offs);
            }

            return ss.str();
        }

        timespec_wrapper &from_iso_8601(std::string const &dts) {
            parse_iso_8601<char>((char const *)dts.data(), dts.size());
            return *this;
        }

        timespec_wrapper &from_iso_8601(std::wstring const &dts) {
            parse_iso_8601<std::wstring::value_type>((std::wstring::value_type const *)dts.data(), dts.size());
            return *this;
        }

        std::string asctime() const {
            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            return ::asctime(tmp);
        }

        timespec_wrapper &from_mysql_datetime_str(std::string const &dts) {
            try {
                if(dts.size() > 0) {
                    enum states{ye, mo, da, ho, mi, se, sf, fi};
                    std::array<std::int64_t, (std::size_t)states::fi> brdn{};
                    int sec_frac_nums{0};
                    states state{ye};
                    std::string buf{};
                    bool sign_set{false};
                    bool neg{false};
                    for(std::size_t cno{0}; cno < dts.size(); ++cno) {
                        if(state >= 0 && state < fi) {
                            char currc{dts[cno]};
                            if(!std::isdigit(currc) /*std::ispunct(currc) || std::isblank(currc)*/) {
                                if(state == ye && !sign_set && (currc == '-' || currc == '+')) {
                                    neg = currc == '-';
                                } else {
                                    if(!buf.empty()) {
                                        brdn[state] = scfx::str_util::atoi(std::string{buf});
                                        buf.clear();
                                        state = (states)((int)state + 1);
                                    }
                                }
                            } else {
                                if(std::isdigit(currc)) {
                                    buf += currc;
                                    if(state == sf) { ++sec_frac_nums; }
                                    if(state == ye) { sign_set = true; }
                                }
                                if(cno + 1 == dts.size() && !buf.empty()) {
                                    std::int64_t bval{scfx::str_util::atoi(std::string{buf})};
                                    brdn[state] = bval;
                                }
                            }
                        } else {
                            break;
                        }
                    }
                    struct tm tm;
                    tm.tm_sec = brdn[se];
                    tm.tm_min = brdn[mi];
                    tm.tm_hour = brdn[ho];
                    tm.tm_mday = brdn[da];
                    tm.tm_mon = brdn[mo] - 1;
                    tm.tm_year = (neg ? -brdn[ye] : brdn[ye]) - 1900;
                    tm.tm_isdst = -1;
                    long double res{static_cast<long double>(std::mktime(&tm))};
                    if(brdn[sf] > 0) {
                        long double fssdiv{1.0L};
                        for(int i{}; i < sec_frac_nums; ++i) { fssdiv *= 10.0L; }
                        long double fss{(long double)brdn[sf] / fssdiv};
                        res += res < 0 ? -(1 - fss) : fss;
                    }
                    t_.tv_sec = res;
                    t_.tv_nsec = (std::abs(res) - std::abs(t_.tv_sec)) * 1'000'000'000LL;
                } else {
                    t_.tv_sec = 0LL;
                    t_.tv_nsec = 0;
                }
            } catch (...) {
                t_.tv_sec = 0LL;
                t_.tv_nsec = 0;
            }
            return *this;
        }

        timespec_wrapper &from_gmt_mysql_datetime_str(std::string const &dts) {
            from_mysql_datetime_str(dts);
            to_local();
            return *this;
        }

        std::string as_mysql_datetime_str(std::size_t prec = 9) const {
            long double fsecs{fseconds()};
            std::int64_t secs{static_cast<std::int64_t>(fsecs)};
            long double subseconds{fsecs - secs};

            std::stringstream ss;
            struct tm *tmp{::localtime(&(t_.tv_sec))};
            ss << tmp->tm_year + 1900 << '-' << std::setfill('0')
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mon + 1) << '-'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_mday)
               << ' '
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_hour) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_min) << ':'
               << std::setfill('0') << std::setw(2) << static_cast<int>(tmp->tm_sec);

            if(prec > 0) {
                if(prec > 9) { prec = 9; }
                ss << subseconds_to_str(subseconds, prec);
            }
            return ss.str();
        }

        timespec_wrapper &to_local() {
            *this += date_gmtoffset(*this);
            return *this;
        }

        timespec_wrapper &to_gmt() {
            *this -= date_gmtoffset(*this);
            return *this;
        }

        static timespec_wrapper now() noexcept {
            timespec_wrapper result;
            timespec ts;
            std::timespec_get(&ts, TIME_UTC);
            result.t_.tv_sec = ts.tv_sec;
            result.t_.tv_nsec = ts.tv_nsec;
            return result;
        }

        static timespec_wrapper gmtnow() noexcept {
            timespec_wrapper result;
            timespec ts;
            std::timespec_get(&ts, TIME_UTC);
            time_t t{ts.tv_sec};
            struct tm *ltm{std::gmtime(&t)};
            ts.tv_sec = std::mktime(ltm);
            result.t_.tv_sec = ts.tv_sec;
            result.t_.tv_nsec = ts.tv_nsec;
            return result;
        }

        static timespec_wrapper date_gmtoffset(timespec_wrapper const &d) noexcept {
            timespec_wrapper result;
            time_t t{d.t_.tv_sec};
            struct tm *ltm{std::localtime(&t)};
            result.t_.tv_sec = ltm->tm_gmtoff;
            result.t_.tv_nsec = 0;
            return result;
        }

        static timespec_wrapper system_gmtoffset() noexcept {
            return date_gmtoffset(now());
        }

        timespec_wrapper gmtoffset() const noexcept {
            timespec_wrapper result;
            time_t t{static_cast<time_t>(seconds())};
            struct tm *ltm{std::localtime(&t)};
            result.t_.tv_sec = ltm->tm_gmtoff;
            result.t_.tv_nsec = 0;
            return result;
        }

        timespec_wrapper &operator+=(const timespec_wrapper &rhs) noexcept {
            t_.tv_sec += rhs.t_.tv_sec + ((t_.tv_nsec + rhs.t_.tv_nsec) / 1'000'000'000L > 0L ? 1LL : 0LL);
            t_.tv_nsec = (t_.tv_nsec + rhs.t_.tv_nsec) % 1'000'000'000L;
            return *this;
        }

        timespec_wrapper &operator-=(const timespec_wrapper &rhs) noexcept {
            if(rhs.t_.tv_nsec > t_.tv_nsec) {
                t_.tv_nsec += 1'000'000'000L;
                t_.tv_sec -= 1LL;
            }
            t_.tv_sec -= rhs.t_.tv_sec;
            t_.tv_nsec -= rhs.t_.tv_nsec;
            return *this;
        }

        template<typename T>
        timespec_wrapper &operator*=(T d) noexcept {
            long double t = fseconds();
            t *= d;
            *this = timespec_wrapper{t};
            return *this;
        }

        template<typename T>
        timespec_wrapper &operator/=(T d) noexcept {
            long double t = fseconds();
            t /= d;
            *this = timespec_wrapper{t};
            return *this;
        }

        const struct timespec &ts() const noexcept {
            return t_;
        }

        struct timespec &ts() noexcept {
            return t_;
        }

        friend timespec_wrapper operator+(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            timespec_wrapper result(l); result += r; return result;
        }

        friend timespec_wrapper operator-(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            timespec_wrapper result(l); result -= r; return result;
        }

        template<typename T>
        friend timespec_wrapper operator*(const timespec_wrapper &l, T r) noexcept {
            timespec_wrapper result(l); result *= r; return result;
        }

        template<typename T>
        friend timespec_wrapper operator/(const timespec_wrapper &l, T r) noexcept {
            timespec_wrapper result(l); result /= r; return result;
        }

        friend bool operator<(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec < r.t_.tv_nsec) || l.t_.tv_sec < r.t_.tv_sec;
        }

        friend bool operator>(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec > r.t_.tv_nsec) || l.t_.tv_sec > r.t_.tv_sec;
        }

        friend bool operator==(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec == r.t_.tv_nsec;
        }

        friend bool operator<=(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec <= r.t_.tv_nsec) || (l.t_.tv_sec < r.t_.tv_sec);
        }

        friend bool operator>=(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return (l.t_.tv_sec == r.t_.tv_sec && l.t_.tv_nsec >= r.t_.tv_nsec) || (l.t_.tv_sec > r.t_.tv_sec);
        }

        friend bool operator!=(const timespec_wrapper &l, const timespec_wrapper &r) noexcept {
            return l.t_.tv_nsec != r.t_.tv_nsec || l.t_.tv_sec != r.t_.tv_sec;
        }

    private:
        static std::string subseconds_to_str(long double val, std::size_t prec) {
            if(prec > 0) {
                if(prec > 9) { prec = 9; }
                std::string sscnd{scfx::str_util::ftoa(val, prec)};
                if(sscnd == "0") { sscnd = "0.0";}
                sscnd = sscnd.substr(1);
                while(sscnd.size() < prec + 1) {
                    sscnd += '0';
                }
                return sscnd;
            }
            return {};
        }

        static std::pair<int, int> from_year_week_day(int y, int w, int d = 1) {
            if(w < 1 || w > 53 || d < 1 || d > 7) {
                return std::pair<int, int>{-1, -1};
            }
            int initial_y{y - 1900};
            std::tm tm{};
            tm.tm_mday = 1;
            tm.tm_year = initial_y;
            tm.tm_hour = 1;
            timespec tspc{std::mktime(&tm), 0};
            std::int64_t intial_sec{(std::int64_t)tspc.tv_sec};
            struct tm *tmp{std::localtime(&(tspc.tv_sec))};
            int first_day_of_first_week{tmp->tm_wday == 0 ? 7 : tmp->tm_wday};

            int flat_day{0};
            if(w == 1) {
                if(d < first_day_of_first_week) {
                    return {-1, -1};
                } else {
                    flat_day = d - first_day_of_first_week;
                }
            } else {
                int days_of_first_week{8 - first_day_of_first_week};
                flat_day += days_of_first_week;
                if(w > 2) {
                    flat_day += (w - 2) * 7;
                }
                flat_day += d - 1;
            }
            tspc.tv_sec = intial_sec + 86400 * flat_day;
            tmp = std::localtime(&(tspc.tv_sec));
            if(tmp->tm_year != initial_y) {
                return std::pair<int, int>{-1, -1};
            }
            return {tmp->tm_mon + 1, tmp->tm_mday};
        }

        static std::pair<int, int> from_year_ord_day(int y, int yd) {
            if(yd < 1 || yd > 366) {
                return std::pair<int, int>{-1, -1};
            }
            int initial_y{y - 1900};
            std::tm tm{};
            tm.tm_mday = 1;
            tm.tm_year = initial_y;
            tm.tm_hour = 1;
            timespec tspc{std::mktime(&tm), 0};
            std::int64_t intial_sec{(std::int64_t)tspc.tv_sec};
            tspc.tv_sec = intial_sec + 86400 * (yd - 1);
            struct tm *tmp{std::localtime(&(tspc.tv_sec))};
            if(initial_y != tmp->tm_year) {
                return std::pair<int, int>{-1, -1};
            }
            return {tmp->tm_mon + 1, tmp->tm_mday};
        }

        template<typename CHAR_T>
        void parse_iso_8601(CHAR_T const *buff, std::size_t bsize) {
            try {
                enum class tk_t {none, num, colon, dot, comma, minus, plus, t, w, z, spc};
                scfx::static_buff<int, 24> num_idx{};
                scfx::static_buff<int, 24> colon_idx{};
                scfx::static_buff<int, 24> dot_idx{};
                scfx::static_buff<int, 24> comma_idx{};
                scfx::static_buff<int, 24> minus_idx{};
                scfx::static_buff<int, 24> plus_idx{};
                scfx::static_buff<int, 24> t_idx{};
                scfx::static_buff<int, 24> w_idx{};
                scfx::static_buff<int, 24> z_idx{};
                scfx::static_buff<int, 24> spc_idx{};
                struct bufs {
                    scfx::static_buff<char, 16> buff{};
                    tk_t t{tk_t::none};
                    bool is_none() const { return t == tk_t::none; }
                    bool is_num() const { return t == tk_t::num; }
                    bool is_colon() const { return t == tk_t::colon; }
                    bool is_dot() const { return t == tk_t::dot; }
                    bool is_comma() const { return t == tk_t::comma; }
                    bool is_minus() const { return t == tk_t::minus; }
                    bool is_sign() const { return t == tk_t::minus || t == tk_t::plus; }
                    bool is_plus() const { return t == tk_t::plus; }
                    bool is_t() const { return t == tk_t::t; }
                    bool is_w() const { return t == tk_t::w; }
                    bool is_z() const { return t == tk_t::z; }
                    bool is_spc() const { return t == tk_t::spc; }
                    std::string_view val() const {
                        return std::string_view{buff.data(), buff.size()};
                    }
                    std::int64_t num() const {
                        return scfx::str_util::atoi(std::string_view{buff.data(), buff.size()});
                    }
                };
                scfx::static_buff<bufs, 24> b{};
                tk_t curr_class{tk_t::none};
                for(std::size_t i{}; i < bsize; ++i) {
                    char c{(char)buff[i]};
                    switch(curr_class) {
                    case tk_t::none:
                        b.push_back({});
                        if(c >= '0' && c <= '9') { num_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::num; curr_class = tk_t::num;
                        } else if(c == '-') { minus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::minus;
                        } else if(c == ':') { colon_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::colon;
                        } else if(c == ' ' || c == '\t') { spc_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::spc; curr_class = tk_t::spc;
                        } else if(c == '.') { dot_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::dot;
                        } else if(c == 'Z') { z_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::z;
                        } else if(c == 'T') { t_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::t;
                        } else if(c == 'W') { w_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::w;
                        } else if(c == '+') { plus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::plus;
                        } else if(c == ',') { comma_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::comma; }
                        break;
                    case tk_t::num:
                        if(c >= '0' && c <= '9') {
                            b.back().buff.push_back(c);
                        } else {
                            curr_class = tk_t::none;
                            b.push_back({});
                            if(c == '-') { minus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::minus;
                            } else if(c == ':') { colon_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::colon;
                            } else if(c == ' ' || c == '\t') { spc_idx.push_back(b.size() - 1); b.back().buff.push_back(' '); b.back().t = tk_t::spc; curr_class = tk_t::spc;
                            } else if(c == '.') { dot_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::dot;
                            } else if(c == 'T') { t_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::t;
                            } else if(c == 'W') { w_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::w;
                            } else if(c == 'Z') { z_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::z;
                            } else if(c == '+') { plus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::plus;
                            } else if(c == ',') { comma_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::comma; }
                        }
                        break;
                    case tk_t::spc:
                        if(c == ' ' || c == '\t') {
                        } else {
                            b.push_back({});
                            curr_class = tk_t::none;
                            if(c >= '0' && c <= '9') { b.back().buff.push_back(c); b.back().t = tk_t::num; curr_class = tk_t::num; } else
                            if(c == '-') { minus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::minus; } else
                            if(c == ':') { colon_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::colon; } else
                            if(c == '.') { dot_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::dot; } else
                            if(c == 'T') { t_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::t; } else
                            if(c == 'W') { w_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::w; } else
                            if(c == 'Z') { z_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::z; } else
                            if(c == '+') { plus_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::plus; } else
                            if(c == ',') { comma_idx.push_back(b.size() - 1); b.back().buff.push_back(c); b.back().t = tk_t::comma; }
                        }
                        break;
                    default:
                        break;
                    }
                }

                std::int64_t res_year{0};
                std::int64_t res_month{1};
                std::int64_t res_day{1};
                std::int64_t res_year_day{1};
                std::int64_t res_week{1};
                std::int64_t res_week_day{1};
                std::int64_t res_hour{0};
                std::int64_t res_minute{0};
                std::int64_t res_second{0};
                std::int64_t res_subsecond{0};
                std::int64_t res_gmt_offs_hour{0};
                std::int64_t res_gmt_offs_min{0};
                std::int64_t res_gmt_offs_sign{1};
                std::int64_t res_leading_sign{1};
                if(b.size() > 0 && (b.front().is_minus() || b.front().is_plus())) {
                    if(b.front().is_minus()) { res_leading_sign = -1; }
                    b.pop_front();

                    num_idx.traverse([](int &v) { --v; });
                    colon_idx.traverse([](int &v) { --v; });
                    dot_idx.traverse([](int &v) { --v; });
                    comma_idx.traverse([](int &v) { --v; });
                    minus_idx.traverse([](int &v) { --v; });
                    plus_idx.traverse([](int &v) { --v; });
                    t_idx.traverse([](int &v) { --v; });
                    w_idx.traverse([](int &v) { --v; });
                    z_idx.traverse([](int &v) { --v; });
                    spc_idx.traverse([](int &v) { --v; });

                    minus_idx.pop_front();
                }

                int min_time_index{0};
                bool have_t{t_idx.size() > 0};
                if(have_t) { min_time_index = t_idx[0]; }
                bool have_time{have_t};

                bool have_ymd{false};
                bool have_yod{false};
                bool have_yw{false};
                bool have_ywd{false};

                if(!(have_t && t_idx[0] == 0)) {
                    if(w_idx.size() == 1) {
                        int y_indx{w_idx[0] - 1};
                        while(y_indx >= 0 && !b[y_indx].is_num()) { y_indx--; }
                        int week_indx{w_idx[0] + 1};
                        while(week_indx < (int)b.size() && !b[week_indx].is_num()) { week_indx++; }
                        if(
                            w_idx[0] - 1 >= 0 && b[w_idx[0] - 1].is_minus() &&
                            w_idx[0] - 2 >= 0 && b[w_idx[0] - 2].is_num()
                            ) {
                            res_year = b[w_idx[0] - 2].num();
                        } else if(w_idx[0] - 1 >= 0 && b[w_idx[0] - 1].is_num()) {
                            res_year = b[w_idx[0] - 1].num();
                        }
                        if(w_idx[0] + 1 < (int)b.size() && b[w_idx[0] + 1].is_num()) {
                            if(b[w_idx[0] + 1].buff.size() == 2) {
                                res_week = b[w_idx[0] + 1].num();
                                have_yw = true;
                                min_time_index = w_idx[0] + 2;
                                if(
                                    w_idx[0] + 2 < (int)b.size() && b[w_idx[0] + 2].is_minus() &&
                                    w_idx[0] + 3 < (int)b.size() && b[w_idx[0] + 3].is_num() && b[w_idx[0] + 3].num() <= 7
                                    ) {
                                    res_week_day = b[w_idx[0] + 3].num();
                                    have_yw = false;
                                    have_ywd = true;
                                    min_time_index = w_idx[0] + 4;
                                }
                            } else if(b[w_idx[0] + 1].buff.size() == 3) {
                                std::string wwd{b[w_idx[0] + 1].buff.data(), b[w_idx[0] + 1].buff.size()};
                                res_week = scfx::str_util::atoi(wwd.substr(0, 2));
                                res_week_day = scfx::str_util::atoi(wwd.substr(2));
                                have_yw = false;
                                have_ywd = true;
                                min_time_index = w_idx[0] + 2;
                            }
                        }
                    } else {
                        if(minus_idx.size() > 0) {
                            if(
                                minus_idx.size() >= 2 &&
                                minus_idx[0] - 1 >= 0 && minus_idx[1] + 1 < (int)b.size() &&
                                minus_idx[0] + 2 == minus_idx[1] &&
                                num_idx.size() >= 3 &&
                                b[minus_idx[0] - 1].is_num() && b[minus_idx[0] + 1].is_num() && b[minus_idx[1] + 1].is_num()
                                ) {
                                int y_index{minus_idx[0] - 1};
                                int m_index{minus_idx[0] + 1};
                                int d_index{minus_idx[1] + 1};
                                res_year = b[y_index].num();
                                res_month = b[m_index].num();
                                res_day = b[d_index].num();
                                have_ymd = true;
                                min_time_index = d_index + 1;
                            } else if(
                                minus_idx.size() >= 1 &&
                                minus_idx[0] - 1 >= 0 && minus_idx[0] + 1 < (int)b.size() &&
                                num_idx.size() >= 2 &&
                                b[minus_idx[0] - 1].is_num() && (b[minus_idx[0] + 1].is_num() && b[minus_idx[0] + 1].buff.size() == 3)
                                ) {
                                int y_index{minus_idx[0] - 1};
                                int yd_index{minus_idx[0] + 1};
                                res_year = b[y_index].num();
                                res_year_day = b[yd_index].num();
                                have_yod = true;
                                min_time_index = yd_index + 1;
                            } else if(
                                minus_idx.size() == 1 &&
                                minus_idx[0] - 1 >= 0 && minus_idx[0] + 1 < (int)b.size() &&
                                num_idx.size() >= 2 &&
                                b[minus_idx[0] - 1].is_num() && (b[minus_idx[0] + 1].is_num())
                                ) {
                                int y_index{minus_idx[0] - 1};
                                int m_index{minus_idx[0] + 1};
                                res_year = b[y_index].num();
                                res_month = b[m_index].num();
                                have_ymd = true;
                                min_time_index = m_index + 1;
                            } else if(num_idx.size() >= 1 && b[0].is_num()) {
                                int y_index{minus_idx[0] - 1};
                                res_year = b[y_index].num();
                                have_ymd = true;
                                min_time_index = y_index + 1;
                            }
                        } else {
                            int date_index{0};
                            if(
                                (int)b.size() > date_index &&
                                b[date_index].is_num() &&
                                b[date_index].buff.size() == 8
                                ) {
                                std::string yyyymmdd{b[date_index].buff.data(), b[date_index].buff.size()};
                                res_year = scfx::str_util::atoi(yyyymmdd.substr(0, 4));
                                res_month = scfx::str_util::atoi(yyyymmdd.substr(4, 2));
                                res_day = scfx::str_util::atoi(yyyymmdd.substr(6, 2));
                                have_ymd = true;
                                if(min_time_index < 0) { min_time_index = date_index + 1; }
                            } else if(
                                (int)b.size() > date_index &&
                                b[date_index].is_num() &&
                                b[date_index].buff.size() == 7
                                ) {
                                std::string yyyyDDD{b[date_index].buff.data(), b[date_index].buff.size()};
                                res_year = scfx::str_util::atoi(yyyyDDD.substr(0, 4));
                                res_year_day = scfx::str_util::atoi(yyyyDDD.substr(4, 3));
                                have_yod = true;
                                if(min_time_index < 0) { min_time_index = date_index + 1; }
                            }
                        }
                    }
                }

                int subeconds_index{};
                int min_gmt_offs_start{-1};
                if(min_time_index >= 0 && min_time_index < (int)b.size()) {
                    int hour_index{min_time_index};
                    for(; hour_index < (int)b.size(); ++hour_index) {
                        if(b[hour_index].is_num()) {
                            break;
                        }
                    }
                    if(colon_idx.size() >= 1) {
                        if(
                            hour_index >= 0 &&
                            (int)b.size() > hour_index && b[hour_index].is_num() &&
                            (int)b.size() > hour_index + 1 && b[hour_index + 1].is_colon() &&
                            (int)b.size() > hour_index + 2 && b[hour_index + 2].is_num()
                            ) {
                            res_hour = b[hour_index].num();
                            res_minute = b[hour_index + 2].num();
                            have_time = true;
                            min_gmt_offs_start = hour_index + 3;
                            if(colon_idx.size() >= 2) {
                                int seconds_index{colon_idx[1] + 1};
                                if((int)b.size() > seconds_index && b[seconds_index].is_num()) {
                                    res_second = b[seconds_index].num();
                                    min_gmt_offs_start = seconds_index + 1;
                                    if(
                                        (int)b.size() > seconds_index + 2 &&
                                        b[seconds_index + 1].t == tk_t::dot &&
                                        b[seconds_index + 2].t == tk_t::num
                                        ) {
                                        subeconds_index = seconds_index + 2;
                                        res_subsecond = b[seconds_index + 2].num();
                                        min_gmt_offs_start = seconds_index + 3;
                                    }
                                }
                            }
                            have_time = true;
                        }
                    } else {
                        int hhmmss_index{hour_index};
                        if(
                            (int)b.size() > hhmmss_index && b[hhmmss_index].is_num() && b[hhmmss_index].buff.size() == 6
                            ) {
                            std::string hhmmss{b[hhmmss_index].buff.data(), b[hhmmss_index].buff.size()};
                            res_hour = scfx::str_util::atoi(hhmmss.substr(0, 2));
                            res_minute = scfx::str_util::atoi(hhmmss.substr(2, 2));
                            res_second = scfx::str_util::atoi(hhmmss.substr(4, 2));
                            min_gmt_offs_start = hhmmss_index + 1;
                            if(
                                (int)b.size() > hhmmss_index + 2 &&
                                b[hhmmss_index + 1].t == tk_t::dot &&
                                b[hhmmss_index + 2].t == tk_t::num
                                ) {
                                subeconds_index = hhmmss_index + 2;
                                res_subsecond = b[hhmmss_index + 2].num();
                                min_gmt_offs_start = hhmmss_index + 3;
                            }
                            have_time = true;
                        }
                    }
                }

                bool have_gmtoffs{false};
                bool have_z{z_idx.size() == 1};
                if(have_z) {
                    have_gmtoffs = true;
                } else {
                    if(min_gmt_offs_start >= 0 && (int)b.size() > min_gmt_offs_start && b[min_gmt_offs_start].is_sign()) {
                        int gmt_val_offs{min_gmt_offs_start + 1};
                        if(
                            (int)b.size() > gmt_val_offs && b[gmt_val_offs].is_num() &&
                            (int)b.size() > gmt_val_offs + 1 && b[gmt_val_offs + 1].is_colon() &&
                            (int)b.size() > gmt_val_offs + 2 && b[gmt_val_offs + 2].is_num()
                            ) {
                            res_gmt_offs_hour = b[gmt_val_offs].num();
                            res_gmt_offs_min = b[gmt_val_offs + 2].num();
                            if(b[min_gmt_offs_start].is_minus()) { res_gmt_offs_sign = -1; }
                            have_gmtoffs = true;
                        } else if(
                            (int)b.size() > gmt_val_offs && b[gmt_val_offs].is_num() && b[gmt_val_offs].buff.size() == 4
                            ) {
                            std::string val{b[gmt_val_offs].buff.data(), b[gmt_val_offs].buff.size()};
                            res_gmt_offs_hour = scfx::str_util::atoi(val.substr(0, 2));
                            res_gmt_offs_min = scfx::str_util::atoi(val.substr(2, 2));
                            if(b[min_gmt_offs_start].is_minus()) { res_gmt_offs_sign = -1; }
                            have_gmtoffs = true;
                        } else if((int)b.size() == gmt_val_offs + 1 && b[gmt_val_offs].is_num()) {
                            std::string val{b[gmt_val_offs].buff.data(), b[gmt_val_offs].buff.size()};
                            res_gmt_offs_hour = scfx::str_util::atoi(val.substr(0, 2));
                            res_gmt_offs_min = 0;
                            if(b[min_gmt_offs_start].is_minus()) { res_gmt_offs_sign = -1; }
                            have_gmtoffs = true;
                        }
                    }
                }

                std::tm tm{};
                tm.tm_mday = 1;
                tm.tm_mon = 0;
                tm.tm_year = 70;
                if(have_ymd) {
                    tm.tm_mday = res_day;
                    tm.tm_mon = res_month - 1;
                    tm.tm_year = (res_year * res_leading_sign) - 1900;
                } else if(have_yod) {
                    auto md{from_year_ord_day(res_year * res_leading_sign, res_year_day)};
                    if(md.first != -1 && md.second != -1) {
                        tm.tm_year = (res_year * res_leading_sign) - 1900;
                        tm.tm_mon = md.first - 1;
                        tm.tm_mday = md.second;
                    }
                } else if(have_ywd) {
                    auto md{from_year_week_day(res_year * res_leading_sign, res_week, have_ywd ? res_week_day : 1)};
                    if(md.first != -1 && md.second != -1) {
                        tm.tm_year = (res_year * res_leading_sign) - 1900;
                        tm.tm_mon = md.first - 1;
                        tm.tm_mday = md.second;
                    }
                } else if(have_yw) {
                    auto md{from_year_week_day(res_year * res_leading_sign, res_week)};
                    if(md.first != -1 && md.second != -1) {
                        tm.tm_year = (res_year * res_leading_sign) - 1900;
                        tm.tm_mon = md.first - 1;
                        tm.tm_mday = md.second;
                    }
                }
                tm.tm_sec = res_second;
                tm.tm_min = res_minute;
                tm.tm_hour = res_hour;

                std::int64_t val_sec{0};
                std::int64_t val_nsec{0};

                bool have_date{have_yod || have_ymd || have_yw || have_ywd};
                if(have_time || have_date) {
                    tm.tm_isdst = -1;
                    val_sec = std::mktime(&tm);
                    if(have_time && res_subsecond != 0) {
                        std::int64_t mltpl{1'000'000'000LL};
                        for(int i = 0; i < (int)b[subeconds_index].buff.size(); ++i) {
                            mltpl /= 10;
                        }
                        val_nsec = res_subsecond * mltpl;
                    } else {
                        val_nsec = 0;
                    }
                    if(have_gmtoffs) {
                        val_sec += -(res_gmt_offs_hour * 3600 + res_gmt_offs_min * 60) * res_gmt_offs_sign;
                        struct timeval tv;
                        tv.tv_sec = val_sec;
                        time_t t{tv.tv_sec};
                        struct tm *ltm{std::localtime(&t)};
                        val_sec += ltm->tm_gmtoff;
                    }
                }
                t_.tv_sec = val_sec;
                t_.tv_nsec = val_nsec;
            } catch (...) {
                t_ = {0, 0};
            }
        }

    private:
        struct timespec t_{0, 0};
    };

    constexpr timespec_wrapper eternity{static_cast<time_t>(0x7fff'ffff'ffff'ffffULL), static_cast<long int>(0)};

}

inline std::ostream &operator<<(std::ostream &os, scfx::timespec_wrapper const &ts) {
    os << ts.as_iso_8601_str(3);
    return os;
}
