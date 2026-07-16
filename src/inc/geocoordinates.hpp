#pragma once

#include "commondefs.hpp"
#include "str_util.hpp"
#include "math/math_util.hpp"
#include "fsm_tokenizer.hpp"

namespace teal {

    constexpr double WGS84_A = 6378137.0;
    constexpr double WGS84_F = 1.0 / 298.257223563;
    constexpr double WGS84_E2 = WGS84_F * (2.0 - WGS84_F);
    constexpr double WGS84_EP2 = WGS84_E2 / (1.0 - WGS84_E2);
    constexpr double WGS84_B = WGS84_A * math::constexpr_sqrt(1.0 - WGS84_E2);

    class LLA {
    public:
        LLA(double lat = 0, double lon = 0, double alt = 0):
            lat_{check_lattitude(lat)},
            lon_{check_longitude(lon)},
            alt_{check_altitude(alt)}
        {
        }

        LLA(double lat_deg, double lat_min, double lat_sec, double lon_deg, double lon_min, double lon_sec, double alt = 0):
            lat_{check_lattitude(lat_deg + lat_min / 60 + lat_sec / 3600)},
            lon_{check_longitude(lon_deg + lon_min / 60 + lon_sec / 3600)},
            alt_{check_altitude(alt)}
        {
        }

        explicit LLA(std::string const &cs) {
            parse(cs);
        }

        double distance_to(LLA const &other) {
            double const phi1{math::deg2rad(lat_)};
            double const phi2{math::deg2rad(other.lat_)};
            double const dphi{math::deg2rad(other.lat_ - lat_)};
            double const dl  {math::deg2rad(other.lon_ - lon_)};
            double const a   {std::sin(dphi / 2) * std::sin(dphi / 2) + std::cos(phi1) * std::cos(phi2) * std::sin(dl / 2) * std::sin(dl / 2)};
            double const c   {2 * std::atan2(std::sqrt(a), std::sqrt(1 - a))};
            return 6378136.5 * c;
        }

        std::string to_string_traditional(std::size_t seconds_prec = 3) const {
            double const abslat{std::abs(lat_)};
            double const abslon{std::abs(lon_)};
            int const lat_deg{(int)abslat};
            int const lat_min{(int)((abslat - lat_deg) * 60.0)};
            double const lat_sec{((abslat - lat_deg) * 60.0 - lat_min) * 60.0};
            int const lon_deg{(int)abslon};
            int const lon_min{(int)((abslon - lon_deg) * 60.0)};
            double const lon_sec{((abslon - lon_deg) * 60.0 - lon_min) * 60.0};
            std::stringstream ss{};
            ss << lat_deg << degree_symbol_str()
               << lat_min << minute_symbol_str()
               << str_util::ftoa(lat_sec, seconds_prec) << second_symbol_str()
               << (lat_ < 0 ? "S" : "N")
               << ", "
               << lon_deg << degree_symbol_str()
               << lon_min << minute_symbol_str()
               << str_util::ftoa(lon_sec, seconds_prec) << second_symbol_str()
               << (lon_ < 0 ? "W" : "E");
            if(alt_ != 0) {
                ss << ", " << str_util::ftoa(alt_, 2) << "m";
            }
            return ss.str();
        }

        std::string to_string_decimal_degrees() const {
            std::stringstream ss{};
            ss << str_util::ftoa(lat_, 8) << ", " << str_util::ftoa(lon_, 8);
            if(alt_ != 0) {
                ss << ", " << str_util::ftoa(alt_, 2);
            }
            return ss.str();
        }

        std::string to_string_iso_6709() const {
            std::stringstream ss{};
            ss << (lat_ < 0 ? "-" : "+") << str_util::ftoa(lat_, 8)
               << (lon_ < 0 ? "-" : "+") << str_util::ftoa(lon_, 8);
            if(alt_ != 0) {
                ss << (alt_ < 0 ? "-" : "+") << str_util::ftoa(alt_, 8) << "CRSWGS_84";
            }
            ss << "/";
            return ss.str();
        }

        double latitude() const {
            return lat_;
        }

        double longitude() const {
            return lon_;
        }

        double altitude() const {
            return alt_;
        }

        math::vector4<double> cartesian_coords() const {
            math::vector4<double> res{};
            double plr{math::deg2rad<double>(lat_)};
            double azm{math::deg2rad<double>(lon_)};
            double r{6378136.5 + alt_};
            double cplr{std::cos(plr)};
            res.x() = cplr * std::cos(azm) * r;
            res.y() = cplr * std::sin(azm) * r;
            res.z() = std::sin(plr) * r;
            return res;
        }

