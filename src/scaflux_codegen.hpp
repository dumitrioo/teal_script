#pragma once

#include "include/commondefs.hpp"
#include "include/str_util.hpp"
#include "include/json.hpp"

#include "scaflux_util.hpp"
#include "scaflux_token.hpp"
#include "scaflux_expr.hpp"
#include "scaflux_statement.hpp"
#include "scaflux_cells.hpp"

namespace scfx {

    class code_generator {
    public:
        void chop(
            scfx::json const &ast,
            map_t<std::string, input_cell> &input_cells,
            map_t<std::string, std::string> &input_names_to_instances_mapping,
            map_t<std::string, worker_cell_definition_info> &worker_cells_templates,
            map_t<std::string, worker_cell_instance> &worker_cells,
            map_t<std::string, statement_ptr> &worker_bodies,
            map_t<std::string, function_definition> &user_functions,
            dict_map_t<std::string, valbox> const &global_functions_dictionary
        ) {
            for(std::size_t i = 0; i < ast.size(); ++i) {
                scfx::json const &cur{ast[i]};
                if(cur["subtype"].as_string() == "cell_definition") {
                    scfx::json const &cur_cnt{cur["content"]};
                    if(worker_cells_templates.find(cur_cnt["cell_name"].as_string()) != worker_cells_templates.end()) {
                        throw compilation_error{
                            cur["loc"]["line"].try_as_number(),
                            cur["loc"]["col"].try_as_number(),
                            cur_cnt["cell_name"].as_string() + ": duplicated cell template identifier"
                        };
                    }
                    statement_ptr cb{chop_statement(cur_cnt["cell_body"])};
                    worker_cell_definition_info wc{};
                    wc.set_loc(cur["loc"]["line"].try_as_number(), cur["loc"]["col"].try_as_number());
                    for(std::size_t ai = 0; ai < cur_cnt["arg_names"].size(); ++ai) {
                        wc.set_arg_name(ai, cur_cnt["arg_names"][ai].as_string());
                    }
                    wc.set_type_name(cur_cnt["cell_name"].as_string());
                    cb->skip_frame_creation();
                    worker_bodies[wc.type_name()] = cb;
                    worker_cells_templates[wc.type_name()] = wc;
                } else if(cur["subtype"].as_string() == "cell_inst") {
                    scfx::json const &cur_cnt{cur["content"]};
                    if(array_contains_str(cur_cnt["cell_flags"], "input")) {
                        std::string cnm{cur_cnt["cell_name"].as_string()};
                        std::string inm{cur_cnt["input_name"].as_string()};
                        if(worker_cells.find(cnm) != worker_cells.end() || input_cells.find(cnm) != input_cells.end()) {
                            throw compilation_error{
                                cur["loc"]["line"].try_as_number(),
                                cur["loc"]["col"].try_as_number(),
                                cnm + ": duplicated cell identifier"
                            };
                        }
                        input_cell &ic{input_cells[cnm]};
                        ic.set_input_name(inm);
                        ic.set_inst_name(cnm);
                        input_names_to_instances_mapping[inm] = cnm;
                        ic.set_loc(cur["loc"]["line"].try_as_number(), cur["loc"]["col"].try_as_number());
                    } else if(array_contains_str(cur_cnt["cell_flags"], "regular")) {
                        std::string cnm{cur_cnt["cell_name"].as_string()};
                        if(worker_cells.find(cnm) != worker_cells.end() || input_cells.find(cnm) != input_cells.end()) {
                            throw compilation_error{
                                cur["loc"]["line"].try_as_number(),
                                cur["loc"]["col"].try_as_number(),
                                cnm + ": duplicated cell identifier"
                            };
                        }
                        worker_cell_instance &wci{worker_cells[cnm]};
                        wci.set_loc(cur["loc"]["line"].try_as_number(), cur["loc"]["col"].try_as_number());
                        wci.set_inst_name(cnm);
                        wci.set_type_name(cur_cnt["cell_type"].as_string());
                        if(cur_cnt["args"].key_exists("content")) {
                            for(std::size_t ai = 0; ai < cur_cnt["args"]["content"].size(); ++ai) {
                                scfx::json const &arg_cnt{cur_cnt["args"]["content"][ai]};
                                if(arg_cnt["subtype"].as_string() == "identifier") {
                                    wci.set_act_arg_source(ai, arg_cnt["content"].as_string());
                                } else {
                                    wci.set_act_arg_expr(ai, chop_expression(arg_cnt));
                                }
                            }
                        }
                        if(array_contains_str(cur_cnt["cell_flags"], "output")) {
                            wci.set_output_name(cur_cnt["output_name"].as_string());
                        }
                    }
                } else if(cur["subtype"].as_string() == "function_definition") {
                    scfx::json const &cur_cnt{cur["content"]};
                    std::string func_name{cur_cnt["function_name"].as_string()};
                    if(
                        user_functions.find(func_name) != user_functions.end() ||
                        global_functions_dictionary.find(func_name) != global_functions_dictionary.end()
                    ) {
                        throw compilation_error{
                            cur["loc"]["line"].try_as_number(),
                            cur["loc"]["col"].try_as_number(),
                            "symbol already exists"
                        };
                    }
                    statement_ptr cb{chop_statement(cur_cnt["function_body"])};
                    function_definition fn{};
                    fn.set_loc(cur["loc"]["line"].try_as_number(), cur["loc"]["col"].try_as_number());
                    fn.set_name(func_name);
                    fn.set_body(cb);
                    for(std::size_t ai = 0; ai < cur_cnt["arg_names"].size(); ++ai) {
                        fn.set_arg_name(ai, cur_cnt["arg_names"][ai].as_string());
                    }
                    user_functions[func_name] = std::move(fn);
                }
            }
        }

