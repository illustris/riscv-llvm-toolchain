rm -f test.ll
rm -f testx.ll
rm -f testx.o
rm -f debug.o
rm -f a.out
rm -f craft.o
riscv64-unknown-elf-clang -c -S -O0 -emit-llvm test.c -O0
opt -S -load ~/scratch/riscv-llvm-toolchain/build-llvm/lib/LLVMshakti.so -t < test.ll -o testx.ll -O0
sed -i 's/llvm.RISCV.hash/hash/g' testx.ll
sed -i 's/llvm.RISCV.validate/val/g' testx.ll
riscv64-unknown-elf-clang -c debug.c
riscv64-unknown-elf-clang -c craft.s
riscv64-unknown-elf-clang -c testx.ll -O0
riscv64-unknown-elf-clang testx.o craft.o debug.o
