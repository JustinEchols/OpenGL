#if !defined(APP_INTRINSICS_H)
#include "math.h"

inline f32
AbsVal(f32 X)
{
	f32 Result = (f32)fabs(X);
	return(Result);
}

inline f32
F32Max(void)
{
	u32 Inf = 0x7f800000;
	f32 *Result = (f32 *)&Inf;
	return(*Result);
}

inline f32
F32Min(void)
{
	u32 NegInf = 0xff800000;
	f32 *Result = (f32 *)&NegInf;
	return(*Result);	
}

inline s16
F32RoundToS16(f32 X)
{
	s16 Result = (s16)floorf(X);
	return(Result);
}

inline s32
F32RoundToS32(f32 X)
{
	s32 Result = (s32)roundf(X);
	return(Result);
}

inline u32
F32RoundToU32(f32 X)
{
	u32 Result = (u32)(X + 0.5f);
	return(Result);
}

inline s32
F32TruncateToS32(f32 X)
{
	s32 Result = (s32)X;
	return(Result);
}

inline f32
F32FractPart(f32 X)
{
	f32 Result = 0.0f;
	if(X > 0)
	{
		Result = X - F32RoundToS32(X);
	}
	else
	{
		Result = X - F32RoundToS32(X + 1.0f);
	}
	return(Result);
}

inline s32
F32FloorToS32(f32 X)
{
	s32 Result = (s32)floorf(X);
	return(Result);
}

inline s32
F32CeilToS32(f32 X)
{
	s32 Result = (s32)ceil(X);
	return(Result);
}

inline f32
Sin(f32 angle)
{
	f32 Result = sinf(angle);
	return(Result);
}

inline f32
Cos(f32 angle)
{
	f32 Result = cosf(angle);
	return(Result);
}

inline f32
Sqrt(f32 X)
{
	f32 Result = sqrtf(X);
	return(Result);
}

inline f32
Min(f32 X, f32 Y)
{
	f32 Result = fminf(X, Y);
	return(Result);
}

inline f32
Max(f32 X, f32 Y)
{
	f32 Result = fmaxf(X, Y);
	return(Result);
}

struct bit_scan_result
{
	u32 Index;
	b32 Found;
};

inline bit_scan_result
U32FindFirstLeastSigBit(u32 X)
{
	bit_scan_result Result = {};
#if COMPILER_MSVC
	Result.Found = _BitScanForward((unsigned long *)&Result.Index, X);
#else
	for(u32 Index = 0; Index < 32; ++Index)
	{
		if((1 << Index) & X)
		{
			Result.Index = Index;
			Result.Found = true;
			break;
		}
	}
#endif
	return(Result);
}

inline u32
RotateLeft(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
	u32 Result = _rotl(Value, Amount);
#else
	Amount &= 31;
	u32 Result = ((Value >> Amount) | (Value >> (32 - Amount)));
#endif
	return(Result);
}

#define APP_INTRINSICS_H
#endif
