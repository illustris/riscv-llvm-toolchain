; ModuleID = 'debug.c'
source_filename = "debug.c"
target datalayout = "e-m:e-i64:64-n32:64-S128"
target triple = "riscv64-unknown--elf"

@PRNG = global i64 -2401053092093640493, align 8
@.str = private unnamed_addr constant [45 x i8] c"!!!!VALIDATE ERROR\0Agot NULL pointer for base\00", align 1
@.str.1 = private unnamed_addr constant [21 x i8] c"BOUND:BASE  %016llx\0A\00", align 1
@.str.2 = private unnamed_addr constant [21 x i8] c"IDHASH:PTR  %016llx\0A\00", align 1
@.str.3 = private unnamed_addr constant [55 x i8] c"!!!!VALIDATE ERROR\0Agot hash %x from %08llx\0Aexpected %x\00", align 1
@.str.4 = private unnamed_addr constant [35 x i8] c"Malloc got size %lu, pointer %llx\0A\00", align 1
@.str.5 = private unnamed_addr constant [23 x i8] c"FREE called with %llx\0A\00", align 1
@.str.6 = private unnamed_addr constant [14 x i8] c"Pointer: %llx\00", align 1

; Function Attrs: noinline nounwind optnone
define i64 @random64() #0 {
  %1 = load i64, i64* @PRNG, align 8
  %2 = load i64, i64* @PRNG, align 8
  %3 = lshr i64 %2, 1
  %4 = load i64, i64* @PRNG, align 8
  %5 = lshr i64 %4, 17
  %6 = add i64 %3, %5
  %7 = xor i64 %1, %6
  store i64 %7, i64* @PRNG, align 8
  %8 = load i64, i64* @PRNG, align 8
  ret i64 %8
}

; Function Attrs: noinline nounwind optnone
define i32 @hash_fn(i64 zeroext) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i32, align 4
  store i64 %0, i64* %2, align 8
  %4 = load i64, i64* %2, align 8
  %5 = trunc i64 %4 to i32
  store i32 %5, i32* %3, align 4
  %6 = load i32, i32* %3, align 4
  %7 = load i64, i64* %2, align 8
  %8 = lshr i64 %7, 32
  %9 = trunc i64 %8 to i32
  %10 = xor i32 %6, %9
  store i32 %10, i32* %3, align 4
  %11 = load i32, i32* %3, align 4
  ret i32 %11
}

; Function Attrs: noinline nounwind optnone
define i32 @hash_fn_debug(i64 zeroext) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i32, align 4
  store i64 %0, i64* %2, align 8
  %4 = load i64, i64* %2, align 8
  %5 = trunc i64 %4 to i32
  store i32 %5, i32* %3, align 4
  %6 = load i32, i32* %3, align 4
  %7 = load i64, i64* %2, align 8
  %8 = lshr i64 %7, 32
  %9 = trunc i64 %8 to i32
  %10 = xor i32 %6, %9
  store i32 %10, i32* %3, align 4
  %11 = load i32, i32* %3, align 4
  ret i32 %11
}

; Function Attrs: noinline nounwind optnone
define i64 @hash(i64*) #0 {
  %2 = alloca i64*, align 8
  store i64* %0, i64** %2, align 8
  %3 = call i64 @random64()
  %4 = load i64*, i64** %2, align 8
  store i64 %3, i64* %4, align 8
  %5 = load i64*, i64** %2, align 8
  %6 = load i64, i64* %5, align 8
  %7 = call i32 @hash_fn(i64 zeroext %6)
  %8 = zext i32 %7 to i64
  ret i64 %8
}

; Function Attrs: noinline nounwind optnone
define void @val(i64 zeroext, i64 zeroext) #0 {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  %6 = alloca i64*, align 8
  store i64 %0, i64* %3, align 8
  store i64 %1, i64* %4, align 8
  %7 = load i64, i64* %4, align 8
  %8 = and i64 %7, -4294967296
  %9 = lshr i64 %8, 32
  %10 = trunc i64 %9 to i32
  store i32 %10, i32* %5, align 4
  %11 = load i64, i64* %3, align 8
  %12 = and i64 %11, 4294967295
  %13 = inttoptr i64 %12 to i64*
  store i64* %13, i64** %6, align 8
  %14 = load i64*, i64** %6, align 8
  %15 = icmp eq i64* %14, null
  br i1 %15, label %16, label %22

; <label>:16:                                     ; preds = %2
  %17 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str, i32 0, i32 0))
  %18 = load i64, i64* %3, align 8
  %19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.1, i32 0, i32 0), i64 zeroext %18)
  %20 = load i64, i64* %4, align 8
  %21 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.2, i32 0, i32 0), i64 zeroext %20)
  call void @exit(i32 signext 0) #3
  unreachable

