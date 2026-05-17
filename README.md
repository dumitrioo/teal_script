# TealScript

TealScript is an embedded scripting engine. Lightweight, easy to integrate, intended to create control logic. Distributed, but free from heavy message brokers (ROS/MQTT). Free from garbage collection overhead, with native C++ integration, designed to seamlessly wire physical and virtual devices into a unified control system. The language in data-flow graph paradigm enforces clear declarative rules for building schemas, making complex logic highly readable and maintainable. Combined with seamless C++ host extensibility, the result is a cohesive hardware-software ecosystem driven entirely by data-flow graphs.


## Data-Flow Graph

While based on the data-centric discrete-time, clocked data-flow static declarative stream functional programming paradigm, TealScript departs from strict functional purity. Each compute node is an instance of an object that syntactically resembles a function, but can retain state between execution cycles via instance variables accessible through the "this" keyword. This makes writing complex state machines or PID controllers as easy as writing simple functions.


## Why TealScript?

When designing control logic, it is tempting to specify what the outcome should be, rather than describe how to achieve it step-by-step in detail. While C++ is imperative and requires detailed architectural design, TealScript allows you to broaden your programming approaches without changing your C++ toolchain. 

You get a problem-specific tool to handle complex control schemes for multiple actuators based on numerous sensor signals, drastically reducing the low-level C++ boilerplate required for wiring, state management, and multi-threading, while keeping full native extensibility.


## Key Features
 * Intuitive C-like sytax, conciseness and readability of the program, most of C math functions provided.
  * Turing Complete & Extensible: Handle general-purpose tasks, system interaction, math (functions, matrices), JSON, and custom host-provided types. Easily inject host functions into the scripting runtime. Simple integration, well defined rules of host-script interaction.
 * Partially Stateful: DFG computation nodes are able to store a state between execution cycles which gives the ability to perform imperative work inside the node while the overall structure of the program is static declarative.
 * Uniform Function Call Syntax (UFCS), where __func(obj, arg)__ call is fully equivalent of __obj.func(arg)__.
 * True Multi-Threading: Execute graph schemas in parallel across available CPU cores. The interpreter safely handles node execution without requiring the user to manage C++ threads or locks.
 * Zero Dependencies & Portable: Implemented as a custom execution tree interpreter (no LLVM/external lexers). It compiles into any C++20 codebase via CMake and is completely hardware-agnostic.
 * Network-Agnostic Distributed Graphs: Seamlessly link variables across different hosts using extern URIs. Built on a custom UDP multiplexing protocol (MTU-safe 1400 bytes) that eliminates Head-of-Line blocking without the overhead of TCP or heavy brokers like MQTT (see [example script](examples/external_value.teal)).


## Quick Example

A logical schema written in TealScript manages physical actuators in parallel by analyzing signals from input devices:

```TealScript
pass(v) return v;

// Configuration for host code
pass friction_force(.5) 'friction_force';
pass cart_mass(1.) 'cart_mass';
pass pend_mass(.3) 'pend_mass';
pass start_force_impulse(1.) 'start_force_impulse';

balance_pid(angle, dt) { // PID balancing
    if(this.p == undefined) {
        this.p = 0.0;
        this.i = 0.0;
    }
    prev_ang = this.p;
    this.p = angle;
    this.i = dt * this.i + angle;
    d = (angle - prev_ang) / dt;
    return 60.0 * this.p + 20.0 * this.i + 20.0 * d;
}

center_pid(cart_pos, cart_vel) { // Lazy centering
    return abs(cart_pos) < 2.0
        ?
            4.0 * sqrt(abs(cart_pos)) * sign(cart_pos) +
            4.0 * sqrt(abs(cart_vel)) * sign(cart_vel)
        :
            2.0 * sign(cart_pos) + 2.0 * sign(cart_vel);
}

soft_wall(cp) { // "Soft wall"
    return abs(cp) > 4.5 ? sign(cp) * 8.0 : 0.0;
}

mixer(balance_force, center_force, wall_force) { // Mix & clamp
    v = balance_force + center_force + wall_force;
    d = 80.0;
    return v < -d ? -d : v > d ? d : v;
}

// ---------------------------------------------------------
// The Graph
// ---------------------------------------------------------

// inputs from host code
'dt' dt;
'ang' angle;
'cart_pos' cart_pos;
'cart_vel' cart_vel;

// workers
balance_pid balancer(angle, dt);
center_pid centerer(cart_pos, cart_vel);
soft_wall wall(cart_pos);
mixer motor_control(balancer, centerer, wall) 'motor_force';
```

