#  Welcome to SCAFlux

SCAFlux is a dynamically typed scripting language in the declarative, pure functional Data-flow Graph Paradigm, but SCAFlux breaks this functional purity and is partialy statelful, in favor of convenience while resolving many tasks. Unlike conventional programming languages, program in this language does not have a single entry point. Instead, programmer writes logic by defining a bunch (or a lot) of compute elements connected into a computation network accordingly to a logical schema that is a solution of the certain problem, in order to perform necessary computations resolving that problem. All the compute elements are executed independently (simultaneously in case of multi-threaded execution mode), and every time the compute element is done, it is executed (with updated input parameters coming from other elements outputs) again and again and again and so on, until the system stops working.

The class of problems, that can be easily solved by this scripting language, is the complex real-time control of multitude actuators/indicators/displays/etc. within a logical circuit, based on the analysis of incoming signals from multitude input devices. However, the language's capabilities also allow it to solve problems typically solved by Turing-complete languages, as its type system, language constructs, and set of operations make this scripting language Turing-complete. The language's extensibility from host code also enhances its ability to solve the diverse tasks assigned to scripting engines.

# Implementation

Proposed implementation is an interpreter accomplished as an embedded library without dependencies written in C++ and designed to be used in couple with C++.

To add support for SCAFlux into your C++ application, you need to include header file, instantiate SCAFlux runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend the scripting language with functions, variables and objects, you should add entities separately or in form of extension to this runtime object, using rules described in the document mentioned below, in "More information" section.
You can also explore the [interpreter](examples/interpreter/main.cpp) application source code as the example of embedding a library and extending of the language's functionality. 

# More information

Please, read this [document](doc/scaflux_overview.pdf) for more information...