    private:
        static bool array_contains_str(scfx::json const &arr, std::string const &s) {
            if(!arr.is_array()) {
                return false;
            }
            for(std::size_t i{}; i < arr.size(); ++i) {
                if(arr[i].as_string() == s) {
                    return true;
                }
            }
            return false;
        }

        statement_ptr chop_statement(scfx::json const &ast) {
            statement_ptr res{};
            if(
                ast.is_object() &&
                ast.key_exists("type") &&
                ast["type"].as_string() == "statement"
            ) {
                if(ast["subtype"].as_string() == "compound") {
                    res = chop_compound_statement(ast);
                } else if(ast["subtype"].as_string() == "empty") {
                    res = std::make_shared<statement_empty>();
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "expression") {
                    res = chop_expression_statement(ast);
                } else if(ast["subtype"].as_string() == "while") {
                    res = std::make_shared<statement_while>(
                        chop_expression(ast["content"]["cond"]),
                        chop_statement(ast["content"]["statement"])
                    );
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "for") {
                    res = std::make_shared<statement_for>(
                        chop_expression(ast["content"]["init"]),
                        chop_expression(ast["content"]["cond"]),
                        chop_expression(ast["content"]["incr"]),
                        chop_statement(ast["content"]["statement"])
                    );
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "if") {
                    res = std::make_shared<statement_if_else>(
                        chop_expression(ast["content"]["cond"]),
                        chop_statement(ast["content"]["then_statement"]),
                        chop_statement(ast["content"]["else_statement"])
                    );
                } else if(ast["subtype"].as_string() == "throw") {
                    res = std::make_shared<statement_throw>(
                        chop_expression(ast["content"])
                    );
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "try") {
                    res = std::make_shared<statement_try_catch>(
                        chop_statement(ast["content"]["try_statement"]),
                        chop_expression(ast["content"]["catch_expression"]),
                        chop_statement(ast["content"]["catch_statement"])
                    );
                } else if(ast["subtype"].as_string() == "return") {
                    res = std::make_shared<statement_return>(
                        chop_expression(ast["content"])
                    );
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "break") {
                    res = std::make_shared<statement_break>();
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["subtype"].as_string() == "continue") {
                    res = std::make_shared<statement_continue>();
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                }
            }
            return res;
        }

        statement_ptr chop_compound_statement(scfx::json const &ast) {
            statement_ptr res{std::make_shared<statement_compound>()};
            res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            for(std::size_t i = 0; i < ast["content"].size(); ++i) {
                static_cast<statement_compound *>(res.get())->push_back(
                    chop_statement(ast["content"][i])
                );
            }
            if(static_cast<statement_compound *>(res.get())->num_substatements() == 0) {
                return std::make_shared<statement_empty>();
            } else if(static_cast<statement_compound *>(res.get())->num_substatements() == 1) {
                return static_cast<statement_compound *>(res.get())->get_tatement_at(0);
            }
            return res;
        }

        statement_ptr chop_expression_statement(scfx::json const &ast) {
            auto res {std::make_shared<statement_expr>(chop_expression(ast["content"]))};
            res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            return res;
        }

