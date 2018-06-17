; ModuleID = 'debug.c'
source_filename = "debug.c"
target datalayout = "e-m:e-i64:64-n32:64-S128"
target triple = "riscv64-unknown--elf"

@PRNG = local_unnamed_addr global i64 -2401053092093640493, align 8
@.str = private unnamed_addr constant [45 x i8] c"!!!!VALIDATE ERROR\0Agot NULL pointer for base\00", align 1
@.str.1 = private unnamed_addr constant [21 x i8] c"BOUND:BASE  %016llx\0A\00", align 1
@.str.2 = private unnamed_addr constant [21 x i8] c"IDHASH:PTR  %016llx\0A\00", align 1
@.str.3 = private unnamed_addr constant [55 x i8] c"!!!!VALIDATE ERROR\0Agot hash %x from %08llx\0Aexpected %x\00", align 1
@.str.4 = private unnamed_addr constant [35 x i8] c"Malloc got size %lu, pointer %llx\0A\00", align 1
@.str.5 = private unnamed_addr constant [23 x i8] c"FREE called with %llx\0A\00", align 1
@.str.6 = private unnamed_addr constant [14 x i8] c"Pointer: %llx\00", align 1

; Function Attrs: norecurse nounwind
define i64 @random64() local_unnamed_addr #0 {
  %1 = load i64, i64* @PRNG, align 8, !tbaa !2
  %2 = lshr i64 %1, 1
  %3 = lshr i64 %1, 17
  %4 = add nuw i64 %2, %3
  %5 = xor i64 %4, %1
  store i64 %5, i64* @PRNG, align 8, !tbaa !2
  ret i64 %5
}

; Function Attrs: norecurse nounwind readnone
define i32 @hash_fn(i64 zeroext) local_unnamed_addr #1 {
  %2 = lshr i64 %0, 32
  %3 = xor i64 %2, %0
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: norecurse nounwind readnone
define i32 @hash_fn_debug(i64 zeroext) local_unnamed_addr #1 {
  %2 = lshr i64 %0, 32
  %3 = xor i64 %2, %0
  %4 = trunc i64 %3 to i32
  ret i32 %4
}

; Function Attrs: norecurse nounwind
define i64 @hash(i64* nocapture) local_unnamed_addr #0 {
  %2 = load i64, i64* @PRNG, align 8, !tbaa !2
  %3 = lshr i64 %2, 1
  %4 = lshr i64 %2, 17
  %5 = add nuw i64 %3, %4
  %6 = xor i64 %5, %2
  store i64 %6, i64* @PRNG, align 8, !tbaa !2
  store i64 %6, i64* %0, align 8, !tbaa !2
  %7 = lshr i64 %6, 32
  %8 = and i64 %6, 4294967295
  %9 = xor i64 %7, %8
  ret i64 %9
}

; Function Attrs: nounwind
define void @val(i64 zeroext, i64 zeroext) local_unnamed_addr #2 {
  %3 = lshr i64 %1, 32
  %4 = trunc i64 %3 to i32
  %5 = and i64 %0, 4294967295
  %6 = icmp eq i64 %5, 0
  br i1 %6, label %7, label %11

; <label>:7:                                      ; preds = %2
  %8 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str, i64 0, i64 0))
  %9 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.1, i64 0, i64 0), i64 zeroext %0)
  %10 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.2, i64 0, i64 0), i64 zeroext %1)
  tail call void @exit(i32 signext 0) #6
  unreachable

; <label>:11:                                     ; preds = %2
  %12 = inttoptr i64 %5 to i64*
  %13 = load i64, i64* %12, align 8, !tbaa !2
  %14 = lshr i64 %13, 32
  %15 = xor i64 %14, %13
  %16 = trunc i64 %15 to i32
  %17 = icmp eq i32 %16, %4
  br i1 %17, label %20, label %18

; <label>:18:                                     ; preds = %11
  %19 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([55 x i8], [55 x i8]* @.str.3, i64 0, i64 0), i32 zeroext %4, i64 zeroext %5, i32 zeroext %16)
  tail call void @exit(i32 signext 0) #6
  unreachable

; <label>:20:                                     ; preds = %11
  ret void
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: noreturn
declare void @exit(i32 signext) local_unnamed_addr #4

; Function Attrs: nounwind
define i128 @safemalloc(i64 zeroext) local_unnamed_addr #2 {
  %2 = add i64 %0, 8
  %3 = tail call i8* @malloc(i64 zeroext %2)
  %4 = bitcast i8* %3 to i64*
  %5 = load i64, i64* @PRNG, align 8, !tbaa !2
  %6 = lshr i64 %5, 1
  %7 = lshr i64 %5, 17
  %8 = add nuw i64 %6, %7
  %9 = xor i64 %8, %5
  store i64 %9, i64* @PRNG, align 8, !tbaa !2
  store i64 %9, i64* %4, align 8, !tbaa !2
  %10 = lshr i64 %9, 32
  %11 = xor i64 %10, %9
  %12 = getelementptr i8, i8* %3, i64 8
  %13 = ptrtoint i8* %12 to i64
  %14 = trunc i64 %13 to i32
  %15 = ptrtoint i8* %3 to i64
  %16 = trunc i64 %15 to i32
  %17 = getelementptr i8, i8* %3, i64 %2
  %18 = ptrtoint i8* %17 to i64
  %19 = trunc i64 %18 to i32
  %20 = trunc i64 %11 to i32
  %21 = tail call i128 @craft(i32 zeroext %14, i32 zeroext %16, i32 zeroext %19, i32 zeroext %20) #7
  %22 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @.str.4, i64 0, i64 0), i64 zeroext %0, i64 zeroext %15)
  ret i128 %21
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 zeroext) local_unnamed_addr #3

declare i128 @craft(i32 zeroext, i32 zeroext, i32 zeroext, i32 zeroext) local_unnamed_addr #5

; Function Attrs: nounwind
define void @safefree(i128 signext) local_unnamed_addr #2 {
  %2 = lshr i128 %0, 64
  %3 = trunc i128 %2 to i64
  %4 = trunc i128 %0 to i64
  tail call void @val(i64 zeroext %3, i64 zeroext %4)
  %5 = and i64 %3, 2147483647
  %6 = inttoptr i64 %5 to i8*
  %7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.5, i64 0, i64 0), i64 zeroext %5)
  tail call void @free(i8* %6)
  ret void
}

; Function Attrs: nounwind
declare void @free(i8* nocapture) local_unnamed_addr #3

; Function Attrs: nounwind
define void @printptr(i8*) local_unnamed_addr #2 {
  %2 = ptrtoint i8* %0 to i64
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.6, i64 0, i64 0), i64 zeroext %2)
  ret void
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 "}
!2 = !{!3, !3, i64 0}
!3 = !{!"long long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
