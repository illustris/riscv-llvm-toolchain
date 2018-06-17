; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-n32:64-S128"
target triple = "riscv64-unknown--elf"

%struct.uwb = type { i32 }
%struct.test = type { [16 x i32] }

@md5.h0 = internal global [4 x i32] [i32 1732584193, i32 -271733879, i32 -1732584194, i32 271733878], align 4
@md5.M = internal global [4 x i16] [i16 1, i16 5, i16 3, i16 7], align 2
@md5.O = internal global [4 x i16] [i16 0, i16 1, i16 5, i16 0], align 2
@md5.rot0 = internal global [4 x i16] [i16 7, i16 12, i16 17, i16 22], align 2
@md5.rot1 = internal global [4 x i16] [i16 5, i16 9, i16 14, i16 20], align 2
@md5.rot2 = internal global [4 x i16] [i16 4, i16 11, i16 16, i16 23], align 2
@md5.rot3 = internal global [4 x i16] [i16 6, i16 10, i16 15, i16 21], align 2
@md5.rots = internal global [4 x i16*] [i16* getelementptr inbounds ([4 x i16], [4 x i16]* @md5.rot0, i32 0, i32 0), i16* getelementptr inbounds ([4 x i16], [4 x i16]* @md5.rot1, i32 0, i32 0), i16* getelementptr inbounds ([4 x i16], [4 x i16]* @md5.rot2, i32 0, i32 0), i16* getelementptr inbounds ([4 x i16], [4 x i16]* @md5.rot3, i32 0, i32 0)], align 8
@md5.kspace = internal global [64 x i32] zeroinitializer, align 4
@md5.k = internal global i32* null, align 8
@md5.h = internal global [4 x i32] zeroinitializer, align 4
@.str = private unnamed_addr constant [45 x i8] c"The quick brown fox jumps over the lazy dog.\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"= 0x\00", align 1
@.str.2 = private unnamed_addr constant [5 x i8] c"%02x\00", align 1
@.str.3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind optnone
define i32 @f0(i32*) #0 {
  %2 = alloca i32*, align 8
  store i32* %0, i32** %2, align 8
  %3 = load i32*, i32** %2, align 8
  %4 = getelementptr inbounds i32, i32* %3, i64 1
  %5 = load i32, i32* %4, align 4
  %6 = load i32*, i32** %2, align 8
  %7 = getelementptr inbounds i32, i32* %6, i64 2
  %8 = load i32, i32* %7, align 4
  %9 = and i32 %5, %8
  %10 = load i32*, i32** %2, align 8
  %11 = getelementptr inbounds i32, i32* %10, i64 1
  %12 = load i32, i32* %11, align 4
  %13 = xor i32 %12, -1
  %14 = load i32*, i32** %2, align 8
  %15 = getelementptr inbounds i32, i32* %14, i64 3
  %16 = load i32, i32* %15, align 4
  %17 = and i32 %13, %16
  %18 = or i32 %9, %17
  ret i32 %18
}

; Function Attrs: noinline nounwind optnone
define i32 @f1(i32*) #0 {
  %2 = alloca i32*, align 8
  store i32* %0, i32** %2, align 8
  %3 = load i32*, i32** %2, align 8
  %4 = getelementptr inbounds i32, i32* %3, i64 3
  %5 = load i32, i32* %4, align 4
  %6 = load i32*, i32** %2, align 8
  %7 = getelementptr inbounds i32, i32* %6, i64 1
  %8 = load i32, i32* %7, align 4
  %9 = and i32 %5, %8
  %10 = load i32*, i32** %2, align 8
  %11 = getelementptr inbounds i32, i32* %10, i64 3
  %12 = load i32, i32* %11, align 4
  %13 = xor i32 %12, -1
  %14 = load i32*, i32** %2, align 8
  %15 = getelementptr inbounds i32, i32* %14, i64 2
  %16 = load i32, i32* %15, align 4
  %17 = and i32 %13, %16
  %18 = or i32 %9, %17
  ret i32 %18
}