        static bool valid_lattitude(double v) {
            return (std::isnormal(v) || v == 0) && std::abs(v) <= 90;
        }

        static bool valid_longitude(double v) {
            return (std::isnormal(v) || v == 0) && std::abs(v) <= 180;
        }

    private:
        static double check_lattitude(double lat) {
            if(!valid_lattitude(lat)) {
                throw std::runtime_error{std::string{"invalid lattitude value: "} + str_util::ftoa<double>(lat)};
            }
            return lat;
        }

        static double check_longitude(double v) {
            if(!valid_longitude(v)) {
                throw std::runtime_error{std::string{"invalid longitude value: "} + str_util::ftoa<double>(v)};
            }
            return v;
        }

        static bool valid_altitude(double v) {
            return std::isnormal(v) || v == 0;
        }

        static double check_altitude(double v) {
            if(!valid_altitude(v)) {
                throw std::runtime_error{std::string{"invalid altitude value: "} + str_util::ftoa<double>(v)};
            }
            return v;
        }

        static bool valid_coords(double lat, double lon, double alt) {
            return valid_lattitude(lat) && valid_longitude(lon) && valid_altitude(alt);
        }

        static inline std::set<int> const degrees_characters_{0x00b0};
        static inline std::set<int> const minutes_characters_{0x2019, 0x2032, '\''};
        static inline std::set<int> const seconds_characters_{0x201d, 0x2033, '\"'};

        static bool is_degree_char(int c) {
            return degrees_characters_.find(c) != degrees_characters_.end();
        }

        template<typename STR_T>
        static bool is_degree_str(STR_T const &s) {
            return s.size() == 1 && is_degree_char(s[0]);
        }

        static bool is_minute_char(int c) {
            return minutes_characters_.find(c) != minutes_characters_.end();
        }

        template<typename STR_T>
        static bool is_minute_str(STR_T const &s) {
            return s.size() == 1 && is_minute_char(s[0]);
        }

        static bool is_second_char(int c) {
            return seconds_characters_.find(c) != seconds_characters_.end();
        }

        template<typename STR_T>
        static bool is_second_str(STR_T const &s) {
            return s.size() == 1 && is_second_char(s[0]);
        }

        static bool is_n_char(int c) { return c == 'n' || c == 'N'; }

        template<typename STR_T>
        static bool is_n_str(STR_T const &s) {
            return s.size() == 1 && is_n_char(s[0]);
        }

        static bool is_s_char(int c) { return c == 's' || c == 'S'; }

        template<typename STR_T>
        static bool is_s_str(STR_T const &s) {
            return s.size() == 1 && is_s_char(s[0]);
        }

        static bool is_e_char(int c) { return c == 'e' || c == 'E'; }

        template<typename STR_T>
        static bool is_e_str(STR_T const &s) {
            return s.size() == 1 && is_e_char(s[0]);
        }

        static bool is_w_char(int c) { return c == 'w' || c == 'W'; }

        template<typename STR_T>
        static bool is_w_str(STR_T const &s) {
            return s.size() == 1 && is_w_char(s[0]);
        }

        static std::string degree_symbol_str() {
            std::wstring w{}; w += (wchar_t)0x00b0;
            return str_util::to_utf8(w);
        }

        static std::string minute_symbol_str() {
            std::wstring w{}; w += (wchar_t)0x2032;
            return str_util::to_utf8(w);
        }

        static std::string second_symbol_str() {
            std::wstring w{}; w += (wchar_t)0x2033;
            return str_util::to_utf8(w);
        }

        enum class world_side {
            none, north, south, east, west,
        };

        enum class node_kind {
            none, degree_sym, min_sym, sec_sym, minus_sym, plus_sym, num_val, coord, alt, coord_lat, coord_lon, location
        };

        struct ltd{
            double deg{};
            std::wstring token_type{};
        };

        struct node {
            std::wstring token{};
            std::wstring token_type{};
            std::shared_ptr<node> left_sub_element{};
            std::shared_ptr<node> right_sub_element{};
            double number{};
            node_kind kind{node_kind::none};
            world_side wrldside{world_side::none};

            double calc() const {
                int depth{};
                return calc_internal(depth);
            }

