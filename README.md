# Welcome to TealScript

TealScript is a high‑throughput signal processing engine: ingest sensor data, perform intelligent analysis, and emit control commands in real time. Optimized for embedded systems, autonomous systems, and distributed controllers. TealScript built-in language is a dynamically typed, embedded, extensible from the host code scripting language in the Data-flow Graph Paradigm intended to make the process of creation a complex control scenarios extremely easy.

This is a "sense → compute → act" system that combines reliable telemetry ingestion, preprocessing, and configurable decision engines. Its architecture is built for parallel processing and high throughput: streaming pipelines, optimized filters, and an asynchronous parallel dispatcher for output commands ensure performance in demanding real-world environments. 

The platform supports flexible input formats, allowing integration with virtually any data source, extended analysis (filtering, event detection, ML inference), and multiple output channels for driving actuators and networked controllers. TealScript integrates easily at source code level becoming a part of telemetry, remote monitoring and controlling workflows.

The engine was designed for straightforward digital integration with real and virtual devices via a data-exchange protocol, clear rules for building control schemas, and simple extension mechanisms for the language from the host-side code. The result is a cohesive hardware-software system that runs according to schemas written in the embedded language - developing those schemas is a pleasure.

The engine is implemented as an execution tree interpreter and does not rely on third-party tools for lexical analysis, parsing, or code generation, giving it a very small memory footprint. Because source code is compiled into an execution tree rather than machine code, the library is hardware-agnostic and portable to any system with a C++ compiler supporting standards 17 and higher and CMake build system.

TealScript, while based on the data-flow graph paradigm, departs from strict functional purity being partialy statelful for convenience of solving many tasks. Each compute node (cell or element) is an instance of object that syntactically resembles a function definition but can retain state between execution cycles via instance's variables accessible through the "this" keyword.

The system can run synchronously in single-threaded mode or asynchronously in multi-threaded mode, using as many processor cores as needed from available amount of CPU cores. Performance of code written in the embedded language is comparable to Python for single-threaded workloads; however, multi-threaded execution (the most common and natural mode of the system) can yield substantially higher overall throughput.


## Why?

When designing any control logic, it is tempting to specify only what the final outcome should be, rather than provide detailed, step‑by‑step instructions for how to achieve it. The functional programming paradigm, in a sense, aligns with this declarative approach to problem solving. Although C++ has acquired some rudimentary functional features, the core language remains imperative and retains built‑in support for object‑oriented programming. That means C++ development typically requires architects and programmers to design the system in detail and to script the entire computational process down to the smallest steps.

At the same time, it is desirable to have a dedicated tool or abstraction for each problem domain while continuing to use familiar development tools. In other words, we would like to broaden the range of programming approaches available without changing our toolchain. This goal has long been achieved by combining application source code with additional tooling in the form of libraries or inline source modules implemented using the language itself. Employing specialized components to solve domain‑specific problems is common practice.

Here we present another tool that significantly simplifies building complex control schemes for many actuators based on processing numerous sensor signals via programming. Although there is a current trend toward coupling artificial neural networks directly with hardware for control purposes, part of any application will inevitably remain explicitly programmed—for example, to provide integration points. Adopting the proposed solution will substantially ease the manual development of low‑level, programmatically expressed control logic and will also facilitate integration with machine‑learning products.

So, the answer to the question "why" is: to have a problem-specific tool and use it instead of C++ where it make sense.


## The Data-flow Graph Paradigm

The data-flow graph paradigm (discrete-time, clocked, modular, data-centric) is a declarative, purely functional approach. Unlike imperative languages, programs in this paradigm has no single entry point. Instead, the developer describes a set of compute elements (also called "cells" or "nodes") wired together into a computation network according to a logical schema that represents the solution to a given problem. Each compute element runs independently - possibly concurrently in multi-threaded mode - and whenever an element finishes it re-executes (with updated inputs which are outputs from other elements). This cycle continues until the system is stopped. Good examples of some of the data-flow graph paradigm concepts adoptions are artificial neural networks and Unreal Engine Blueprints.


## Application

This scripting language excels at complex real-time control logic: logical schemas written in the language can manage many physical or software actuators, indicators, and displays in parallel by analyzing signals from numerous input devices. The language is also Turing-complete - its type system, constructs, and operations enable it to handle tasks normally solved by general-purpose languages. Extensibility from host code further broadens its applicability.

Possible applications for this engine include:

 * Automating systems that combine embedded computers, electronics, sensors, actuators, and monitoring.
 * Logically linking separately attached hardware or virtual devices.
 * Adding programmatic intelligence to simpler components.
 * Parallel analysis and consolidation of related input data from many sources.


## Usage

To add support for TealScript into your C++ application, you need at least C++20 standard capable compiler. Include [header file](src/tealscript_runtime.hpp) from your C++ source file, then instantiate TealScript runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend the scripting language with functions, variables and objects, you should add entities separately or in form of extension to this runtime object, using rules described in the document mentioned below, in "More information" section. You can explore the [interpreter](examples/interpreter/main.cpp) application source code and scripts [extending_example](examples/extending_example.teal), [alu74181](examples/alu74181.teal), [tbbt_2cola](examples/tbbt_2cola.teal), [example](examples/example.teal), [draft](examples/draft.teal) as the examples of embedding a library and extending of the language's functionality. 

Note: In order to run some of the examples, you should install Raylib and Zeromq libraries before the build in the Linux environment.


## Demos and Examples

To build example application, CMake script is provided. In addition, for Linux users there is a Shell scripts for building and running some of the examples...


# More information

For more information, read this [PDF document](doc/tealscript_overview.pdf)