; Function Attrs: noinline nounwind optnone
define i32 @f2(i32*) #0 {
  %2 = alloca i32*, align 8
  store i32* %0, i32** %2, align 8
  %3 = load i32*, i32** %2, align 8
  %4 = getelementptr inbounds i32, i32* %3, i64 1
  %5 = load i32, i32* %4, align 4
  %6 = load i32*, i32** %2, align 8
  %7 = getelementptr inbounds i32, i32* %6, i64 2
  %8 = load i32, i32* %7, align 4
  %9 = xor i32 %5, %8
  %10 = load i32*, i32** %2, align 8
  %11 = getelementptr inbounds i32, i32* %10, i64 3
  %12 = load i32, i32* %11, align 4
  %13 = xor i32 %9, %12
  ret i32 %13
}

; Function Attrs: noinline nounwind optnone
define i32 @f3(i32*) #0 {
  %2 = alloca i32*, align 8
  store i32* %0, i32** %2, align 8
  %3 = load i32*, i32** %2, align 8
  %4 = getelementptr inbounds i32, i32* %3, i64 2
  %5 = load i32, i32* %4, align 4
  %6 = load i32*, i32** %2, align 8
  %7 = getelementptr inbounds i32, i32* %6, i64 1
  %8 = load i32, i32* %7, align 4
  %9 = load i32*, i32** %2, align 8
  %10 = getelementptr inbounds i32, i32* %9, i64 3
  %11 = load i32, i32* %10, align 4
  %12 = xor i32 %11, -1
  %13 = or i32 %8, %12
  %14 = xor i32 %5, %13
  ret i32 %14
}

; Function Attrs: noinline nounwind optnone
define zeroext i8 @getb(i32, i32 signext) #0 {
  %3 = alloca %struct.uwb, align 4
  %4 = alloca i32, align 4
  %5 = alloca [4 x i8], align 1
  %6 = getelementptr inbounds %struct.uwb, %struct.uwb* %3, i32 0, i32 0
  store i32 %0, i32* %6, align 4
  store i32 %1, i32* %4, align 4
  %7 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i32 0, i32 0
  %8 = bitcast %struct.uwb* %3 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %7, i8* %8, i64 4, i32 1, i1 false)
  %9 = load i32, i32* %4, align 4
  %10 = sext i32 %9 to i64
  %11 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 %10
  %12 = load i8, i8* %11, align 1
  ret i8 %12
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

; Function Attrs: noinline nounwind optnone
define i32* @calcKs(i32*) #0 {
  %2 = alloca i32*, align 8
  %3 = alloca double, align 8
  %4 = alloca double, align 8
  %5 = alloca i32, align 4
  store i32* %0, i32** %2, align 8
  %6 = call double @pow(double 2.000000e+00, double 3.200000e+01) #5
  store double %6, double* %4, align 8
  store i32 0, i32* %5, align 4
  br label %7

; <label>:7:                                      ; preds = %24, %1
  %8 = load i32, i32* %5, align 4
  %9 = icmp slt i32 %8, 64
  br i1 %9, label %10, label %27

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %5, align 4
  %12 = add nsw i32 1, %11
  %13 = sitofp i32 %12 to double
  %14 = call double @sin(double %13) #5
  %15 = call double @fabs(double %14) #6
  store double %15, double* %3, align 8
  %16 = load double, double* %3, align 8
  %17 = load double, double* %4, align 8
  %18 = fmul double %16, %17
  %19 = fptoui double %18 to i32
  %20 = load i32*, i32** %2, align 8
  %21 = load i32, i32* %5, align 4
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds i32, i32* %20, i64 %22
  store i32 %19, i32* %23, align 4
  br label %24

; <label>:24:                                     ; preds = %10
  %25 = load i32, i32* %5, align 4
  %26 = add nsw i32 %25, 1
  store i32 %26, i32* %5, align 4
  br label %7

; <label>:27:                                     ; preds = %7
  %28 = load i32*, i32** %2, align 8
  ret i32* %28
}

; Function Attrs: nounwind
declare double @pow(double, double) #2

; Function Attrs: nounwind readnone
declare double @fabs(double) #3

; Function Attrs: nounwind
declare double @sin(double) #2

