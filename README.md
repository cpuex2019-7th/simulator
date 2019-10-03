# Simulator

RISC-V Simulator.

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

- `-i`: to be implemented in the future
- `-o`: to be implemented in the future
- `--breakpoint <addr in hex (e.g. FF)>` (or `-b <addr>`)
  - You can set multiple breakpoints.
  - e.g. `--breakpoint F0`
- `--debug`, `--info`
  - 
- `--strict`
  - if this directive is set, this simulator quits when any errors occur.
