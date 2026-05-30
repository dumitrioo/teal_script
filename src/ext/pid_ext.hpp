#pragma once

#include "../inc/commondefs.hpp"
#include "../inc/str_util.hpp"

#include "../tealscript_value.hpp"
#include "../tealscript_util.hpp"
#include "../tealscript_interfaces.hpp"

namespace teal {

    class pid_ext: public extension_interface {
    public:
        pid_ext() = default;
        ~pid_ext() {
            unregister_runtime();
        }
        pid_ext(pid_ext const &) = delete;
        pid_ext &operator=(pid_ext const &) = delete;
        pid_ext(pid_ext &&) = delete;
        pid_ext &operator=(pid_ext &&) = delete;

        void register_runtime(runtime_interface *rt) override {
            std::unique_lock l{rt_mtp_};
            if(rt_ != nullptr) {
                return;
            }
            rt_ = rt;
            if(rt_ == nullptr) {
                return;
            }
            rt->add_function("pid_regulator", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 4, 5)
                if(args.size() == 4) {
                    return teal::valbox{std::make_shared<pid>(args[1].cast_to_double(), args[2].cast_to_double(), args[3].cast_to_double()), "pid_regulator"};
                } else {
                    return teal::valbox{std::make_shared<pid>(args[1].cast_to_double(), args[2].cast_to_double(), args[3].cast_to_double(), args[4].cast_to_double()), "pid_regulator"};
                }
            });
            rt->add_method("pid_regulator", "set_p", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_p(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_i", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_i(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_d", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_d(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_f", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_f(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_pid", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 4, 5)
                if(args.size() == 4) {
                    TEALTHIS(args, std::shared_ptr<pid>)->set_pid(args[1].cast_to_double(), args[2].cast_to_double(), args[3].cast_to_double());
                } else {
                    TEALTHIS(args, std::shared_ptr<pid>)->set_pid(args[1].cast_to_double(), args[2].cast_to_double(), args[3].cast_to_double(), args[4].cast_to_double());
                }
                return true;
            });
            rt->add_method("pid_regulator", "set_max_ioutput", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_max_ioutput(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_output_limits", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 2, 3)
                if(args.size() == 2) {
                    TEALTHIS(args, std::shared_ptr<pid>)->set_output_limits(args[1].cast_to_double());
                } else {
                    TEALTHIS(args, std::shared_ptr<pid>)->set_output_limits(args[1].cast_to_double(), args[2].cast_to_double());
                }
                return true;
            });
            rt->add_method("pid_regulator", "set_direction", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_direction(args[1].cast_to_bool());
                return true;
            });
            rt->add_method("pid_regulator", "set_setpoint", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_setpoint(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_setpoint_range", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_setpoint_range(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "reset", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 1)
                TEALTHIS(args, std::shared_ptr<pid>)->reset();
                return true;
            });
            rt->add_method("pid_regulator", "set_output_ramp_rate", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_output_ramp_rate(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "set_output_filter", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_EQ(args, 2)
                TEALTHIS(args, std::shared_ptr<pid>)->set_output_filter(args[1].cast_to_double());
                return true;
            });
            rt->add_method("pid_regulator", "get_output", TEALFUN(args) {
                TEAL_CHCK_FUN_PARMS_NUM_IN_RANGE(args, 1, 3)
                if(args.size() == 1) {
                    return TEALTHIS(args, std::shared_ptr<pid>)->get_output();
                } else if(args.size() == 2) {
                    return TEALTHIS(args, std::shared_ptr<pid>)->get_output(args[1].cast_to_double());
                } else {
                    return TEALTHIS(args, std::shared_ptr<pid>)->get_output(args[1].cast_to_double(), args[2].cast_to_double());
                }
            });
        }

        void unregister_runtime() override {
            std::unique_lock l{rt_mtp_};
            if(rt_ == nullptr) {
                return;
            }
            rt_->remove_function("pid_regulator");
            rt_->remove_method("pid_regulator", "set_p");
            rt_->remove_method("pid_regulator", "set_i");
            rt_->remove_method("pid_regulator", "set_d");
            rt_->remove_method("pid_regulator", "set_f");
            rt_->remove_method("pid_regulator", "set_pid");
            rt_->remove_method("pid_regulator", "set_max_ioutput");
            rt_->remove_method("pid_regulator", "set_output_limits");
            rt_->remove_method("pid_regulator", "set_direction");
            rt_->remove_method("pid_regulator", "set_setpoint");
            rt_->remove_method("pid_regulator", "set_setpoint_range");
            rt_->remove_method("pid_regulator", "reset");
            rt_->remove_method("pid_regulator", "set_output_ramp_rate");
            rt_->remove_method("pid_regulator", "set_output_filter");
            rt_->remove_method("pid_regulator", "get_output");
            rt_ = nullptr;
        }

    private:
        class pid {
        public:
            pid(double p, double i, double d) {
                init();
                P_ = p; I_ = i; D_ = d;
            }
            pid(double p, double i, double d, double f) {
                init();
                P_ = p; I_ = i; D_ = d; F_ = f;
            }
            void set_p(double p) {
                P_ = p;
                check_signs();
            }
            void set_i(double i) {
                if(I_ != 0) { errorSum_ = errorSum_ * I_ / i; }
                if(maxIOutput_ != 0) { maxError_ = maxIOutput_ / i; }
                I_ = i;
                check_signs();
            }
            void set_d(double d) {
                D_ = d;
                check_signs();
            }
            void set_f(double f) {
                F_ = f;
                check_signs();
            }
            void set_pid(double p, double i, double d) {
                P_ = p; I_ = i; D_ = d;
                check_signs();
            }
            void set_pid(double p, double i, double d,double f) {
                P_ = p; I_ = i; D_ = d; F_ = f;
                check_signs();
            }
            void set_max_ioutput(double maximum) {
                maxIOutput_ = maximum;
                if(I_ != 0) {
                    maxError_ = maxIOutput_ / I_;
                }
            }
            void set_output_limits(double output) {
                set_output_limits(-output,output);
            }
            void set_output_limits(double minimum,double maximum) {
                if(maximum<minimum) { return; }
                maxOutput_ = maximum;
                minOutput_ = minimum;
                if(maxIOutput_ == 0 || maxIOutput_ > (maximum - minimum)) {
                    set_max_ioutput(maximum-minimum);
                }
            }
            void set_direction(bool reversed) {
                reversed_=reversed;
            }
            void set_setpoint(double setpoint) {
                setpoint_=setpoint;
            }
            void reset() {
                firstRun_ = true;
                errorSum_ = 0;
            }
            void set_output_ramp_rate(double rate) {
                outputRampRate_ = rate;
            }
            void set_setpoint_range(double range) {
                setpointRange_ = range;
            }
            void set_output_filter(double strength) {
                if(strength==0 || bounded(strength,0,1)) {
                    outputFilter_ = strength;
                }
            }
            double get_output() {
                return get_output(lastActual_, setpoint_);
            }
            double get_output(double actual) {
                return get_output(actual, setpoint_);
            }
            double get_output(double actual, double setpoint) {
                double output;
                double Poutput;
                double Ioutput;
                double Doutput;
                double Foutput;

                setpoint_ = setpoint;
                if(setpointRange_!= 0) {
                    setpoint = clamp(setpoint, actual-setpointRange_, actual + setpointRange_);
                }
                double error = setpoint - actual;
                Foutput = F_ * setpoint;
                Poutput = P_ * error;
                if(firstRun_) {
                    lastActual_ = actual;
                    lastOutput_ = Poutput + Foutput;
                    firstRun_ = false;
                }
                Doutput = -D_ * (actual - lastActual_);
                lastActual_ = actual;
                Ioutput = I_ * errorSum_;
                if(maxIOutput_ != 0) {
                    Ioutput = clamp(Ioutput, -maxIOutput_, maxIOutput_);
                }
                output = Foutput + Poutput + Ioutput + Doutput;
                if(minOutput_ != maxOutput_ && !bounded(output, minOutput_, maxOutput_)) {
                    errorSum_ = error;
                }
                else if(outputRampRate_ != 0 && !bounded(output, lastOutput_ - outputRampRate_, lastOutput_ + outputRampRate_)) {
                    errorSum_ = error;
                }
                else if(maxIOutput_!=0) {
                    errorSum_ = clamp(errorSum_ + error, -maxError_, maxError_);
                }
                else{
                    errorSum_ += error;
                }
                if(outputRampRate_ != 0) {
                    output = clamp(output, lastOutput_-outputRampRate_,lastOutput_+outputRampRate_);
                }
                if(minOutput_ != maxOutput_) {
                    output = clamp(output, minOutput_, maxOutput_);
                }
                if(outputFilter_ != 0) {
                    output = lastOutput_ * outputFilter_ + output * (1 - outputFilter_);
                }

                lastOutput_=output;
                return output;
            }

        private:
            static double clamp(double value, double mini, double maxi) {
                if(value > maxi) { return maxi; }
                if(value < mini) { return mini; }
                return value;
            }
            static bool bounded(double value, double min, double max) {
                return (min<value) && (value<max);
            }
            void check_signs() {
                if(reversed_) {	// all values should be below zero
                    if(P_ > 0) { P_ *= -1; }
                    if(I_ > 0) { I_ *= -1; }
                    if(D_ > 0) { D_ *= -1; }
                    if(F_ > 0) { F_ *= -1; }
                }
                else{	// all values should be above zero
                    if(P_ < 0) { P_ *= -1; }
                    if(I_ < 0) { I_ *= -1; }
                    if(D_ < 0) { D_ *= -1; }
                    if(F_ < 0) { F_ *= -1; }
                }
            }
            void init() {
                P_ = 0;
                I_ = 0;
                D_ = 0;
                F_ = 0;
                maxIOutput_ = 0;
                maxError_ = 0;
                errorSum_ = 0;
                maxOutput_ = 0;
                minOutput_ = 0;
                setpoint_ = 0;
                lastActual_ = 0;
                firstRun_ = true;
                reversed_ = false;
                outputRampRate_ = 0;
                lastOutput_ = 0;
                outputFilter_ = 0;
                setpointRange_ = 0;
            }
            double P_;
            double I_;
            double D_;
            double F_;

            double maxIOutput_;
            double maxError_;
            double errorSum_;

            double maxOutput_;
            double minOutput_;

            double setpoint_;

            double lastActual_;

            bool firstRun_;
            bool reversed_;

            double outputRampRate_;
            double lastOutput_;

            double outputFilter_;

            double setpointRange_;
        };

    private:
        shared_mutex rt_mtp_{};
        runtime_interface *rt_{nullptr};
    };

}
