# Dynzasm
Interactive Disassembly Library.

Instruction represented are trees that can be "squashed" into strings
allowing arbitrary formatting rules to rewrite the disassembly, while also providing implicit information about all used registers, immediate values, and addresses used by the instruction.

Currently an x86 disassembler is being developed but aiming to be architecture independent.

Trie structure for holding instruction sets.

Will eventually have builtin formatting engine allowing any tree structure to be rewritten as anything (eg: dword [ebp-0x4] can be rewritten as var4.d).

Current progress:

* X86 disassembler supports most instructions
* Trie Structure
* Supports loading instructions from file into trie, with flag operations
* Disassemble from stdin