; <label>:22:                                     ; preds = %2
  %23 = load i32, i32* %5, align 4
  %24 = load i64*, i64** %6, align 8
  %25 = load i64, i64* %24, align 8
  %26 = call i32 @hash_fn(i64 zeroext %25)
  %27 = icmp ne i32 %23, %26
  br i1 %27, label %28, label %36

; <label>:28:                                     ; preds = %22
  %29 = load i32, i32* %5, align 4
  %30 = load i64*, i64** %6, align 8
  %31 = ptrtoint i64* %30 to i64
  %32 = load i64*, i64** %6, align 8
  %33 = load i64, i64* %32, align 8
  %34 = call i32 @hash_fn_debug(i64 zeroext %33)
  %35 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([55 x i8], [55 x i8]* @.str.3, i32 0, i32 0), i32 zeroext %29, i64 zeroext %31, i32 zeroext %34)
  call void @exit(i32 signext 0) #3
  unreachable

; <label>:36:                                     ; preds = %22
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: noreturn
declare void @exit(i32 signext) #2

; Function Attrs: noinline nounwind optnone
define i128 @safemalloc(i64 zeroext) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i8*, align 8
  %4 = alloca i64, align 8
  %5 = alloca i128, align 16
  store i64 %0, i64* %2, align 8
  %6 = load i64, i64* %2, align 8
  %7 = add i64 %6, 8
  store i64 %7, i64* %2, align 8
  %8 = load i64, i64* %2, align 8
  %9 = call i8* @malloc(i64 zeroext %8)
  store i8* %9, i8** %3, align 8
  %10 = load i8*, i8** %3, align 8
  %11 = bitcast i8* %10 to i64*
  %12 = call i64 @hash(i64* %11)
  store i64 %12, i64* %4, align 8
  %13 = load i8*, i8** %3, align 8
  %14 = getelementptr i8, i8* %13, i64 8
  %15 = ptrtoint i8* %14 to i32
  %16 = load i8*, i8** %3, align 8
  %17 = ptrtoint i8* %16 to i32
  %18 = load i8*, i8** %3, align 8
  %19 = load i64, i64* %2, align 8
  %20 = getelementptr i8, i8* %18, i64 %19
  %21 = ptrtoint i8* %20 to i32
  %22 = load i64, i64* %4, align 8
  %23 = trunc i64 %22 to i32
  %24 = call i128 @craft(i32 zeroext %15, i32 zeroext %17, i32 zeroext %21, i32 zeroext %23)
  store i128 %24, i128* %5, align 16
  %25 = load i64, i64* %2, align 8
  %26 = sub i64 %25, 8
  %27 = load i8*, i8** %3, align 8
  %28 = ptrtoint i8* %27 to i64
  %29 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @.str.4, i32 0, i32 0), i64 zeroext %26, i64 zeroext %28)
  %30 = load i128, i128* %5, align 16
  ret i128 %30
}

declare i8* @malloc(i64 zeroext) #1

declare i128 @craft(i32 zeroext, i32 zeroext, i32 zeroext, i32 zeroext) #1

; Function Attrs: noinline nounwind optnone
define void @safefree(i128 signext) #0 {
  %2 = alloca i128, align 16
  %3 = alloca i8*, align 8
  store i128 %0, i128* %2, align 16
  %4 = load i128, i128* %2, align 16
  %5 = ashr i128 %4, 64
  %6 = trunc i128 %5 to i64
  %7 = inttoptr i64 %6 to i8*
  store i8* %7, i8** %3, align 8
  %8 = load i8*, i8** %3, align 8
  %9 = ptrtoint i8* %8 to i64
  %10 = load i128, i128* %2, align 16
  %11 = trunc i128 %10 to i64
  call void @val(i64 zeroext %9, i64 zeroext %11)
  %12 = load i8*, i8** %3, align 8
  %13 = ptrtoint i8* %12 to i64
  %14 = and i64 %13, 2147483647
  %15 = inttoptr i64 %14 to i8*
  store i8* %15, i8** %3, align 8
  %16 = load i8*, i8** %3, align 8
  %17 = ptrtoint i8* %16 to i64
  %18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.5, i32 0, i32 0), i64 zeroext %17)
  %19 = load i8*, i8** %3, align 8
  call void @free(i8* %19)
  ret void
}

declare void @free(i8*) #1

; Function Attrs: noinline nounwind optnone
define void @printptr(i8*) #0 {
  %2 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = ptrtoint i8* %3 to i64
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.6, i32 0, i32 0), i64 zeroext %4)
  ret void
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 "}