        expr_ptr chop_expression(scfx::json const &ast) {
            expr_ptr res{};
            static std::set<std::string> const hobi{"hex", "oct", "bin", "int"};
            if(ast.is_null()) {
                return std::make_shared<void_expression>();
            }
            if(ast["subtype"].as_string() == "binop") {
                scfx::json const &cnt{ast["content"]};
                if(cnt["left"].is_null() || cnt["right"].is_null()) {
                    throw compilation_error{ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number(),
                        std::string{"invalid expression"}
                    };
                }
                res = std::make_shared<binop_expression>(
                    static_cast<token::type>(cnt["oper_enum"].as_int()),
                    chop_expression(cnt["left"]),
                    chop_expression(cnt["right"])
                );
                res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            } else if(ast["subtype"].as_string() == "ternop") {
                scfx::json const &cnt{ast["content"]};
                if(cnt["condition"].is_null() || cnt["true_expr"].is_null() || cnt["false_expr"].is_null()) {
                    throw compilation_error{ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number(),
                        std::string{"invalid expression"}
                    };
                }
                if(static_cast<token::type>(cnt["oper_enum"].as_int(-1)) == token::type::QUESTION) {
                    res = std::make_shared<ternop_expression>(
                        chop_expression(cnt["condition"]),
                        chop_expression(cnt["true_expr"]),
                        chop_expression(cnt["false_expr"])
                    );
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                }
            } else if(ast["subtype"].as_string() == "prefix") {
                scfx::json const &cnt{ast["content"]};
                if(cnt["operand"].is_null() || cnt["oper_enum"].is_null()) {
                    throw compilation_error{ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number(),
                        std::string{"invalid expression"}
                    };
                }
                res = std::make_shared<prefix_unop_expression>(
                    static_cast<token::type>(cnt["oper_enum"].as_int()),
                    chop_expression(cnt["operand"])
                );
                res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            } else if(ast["subtype"].as_string() == "postfix") {
                scfx::json const &cnt{ast["content"]};
                if(cnt["operand"].is_null() || cnt["oper_enum"].is_null()) {
                    throw compilation_error{ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number(),
                        std::string{"invalid expression"}
                    };
                }
                res = std::make_shared<postfix_unop_expression>(
                    chop_expression(cnt["operand"]),
                    static_cast<token::type>(cnt["oper_enum"].as_int())
                );
                res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            } else if(ast["subtype"].as_string() == "identifier") {
                scfx::json const &cnt{ast["content"]};
                res = std::make_shared<sym_expression>(cnt.as_string());
                res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            } else if(ast["subtype"].as_string() == "literal") {
                scfx::json const &cnt{ast["content"]};
                if(ast["literal"].as_string() == "flt") {
                    res = std::make_shared<primary_expression>(cnt.as_longdouble());
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(
                    hobi.find(ast["literal"].as_string()) != hobi.end()
                ) {
                    res = std::make_shared<primary_expression>(cnt.as_number());
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["literal"].as_string() == "bool") {
                    res = std::make_shared<primary_expression>(cnt.as_boolean());
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["literal"].as_string() == "undefined") {
                    res = std::make_shared<primary_expression>(valbox{valbox_no_initialize::dont_do_it});
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["literal"].as_string() == "str") {
                    res = std::make_shared<primary_expression>(cnt.as_string());
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                } else if(ast["literal"].as_string() == "chr") {
                    std::wstring chr_str{scfx::str_util::from_utf8(cnt.as_string())};
                    if(chr_str.size() == 1) {
                        if(chr_str[0] < 256) {
                            res = std::make_shared<primary_expression>((char)chr_str[0]);
                        } else {
                            res = std::make_shared<primary_expression>(chr_str[0]);
                        }
                    } else if(chr_str.size() > 1) {
                        uint32_t c{};
                        int pos{(int)(chr_str.size() - 1)};
                        for(size_t i{0}; i < 4 && pos >= 0; ++i) {
                            int cc{(std::uint8_t)chr_str[pos]};
                            c |= cc << (i * 8);
                            --pos;
                        }
                        res = std::make_shared<primary_expression>((wchar_t)c);
                    } else {
                        throw compilation_error{
                            ast["loc"]["line"].try_as_number(),
                                           ast["loc"]["col"].try_as_number(),
                                           "invalid character"
                        };
                    }
                    res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
                }
            } else if(ast["subtype"].as_string() == "func_call") {
                scfx::json const &cnt{ast["content"]};
                if(cnt["func"].is_null()) {
                    throw compilation_error{ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number(),
                        std::string{"invalid expression"}
                    };
                }
                res = std::make_shared<func_call_expression>(
                    chop_expression(cnt["func"]),
                    func_call_args(cnt["args"])
                );
                res->set_loc(ast["loc"]["line"].try_as_number(), ast["loc"]["col"].try_as_number());
            }
            return res;
        }

        std::vector<expr_ptr> func_call_args(scfx::json const &ast) {
            std::vector<expr_ptr> res{};
            for(std::size_t i = 0; i < ast.size(); ++i) {
                res.push_back(chop_expression(ast[i]));
            }
            return res;
        }
    };

}
