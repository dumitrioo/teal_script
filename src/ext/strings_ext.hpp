#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/sequence_generator.hpp"
#include "../inc/str_util.hpp"
#include "../inc/base16.hpp"
#include "../inc/base64.hpp"
#include "../inc/base85.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class strings_ext: public extension_interface {
    public:
        template<typename STR_T>
        static STR_T py_slice(
            STR_T const &str,
            std::optional<std::int64_t> start = std::nullopt,
            std::optional<std::int64_t> stop = std::nullopt,
            std::optional<std::int64_t> step = std::nullopt
        ) {
            STR_T result{};

            std::int64_t length = static_cast<std::int64_t>(str.length());

            if(!step.has_value()) {
                step = 1;
            }
            if(step == 0) {
                return result;
            }

            std::int64_t s{};
            std::int64_t e{};
            std::int64_t st{step.value()};

            if(st > 0) {
                s = start.value_or(0);
                e = stop.value_or(length);

                if(s < 0) {
                    s += length;
                    if(s < 0) s = 0;
                } else if(s > length) {
                    s = length;
                }

                if(e < 0) {
                    e += length;
                    if(e < 0) e = 0;
                } else if(e > length) {
                    e = length;
                }
            } else {
                s = start.value_or(length - 1);
                e = stop.value_or(-1);

                if(s < 0) {
                    s += length;
                    if(s < 0) s = -1;
                } else if(s >= length) {
                    s = length - 1;
                }

                if(e < 0) {
                    e += length;
                    if(e < 0) e = -1;
                } else if(e >= length) {
                    e = length - 1;
                }
            }

            if(st > 0 && e > s) {
                result.reserve((e - s + st - 1) / st);
            } else if(st < 0 && s > e) {
                result.reserve((s - e - st - 1) / -st);
            }

            if(st > 0) {
                for(std::int64_t i = s; i < e; i += st) {
                    result += str[i];
                }
            } else {
                for(std::int64_t i = s; i > e; i += st) {
                    result += str[i];
                }
            }

            return result;
        }

        static valbox py_slice(
            valbox::array_t const &str,
            std::optional<std::int64_t> start = std::nullopt,
            std::optional<std::int64_t> stop = std::nullopt,
            std::optional<std::int64_t> step = std::nullopt
        ) {
            valbox result{};
            result.become_array();

            std::int64_t length = static_cast<std::int64_t>(str.size());

            if(!step.has_value()) {
                step = 1;
            }
            if(step == 0) {
                return result;
            }

            std::int64_t s{};
            std::int64_t e{};
            std::int64_t st{step.value()};

            if(st > 0) {
                s = start.value_or(0);
                e = stop.value_or(length);

                if(s < 0) {
                    s += length;
                    if(s < 0) s = 0;
                } else if(s > length) {
                    s = length;
                }

                if(e < 0) {
                    e += length;
                    if(e < 0) e = 0;
                } else if(e > length) {
                    e = length;
                }
            } else {
                s = start.value_or(length - 1);
                e = stop.value_or(-1);

                if(s < 0) {
                    s += length;
                    if(s < 0) s = -1;
                } else if(s >= length) {
                    s = length - 1;
                }

                if(e < 0) {
                    e += length;
                    if(e < 0) e = -1;
                } else if(e >= length) {
                    e = length - 1;
                }
            }

            if(st > 0 && e > s) {
                result.as_array().reserve((e - s + st - 1) / st);
            } else if(st < 0 && s > e) {
                result.as_array().reserve((s - e - st - 1) / -st);
            }

            if(st > 0) {
                for(std::int64_t i = s; i < e; i += st) {
                    result.as_array().push_back(str[i].clone());
                }
            } else {
                for(std::int64_t i = s; i > e; i += st) {
                    result.as_array().push_back(str[i].clone());
                }
            }

            return result;
        }

    public:
        strings_ext() = default;
        ~strings_ext() {
            unregister_runtime();
        }
        strings_ext(strings_ext const &) = delete;
        strings_ext &operator=(strings_ext const &) = delete;
        strings_ext(strings_ext &&) = delete;
        strings_ext &operator=(strings_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }

            rt_->add_function("atoi", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return str_util::atoi(args[0].cast_to_string());
                } else if(args.size() == 2) {
                    return str_util::atoi(args[0].cast_to_string(), args[1].cast_to_u64());
                } else if(args.size() == 3) {
                    return str_util::atoi(args[0].cast_to_string(), args[1].cast_to_u64(), args[2].cast_to_bool());
                }
                return std::int64_t{0};
            });

            rt_->add_function("atoui", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 2)
                if(args.size() == 1) {
                    return str_util::atoui(args[0].cast_to_string());
                } else if(args.size() == 2) {
                    return str_util::atoui(args[0].cast_to_string(), args[1].cast_to_u64());
                }
                return std::uint64_t{0};
            });

            rt_->add_function("atof", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                return str_util::atof(args[0].cast_to_string());
            });

            rt_->add_function("itoa", TEALFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        if(args.size() > 2) {
                            if(args.size() > 3) {
                                return str_util::itoa<std::string>(args[0].cast_to_s64(), args[1].cast_to_s64(), args[2].cast_to_s64(), args[3].cast_to_bool());
                            } else {
                                return str_util::itoa<std::string>(args[0].cast_to_s64(), args[1].cast_to_s64(), args[2].cast_to_s64());
                            }
                        } else {
                            return str_util::itoa<std::string>(args[0].cast_to_s64(), args[1].cast_to_s64());
                        }
                    } else {
                        return str_util::itoa<std::string>(args[0].cast_to_s64());
                    }
                }
                return std::string{};
            });

            rt_->add_function("utoa", TEALFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        if(args.size() > 2) {
                            if(args.size() > 3) {
                                return str_util::utoa<std::string>(args[0].cast_to_u64(), args[1].cast_to_s64(), args[2].cast_to_s64(), args[3].cast_to_bool());
                            } else {
                                return str_util::utoa<std::string>(args[0].cast_to_u64(), args[1].cast_to_s64(), args[2].cast_to_s64());
                            }
                        } else {
                            return str_util::utoa<std::string>(args[0].cast_to_u64(), args[1].cast_to_s64());
                        }
                    } else {
                        return str_util::utoa<std::string>(args[0].cast_to_u64());
                    }
                }
                return std::string{};
            });

            rt_->add_function("ftoa", TEALFUN(args) {
                if(args.size() > 0) {
                    if(args.size() > 1) {
                        if(args[0].is_long_double()) {
                            return str_util::ftoa(args[0].as_long_double(), args[1].cast_to_size_t());
                        } else if(args[0].is_double()) {
                            return str_util::ftoa(args[0].as_double(), args[1].cast_to_size_t());
                        } else if(args[0].is_float()) {
                            return str_util::ftoa(args[0].as_float(), args[1].cast_to_size_t());
                        } else if(args[0].is_numeric()) {
                            return str_util::ftoa(args[0].cast_to_double(), args[1].cast_to_size_t());
                        }
                    } else {
                        if(args[0].is_long_double()) {
                            return str_util::ftoa(args[0].as_long_double());
                        } else if(args[0].is_double()) {
                            return str_util::ftoa(args[0].as_double());
                        } else if(args[0].is_float()) {
                            return str_util::ftoa(args[0].as_float());
                        } else if(args[0].is_numeric()) {
                            return str_util::ftoa(args[0].cast_to_double());
                        }
                    }
                }
                return std::string{"0.0"};
            });

            rt_->add_function("toupper", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char()) {
                    return str_util::fltr<std::string>::toupper(args[0].as_char());
                } else {
                    return str_util::fltr<std::wstring>::toupper(args[0].cast_to_u64());
                }
            });

            rt_->add_function("tolower", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_char()) {
                    return str_util::fltr<std::string>::tolower(args[0].as_char());
                } else {
                    return str_util::fltr<std::wstring>::tolower(args[0].cast_to_u64());
                }
            });

            rt_->add_function("strtoupper", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string()) {
                    return str_util::fltr<std::string>::strtoupper(args[0].as_string());
                } else if(args[0].is_wstring()) {
                    return str_util::fltr<std::wstring>::strtoupper(args[0].as_wstring());
                } else {
                    return str_util::fltr<std::string>::strtoupper(args[0].cast_to_string());
                }
            });
            rt_->add_function("strtolower", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string()) {
                    return str_util::fltr<std::string>::strtolower(args[0].as_string());
                } else if(args[0].is_wstring()) {
                    return str_util::fltr<std::wstring>::strtolower(args[0].as_wstring());
                } else {
                    return str_util::fltr<std::string>::strtolower(args[0].cast_to_string());
                }
            });

            rt_->add_function("replace_substr", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 2) {
                    return str_util::to_utf8(str_util::replace_substring<std::wstring>(
                        args[0].cast_to_wstring(), args[1].cast_to_wstring(), std::wstring{}
                        ));
                }
                if(args.size() == 3) {
                    return str_util::to_utf8(str_util::replace_substring<std::wstring>(
                        args[0].cast_to_wstring(), args[1].cast_to_wstring(), args[2].cast_to_wstring()
                        ));
                }
                return std::string{};
            });

            rt_->add_function("substr", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return args[0].cast_to_string();
                } else if(args.size() == 2) {
                    std::string s{args[0].cast_to_string()};
                    std::size_t from{args[1].cast_to_u64()};
                    return s.substr(from);
                } else if(args.size() == 3) {
                    std::string s{args[0].cast_to_string()};
                    std::size_t from{args[1].cast_to_u64()};
                    std::size_t num{args[2].cast_to_u64()};
                    return s.substr(from, num);
                } else if(args.size() > 3) {
                    return args[0].cast_to_string();
                }
                return std::string{};
            });

            rt_->add_function("split", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                valbox res{};
                res.become_array();
                if(args[0].is_string()) {
                    std::vector<std::string> sv{str_util::str_tok(args[0].cast_to_string(), args[1].cast_to_string())};
                    for(auto &&s: sv) {
                        res.as_array().push_back(s);
                    }
                } else if(args[0].is_wstring()) {
                    std::vector<std::wstring> sv{str_util::str_tok(args[0].cast_to_wstring(), args[1].cast_to_wstring())};
                    for(auto &&s: sv) {
                        res.as_array().push_back(s);
                    }
                }
                return res;
            });

            rt_->add_function("ltrim", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string()) {
                    return str_util::ltrim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring()) {
                    return str_util::ltrim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });

            rt_->add_function("rtrim", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string()) {
                    return str_util::rtrim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring()) {
                    return str_util::rtrim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });
            rt_->add_function("trim", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                if(args[0].is_string()) {
                    return str_util::trim<std::string>(args[0].as_string());
                } else if(args[0].is_wstring()) {
                    return str_util::trim<std::wstring>(args[0].as_wstring());
                } else {
                    return args[0];
                }
            });

            rt_->add_function("isspace", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isspace(args[0].cast_to_int()); });
            rt_->add_function("isalpha", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isalpha(args[0].cast_to_int()); });
            rt_->add_function("isdigit", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isdigit(args[0].cast_to_int()); });
            rt_->add_function("isalnum", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isalnum(args[0].cast_to_int()); });
            rt_->add_function("ispunct", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::ispunct(args[0].cast_to_int()); });
            rt_->add_function("iscntrl", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::iscntrl(args[0].cast_to_int()); });
            rt_->add_function("ishexdigit", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::ishexdigit(args[0].cast_to_int()); });
            rt_->add_function("isoctdigit", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isoctdigit(args[0].cast_to_int()); });
            rt_->add_function("isbindigit", TEALFUN(args) { TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1) return str_util::fltr<std::wstring>::isbindigit(args[0].cast_to_int()); });

            rt_->add_function("hexdump", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 4)
                if(args.size() == 1) {
                    return str_util::hexdump(args[0].cast_to_byte_array());
                } else if(args.size() == 2) {
                    return str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64()
                        );
                } else if(args.size() == 3) {
                    return str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64(),
                        args[2].cast_to_string()
                        );
                } else if(args.size() == 4) {
                    return str_util::hexdump(
                        args[0].cast_to_byte_array(),
                        args[1].cast_to_u64(),
                        args[2].cast_to_string(),
                        args[3].cast_to_bool()
                        );
                }
                return std::string{};
            });

            rt_->add_function("data_to_base85_str", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return data_to_base85_str(src);
            });

            rt_->add_function("base85_str_to_data", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{base85_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            rt_->add_function("data_to_base64_str", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return data_to_base64_str(src);
            });

            rt_->add_function("base64_str_to_data", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{base64_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            rt_->add_function("data_to_hex_str", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_byte_array()};
                return data_to_hex_str(src);
            });

            rt_->add_function("hex_str_to_data", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 1)
                auto src{args[0].cast_to_string()};
                auto d{hex_str_to_data(src)};
                return std::string{d.begin(), d.end()};
            });

            rt_->add_function("find_substr", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args[0].is_string()) {
                    if(args[1].is_string()) {
                        if(args.size() == 3) {
                            return substr_finder{}.find(args[0].as_string(), args[1].as_string(), args[2].cast_to_u64());
                        } else {
                            return substr_finder{}.find(args[0].as_string(), args[1].as_string());
                        }
                    } else {
                        if(args.size() == 3) {
                            return substr_finder{}.find(args[0].as_string(), args[1].cast_to_string(), args[2].cast_to_u64());
                        } else {
                            return substr_finder{}.find(args[0].as_string(), args[1].cast_to_string());
                        }
                    }
                } else if(args[0].is_wstring()) {
                    if(args[1].is_wstring()) {
                        if(args.size() == 3) {
                            return substr_finder{}.find(args[0].as_wstring(), args[1].as_wstring(), args[2].cast_to_u64());
                        } else {
                            return substr_finder{}.find(args[0].as_wstring(), args[1].as_wstring());
                        }
                    } else {
                        if(args.size() == 3) {
                            return substr_finder{}.find(args[0].as_wstring(), args[1].cast_to_wstring(), args[2].cast_to_u64());
                        } else {
                            return substr_finder{}.find(args[0].as_wstring(), args[1].cast_to_wstring());
                        }
                    }
                } else {
                    return static_cast<int64_t>(-1);
                }
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }

            rt_->remove_function("py_slice");
            rt_->remove_function("atoi");
            rt_->remove_function("atoui");
            rt_->remove_function("atof");
            rt_->remove_function("itoa");
            rt_->remove_function("utoa");
            rt_->remove_function("ftoa");
            rt_->remove_function("toupper");
            rt_->remove_function("tolower");
            rt_->remove_function("strtoupper");
            rt_->remove_function("strtolower");
            rt_->remove_function("replace_substr");
            rt_->remove_function("substr");
            rt_->remove_function("split");
            rt_->remove_function("ltrim");
            rt_->remove_function("rtrim");
            rt_->remove_function("trim");
            rt_->remove_function("isspace");
            rt_->remove_function("isalpha");
            rt_->remove_function("isdigit");
            rt_->remove_function("isalnum");
            rt_->remove_function("ispunct");
            rt_->remove_function("iscntrl");
            rt_->remove_function("ishexdigit");
            rt_->remove_function("isoctdigit");
            rt_->remove_function("isbindigit");
            rt_->remove_function("hexdump");
            rt_->remove_function("data_to_base85_str");
            rt_->remove_function("base85_str_to_data");
            rt_->remove_function("data_to_base64_str");
            rt_->remove_function("base64_str_to_data");
            rt_->remove_function("data_to_hex_str");
            rt_->remove_function("hex_str_to_data");

            rt_ = nullptr;
        }

    private:
        class substr_finder {
        public:
            int64_t find(std::string const &s, std::string const &sub, uint64_t starting_point = 0) {
                if(s.size() == 0 || sub.size() == 0 || s.size() < sub.size() + starting_point) { return -1; }
                uint64_t sub_cksum{cksum(std::string_view{sub})};
                uint64_t curr_cksum{cksum(std::string_view{s.data() + starting_point, sub.size()})};
                for(int64_t i{(int64_t)starting_point}; i <= (int64_t)s.size() - (int64_t)sub.size(); ++i) {
                    if(i > 0) {
                        curr_cksum -= (uint8_t)s[i - 1];
                        curr_cksum += (uint8_t)s[i + sub.size() - 1];
                    }
                    if(
                        curr_cksum == sub_cksum &&
                        std::string_view{sub} == std::string_view{s.data() + i, sub.size()}
                    ) {
                        return i;
                    }
                }
                return -1;
            }
            int64_t find(std::wstring const &s, std::wstring const &sub, uint64_t starting_point = 0) {
                if(s.size() == 0 || sub.size() == 0 || s.size() < sub.size() + starting_point) { return -1; }
                uint64_t sub_cksum{cksum(std::wstring_view{sub})};
                uint64_t curr_cksum{cksum(std::wstring_view{s.data() + starting_point, sub.size()})};
                for(int64_t i{(int64_t)starting_point}; i <= (int64_t)s.size() - (int64_t)sub.size(); ++i) {
                    if(i > 0) {
                        curr_cksum -= (uint8_t)s[i - 1];
                        curr_cksum += (uint8_t)s[i + sub.size() - 1];
                    }
                    if(
                        curr_cksum == sub_cksum &&
                        std::wstring_view{sub} == std::wstring_view{s.data() + i, sub.size()}
                    ) {
                        return i;
                    }
                }
                return -1;
            }

        private:
            uint64_t cksum(std::string_view s) {
                uint64_t res{0};
                for(size_t i{}; i < s.size(); ++i) {
                    res += (std::make_unsigned_t<std::string_view::value_type>)s[i];
                }
                return res;
            }
            uint64_t cksum(std::wstring_view s) {
                uint64_t res{0};
                for(size_t i{}; i < s.size(); ++i) {
                    res += (std::make_unsigned_t<std::wstring_view::value_type>)s[i];
                }
                return res;
            }
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
