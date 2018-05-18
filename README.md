# Dynzasm

| | |
|----------|--------------------------------|
|**TRAVIS**|[![Build Status](https://travis-ci.org/Mithreindeir/Dynzasm.svg?branch=master)](https://travis-ci.org/Mithreindeir/Dynzasm)|
|**Coverity**|[![Coverity Scan Build Status](https://scan.coverity.com/projects/15646/badge.svg)](https://scan.coverity.com/projects/mithreindeir-dynzasm)|

Interactive Disassembly Library WIP

Currently supports x86/x86_64/MIPS disassembling.

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