; Function Attrs: noinline nounwind optnone
define i32 @rol(i32 zeroext, i16 signext) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i16, align 2
  %5 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i16 %1, i16* %4, align 2
  %6 = load i16, i16* %4, align 2
  %7 = sext i16 %6 to i32
  %8 = shl i32 1, %7
  %9 = sub nsw i32 %8, 1
  store i32 %9, i32* %5, align 4
  %10 = load i32, i32* %3, align 4
  %11 = load i16, i16* %4, align 2
  %12 = sext i16 %11 to i32
  %13 = sub nsw i32 32, %12
  %14 = lshr i32 %10, %13
  %15 = load i32, i32* %5, align 4
  %16 = and i32 %14, %15
  %17 = load i32, i32* %3, align 4
  %18 = load i16, i16* %4, align 2
  %19 = sext i16 %18 to i32
  %20 = shl i32 %17, %19
  %21 = load i32, i32* %5, align 4
  %22 = xor i32 %21, -1
  %23 = and i32 %20, %22
  %24 = or i32 %16, %23
  ret i32 %24
}

; Function Attrs: noinline nounwind optnone
define i32* @md5(i8*, i32 signext) #0 {
  %3 = alloca i8*, align 8
  %4 = alloca i32, align 4
  %5 = alloca [4 x i32], align 4
  %6 = alloca i32 (i32*)*, align 8
  %7 = alloca i16, align 2
  %8 = alloca i16, align 2
  %9 = alloca i16, align 2
  %10 = alloca i32, align 4
  %11 = alloca i16*, align 8
  %12 = alloca %struct.test, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  %16 = alloca i32, align 4
  %17 = alloca i32, align 4
  %18 = alloca i8*, align 8
  %19 = alloca %struct.uwb, align 4
  store i8* %0, i8** %3, align 8
  store i32 %1, i32* %4, align 4
  store i32 0, i32* %13, align 4
  %20 = load i32*, i32** @md5.k, align 8
  %21 = icmp eq i32* %20, null
  br i1 %21, label %22, label %24

; <label>:22:                                     ; preds = %2
  %23 = call i32* @calcKs(i32* getelementptr inbounds ([64 x i32], [64 x i32]* @md5.kspace, i32 0, i32 0))
  store i32* %23, i32** @md5.k, align 8
  br label %24

; <label>:24:                                     ; preds = %22, %2
  store i32 0, i32* %16, align 4
  br label %25

; <label>:25:                                     ; preds = %36, %24
  %26 = load i32, i32* %16, align 4
  %27 = icmp slt i32 %26, 4
  br i1 %27, label %28, label %39

; <label>:28:                                     ; preds = %25
  %29 = load i32, i32* %16, align 4
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds [4 x i32], [4 x i32]* @md5.h0, i64 0, i64 %30
  %32 = load i32, i32* %31, align 4
  %33 = load i32, i32* %16, align 4
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds [4 x i32], [4 x i32]* @md5.h, i64 0, i64 %34
  store i32 %32, i32* %35, align 4
  br label %36

; <label>:36:                                     ; preds = %28
  %37 = load i32, i32* %16, align 4
  %38 = add nsw i32 %37, 1
  store i32 %38, i32* %16, align 4
  br label %25

; <label>:39:                                     ; preds = %25
  %40 = load i32, i32* %4, align 4
  %41 = add nsw i32 %40, 8
  %42 = sdiv i32 %41, 64
  %43 = add nsw i32 1, %42
  store i32 %43, i32* %15, align 4
  %44 = load i32, i32* %15, align 4
  %45 = mul nsw i32 64, %44
  %46 = sext i32 %45 to i64
  %47 = call i8* @malloc(i64 zeroext %46)
  store i8* %47, i8** %18, align 8
  %48 = load i8*, i8** %18, align 8
  %49 = load i8*, i8** %3, align 8
  %50 = load i32, i32* %4, align 4
  %51 = sext i32 %50 to i64
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %48, i8* %49, i64 %51, i32 1, i1 false)
  %52 = load i8*, i8** %18, align 8
  %53 = load i32, i32* %4, align 4
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds i8, i8* %52, i64 %54
  store i8 -128, i8* %55, align 1
  %56 = load i32, i32* %4, align 4
  %57 = add nsw i32 %56, 1
  store i32 %57, i32* %16, align 4
  br label %58

; <label>:58:                                     ; preds = %63, %39
  %59 = load i32, i32* %16, align 4
  %60 = load i32, i32* %15, align 4
  %61 = mul nsw i32 64, %60
  %62 = icmp slt i32 %59, %61
  br i1 %62, label %63, label %70

