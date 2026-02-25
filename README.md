# SCAFlux

SCAFlux is a programming language in the declarative Data-Flow Graph paradigm. Unlike conventional programming languages, program in this language does not have a "single entry point". Instead, programmer writes logic for each individual compute element and connects that elements together in order to perform needed computations.

# Implementation

Proposed implementation is an embedded library written in C++ and designed to be used in couple with C++. To add support for SCAFlux into your C++ application, you need to add include directories inside your project and include header file. And after that you can instantiate SCAFlux runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend embedded languages with functions, variables and objects, you should add respective entities using rules described in the document mentioned below.

# More information

Please, read this [document](doc/scaflux_overview.pdf) for more information...
