elf-utils
=========

ELF related utilities

libelf is required. (Ubuntu package: libelf-dev )

elfaddsymbol: Adds an "undefined" dynamic symbol that will be resolved at link time.
  - Primary usage of this is to add hooks into a dlopen()'d library that are 
    actually implemented in an LD_PRELOAD library.