; <label>:63:                                     ; preds = %58
  %64 = load i8*, i8** %18, align 8
  %65 = load i32, i32* %16, align 4
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds i8, i8* %64, i64 %66
  store i8 0, i8* %67, align 1
  %68 = load i32, i32* %16, align 4
  %69 = add nsw i32 %68, 1
  store i32 %69, i32* %16, align 4
  br label %58

; <label>:70:                                     ; preds = %58
  %71 = load i32, i32* %4, align 4
  %72 = mul nsw i32 8, %71
  %73 = getelementptr inbounds %struct.uwb, %struct.uwb* %19, i32 0, i32 0
  store i32 %72, i32* %73, align 4
  %74 = load i32, i32* %16, align 4
  %75 = sub nsw i32 %74, 8
  store i32 %75, i32* %16, align 4
  %76 = load i8*, i8** %18, align 8
  %77 = load i32, i32* %16, align 4
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds i8, i8* %76, i64 %78
  %80 = getelementptr inbounds %struct.uwb, %struct.uwb* %19, i32 0, i32 0
  %81 = bitcast i32* %80 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %79, i8* %81, i64 4, i32 1, i1 false)
  store i32 0, i32* %14, align 4
  br label %82

; <label>:82:                                     ; preds = %220, %70
  %83 = load i32, i32* %14, align 4
  %84 = load i32, i32* %15, align 4
  %85 = icmp slt i32 %83, %84
  br i1 %85, label %86, label %223

; <label>:86:                                     ; preds = %82
  %87 = bitcast %struct.test* %12 to i8*
  %88 = load i8*, i8** %18, align 8
  %89 = load i32, i32* %13, align 4
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds i8, i8* %88, i64 %90
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %87, i8* %91, i64 64, i32 1, i1 false)
  store i32 0, i32* %16, align 4
  br label %92

; <label>:92:                                     ; preds = %103, %86
  %93 = load i32, i32* %16, align 4
  %94 = icmp slt i32 %93, 4
  br i1 %94, label %95, label %106

; <label>:95:                                     ; preds = %92
  %96 = load i32, i32* %16, align 4
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds [4 x i32], [4 x i32]* @md5.h, i64 0, i64 %97
  %99 = load i32, i32* %98, align 4
  %100 = load i32, i32* %16, align 4
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 %101
  store i32 %99, i32* %102, align 4
  br label %103

; <label>:103:                                    ; preds = %95
  %104 = load i32, i32* %16, align 4
  %105 = add nsw i32 %104, 1
  store i32 %105, i32* %16, align 4
  br label %92

; <label>:106:                                    ; preds = %92
  store i32 0, i32* %17, align 4
  br label %107

; <label>:107:                                    ; preds = %197, %106
  %108 = load i32, i32* %17, align 4
  %109 = icmp slt i32 %108, 4
  br i1 %109, label %110, label %200

; <label>:110:                                    ; preds = %107
  %111 = load i32, i32* %17, align 4
  %112 = icmp eq i32 %111, 0
  br i1 %112, label %113, label %114

; <label>:113:                                    ; preds = %110
  store i32 (i32*)* @f0, i32 (i32*)** %6, align 8
  br label %125

; <label>:114:                                    ; preds = %110
  %115 = load i32, i32* %17, align 4
  %116 = icmp eq i32 %115, 1
  br i1 %116, label %117, label %118

; <label>:117:                                    ; preds = %114
  store i32 (i32*)* @f1, i32 (i32*)** %6, align 8
  br label %124

; <label>:118:                                    ; preds = %114
  %119 = load i32, i32* %17, align 4
  %120 = icmp eq i32 %119, 2
  br i1 %120, label %121, label %122

; <label>:121:                                    ; preds = %118
  store i32 (i32*)* @f2, i32 (i32*)** %6, align 8
  br label %123

; <label>:122:                                    ; preds = %118
  store i32 (i32*)* @f3, i32 (i32*)** %6, align 8
  br label %123

; <label>:123:                                    ; preds = %122, %121
  br label %124

; <label>:124:                                    ; preds = %123, %117
  br label %125

