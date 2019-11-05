# Simulator & Disassembler

RISC-V (rv32im) Simulator & Disassmbler.

## How to use

You can use this simulator by the following commands:

```sh
make install
cpuex_sim <filename of RISC-V executable>
```

If you'd like to execute step by step, the following command will help you:

```sh
make install
cpuex_sim <filename of RISC-V executable> --debug
```

If you want to disassemble the binary, you can do it with following commands.

```
make install
cpuex_disasm <filename of RISC-V executable> # quietly
cpuex_disasm <filename of RISC-V executable> -v # verbosely
```

When you have symbol information `cpuex_asm` omits, you can use it as follows.

```
make install
cpuex_disasm <filename of RISC-V executable> --symbols <.symbols file> -vv
```

## Test

```sh
make
./test.sh
```

## Options

- `-i`: the file given to UART input
- `-o`: the file written by UART output
- `--debug`, `--info`
  - You can set debugging level.
- `--strict`
  - if this directive is set, this simulator quits when any errors occur.
- `--symbols <filename>`
  - if this directive is set, this simulator use it as a hint of function definition(s).
- `--statout <filename>`
  - if this directive is set, this simulator outputs statistics of the simulation at the end of execution.