            double calc_internal(int &depth, node_kind upper_node_kind = node_kind::none) const {
                double deep_result{};
                if(left_sub_element) {
                    deep_result = left_sub_element->calc_internal(depth, kind);
                }
                if(kind == node_kind::coord_lat) {
                    if(wrldside == world_side::south) {
                        deep_result = -std::abs(deep_result);
                    }
                } else if(kind == node_kind::coord_lon) {
                    if(wrldside == world_side::west) {
                        deep_result = -std::abs(deep_result);
                    }
                }
                if(kind == node_kind::num_val) {
                    if(upper_node_kind == node_kind::degree_sym || depth == 0) {
                        deep_result += number;
                    } else if(upper_node_kind == node_kind::min_sym || depth == 1) {
                        deep_result += number / 60;
                    } else if(upper_node_kind == node_kind::sec_sym || depth == 2) {
                        deep_result += number / 3600;
                    } else {
                        throw std::runtime_error{"wrong format: too many numbers"};
                    }
                }
                if(kind == node_kind::num_val) {
                    depth++;
                }
                return deep_result;
            }
        };

        static bool valid_indexes(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens, int begin_indx, int end_indx) {
            return begin_indx >= 0 && begin_indx < (int)tokens.size() && end_indx >= 0 && end_indx < (int)tokens.size() && begin_indx <= end_indx;
        }

        std::shared_ptr<node> parse_most_right_num(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens, int begin_indx, int end_indx) {
            if(!valid_indexes(tokens, begin_indx, end_indx)) {
                return {};
            }
            std::shared_ptr<node> res{std::make_shared<node>()};
            int new_end{end_indx - 1};
            if(teal::is_first_one_of_the_rest(tokens[end_indx].type, L"int", L"fpl")) {
                res->kind = node_kind::num_val;
                res->number = std::atof(teal::str_util::to_utf8(tokens[end_indx].value).c_str());

                bool sgn_done{false};
                if(new_end >= begin_indx) {
                    if(teal::is_first_one_of_the_rest(tokens[new_end].type, L"minus", L"plus")) {
                        if(tokens[new_end].type == L"minus") { res->number = -res->number; }
                        new_end--;
                        sgn_done = true;
                    } else if(teal::is_first_one_of_the_rest(tokens[new_end].type, L"spc")) {
                        new_end--;
                    }
                }
                if(!sgn_done && new_end >= begin_indx) {
                    if(teal::is_first_one_of_the_rest(tokens[new_end].type, L"minus", L"plus")) {
                        if(tokens[new_end].type == L"minus") { res->number = -res->number; }
                        new_end--;
                    }
                }
            }
            if(new_end >= begin_indx && valid_indexes(tokens, begin_indx, new_end)) {
                res->left_sub_element = parse_dms_sym(tokens, begin_indx, new_end);
            }
            return res;
        }

        std::shared_ptr<node> parse_dms_sym(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens, int begin_indx, int end_indx) {
            if(!valid_indexes(tokens, begin_indx, end_indx)) {
                return {};
            }
            std::shared_ptr<node> res{std::make_shared<node>()};
            if(is_degree_str(tokens[end_indx].value)) {
                res->kind = node_kind::degree_sym;
            } else if(is_minute_str(tokens[end_indx].value)) {
                res->kind = node_kind::min_sym;
            } else if(is_second_str(tokens[end_indx].value)) {
                res->kind = node_kind::sec_sym;
            }
            bool is_cur_num{teal::is_first_one_of_the_rest(tokens[end_indx].type, L"fpl", L"int")};
            if(res->kind != node_kind::none || !is_cur_num) {
                res->left_sub_element = parse_most_right_num(tokens, begin_indx, end_indx - 1);
            } else if(is_cur_num) {
                res->left_sub_element = parse_most_right_num(tokens, begin_indx, end_indx);
            }
            return res;
        }

        std::shared_ptr<node> parse_height(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens, int begin_indx, int end_indx) {
            while(
                begin_indx < (int)tokens.size() && begin_indx <= end_indx &&
                !teal::is_first_one_of_the_rest(tokens[begin_indx].type, L"int", L"fpl", L"plus", L"minus")
            ) {
                ++begin_indx;
            }
            if(!valid_indexes(tokens, begin_indx, end_indx)) {
                return {};
            }
            std::shared_ptr<node> res{std::make_shared<node>()};
            if(tokens[end_indx].value == L"m") {
                res->kind = node_kind::alt;
            }
            bool is_cur_num{teal::is_first_one_of_the_rest(tokens[end_indx].type, L"fpl", L"int")};
            if(res->kind != node_kind::none || !is_cur_num) {
                res->left_sub_element = parse_most_right_num(tokens, begin_indx, end_indx - 1);
            } else if(is_cur_num) {
                res = parse_most_right_num(tokens, begin_indx, end_indx);
            }
            return res;
        }

