Where does this code come from
===============================
Some of the source code in this repo was written originally by the owner of
the repo while the rest was adapted and/or modified from other contributors
See the individual files to see the copyright(s) and/or disclaimer(s)

What is this code
==================
This repo contains some basic algorithms written in C language
The primary objectives are
- the code does NOT allocate or free memory. The user is reponsible for
  managing all pointers such as the root of a tree or binary heap
- in all trees, the node of the tree is part of the structure that is
  being inserted instead of a being separately allocated by the library
- We use regular Makefile instead of Automalke

The good thing about this memory approach is that
- We reduce the probability of fragmentation
- the memory can be totally managed by the user and hence can use any kind
  of memory (e.g. HW or file mapped memory

There are few drawbacks
- The node of the tree must now be part of the user's data
- User must manage the memory completely


How to build
=============
- go to the directory where you cloned the workspace
- To build everything
  make
  or
  make all
  This will generate the libraries and the test suites in obj/

- To build shared library ONLY
   make libcalgo.so

- To build static library only
   make libcalgo.a

- To build test suites
  make test
