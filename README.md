# Simulator & Disassembler

RISC-V (rv32im) Simulator & Disassmbler.

## How to use

You can use this simulator by the following commands:

```sh
make install
./cpuex_sim <filename of RISC-V executable>
```

If you'd like to execute step by step, the following command will help you:

```sh
make install
./cpuex_sim <filename of RISC-V executable> --breakpoint 0 --debug
```

## Test

```sh
make
./test.sh
```

## Options

- `-i`: the file given to UART input
- `-o`: the file written by UART output
- `--breakpoint <addr in hex (e.g. FF)>` (or `-b <addr>`)
  - You can set multiple breakpoints.
  - e.g. `--breakpoint F0`
- `--debug`, `--info`
  - You can set debugging level.
- `--strict`
  - if this directive is set, this simulator quits when any errors occur.
- `--symbol <filename>`
  - if this directive is set, this simulator use it as a hint of function definition(s).
- `--statout <filename>`
  - if this directive is set, this simulator outputs statistics of the simulation at the end of execution.