        std::shared_ptr<node> parse_coord(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens, int begin_indx, int end_indx) {
            while(
                begin_indx < (int)tokens.size() && begin_indx <= end_indx &&
                !teal::is_first_one_of_the_rest(tokens[begin_indx].type, L"int", L"fpl", L"plus", L"minus")
            ) {
                ++begin_indx;
            }
            if(!valid_indexes(tokens, begin_indx, end_indx)) {
                return {};
            }
            std::shared_ptr<node> res{std::make_shared<node>()};
            res->kind = node_kind::coord;
            struct world_side_indx { world_side side{world_side::none}; int indx{-1}; };
            std::vector<world_side_indx> wsides{};
            for(int i{begin_indx}; i <= end_indx; ++i) {
                if(tokens[i].type == L"N") {
                    world_side_indx ws{}; ws.side = world_side::north; ws.indx = i; wsides.push_back(ws);
                } else if(tokens[i].type == L"S") {
                    world_side_indx ws{}; ws.side = world_side::south; ws.indx = i; wsides.push_back(ws);
                } else if(tokens[i].type == L"E") {
                    world_side_indx ws{}; ws.side = world_side::east; ws.indx = i; wsides.push_back(ws);
                } else if(tokens[i].type == L"W") {
                    world_side_indx ws{}; ws.side = world_side::west; ws.indx = i; wsides.push_back(ws);
                }
            }
            if(wsides.size() == 1) {
                res->wrldside = wsides[0].side;
                res->kind = teal::is_first_one_of_the_rest(res->wrldside, world_side::north, world_side::south) ? node_kind::coord_lat : node_kind::coord_lon;
                if(wsides[0].indx == end_indx) {
                    res->left_sub_element = parse_dms_sym(tokens, begin_indx, end_indx - 1);
                } else if(wsides[0].indx == begin_indx) {
                    res->left_sub_element = parse_dms_sym(tokens, begin_indx + 1, end_indx);
                } else {
                    res->left_sub_element = parse_dms_sym(tokens, begin_indx, end_indx);
                }
            } else if(wsides.size() == 0) {
                res->left_sub_element = parse_dms_sym(tokens, begin_indx, end_indx);
            } if(wsides.size() > 1) {
                throw std::runtime_error{"invalid geo coordinates string"};
            }
            return res;
        }

