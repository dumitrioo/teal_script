# TealScript

TealScript is a lightweight, distributed, and deterministic control logic engine - free from heavy message brokers (ROS/MQTT) and garbage collection overhead, with native C++ integration. 

It is an embedded scripting language designed to seamlessly wire physical and virtual devices into a unified control system. The language enforces strict, declarative rules for building schemas, making complex logic highly readable and maintainable. Combined with seamless C++ host extensibility, the result is a deterministic, cohesive hardware-software ecosystem driven entirely by data-flow graphs.


## Stateful Data-Flow

While based on the discrete-time, clocked data-flow paradigm (similar to Unreal Engine Blueprints), TealScript departs from strict functional purity. Each compute node is an instance of an object that syntactically resembles a function, but can retain state between execution cycles via instance variables accessible through the this keyword. This makes writing complex state machines or PID controllers as easy as writing simple functions.


## Why TealScript?

When designing control logic, it is tempting to specify what the outcome should be, rather than script how to achieve it step-by-step. While C++ is imperative and requires detailed architectural design, TealScript allows you to broaden your programming approaches without changing your C++ toolchain. 

You get a problem-specific tool to handle complex control schemes for multiple actuators based on numerous sensor signals, drastically reducing the low-level C++ boilerplate required for wiring, state management, and multi-threading, while keeping full native extensibility.

## Key Features

 * Network-Agnostic Distributed Graphs: Seamlessly link variables across different hosts using extern URIs. Built on a custom UDP multiplexing protocol (MTU-safe 1400 bytes) that eliminates Head-of-Line blocking without the overhead of TCP or heavy brokers like MQTT.
 * True Multi-Threading: Execute graph schemas in parallel across available CPU cores. The interpreter safely handles node execution without requiring the user to manage C++ threads or locks.
 * Zero Dependencies & Portable: Implemented as a custom execution tree interpreter (no LLVM/external lexers). It compiles into any C++20 codebase via CMake and is completely hardware-agnostic.
 * Turing Complete & Extensible: Handle general-purpose tasks, math (vec4, mat4), JSON, and custom C++ types. Easily inject host functions into the scripting runtime.


## Quick Example

A logical schema written in TealScript manages physical actuators in parallel by analyzing signals from input devices:

``` TealScript
// Node 1: Smooth balancer
balance_pid(angle, ang_vel) {
    // 1. Low-Pass Filter
    this.smooth_vel = 0.85 * this.smooth_vel + 0.15 * ang_vel;

    // 2. Accelerator lower, brakes on
    p_term = 25.0 * angle;
    d_term = -35.0 * this.smooth_vel;

    // 3. Rid of integral
    out = p_term + d_term;

    return out;
}

// Node 2: Lazy centering
center_pid(cart_pos, cart_vel) {
    return -0.5 * cart_pos - 1.0 * cart_vel;
}

// Node 3: "Soft wall"
soft_wall(cart_pos) {
    wall_force = 0.0;
    limit = 4.5;

    if (cart_pos > limit) {
        wall_force = -150.0 * (cart_pos - limit);
    } else if (cart_pos < -limit) {
        wall_force = -150.0 * (cart_pos + limit);
    }

    return wall_force;
}

// Node 4
mixer(balance_force, center_force, wall_force) {
    out = balance_force + center_force + wall_force;
    if (out > 80.0) out = 80.0;
    if (out < -80.0) out = -80.0;
    return out;
}

// ---------------------------------------------------------
// The Graph
// ---------------------------------------------------------
'sensor_angle' sensor_angle;
'sensor_vel' sensor_vel;
'cart_position' cart_position;
'cart_velocity' cart_velocity;
balance_pid balancer(sensor_angle, sensor_vel);
center_pid centerer(cart_position, cart_velocity);
soft_wall wall(cart_position);
mixer motor_control(balancer, centerer, wall) 'motor_force';
```

## Application & Use Cases

TealScript excels at "sense → compute → act" pipelines: 

 * Robotics & Autonomous Systems: High-throughput signal processing, sensor fusion, and driving actuators in real time.
 * Industrial Automation: Replacing heavy PLC logic with portable C++ integrations.
 * Edge Computing / IoT: Orchestration of distributed controllers without centralized message brokers.
 * Simulations & Digital Twins: Logical linking of separately attached hardware or virtual devices.
     

## Usage & Integration

Requires a C++20 compatible compiler.

1. Include the header file: #include "tealscript_runtime.hpp" 
2. Instantiate the TealScript runtime object. 
3. Load source code and register C++ extensions (functions, variables, custom types). 
4. Execute in single- or multi-threaded mode. 

To build the examples (on Linux): 
``` bash 
mkdir build && cd build
cmake ..
make
```

Explore the [examples](examples/), directory for advanced use cases like the ALU 74181 hardware simulation  and host-side C++ extensions. 


## More Information

For the complete language specification, type system, and host API reference, see the [Documentation](doc/tealscript_overview.pdf). (PDF - Migrating to Markdown docs is in progress).