; <label>:125:                                    ; preds = %124, %113
  %126 = load i32, i32* %17, align 4
  %127 = sext i32 %126 to i64
  %128 = getelementptr inbounds [4 x i16*], [4 x i16*]* @md5.rots, i64 0, i64 %127
  %129 = load i16*, i16** %128, align 8
  store i16* %129, i16** %11, align 8
  %130 = load i32, i32* %17, align 4
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds [4 x i16], [4 x i16]* @md5.M, i64 0, i64 %131
  %133 = load i16, i16* %132, align 2
  store i16 %133, i16* %7, align 2
  %134 = load i32, i32* %17, align 4
  %135 = sext i32 %134 to i64
  %136 = getelementptr inbounds [4 x i16], [4 x i16]* @md5.O, i64 0, i64 %135
  %137 = load i16, i16* %136, align 2
  store i16 %137, i16* %8, align 2
  store i32 0, i32* %16, align 4
  br label %138

; <label>:138:                                    ; preds = %193, %125
  %139 = load i32, i32* %16, align 4
  %140 = icmp slt i32 %139, 16
  br i1 %140, label %141, label %196

; <label>:141:                                    ; preds = %138
  %142 = load i16, i16* %7, align 2
  %143 = sext i16 %142 to i32
  %144 = load i32, i32* %16, align 4
  %145 = mul nsw i32 %143, %144
  %146 = load i16, i16* %8, align 2
  %147 = sext i16 %146 to i32
  %148 = add nsw i32 %145, %147
  %149 = srem i32 %148, 16
  %150 = trunc i32 %149 to i16
  store i16 %150, i16* %9, align 2
  %151 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 1
  %152 = load i32, i32* %151, align 4
  %153 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 0
  %154 = load i32, i32* %153, align 4
  %155 = load i32 (i32*)*, i32 (i32*)** %6, align 8
  %156 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i32 0, i32 0
  %157 = call i32 %155(i32* %156)
  %158 = add i32 %154, %157
  %159 = load i32*, i32** @md5.k, align 8
  %160 = load i32, i32* %16, align 4
  %161 = load i32, i32* %17, align 4
  %162 = mul nsw i32 16, %161
  %163 = add nsw i32 %160, %162
  %164 = sext i32 %163 to i64
  %165 = getelementptr inbounds i32, i32* %159, i64 %164
  %166 = load i32, i32* %165, align 4
  %167 = add i32 %158, %166
  %168 = getelementptr inbounds %struct.test, %struct.test* %12, i32 0, i32 0
  %169 = load i16, i16* %9, align 2
  %170 = sext i16 %169 to i64
  %171 = getelementptr inbounds [16 x i32], [16 x i32]* %168, i64 0, i64 %170
  %172 = load i32, i32* %171, align 4
  %173 = add i32 %167, %172
  %174 = load i16*, i16** %11, align 8
  %175 = load i32, i32* %16, align 4
  %176 = srem i32 %175, 4
  %177 = sext i32 %176 to i64
  %178 = getelementptr inbounds i16, i16* %174, i64 %177
  %179 = load i16, i16* %178, align 2
  %180 = call i32 @rol(i32 zeroext %173, i16 signext %179)
  %181 = add i32 %152, %180
  store i32 %181, i32* %10, align 4
  %182 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 3
  %183 = load i32, i32* %182, align 4
  %184 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 0
  store i32 %183, i32* %184, align 4
  %185 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 2
  %186 = load i32, i32* %185, align 4
  %187 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 3
  store i32 %186, i32* %187, align 4
  %188 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 1
  %189 = load i32, i32* %188, align 4
  %190 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 2
  store i32 %189, i32* %190, align 4
  %191 = load i32, i32* %10, align 4
  %192 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 1
  store i32 %191, i32* %192, align 4
  br label %193

; <label>:193:                                    ; preds = %141
  %194 = load i32, i32* %16, align 4
  %195 = add nsw i32 %194, 1
  store i32 %195, i32* %16, align 4
  br label %138

; <label>:196:                                    ; preds = %138
  br label %197

; <label>:197:                                    ; preds = %196
  %198 = load i32, i32* %17, align 4
  %199 = add nsw i32 %198, 1
  store i32 %199, i32* %17, align 4
  br label %107

; <label>:200:                                    ; preds = %107
  store i32 0, i32* %17, align 4
  br label %201

; <label>:201:                                    ; preds = %214, %200
  %202 = load i32, i32* %17, align 4
  %203 = icmp slt i32 %202, 4
  br i1 %203, label %204, label %217