[![Watch the video](https://github.com/dumitrioo/tealscript/blob/main/resources/pid_demo.png)](https://github.com/dumitrioo/tealscript/blob/main/resources/pid_demo.mp4)

https://github.com/dumitrioo/tealscript/blob/main/resources/pid_demo.mp4

[Example application](examples/pendulum/main.cpp)

## Another Example

How would you implement the following logic circuit in a regular programming language?

![Logic cirquit for 74181](examples/alu74181.png)

And while you are thinking, I will offer you a [solution](examples/alu74181.teal) in a TealScript programming language.

```TealScript
// ---------------------------------------------------------
// logic gates simulation with
// TealScript computation nodes templates
// ---------------------------------------------------------
not(a) { return !a; }
fwd(a) { return (bool)a; }
and2(a, b) { return a && b; }
and3(a, b, c) { return a && b && c; }
and4(a, b, c, d) { return a && b && c && d; }
and5(a, b, c, d, e) { return a && b && c && d && e; }
nand2(a, b) { return !(a && b); }
nand3(a, b, c) { return !(a && b && c); }
nand4(a, b, c, d) { return !(a && b && c && d); }
nand5(a, b, c, d, e) { return !(a && b && c && d && e); }
or2(a, b) { return a || b; }
or3(a, b, c) { return a || b || c; }
or4(a, b, c, d) { return a || b || c || d; }
nor2(a, b) { return !(a || b); }
nor3(a, b, c) { return !(a || b || c); }
nor4(a, b, c, d) { return !(a || b || c || d); }
xor2(a, b) { return a ^ b; }
xor3(a, b, c) { return a ^ b ^ c; }
xor4(a, b, c, d) { return a ^ b ^ c ^ d; }
xnor2(a, b) { return !(a ^ b); }
xnor3(a, b, c) { return !(a ^ b ^ c); }
xnor4(a, b, c, d) { return !(a ^ b ^ c ^ d); }
i2or2(a, b) { return !a || !b; }

// ---------------------------------------------------------
// The Graph
// ---------------------------------------------------------

// inputs --------------------------------------------------
'A0' a0;
'A1' a1;
'A2' a2;
'A3' a3;
'B0' b0;
'B1' b1;
'B2' b2;
'B3' b3;
'S0' s0;
'S1' s1;
'S2' s2;
'S3' s3;
'C_in' c_in;
'M' m;

// workers -------------------------------------------------
not   alu_0(b3);
not   alu_1(b2);
not   alu_2(b1);
not   alu_3(b0);
not   alu_4(m);
and3  alu_5(b3, s3, a3);
and3  alu_6(a3, s2, alu_0);
and2  alu_7(alu_0, s1);
and2  alu_8(s0, b3);
fwd   alu_9(a3);
and3  alu_10(b2, s3, a2);
and3  alu_11(a2, s2, alu_1);
and2  alu_12(alu_1, s1);
and2  alu_13(s0, b2);
fwd   alu_14(a2);
and3  alu_15(b1, s3, a1);
and3  alu_16(a1, s2, alu_2);
and2  alu_17(alu_2, s1);
and2  alu_18(s0, b1);
fwd   alu_19(a1);
and3  alu_20(b0, s3, a0);
and3  alu_21(a0, s2, alu_3);
and2  alu_22(alu_3, s1);
and2  alu_23(s0, b0);
fwd   alu_24(a0);
nor2  alu_25(alu_5, alu_6);
nor3  alu_26(alu_7, alu_8, alu_9);
nor2  alu_27(alu_10, alu_11);
nor3  alu_28(alu_12, alu_13, alu_14);
nor2  alu_29(alu_15, alu_16);
nor3  alu_30(alu_17, alu_18, alu_19);
nor2  alu_31(alu_20, alu_21);
nor3  alu_32(alu_22, alu_23, alu_24);
xor2  alu_33(alu_25, alu_26);
xor2  alu_34(alu_27, alu_28);
xor2  alu_35(alu_29, alu_30);
xor2  alu_36(alu_31, alu_32);
fwd   alu_37(alu_26);
and2  alu_38(alu_25, alu_28);
and3  alu_39(alu_25, alu_27, alu_30);
and4  alu_40(alu_25, alu_27, alu_29, alu_32);
nand5 alu_41(alu_25, alu_27, alu_32, alu_31, c_in);
nand4 alu_42(alu_25, alu_27, alu_32, alu_31)          'P';
and5  alu_43(c_in, alu_31, alu_32, alu_27, alu_4);
and4  alu_44(alu_32, alu_27, alu_32, alu_4);
and3  alu_45(alu_27, alu_30, alu_4);
and2  alu_46(alu_28, alu_4);
and4  alu_47(c_in, alu_31, alu_29, alu_4);
and3  alu_48(alu_29, alu_32, alu_4);
and2  alu_49(alu_30, alu_4);
and3  alu_50(c_in, alu_31, alu_4);
and2  alu_51(alu_32, alu_4);
nand2 alu_52(c_in, alu_4);
nor4  alu_53(alu_37, alu_38, alu_39, alu_40)          'G';
nor4  alu_54(alu_43, alu_44, alu_45, alu_46);
nor3  alu_55(alu_47, alu_48, alu_49);
nor2  alu_56(alu_50, alu_51);
i2or2 alu_57(alu_53, alu_41)                          'C_out';
xor2  alu_58(alu_33, alu_54)                          'f3';
xor2  alu_59(alu_34, alu_55)                          'f2';
xor2  alu_60(alu_35, alu_56)                          'f1';
xor2  alu_61(alu_36, alu_52)                          'f0';
and4  alu_62(alu_58, alu_59, alu_60, alu_61)          'EQ';
```

As you can see, the graph is composed of computation node instances whose implementations described above that graph. And the program itself turned out to be short, understandable and covering the logical circuit one to one.


## Distributed Example

Guess what? Nothing special, just spliting up our application into two parts communicating via the network. To make this possible, we have to turn the network server mode on. After that, ALL the computation nodes of the locally running script are automatically visible from the network. Ok, lets go ahead.

### Server part

```TealScript
setup_cell() {
    if(!external_values_server_enabled()) {
        if(!enable_external_values_server("0.0.0.0", 43987)) {
            throw error_str(last_error());
        }
    }
    return external_values_server_enabled();
}
setup_cell setup();

// ---------------------------------------------------------
// logic gates simulation with
// TealScript computation nodes templates
// ---------------------------------------------------------
not(a) { return !a; }
fwd(a) { return (bool)a; }
and2(a, b) { return a && b; }
and3(a, b, c) { return a && b && c; }
and4(a, b, c, d) { return a && b && c && d; }
and5(a, b, c, d, e) { return a && b && c && d && e; }
nand2(a, b) { return !(a && b); }
nand3(a, b, c) { return !(a && b && c); }
nand4(a, b, c, d) { return !(a && b && c && d); }
nand5(a, b, c, d, e) { return !(a && b && c && d && e); }
or2(a, b) { return a || b; }
or3(a, b, c) { return a || b || c; }
or4(a, b, c, d) { return a || b || c || d; }
nor2(a, b) { return !(a || b); }
nor3(a, b, c) { return !(a || b || c); }
nor4(a, b, c, d) { return !(a || b || c || d); }
xor2(a, b) { return a ^ b; }
xor3(a, b, c) { return a ^ b ^ c; }
xor4(a, b, c, d) { return a ^ b ^ c ^ d; }
xnor2(a, b) { return !(a ^ b); }
xnor3(a, b, c) { return !(a ^ b ^ c); }
xnor4(a, b, c, d) { return !(a ^ b ^ c ^ d); }
i2or2(a, b) { return !a || !b; }

// ---------------------------------------------------------
// The Graph
// ---------------------------------------------------------

// inputs --------------------------------------------------
'A0' a0;
'A1' a1;
'A2' a2;
'A3' a3;
'B0' b0;
'B1' b1;
'B2' b2;
'B3' b3;
'S0' s0;
'S1' s1;
'S2' s2;
'S3' s3;
'C_in' c_in;
'M' m;

// workers -------------------------------------------------
not   alu_0(b3);
not   alu_1(b2);
not   alu_2(b1);
not   alu_3(b0);
not   alu_4(m);
and3  alu_5(b3, s3, a3);
and3  alu_6(a3, s2, alu_0);
and2  alu_7(alu_0, s1);
and2  alu_8(s0, b3);
fwd   alu_9(a3);
and3  alu_10(b2, s3, a2);
and3  alu_11(a2, s2, alu_1);
and2  alu_12(alu_1, s1);
and2  alu_13(s0, b2);
fwd   alu_14(a2);
and3  alu_15(b1, s3, a1);
and3  alu_16(a1, s2, alu_2);
and2  alu_17(alu_2, s1);
and2  alu_18(s0, b1);
fwd   alu_19(a1);
and3  alu_20(b0, s3, a0);
and3  alu_21(a0, s2, alu_3);
and2  alu_22(alu_3, s1);
and2  alu_23(s0, b0);
fwd   alu_24(a0);
nor2  alu_25(alu_5, alu_6);
nor3  alu_26(alu_7, alu_8, alu_9);
nor2  alu_27(alu_10, alu_11);
nor3  alu_28(alu_12, alu_13, alu_14);
nor2  alu_29(alu_15, alu_16);
nor3  alu_30(alu_17, alu_18, alu_19);
nor2  alu_31(alu_20, alu_21);
nor3  alu_32(alu_22, alu_23, alu_24);
xor2  alu_33(alu_25, alu_26);
xor2  alu_34(alu_27, alu_28);
xor2  alu_35(alu_29, alu_30);
xor2  alu_36(alu_31, alu_32);
fwd   alu_37(alu_26);
and2  alu_38(alu_25, alu_28);
and3  alu_39(alu_25, alu_27, alu_30);
and4  alu_40(alu_25, alu_27, alu_29, alu_32);
nand5 alu_41(alu_25, alu_27, alu_32, alu_31, c_in);
nand4 alu_42(alu_25, alu_27, alu_32, alu_31)          'P';
and5  alu_43(c_in, alu_31, alu_32, alu_27, alu_4);
and4  alu_44(alu_32, alu_27, alu_32, alu_4);
and3  alu_45(alu_27, alu_30, alu_4);
and2  alu_46(alu_28, alu_4);
and4  alu_47(c_in, alu_31, alu_29, alu_4);
and3  alu_48(alu_29, alu_32, alu_4);
and2  alu_49(alu_30, alu_4);
and3  alu_50(c_in, alu_31, alu_4);
and2  alu_51(alu_32, alu_4);
nand2 alu_52(c_in, alu_4);
nor4  alu_53(alu_37, alu_38, alu_39, alu_40)          'G';
nor4  alu_54(alu_43, alu_44, alu_45, alu_46);
nor3  alu_55(alu_47, alu_48, alu_49);
nor2  alu_56(alu_50, alu_51);
i2or2 alu_57(alu_53, alu_41)                          'C_out';
xor2  alu_58(alu_33, alu_54)                          'f3';
xor2  alu_59(alu_34, alu_55)                          'f2';
xor2  alu_60(alu_35, alu_56)                          'f1';
xor2  alu_61(alu_36, alu_52)                          'f0';
and4  alu_62(alu_58, alu_59, alu_60, alu_61)          'EQ';
```


### Client part

```TealScript
/////////////////////////////////////////////////////////////////////////////////
//                             ALU 74181 view (renderer)
/////////////////////////////////////////////////////////////////////////////////
extern 'tealscript://localhost:43987/s0' s0;
extern 'tealscript://localhost:43987/s1' s1;
extern 'tealscript://localhost:43987/s2' s2;
extern 'tealscript://localhost:43987/s3' s3;
extern 'tealscript://localhost:43987/a0' a0;
extern 'tealscript://localhost:43987/a1' a1;
extern 'tealscript://localhost:43987/a2' a2;
extern 'tealscript://localhost:43987/a3' a3;
extern 'tealscript://localhost:43987/b0' b0;
extern 'tealscript://localhost:43987/b1' b1;
extern 'tealscript://localhost:43987/b2' b2;
extern 'tealscript://localhost:43987/b3' b3;
extern 'tealscript://localhost:43987/c_in' c_in;
extern 'tealscript://localhost:43987/m' m;
extern 'tealscript://localhost:43987/alu_61' alu_61;
extern 'tealscript://localhost:43987/alu_60' alu_60;
extern 'tealscript://localhost:43987/alu_59' alu_59;
extern 'tealscript://localhost:43987/alu_58' alu_58;
extern 'tealscript://localhost:43987/alu_57' alu_57;
extern 'tealscript://localhost:43987/alu_42' alu_42;
extern 'tealscript://localhost:43987/alu_53' alu_53;
extern 'tealscript://localhost:43987/alu_62' alu_62;

///////////////////////// display
ray_window() {
    if(!this.win_created) {
        ray_init_window(800, 600, "ALU 74181");
        ray_set_target_fps(30);
        ray_begin_drawing();
        ray_clear_background(Color(0, 0, 0, 255));
        this.win_created = true;
        this.tid = thread_id();
    }
    if(this.tid == thread_id()) {
        if(ray_window_should_close()) {
            ray_end_drawing();
            ray_close_window();
            exit(0);
        } else {
            if(ray_get_key_pressed() == 300) ray_toggle_full_screen();
            ray_end_drawing();
            ray_begin_drawing();
        }
    }
    return this.tid;
}
color_select() return Color(0x29, 0xae, 0x9cc, 255);
color_in() return Color(0x64, 0xff, 0x9f, 255);
color_out() return Color(0x1e, 0xed, 0x00, 255);
color_carry() return Color(0xfe, 0x24, 0x02, 255);
color_pgeq() return Color(0x9e, 0xdb, 0x00, 255);
draw_bool(t, v, x, y, f, lbl, crcclr) {
    if(t == thread_id()) {
        if(ray_window_should_close()) { ray_close_window(); exit(0); return 0; }
        if(ray_get_key_pressed() == 300) { ray_toggle_full_screen(); }
        rctclr = Color(64, 64, 64, 255);
        rcdclr = Color(32, 32, 32, 255);
        ray_draw_circle(x * f, y * f, 7 * f, v ? crcclr : Color(0, 0, 0, 255));
        ray_draw_text(lbl, (x - 9) * f, (y + 11) * f, 15 * f, Color(80, 200, 100, 255));
        ray_draw_line((x - 10.5) * f, (y - 10) * f, (x + 10) * f, (y - 10) * f, rctclr);
        ray_draw_line((x - 10) * f, (y - 10) * f, (x - 10) * f, (y + 10) * f, rctclr);
        ray_draw_line((x + 10.5) * f, (y + 10) * f, (x + 10) * f, (y - 10) * f, rcdclr);
        ray_draw_line((x + 10) * f, (y + 10) * f, (x - 10) * f, (y + 10) * f, rcdclr);
    }
    return 0;
}

forward(v) return v;

ray_window ray();
forward sfctr(2.6);
color_select clr_s();
color_in clr_in();
color_out clr_out();
color_carry clr_c();
color_pgeq clr_pgeq();
draw_bool dr_c_in(ray, c_in, 200, 20, sfctr, "inC", clr_c);
draw_bool dr_m(ray, m, 240, 20, sfctr, " M", clr_in);
draw_bool dr_s0(ray, s3, 20, 20, sfctr, "s3", clr_s);
draw_bool dr_s1(ray, s2, 60, 20, sfctr, "s2", clr_s);
draw_bool dr_s2(ray, s1, 100, 20, sfctr, "s1", clr_s);
draw_bool dr_s3(ray, s0, 140, 20, sfctr, "s0", clr_s);
draw_bool dr_a0(ray, a3, 20, 60, sfctr, "a3", clr_in);
draw_bool dr_a1(ray, a2, 60, 60, sfctr, "a2", clr_in);
draw_bool dr_a2(ray, a1, 100, 60, sfctr, "a1", clr_in);
draw_bool dr_a3(ray, a0, 140, 60, sfctr, "a0", clr_in);
draw_bool dr_b0(ray, b3, 20, 100, sfctr, "b3", clr_in);
draw_bool dr_b1(ray, b2, 60, 100, sfctr, "b2", clr_in);
draw_bool dr_b2(ray, b1, 100, 100, sfctr, "b1", clr_in);
draw_bool dr_b3(ray, b0, 140, 100, sfctr, "b0", clr_in);
draw_bool dr_f3(ray, alu_61, 20, 160, sfctr, "f3", clr_out);
draw_bool dr_f2(ray, alu_60, 60, 160, sfctr, "f2", clr_out);
draw_bool dr_f1(ray, alu_59, 100, 160, sfctr, "f1", clr_out);
draw_bool dr_f0(ray, alu_58, 140, 160, sfctr, "f0", clr_out);
draw_bool dr_c_out(ray, alu_57, 200, 160, sfctr, "outC", clr_c);
draw_bool dr_p(ray, alu_42, 20, 200, sfctr, " P", clr_pgeq);
draw_bool dr_g(ray, alu_53, 60, 200, sfctr, " G", clr_pgeq);
draw_bool dr_AeB(ray, alu_62, 120, 200, sfctr, "EQ", clr_pgeq);
```

In this example, we can see a several external nodes declarations:

```
extern 'tealscript://localhost:43987/s0' s0;
extern 'tealscript://localhost:43987/s1' s1;
extern 'tealscript://localhost:43987/s2' s2;
...
...
```

These declarations describe the remote host/port/node using the "extern" keyword and a URL, and give a local identifier to each node (which may be the same or different from the remote node). In this example, "localhost" is used for demonstration purposes, but on real systems the host address can be any real host name on the Internet. On the client side, when using external nodes, no additional effort is required to access them, unlike the server side, where, in fact, you need to start the network server from a script. To avoid the need to launch the network server from the script side, this can be done in the host code, when embedding the engine and starting runtime.


## Application & Use Cases

TealScript excels at "sense → compute → act" pipelines: 

 * Robotics & Autonomous Systems: High-throughput signal processing, sensor fusion, and driving actuators in real time.
 * Industrial Automation: Replacing heavy PLC logic with portable C++ integrations.
 * Edge Computing / IoT: Orchestration of distributed controllers without centralized message brokers.
 * Simulations & Digital Twins: Logical linking of separately attached hardware or virtual devices.
     

## Usage & Integration

The usage of library is as simple as following:

1. Include [tealscript_runtime.hpp](src/tealscript_runtime.hpp) header file
2. Instantiate the TealScript runtime object and load scripting sources (from strings or files) into the runtime.
3. Optionally, add your custom C++ functions, variables, types (if any) into the runtime instance.
4. Execute runtime in single/multi-threaded mode exchanging data.


```C++
#include <tealscript_runtime.hpp>

// This C++ code is a host part related to the script example shown earlier

int main(int argc, char **argv) {
    if(argc < 2) {
        return 0;
    }

    // tealscript runtime instance 
    teal::runtime rt{};

    // loading script file passed from command line
    rt.load_file(argv[1]);
    rt.loading_complete();
    
    ...
    ...
    
    while(simulationInProgress) {
        ...
        ...
        // Sensors update
        rt.set_input("dt", dt);
        rt.set_input("ang", angle);
        rt.set_input("cart_pos", cart_x);
        rt.set_input("cart_vel", cart_vel);

        // Compute in the same thread (single-threaded execution)
        rt.run_cycle();
        
        // Fetch controllong value from runtime
        force = rt.get_output("motor_force").cast_to_double();
        
        // Apply script results
        cartBody->applyCentralForce(btVector3(force, 0, 0));
        ...
        ...
    }

    return 0;
}
```

To build the examples (on Linux): 
``` bash 
mkdir build && cd build
cmake ..
make
```

Or use a provided shell script

``` bash 
git clone https://github.com/dumitrioo/tealscript.git
cd tealscript
./build_linux_cmake.sh
```

Explore the [examples](examples/), directory for advanced use cases like the ALU 74181 hardware simulation  and host-side C++ extensions. 


## More Information

For the complete language specification, type system, and host API reference, see the [Documentation](doc/tealscript_overview.pdf). (PDF - Migrating to Markdown docs is in progress).
