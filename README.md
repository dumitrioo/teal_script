#  Welcome to SCAFlux

SCAFlux is a dynamically typed scripting language in the declarative Data-Flow Graph paradigm, which is a pure functional, but SCAFlux breaks this rule and is partialy statelful in favor of more convenience resolving many tasks. Unlike conventional programming languages, program in this language does not have a "single entry point". Instead, programmer writes logic for each individual compute element and connects that elements together accordingly to a logical schema, which is a solution of certain problem, in order to perform needed computations resolving that problem. All the compute elements are executed independently and may be simultaneously (in case of multi-threaded execution mode). And every time the the compute element is done, it is executed again and again and so on, with updated input parameters from other elements outputs.
The class of problems, easily solvable with this embedded language, is a controlling tasks for multitude of actuators/indicators/displays/other signalling data consumers based on analyzing the incoming signals set from multitude of the inputs within logical schema noted above.

# Implementation

Proposed implementation is an embedded library written in C++ and designed to be used in couple with C++. To add support for SCAFlux into your C++ application, you need to add include directories inside your project and include header file. And after that you can instantiate SCAFlux runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend embedded languages with functions, variables and objects, you should add respective entities using rules described in the document mentioned below.
You can also explore the [interpreter](interpreter/main.cpp) application source code as the example of embedding a library and extending of the language's functionality. Note that interpreted language can call functions and use objects provided by host C++ code but you can not call script functions from the C++ code. 

# More information

Please, read this [document](doc/scaflux_overview.pdf) for more information...
