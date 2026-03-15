# Welcome to SCAFlux

SCAFlux is a dynamically typed, embedded, extensible from the host code scripting language in the Data-flow Graph Paradigm intended to make the process of creation a complex control systems extremely easy.

## Why?

The advantage of C++ is that it can solve almost any development task. And this is also its problem. C++ programmers typically use it for absolutely everything in their work, ignoring the fact that object-oriented and imperative approaches are unsuitable for certain tasks. There is a class of tasks that are much easier to solve using specialized tools. One such tool and a description of the tasks it can solve are presented here...

## The Data-flow Graph Paradigm

The Data-flow Graph paradigm (discrete time, clocked synchronous, modular, data-centric) is a declarative pure functional paradigm. Unlike imperative programming languages, program in this language does not have a single entry point. Instead, programmer writes logic by defining a set of compute elements (cells, nodes) connected to each other into a computation network accordingly to a logical schema that is a solution of the certain problem, in order to perform necessary computations resolving that problem. All the compute elements are executed independently (simultaneously in case of multi-threaded execution mode), and every time the compute element is done, it is executed (with updated input parameters coming from other elements outputs) again and again and again and so on, until the system stops working.

## Application

One of the problems classes, that can be easily solved by this scripting language, is the complex real-time control of multitude actuators/indicators/displays/etc. within a logical circuit, based on the analysis of incoming signals from multitude input devices. However, the language's capabilities also allow it to solve problems that are typically solved by the Turing-complete languages, as its type system, language constructs, and set of operations make this scripting language Turing-complete. The language's extensibility from host code also enhances its ability to solve the diverse tasks assigned to scripting engines.

Possible application for this engine may include:
 * Easy automation of any systems containing embedded computers and electronics together with sensors, actuator units and monitoring system.
 * Logical interconnecting between separately attached hardware/virtual devices.
 * Programmatic functionality extension for simpler components (adding intelligence into simple systems).
 * Parallel analysis of somehow dependant input data from many sources (logical information consolidation).

## Implementation

The library is implemented as an execution tree interpreter, without dependencies on third-party software for source code translation (lexical analyze, parsing, code generation), and therefore has an extremely small memory footprint. Moreover, since the source code is converted not into machine code but into an execution tree, the library is not tied to any hardware architecture, being portable.

SCAFlux language breaks the functional purity of DFG paradigm being partialy statelful in favor of convenience while resolving many tasks. Every compute node (cell, element) is an instance of the "template" - a data transforming object that looks like a function but is an "object" type and is able to keep state between calls in form of instance's variables available through "this" keyword.

The system can operate in a synchronous single threaded mode and in asynchronous multi threaded mode utilizing all the processor cores of the system on which it is running. In terms of performance, a program in the embedded language is similar to a program in Python; however, given the ability to work in multi-threaded mode, the overall resulting performance can be significantly higher.

## Usage

To add support for SCAFlux into your C++ application, you need at least C++20 standard capable compiler. Include [header file](src/scaflux_runtime.hpp) from your C++ source file, then instantiate SCAFlux runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend the scripting language with functions, variables and objects, you should add entities separately or in form of extension to this runtime object, using rules described in the document mentioned below, in "More information" section. You can explore the [interpreter](examples/interpreter/main.cpp) application source code and scripts [extending_example](examples/extending_example.scfx), [alu74181](examples/alu74181.scfx), [tbbt_2cola](examples/tbbt_2cola.scfx), [example](examples/example.scfx), [draft](examples/draft.scfx) as the examples of embedding a library and extending of the language's functionality. 

# More information

For more information, read this [PDF document](doc/scaflux_overview.pdf)