        void parse_coords(std::vector<teal::fsm_tokenizer<std::wstring>::token> const &tokens) {
            if(tokens.size() == 0) {
                return;
            }
            int begin_indx{0};
            int end_indx{(int)tokens.size() - 1};
            struct num_indx { int indx{-1}; bool is_fpl{false}; };
            std::vector<int> signs_indexes{};
            std::vector<int> deg_indexes{};
            std::vector<int> min_indexes{};
            std::vector<int> sec_indexes{};
            std::vector<int> lat_lon_indexes{};
            std::vector<int> height_indexes{};
            std::vector<int> nums_indexes{};
            std::vector<int> int_nums_indexes{};
            std::vector<int> fp_nums_indexes{};
            std::vector<int> commas_indexes{};
            std::vector<int> spaces_indexes{};
            std::vector<int> solidus_indexes{};
            for(int i{begin_indx}; i <= end_indx; ++i) {
                if(is_degree_str(tokens[i].value)) { deg_indexes.push_back(i); } else
                if(is_minute_str(tokens[i].value)) { min_indexes.push_back(i); } else
                if(is_second_str(tokens[i].value)) { sec_indexes.push_back(i); } else
                if(tokens[i].value == L",") { commas_indexes.push_back(i); } else
                if(tokens[i].type == L"spc") { spaces_indexes.push_back(i); } else
                if(tokens[i].type == L"sol") { solidus_indexes.push_back(i);
                } else if(teal::is_first_one_of_the_rest(tokens[i].type, L"N", L"S")) {
                    lat_lon_indexes.push_back(i);
                } else if(teal::is_first_one_of_the_rest(tokens[i].type, L"W", L"E")) {
                    lat_lon_indexes.push_back(i);
                } else if(tokens[i].type == L"m") {
                    height_indexes.push_back(i);
                } else if(teal::is_first_one_of_the_rest(tokens[i].type, L"plus", L"minus")) {
                    signs_indexes.push_back(i);
                } else if(teal::is_first_one_of_the_rest(tokens[i].type, L"int")) {
                    nums_indexes.push_back(i);
                    int_nums_indexes.push_back(i);
                } else if(teal::is_first_one_of_the_rest(tokens[i].type, L"fpl")) {
                    nums_indexes.push_back(i);
                    fp_nums_indexes.push_back(i);
                }
            }

            std::shared_ptr<node> frst_coo{};
            std::shared_ptr<node> scnd_coo{};
            std::shared_ptr<node> alt_node{};
            do {
                if(!lat_lon_indexes.empty()) {
                    frst_coo = parse_coord(tokens, 0, lat_lon_indexes[0]);
                    if(lat_lon_indexes.size() > 1) {
                        scnd_coo = parse_coord(tokens, lat_lon_indexes[0] + 1, lat_lon_indexes[1]);
                    } else {
                        if(height_indexes.empty()) {
                            scnd_coo = parse_coord(tokens, lat_lon_indexes[0] + 1, tokens.size() - 1);
                        } else {
                            scnd_coo = parse_coord(tokens, lat_lon_indexes[0] + 1, height_indexes.size() - 2);
                        }
                    }
                    if(lat_lon_indexes[lat_lon_indexes.size() - 1] < (int)tokens.size() - 1) {
                        alt_node = parse_height(tokens, lat_lon_indexes[lat_lon_indexes.size() - 1] + 1, tokens.size() - 1);
                    }
                    if(teal::is_first_one_of_the_rest(tokens[lat_lon_indexes[0]].type, L"E", L"W")) {
                        std::swap(frst_coo, scnd_coo);
                    }
                } else {
                    if(!commas_indexes.empty()) {
                        if(commas_indexes.size() == 1) {
                            frst_coo = parse_coord(tokens, 0, commas_indexes[0] - 1);
                            scnd_coo = parse_coord(tokens, commas_indexes[0] + 1, tokens.size() - 1);
                            break;
                        } else if(commas_indexes.size() == 2) {
                            frst_coo = parse_coord(tokens, 0, commas_indexes[0] - 1);
                            scnd_coo = parse_coord(tokens, commas_indexes[0] + 1, commas_indexes[1] - 1);
                            alt_node = parse_height(tokens, commas_indexes[1] + 1, tokens.size() - 1);
                            break;
                        }
                    }
                    if(nums_indexes.size() == 2) {
                        frst_coo = parse_coord(tokens, nums_indexes[0], nums_indexes[0]);
                        scnd_coo = parse_coord(tokens, nums_indexes[1], nums_indexes[1]);
                        break;
                    } else if(nums_indexes.size() == 3) {
                        frst_coo = parse_coord(tokens, nums_indexes[0], nums_indexes[0]);
                        scnd_coo = parse_coord(tokens, nums_indexes[1], nums_indexes[1]);
                        alt_node = parse_height(tokens, nums_indexes[2], nums_indexes[2]);
                        break;
                    } else if(nums_indexes.size() > 3) {

                    }
                }
            } while(false);

            lat_ = frst_coo ? frst_coo->calc() : 0;
            lon_ = scnd_coo ? scnd_coo->calc() : 0;
            alt_ = alt_node ? alt_node->calc() : 0;
        }

        void parse(std::string const &coords) {
            std::wstring const wcoords{teal::str_util::from_utf8(coords)};
            std::vector<teal::fsm_tokenizer<std::wstring>::token> tokens{};
            teal::fsm_tokenizer<std::wstring> t{
                L"#rd|[:digit:]>>int;"
                L"int|'.'>>fp|[:digit:]>>int|$df>>^int|$ef>>^int;"
                L"fp|[:digit:]>>fp|$df>>^fpl|$ef>>^fpl;"

                L"#rd|'+'>>^plus;"
                L"#rd|'-'>>^minus;"

                L"#rd|'N'>>^N;"
                L"#rd|'n'>>^N;"
                L"#rd|'S'>>^S;"
                L"#rd|'s'>>^S;"
                L"#rd|'W'>>^W;"
                L"#rd|'w'>>^W;"
                L"#rd|'E'>>^E;"
                L"#rd|'e'>>^E;"
                L"#rd|'m'>>^m;"
                L"#rd|'M'>>^m;"

                L"#rd|'/'>>^sol;"

                L"#rd|'C'>>C;"
                L"C|'R'>>CR|$ef>>#er;"
                L"CR|'S'>>CRS|$ef>>#er;"
                L"CRS|[_\\::alnum:]>>CRS|$df>>^crs|$ef>>^crs;"

                L"#rd|[:space:]>>sp;"
                L"sp|[:space:]>>sp|$df>>^spc|$ef>>^spc;"
                L"#rd|[\\r\\n]>>#er;"
                L"#rd|$df>>^sym;"
                ,
                [&](teal::fsm_tokenizer<std::wstring>::token const &tkn) { tokens.push_back(tkn); }
            };
            t.accept_string(wcoords);
            t.finalize();
            parse_coords(tokens);
        }

