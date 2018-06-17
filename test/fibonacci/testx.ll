; ModuleID = '<stdin>'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-n32:64-S128"
target triple = "riscv64-unknown--elf"

@.str.1 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.2 = private unnamed_addr constant [14 x i8] c"fib(%u) = %u\0A\00", align 1
@str = private unnamed_addr constant [16 x i8] c"Enter a number:\00"
@ro_cookie = private constant i64 -2401053092289724205

; Function Attrs: nounwind readnone
define i32 @fib(i32 zeroext) local_unnamed_addr #0 {
  %stack_cookie = alloca i64
  %stack_cookie_32 = ptrtoint i64* %stack_cookie to i32
  %stack_hash_long = call i64 @hash(i64* %stack_cookie)
  %stack_hash = trunc i64 %stack_hash_long to i32
  %2 = icmp ult i32 %0, 2
  br i1 %2, label %9, label %3

; <label>:3:                                      ; preds = %1
  %4 = add i32 %0, -1
  %5 = tail call i32 @fib(i32 zeroext %4)
  %6 = add i32 %0, -2
  %7 = tail call i32 @fib(i32 zeroext %6)
  %8 = add i32 %7, %5
  %stack_cookie_burn = call i64 @hash(i64* %stack_cookie)
  ret i32 %8

; <label>:9:                                      ; preds = %1
  %stack_cookie_burn1 = call i64 @hash(i64* %stack_cookie)
  ret i32 %0
}

; Function Attrs: nounwind
define i32 @main() local_unnamed_addr #1 {
  %stack_cookie = alloca i64
  %stack_cookie_32 = ptrtoint i64* %stack_cookie to i32
  %stack_hash_long = call i64 @hash(i64* %stack_cookie)
  %stack_hash = trunc i64 %stack_hash_long to i32
  %1 = alloca i32, align 4
  %pti = ptrtoint i32* %1 to i32
  %fpr = call i128 @craft(i32 %pti, i32 %stack_cookie_32, i32 4, i32 %stack_hash)
  %fpr_low = trunc i128 %fpr to i64
  %fpr_hi_big = lshr i128 %fpr, 64
  %fpr_hi = trunc i128 %fpr_hi_big to i64
  call void @val(i64 %fpr_hi, i64 %fpr_low)
  %ptr32_ = and i64 %fpr_low, 2147483647
  %ptrc = inttoptr i64 %ptr32_ to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %ptrc) #4
  %2 = tail call i32 @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @str, i64 0, i64 0))
  %fpr_low1 = trunc i128 %fpr to i64
  %fpr_hi_big2 = lshr i128 %fpr, 64
  %fpr_hi3 = trunc i128 %fpr_hi_big2 to i64
  call void @val(i64 %fpr_hi3, i64 %fpr_low1)
  %ptr32_4 = and i64 %fpr_low1, 2147483647
  %ptrc5 = inttoptr i64 %ptr32_4 to i8*
  %3 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0), i8* nonnull %ptrc5)
  %fpr_low6 = trunc i128 %fpr to i64
  %fpr_hi_big7 = lshr i128 %fpr, 64
  %fpr_hi8 = trunc i128 %fpr_hi_big7 to i64
  call void @val(i64 %fpr_hi8, i64 %fpr_low6)
  %ptr32_9 = and i64 %fpr_low6, 2147483647
  %ptrl = inttoptr i64 %ptr32_9 to i32*
  %4 = load i32, i32* %ptrl, align 4, !tbaa !2
  %5 = call i32 @fib(i32 zeroext %4)
  %6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.2, i64 0, i64 0), i32 signext %4, i32 zeroext %5)
  %fpr_low10 = trunc i128 %fpr to i64
  %fpr_hi_big11 = lshr i128 %fpr, 64
  %fpr_hi12 = trunc i128 %fpr_hi_big11 to i64
  call void @val(i64 %fpr_hi12, i64 %fpr_low10)
  %ptr32_13 = and i64 %fpr_low10, 2147483647
  %ptrc14 = inttoptr i64 %ptr32_13 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %ptrc14) #4
  %stack_cookie_burn = call i64 @hash(i64* %stack_cookie)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind
declare i32 @scanf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #4

declare i128 @safemalloc(i64)

declare void @safefree(i128)

; Function Attrs: nounwind
declare i64 @hash(i64*) #4

declare i128 @craft(i32, i32, i32, i32)

; Function Attrs: nounwind
declare void @val(i64, i64) #4

attributes #0 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 "}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
