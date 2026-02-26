#  Welcome to SCAFlux

SCAFlux is a dynamically typed scripting language in the declarative Data-Flow Graph paradigm. Unlike conventional programming languages, program in this language does not have a "single entry point". Instead, programmer writes logic for each individual compute element and connects that elements together accordingly to a logical schema, which is a solution of certain problem, in order to perform needed computations resolving that problem. All the compute elements are executed independently and may be simultaneously (in case of multi-threaded execution mode). And every time the the compute element is done, it is executed again and again and so on, with updated input parameters from other elements outputs.
The class of problems, easily solvable with this embedded language, is a controlling tasks for multitude of actuators/indicators/displays/other signalling data consumers based on analyzing the incoming signals set from multitude of the inputs within logical schema noted above.

# Implementation

Proposed implementation is an embedded library written in C++ and designed to be used in couple with C++. To add support for SCAFlux into your C++ application, you need to add include directories inside your project and include header file. And after that you can instantiate SCAFlux runtime object, load source code and extensions shared libraries into it and execute in single- or multi-threaded mode. To extend embedded languages with functions, variables and objects, you should add respective entities using rules described in the document mentioned below.
You can also explore the [interpreter](interpreter/main.cpp) application source code as the example of embedding a library and extending of the language's functionality. Note that interpreted language can call functions and use objects provided by host C++ code but you can not call script functions from the C++ code. 

# Example

SCAFlux language was crated to easy solve a problems that are not as easy to solve with "conventional" languages. For example, lets try to implement logic for ALU 74181 integrated circuit.

![](examples/alu74181.png)
_The combinational logic circuitry of the 74181 integrated circuit_

```
//////////////////////////////// cells templates ////////////////////////////////
not(a) { slp(); return !a; }
buf(a) { slp(); return bool(a); }
and2(a, b) { slp(); return a && b; }
and3(a, b, c) { slp(); return a && b && c; }
and4(a, b, c, d) { slp(); return a && b && c && d; }
and5(a, b, c, d, e) { slp(); return a && b && c && d && e; }
nand2(a, b) { slp(); return !(a && b); }
nand3(a, b, c) { slp(); return !(a && b && c); }
nand4(a, b, c, d) { slp(); return !(a && b && c && d); }
nand5(a, b, c, d, e) { slp(); return !(a && b && c && d && e); }
or2(a, b) { slp(); return a || b; }
or3(a, b, c) { slp(); return a || b || c; }
or4(a, b, c, d) { slp(); return a || b || c || d; }
nor2(a, b) { slp(); return !(a || b); }
nor3(a, b, c) { slp(); return !(a || b || c); }
nor4(a, b, c, d) { slp(); return !(a || b || c || d); }
xor2(a, b) { slp(); return a ^ b; }
xor3(a, b, c) { slp(); return a ^ b ^ c; }
xor4(a, b, c, d) { slp(); return a ^ b ^ c ^ d; }
xnor2(a, b) { slp(); return !(a ^ b); }
xnor3(a, b, c) { slp(); return !(a ^ b ^ c); }
xnor4(a, b, c, d) { slp(); return !(a ^ b ^ c ^ d); }
i2or2(a, b) { slp(); return !a || !b; }

//////////////////////////////// instances ////////////////////////////////

///////////////////////// inputs
'A0' a0; 'A1' a1; 'A2' a2; 'A3' a3;
'B0' b0; 'B1' b1; 'B2' b2; 'B3' b3;
'S0' s0; 'S1' s1; 'S2' s2; 'S3' s3;
'C_in' c_in; 'M' m;

///////////////////////// workers
not   alu_0(b3);
not   alu_1(b2);
not   alu_2(b1);
not   alu_3(b0);
not   alu_4(m);
and3  alu_5(b3, s3, a3);
and3  alu_6(a3, s2, alu_0);
and2  alu_7(alu_0, s1);
and2  alu_8(s0, b3);
buf   alu_9(a3);
and3  alu_10(b2, s3, a2);
and3  alu_11(a2, s2, alu_1);
and2  alu_12(alu_1, s1);
and2  alu_13(s0, b2);
buf   alu_14(a2);
and3  alu_15(b1, s3, a1);
and3  alu_16(a1, s2, alu_2);
and2  alu_17(alu_2, s1);
and2  alu_18(s0, b1);
buf   alu_19(a1);
and3  alu_20(b0, s3, a0);
and3  alu_21(a0, s2, alu_3);
and2  alu_22(alu_3, s1);
and2  alu_23(s0, b0);
buf   alu_24(a0);
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
buf   alu_37(alu_26);
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

And that's it. All we need to do now is to set input values and read outputs. You may think that SCAFlux is a language for modelling cirquit logic but this is just one example of using the language. Read document from the next section to know more about what this language is an how to use it.

# More information

Please, read this [document](doc/scaflux_overview.pdf) for more information...
