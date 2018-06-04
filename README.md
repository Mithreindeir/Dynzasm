# Dynzasm
[![Build Status](https://travis-ci.org/Mithreindeir/Dynzasm.svg?branch=master)](https://travis-ci.org/Mithreindeir/Dynzasm)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/15646/badge.svg)](https://scan.coverity.com/projects/mithreindeir-dynzasm)

Dynzasm is a fast lightweight disassembly library written in c99 code with no external dependencies. Disassembly is structured as trees with arbitrary formatting strings, allowing detailed disassembly information and making it easy to support custom syntaxes. 

| ARCH | Disassembly SUPPORT | Assembler Support |
|-----|----------|-----------|
|X86| Most (excluding extensions) | Partial (No fp, or isa ext)|
|X64| Most (excluding extensions)| Partial (No fp, or isa ext)|
|ARM| Partial| None (WIP)|
|MIPS| Most | None (WIP)|

Includes sample commandline utility
```bash
./dynzasm --help
Usage: ./dynzasm options filename
	--arch=<architecture> Set architecture to be disassembled (x86, arm, or mips
	--mode=<mode> Set the architecture mode (32 or 64)
	--entry=<addr> Set a starting address
	-a convert ascii to hex
	-A Assemble
If no file is specified stdin will be used
Must specify architecture and mode
```
```bash
echo "55 48 89 e5 48 83 ec 70" | ./dynzasm --arch=x86 --mode=64 -a --addr=0x2172 
0x002172:	55                            	push	rbp
0x002173:	48 89 e5                      	mov	rbp, rsp
0x002176:	48 83 ec 70                   	sub	rsp, 0x70

```

An example of using the assembler from stdin, and piping it into the disassembler.

```bash
./dynzasm --arch=x86 --mode=64 -A | ./dynzasm --arch=x86 --mode=64 -a
push rbp
mov rbp, rsp
mov eax, 0
ret
00000000:	55                            	push	rbp
0x000001:	48 8b ec                      	mov	rbp, rsp
0x000004:	b8 00 00 00 00                	mov	eax, 0
0x000009:	c3                            	ret
```

It is also very easy to use as a library. Detailed semantics from disassembly for easy analysis coming soon.

```C
#include "disas.h"

int main()
{
	
  struct disassembler *ds = ds_init(X86_ARCH, MODE_64B);
  unsigned char bytes[] =  "\x55\x48\x89\xe5\xb8\x00\x00\x00\x00\xc3";

  ds_decode(ds, bytes, sizeof(bytes)-1, 0x0);
  struct dis *dis = NULL;
  
  DS_FOREACH(ds, dis) {
    printf("%#08lx:\t%s\t%s\n", dis->address, dis->mnemonic, dis->op_squash);
  }
  
  ds_destroy(ds);
  return 0;
}

```
