# Dynzasm
[![Build Status](https://travis-ci.org/Mithreindeir/Dynzasm.svg?branch=master)](https://travis-ci.org/Mithreindeir/Dynzasm)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/15646/badge.svg)](https://scan.coverity.com/projects/mithreindeir-dynzasm)

Dynzasm is a fast lightweight disassembly library written in c99 code with no external dependencies. Disassembly is structured as trees with arbitrary formatting strings, allowing detailed disassembly information and making it easy to support custom syntaxes. 

| ARCH | SUPPORT |
|-----|----------|
|X86| Most (excluding extensions) |
|X64| Most (excluding extensions)|
|ARM| Partial|
|MIPS| Most |

Includes sample commandline utility
```bash
./dynzasm --help
Usage: ./dynzasm options filename
	--arch=<architecture> Set architecture to be disassembled (x86, arm, or mips
	--mode=<mode> Set the architecture mode (32 or 64)
	--entry=<addr> Set a starting address
	-a convert ascii to hex
If no file is specified stdin will be used
Must specify architecture and mode
```
```bash
echo "55 48 89 e5 48 83 ec 70" | ./dynzasm --arch=x86 --mode=64 -a --addr=0x2172 
0x002172:	55                            	push	rbp
0x002173:	48 89 e5                      	mov	rbp, rsp
0x002176:	48 83 ec 70                   	sub	rsp, 0x70

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