    private:
        double lat_{};
        double lon_{};
        double alt_{};
    };


    class ECEF {
    public:
        ECEF(double x = 0, double y = 0, double z = 0):
            x_{x},
            y_{y},
            z_{z}
        {
        }

        double x() const { return x_; }
        double y() const { return y_; }
        double z() const { return z_; }

    private:
        double x_{};
        double y_{};
        double z_{};
    };

    class ENU {
    public:
        ENU(double east = 0, double north = 0, double up = 0):
            east_{east},
            north_{north},
            up_{up}
        {
        }

        double east() const { return east_; }
        double north() const { return north_; }
        double up() const { return up_; }

    private:
        double east_{};
        double north_{};
        double up_{};
    };

    static ECEF LLAtoECEF(const LLA& lla) {
        double sinLat = std::sin(math::deg2rad(lla.latitude()));
        double cosLat = std::cos(math::deg2rad(lla.latitude()));
        double sinLon = std::sin(math::deg2rad(lla.longitude()));
        double cosLon = std::cos(math::deg2rad(lla.longitude()));

        double N = WGS84_A / std::sqrt(1.0 - WGS84_E2 * sinLat * sinLat);

        return {
            (N + lla.altitude()) * cosLat * cosLon, // X
            (N + lla.altitude()) * cosLat * sinLon, // Y
            (N * (1.0 - WGS84_E2) + lla.altitude()) * sinLat // Z
        };
    }

    static LLA ECEFtoLLA(const ECEF& ecef) {
        double x = ecef.x();
        double y = ecef.y();
        double z = ecef.z();

        double p = std::sqrt(x * x + y * y);
        double theta = std::atan2(z * WGS84_A, p * WGS84_B);

        double sinTheta = std::sin(theta);
        double cosTheta = std::cos(theta);

        double lat = std::atan2(z + WGS84_EP2 * WGS84_B * sinTheta * sinTheta * sinTheta,
                                p - WGS84_E2 * WGS84_A * cosTheta * cosTheta * cosTheta);

        double lon = std::atan2(y, x);

        double sinLat = std::sin(lat);
        double N = WGS84_A / std::sqrt(1.0 - WGS84_E2 * sinLat * sinLat);
        double alt = p / std::cos(lat) - N;

        return {math::rad2deg(lat), math::rad2deg(lon), alt};
    }

    static ENU ECEFtoENU(const ECEF& ecef, const LLA& ref) {
        ECEF refEcef = LLAtoECEF(ref);

        double dx = ecef.x() - refEcef.x();
        double dy = ecef.y() - refEcef.y();
        double dz = ecef.z() - refEcef.z();

        double sinLat = std::sin(math::deg2rad(ref.latitude()));
        double cosLat = std::cos(math::deg2rad(ref.latitude()));
        double sinLon = std::sin(math::deg2rad(ref.longitude()));
        double cosLon = std::cos(math::deg2rad(ref.longitude()));

        return {
            -sinLon * dx + cosLon * dy,                                    // East
            -sinLat * cosLon * dx - sinLat * sinLon * dy + cosLat * dz,    // North
            cosLat * cosLon * dx + cosLat * sinLon * dy + sinLat * dz      // Up
        };
    }

}

static std::ostream &operator<<(std::ostream &os, teal::LLA const &v) {
    os << v.to_string_traditional(3);
    return os;
}

static std::ostream &operator<<(std::ostream &os, teal::ECEF const &v) {
    std::stringstream ss{};
    ss << "ECEF{" << std::fixed << v.x() << ", " << v.y() << ", " << v.z() << "}";
    os << ss.str();
    return os;
}

static std::ostream &operator<<(std::ostream &os, teal::ENU const &v) {
    std::stringstream ss{};
    ss << "ENU{" << v.east() << ", " << v.north() << ", " << v.up() << "}";
    os << ss.str();
    return os;
}
