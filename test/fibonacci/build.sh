rm -f test.ll
rm -f testx.ll
rm -f testx.o
rm -f debug.o
rm -f test_secure
rm -f test_insecure
rm -f craft.o
rm -f external.o
riscv64-unknown-elf-clang -c -S -O2 -emit-llvm test.c
riscv64-unknown-elf-clang -c -S -O2 -emit-llvm debug.c
opt -S -load ~/scratch/riscv-llvm-toolchain/build-llvm/lib/LLVMshakti.so -t < test.ll -o testx.ll -O0
sed -i 's/llvm.RISCV.hash/hash/g' testx.ll
sed -i 's/llvm.RISCV.validate/val/g' testx.ll
riscv64-unknown-elf-clang -O2 -c debug.c
riscv64-unknown-elf-clang -c craft.s
riscv64-unknown-elf-clang -c testx.ll
riscv64-unknown-elf-clang -O2 -c external.c
riscv64-unknown-elf-clang testx.o craft.o debug.o external.o -o test_secure
riscv64-unknown-elf-clang test.ll -o test_insecure
