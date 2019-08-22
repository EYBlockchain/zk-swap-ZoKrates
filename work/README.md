# ZoKrates EY Customization

## Specific features

- Support `MNT4`/`MNT6` curves for PGHR13 (Pinnochio) protocol
- Add a `verify` command to check a proof (VK and proof as input
- Add a `batch` command to aggregate two PGHR13 proofs (MNT4 and MNT6 curves)

## How to do

### Build the project

Before starting, you need to have
- Working Rust distribution (nightly)
- The convenient tools to build [libsnark](https://github.com/scipr-lab/libsnark#build-instructions)

```
$ git clone git@github.com:EYBlockchain/zk-swap-ZoKrates.git ZoKrates
$ cd ZoKrates
$ git submodule init && git submodule update
$ (cd deps/libsnark && git submodule init && git submodule update)
$ cd work
$ ./do build
```

### Run an example

This is a step by step script to generate an aggregated proof:
```
$ ./do clean                            # remove old experiences results
$ ZOKRATES_CURVE=MNT4 ./do compile      # compile basic circuit (Private ^ 2 == Public)
$ ZOKRATES_CURVE=MNT4 ./do setup        # generate setup
$ ZOKRATES_CURVE=MNT4 ./do witness 3 9  # generate witness
$ ZOKRATES_CURVE=MNT4 ./do generate     # generate proof
$ ZOKRATES_CURVE=MNT4 ./do verify       # verify proof
$ ./do move 1                           # create a folder 1 with VK and proof
$ ./do move 2                           # create a folder 2 with VK and proof
$ ./do clean                            # remove old experiences results
$ ./do batch MNT4 MNT6 1 2              # generate the aggregated proof
$ ZOKRATES_CURVE=MNT6 ./do verify       # verify the aggregated proof
```

## Caution and Warning

Please note this is still a work in progress:
- All the added features have been tested for specific cases
- The API is not stable
- Shortcuts have been taken to accelerate delivery (duplicate output files to used binary import for example)
- The batching circuit has been used "as is" for now, we are still working on a more deep understanding of this feature (There still crashes on some proofs and false positive verifications)

## Credits

- [Zokrates/ZoKrates](https://github.com/Zokrates/ZoKrates)
- [scipr-lab/libsnark](https://github.com/scipr-lab/libsnark)
- [AlexandreBelling/boojum](https://github.com/AlexandreBelling/boojum)