; <label>:204:                                    ; preds = %201
  %205 = load i32, i32* %17, align 4
  %206 = sext i32 %205 to i64
  %207 = getelementptr inbounds [4 x i32], [4 x i32]* %5, i64 0, i64 %206
  %208 = load i32, i32* %207, align 4
  %209 = load i32, i32* %17, align 4
  %210 = sext i32 %209 to i64
  %211 = getelementptr inbounds [4 x i32], [4 x i32]* @md5.h, i64 0, i64 %210
  %212 = load i32, i32* %211, align 4
  %213 = add i32 %212, %208
  store i32 %213, i32* %211, align 4
  br label %214

; <label>:214:                                    ; preds = %204
  %215 = load i32, i32* %17, align 4
  %216 = add nsw i32 %215, 1
  store i32 %216, i32* %17, align 4
  br label %201

; <label>:217:                                    ; preds = %201
  %218 = load i32, i32* %13, align 4
  %219 = add nsw i32 %218, 64
  store i32 %219, i32* %13, align 4
  br label %220

; <label>:220:                                    ; preds = %217
  %221 = load i32, i32* %14, align 4
  %222 = add nsw i32 %221, 1
  store i32 %222, i32* %14, align 4
  br label %82

; <label>:223:                                    ; preds = %82
  %224 = load i8*, i8** %18, align 8
  %225 = icmp ne i8* %224, null
  br i1 %225, label %226, label %228

; <label>:226:                                    ; preds = %223
  %227 = load i8*, i8** %18, align 8
  call void @free(i8* %227)
  br label %228

; <label>:228:                                    ; preds = %226, %223
  ret i32* getelementptr inbounds ([4 x i32], [4 x i32]* @md5.h, i32 0, i32 0)
}

declare i8* @malloc(i64 zeroext) #4

declare void @free(i8*) #4

; Function Attrs: noinline nounwind optnone
define i32 @main(i32 signext, i8**) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i8*, align 8
  %9 = alloca i32*, align 8
  %10 = alloca %struct.uwb, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  store i8* getelementptr inbounds ([45 x i8], [45 x i8]* @.str, i32 0, i32 0), i8** %8, align 8
  %11 = load i8*, i8** %8, align 8
  %12 = load i8*, i8** %8, align 8
  %13 = call i64 @strlen(i8* %12)
  %14 = trunc i64 %13 to i32
  %15 = call i32* @md5(i8* %11, i32 signext %14)
  store i32* %15, i32** %9, align 8
  %16 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.1, i32 0, i32 0))
  store i32 0, i32* %6, align 4
  br label %17

; <label>:17:                                     ; preds = %41, %2
  %18 = load i32, i32* %6, align 4
  %19 = icmp slt i32 %18, 4
  br i1 %19, label %20, label %44

; <label>:20:                                     ; preds = %17
  %21 = load i32*, i32** %9, align 8
  %22 = load i32, i32* %6, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds i32, i32* %21, i64 %23
  %25 = load i32, i32* %24, align 4
  %26 = getelementptr inbounds %struct.uwb, %struct.uwb* %10, i32 0, i32 0
  store i32 %25, i32* %26, align 4
  store i32 0, i32* %7, align 4
  br label %27

; <label>:27:                                     ; preds = %37, %20
  %28 = load i32, i32* %7, align 4
  %29 = icmp slt i32 %28, 4
  br i1 %29, label %30, label %40

; <label>:30:                                     ; preds = %27
  %31 = load i32, i32* %7, align 4
  %32 = getelementptr inbounds %struct.uwb, %struct.uwb* %10, i32 0, i32 0
  %33 = load i32, i32* %32, align 4
  %34 = call zeroext i8 @getb(i32 %33, i32 signext %31)
  %35 = zext i8 %34 to i32
  %36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.2, i32 0, i32 0), i32 signext %35)
  br label %37

; <label>:37:                                     ; preds = %30
  %38 = load i32, i32* %7, align 4
  %39 = add nsw i32 %38, 1
  store i32 %39, i32* %7, align 4
  br label %27

; <label>:40:                                     ; preds = %27
  br label %41

; <label>:41:                                     ; preds = %40
  %42 = load i32, i32* %6, align 4
  %43 = add nsw i32 %42, 1
  store i32 %43, i32* %6, align 4
  br label %17

; <label>:44:                                     ; preds = %17
  %45 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.3, i32 0, i32 0))
  ret i32 0
}

declare i64 @strlen(i8*) #4

declare i32 @printf(i8*, ...) #4

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+a,+m,+rv64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }
attributes #6 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 "}
