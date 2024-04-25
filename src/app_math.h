#if !defined(APP_MATH_H)

struct v2i
{
	s32 x, y;
};

union v2f
{
	struct
	{
		f32 x, y;
	};
	struct
	{
		f32 u, v;
	};
	f32 e[2];
};

union v3f
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 u, v, w;
	};
	struct
	{
		f32 r, g, b;
	};
	struct
	{
		v2f xy;
		f32 ignored0_;
	};
	struct
	{
		f32 ignored1_;
		v2f yz;
	};
	struct
	{
		v2f uv;
		f32 ignored2_;
	};
	f32 e[3];
};

struct basis
{
	v3f O;
	v3f U;
	v3f V;
	v3f W;
};

union v4f
{
	struct
	{
		f32 x, y, z, w;
	};
	struct
	{
		union
		{
			v3f xyz;
			struct
			{
				f32 x, y, z;
			};
		};
		f32 w;
	};
	struct
	{
		union
		{
			v3f rgb;
			struct
			{
				f32 r, g, b;
			};
		};
		f32 a;
	};
	struct
	{
		v2f xy;
		f32 ignored0_;
		f32 ignored1_;
	};
	struct
	{
		f32 ignored2_;
		v2f yz;
		f32 ignored3_;
	};
	struct
	{
		f32 ignored4_;
		f32 ignored5_;
		v2f zw;
	};
	f32 e[4];
};

struct m2x2
{
	f32 e[2][2];
};

struct m3x3
{
	f32 e[3][3];
};

struct mat4
{
	f32 e[4][4];
};


inline v2i
V2I(s32 x, s32 y)
{
	v2i Result;

	Result.x = x;
	Result.y = y;

	return(Result);

}

inline v2f
V2F(f32 x, f32 y)
{
	v2f Result;

	Result.x = x;
	Result.y = y;

	return(Result);
}

inline v2f
V2F(f32 c)
{
	v2f Result;

	Result.x = c;
	Result.y = c;

	return(Result);
}


inline v3f
V3F(f32 x, f32 y, f32 z)
{
	v3f Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;

	return(Result);
}

inline v3f
V3F(v2f XY, f32 z)
{
	v3f Result;
	
	Result.xy = XY;
	Result.z = z;

	return(Result);
}

inline v3f
V3F(f32 c)
{
	v3f Result;

	Result.x = c;
	Result.y = c;
	Result.z = c;

	return(Result);
}

inline v4f
V4F(f32 x, f32 y, f32 z, f32 w)
{
	v4f Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;
	Result.w = w;

	return(Result);
}

inline v4f
V4F(f32 c)
{
	v4f Result;

	Result.x = c;
	Result.y = c;
	Result.z = c;
	Result.w = c;

	return(Result);
}

inline v4f
V4F(v3f XYZ, f32 w)
{
	v4f Result;

	Result.xyz = XYZ;
	Result.w = w;

	return(Result);
}

inline v3f
XAxis()
{
	v3f Result = {1.0f, 0.0f, 0.0f};
	return(Result);
}

inline v3f
YAxis()
{
	v3f Result = {0.0f, 1.0f, 0.0f};
	return(Result);
}

inline v3f
ZAxis()
{
	v3f Result = {0.0f, 0.0f, 1.0f};
	return(Result);
}

//
// NOTE(Justin): Scalar operations
//

inline f32
Square(f32 X)
{
	f32 Result = X * X;
	return(Result);
}

inline f32
Lerp(f32 a, f32 t, f32 b)
{
	f32 Result = (1.0f - t) * a + t * b;
	
	return(Result);
}

inline f32
Clamp(f32 min, f32 x, f32 max)
{
	f32 Result = x;
	if(Result < min)
	{
		Result = min;
	}
	else if(Result > max)
	{
		Result = max;
	}

	return(Result);
}

inline f32
Clamp01(f32 x)
{
	f32 Result = Clamp(0.0f, x, 1.0f);

	return(Result);
}

inline f32
F32SafeRatioN(f32 Numerator, f32 Divisor, f32 N)
{
	f32 Result = N;

	if(Divisor != 0.0f)
	{
		Result = Numerator / Divisor;
	}

	return(Result);
}

inline f32
F32SafeRatio0(f32 Numerator, f32 Divisor)
{
	f32 Result = F32SafeRatioN(Numerator, Divisor, 0.0f);
	return(Result);
}

inline f32
F32SafeRatio1(f32 Numerator, f32 Divisor)
{
	f32 Result = F32SafeRatioN(Numerator, Divisor, 1.0f);
	return(Result);
}

inline f32
Min3(f32 X, f32 Y, f32 Z)
{
	f32 Result = 0.0f;

	f32 MinXY = Min(X, Y);
	f32 MinXZ = Min(X, Z);

	Result = Min(MinXY, MinXZ);

	return(Result);
}

inline f32
Max3(f32 X, f32 Y, f32 Z)
{
	f32 Result = 0.0f;

	f32 MaxXY = Max(X, Y);
	f32 MaxXZ = Max(X, Z);

	Result = Max(MaxXY, MaxXZ);

	return(Result);
}

//
// NOTE(Justin): v2f operations
//



inline v2i
operator *(f32 c, v2i V)
{
	v2i Result;

	Result.x = (s32)(c * V.x);
	Result.y = (s32)(c * V.y);

	return(Result);
}

inline v2i
operator *(v2i V, f32 c)
{
	v2i Result;

	Result = c * V;

	return(Result);
}

inline v2i
operator +(v2i V1, v2i V2)
{
	v2i Result;

	Result.x = V1.x + V2.x;
	Result.y = V1.y + V2.y;

	return(Result);
}

inline v2f
operator *(f32 c, v2f V)
{
	v2f Result;
	Result.x = c * V.x;
	Result.y = c * V.y;

	return(Result);
}

inline v2f
operator *(v2f V, f32 c)
{
	v2f Result = c * V;

	return(Result);
}

inline v2f &
operator *=(v2f &V, f32 c)
{
	V = c * V;

	return(V);
}

inline v2f
operator +(v2f U, v2f V)
{
	v2f Result;

	Result.x = U.x + V.x;
	Result.y = U.y + V.y;

	return(Result);
}

inline v2f &
operator +=(v2f &U, v2f V)
{
	U = U + V;

	return(U);
}

inline v2f
operator -(v2f U, v2f V)
{
	v2f Result;
	Result.x = U.x - V.x;
	Result.y = U.y - V.y;
	
	return(Result);
}

inline f32
Dot(v2f V1, v2f V2)
{
	f32 Result = V1.x * V2.x + V1.y * V2.y;

	return(Result);
}

inline f32
Length(v2f V)
{
	f32 Result = Sqrt(Dot(V, V));

	return(Result);
}

inline v2f
Normalize(v2f V)
{
	v2f Result;

	Result = (1.0f / Length(V)) * V;

	return(Result);
}

inline f32
LengthSq(v2f V)
{
	f32 Result = Dot(V, V);

	return(Result);
}

inline f32
Det(v2f A, v2f B)
{
	f32 Result = 0.0f;

	Result = A.x * B.y - A.y * B.x;

	return(Result);
}

inline v2f
Hadamard(v2f U, v2f V)
{
	v2f Result;

	Result.x = U.x * V.x;
	Result.y = U.y * V.y;

	return(Result);
}

inline v2f
Perp(v2f V)
{
	v2f Result = {-V.y, V.x};
	return(Result);
}


//
// NOTE(Justin) v3f operations
//

inline b32 
V3FNotZero(v3f V)
{
	b32 Result = ((V.x != 0.0f) ||
				  (V.y != 0.0f) ||
				  (V.z != 0.0f));

	return(Result);
}


inline v3f
operator *(f32 c, v3f V)
{
	v3f Result = {};

	Result.x = c * V.x;
	Result.y = c * V.y;
	Result.z = c * V.z;

	return(Result);
}

inline v3f
operator *(v3f V, f32 c)
{
	v3f Result = c * V;
	return(Result);
}

inline v3f &
operator *=(v3f &V, f32 c)
{
	V = c * V;

	return(V);
}

inline v3f
operator +(v3f A, v3f B)
{
	v3f Result = {};

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return(Result);
}

inline v3f &
operator +=(v3f &U, v3f V)
{
	U = U + V;
	return(U);
}

inline v3f
operator -(v3f A, v3f B)
{
	v3f Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return(Result);
}

inline v3f &
operator -=(v3f &U, v3f V)
{
	U = U - V;
	return(U);
}

inline v3f
Lerp(v3f A, f32 t, v3f B)
{
	v3f Result = (1.0f - t) * A + t * B;
	return(Result);
}

#define SameDirection(A, B) (Dot((A), (B)) > 0 ? 1 : 0)
inline f32
Dot(v3f U, v3f V)
{
	f32 Result = U.x * V.x + U.y * V.y + U.z * V.z;
	return(Result);
}

inline f32
LengthSq(v3f V)
{
	f32 Result = Dot(V, V);
	return(Result);
}

inline f32
Length(v3f V)
{
	f32 Result = Sqrt(Dot(V, V));
	return(Result);
}

inline v3f
Normalize(v3f V)
{
	v3f Result;

	Result = (1.0f / Length(V)) * V;

	return(Result);
}

inline v3f
Hadamard(v3f U, v3f V)
{
	v3f Result;

	Result.x = U.x * V.x;
	Result.y = U.y * V.y;
	Result.z = U.z * V.z;

	return(Result);
}

inline v3f
Cross(v3f U, v3f V)
{
	v3f Result;

	Result.x = U.y * V.z - U.z * V.y;
	Result.y = U.z * V.x - U.x * V.z;
	Result.z = U.x * V.y - U.y * V.x;

	return(Result);
}

//
// NOTE(Justin): v4f operations
//

inline v4f
operator *(f32 c, v4f V)
{
	v4f Result = {};

	Result.x = c * V.x;
	Result.y = c * V.y;
	Result.z = c * V.z;
	Result.w = c * V.w;

	return(Result);
}

inline v4f
operator *(v4f V, f32 c)
{
	v4f Result = c * V;

	return(Result);
}

inline v4f
operator +(v4f U, v4f V)
{
	v4f Result;

	Result.x = U.x + V.x;
	Result.y = U.y + V.y;
	Result.z = U.z + V.z;
	Result.w = U.w + V.w;

	return(Result);
}

inline v4f &
operator *=(v4f &V, f32 c)
{
	V = c * V;

	return(V);
}

inline f32
Dot(v4f U, v4f V)
{
	f32 Result = 0.0f;

	Result = U.x + V.x + U.y * V.y + U.z * V.z + U.w * V.w;

	return(Result);
}

inline f32
Length(v4f V)
{
	f32 Result = Sqrt(Dot(V, V));

	return(Result);
}

inline v4f
Normalize(v4f V)
{
	v4f Result;

	Result = (1.0f / Length(V)) * V;

	return(Result);
}


inline v4f
Lerp(v4f A, f32 t, v4f B)
{
	v4f Result = (1.0f - t) * A + t * B;
	
	return(Result);
}

inline v4f
Hadamard(v4f U, v4f V)
{
	v4f Result;

	Result.x = U.x * V.x;
	Result.y = U.y * V.y;
	Result.z = U.z * V.z;
	Result.w = U.w * V.w;

	return(Result);
}

//
// NOTE(Justin): mat4
//

inline mat4
Mat4(v3f E1, v3f E2, v3f E3)
{
	mat4 R;

	R =
	{
		{{E1.x, E2.x, E3.x, 0.0f},
		{E1.y, E2.y, E3.y, 0.0f},
		{E1.z, E2.z, E3.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}}
	};

	return(R);
}

internal mat4
operator *(mat4 A, mat4 B)
{
	mat4 R = {};

	for(int i = 0; i <= 3; ++i)
	{
		for(int j = 0; j <= 3; ++j)
		{
			for(int k = 0; k <= 3; ++k)
			{
				R.e[i][j] += A.e[i][k] * B.e[k][j];
			}
		}
	}

	return(R);
}

internal v4f
Transform(mat4 A, v4f P)
{
	v4f R;

    R.x = P.x * A.e[0][0] + P.y * A.e[0][1] + P.z * A.e[0][2] + P.w * A.e[0][3];
    R.y = P.x * A.e[1][0] + P.y * A.e[1][1] + P.z * A.e[1][2] + P.w * A.e[1][3];
    R.z = P.x * A.e[2][0] + P.y * A.e[2][1] + P.z * A.e[2][2] + P.w * A.e[2][3];
    R.w = P.x * A.e[3][0] + P.y * A.e[3][1] + P.z * A.e[3][2] + P.w * A.e[3][3];	

	return(R);

}

inline v3f
operator *(mat4 A, v3f P)
{
	v3f R = Transform(A, V4F(P, 1.0f)).xyz;
	return(R);
}

inline v4f
operator *(mat4 A, v4f P)
{
	v4f R = Transform(A, P);
	return(R);
}

inline mat4
Mat4Identity(void)
{
	mat4 R =
	{
		{{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},

	};

	return(R);
}

inline mat4
Mat4Scale(f32 C)
{
	mat4 R =
	{
		{{C, 0, 0, 0},
		{0, C, 0, 0},
		{0, 0, C, 0},
		{0, 0, 0, 1}},
	};

	return(R);
}

inline mat4
Mat4Stretch(f32 A, f32 B, f32 C)
{
	mat4 R =
	{
		{{A, 0, 0, 0},
		{0, B, 0, 0},
		{0, 0, C, 0},
		{0, 0, 0, 1}},
	};

	return(R);
}

inline mat4
Mat4XShear(f32 Y, f32 Z)
{
	mat4 R =
	{
		{{1, Y, Z, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};

	return(R);
}


inline mat4
Mat4Translation(v3f V)
{
	mat4 R =
	{
		{{1, 0, 0, V.x},
		{0, 1, 0, V.y},
		{0, 0, 1, V.z},
		{0, 0, 0, 1}},
	};

	return(R);
}

inline mat4
Mat4Translation(f32 X, f32 Y, f32 Z)
{
	mat4 R =
	{
		{{1, 0, 0, X},
		{0, 1, 0, Y},
		{0, 0, 1, Z},
		{0, 0, 0, 1}},
	};

	return(R);
}

inline mat4
Mat4YRotation(f32 Angle)
{
	mat4 R =
	{
		{{Cos(Angle), 0 , Sin(Angle), 0},
		{0, 1, 0, 0},
		{-1.0f * Sin(Angle), 0, Cos(Angle), 0},
		{0, 0, 0, 1}},
	};

	return(R);
}

inline mat4
Mat4XRotation(f32 Angle)
{
	mat4 R =
	{
		{{1, 0, 0, 0},
		{0, Cos(Angle), Sin(Angle), 0},
		{0, -1.0f * Sin(Angle), Cos(Angle), 0},
		{0, 0, 0, 1}},
	};
	return(R);
}

inline mat4
Mat4ZRotation(f32 Angle)
{
	mat4 R =
	{
		{{Cos(Angle), -1.0f * Sin(Angle), 0, 0},
		{Sin(Angle), Cos(Angle), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};
	return(R);
}



inline mat4
Mat4Transpose(mat4 M)
{
	mat4 R =
	{
		M.e[0][0], M.e[1][0], M.e[2][0], M.e[3][0],
		M.e[0][1], M.e[1][1], M.e[2][1], M.e[3][1],
		M.e[0][2], M.e[1][2], M.e[2][2], M.e[3][2],
		M.e[0][3], M.e[1][3], M.e[2][3], M.e[3][3],
	};

	return(R);
}

inline mat4
Mat4TransposeMat3(mat4 M)
{
	mat4 R = M;

	for(u32 j = 0; j < 3; ++j)
	{
		for(u32 i = 0; i < 3; ++i)
		{
			if((i != j) && (i < j))
			{
				f32 Temp = R.e[j][i];
				R.e[j][i] = R.e[i][j];
				R.e[i][j] = Temp;
			}
		}
	}

	return(R);
}

inline mat4
Mat4WorldSpaceMap(v3f V)
{
	mat4 R = Mat4Translation(V);
	return(R);
}

inline mat4
Mat4PerspectiveGL(f32 FOV, f32 AspectRatio, f32 ZNear, f32 ZFar)
{
	mat4 R = {};

	f32 HalfFOV = FOV / 2.0f;

#if 0
	R.e[0][0] = 1.0f / (tanf(HFOV) * AspectRatio);
    R.e[1][1] = 1.0f / tanf(HFOV);
    R.e[2][3] = 1.0f;
    R.e[2][2] = (ZFar + ZNear) / (ZFar - ZNear);
    R.e[3][2] = -(2.0f *ZFar*ZNear) / (ZFar - ZNear);
#endif
	R =
	{
		{{1.0f / (tanf(HalfFOV) * AspectRatio), 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f / tanf(HalfFOV), 0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f * (ZFar + ZNear) / (ZFar - ZNear), -1.0f},
		{0.0f, 0.0f, -1.0f, 0.0f}}
	};

	return(R);
}

internal mat4
Mat4CameraMap(v3f P, v3f Target)
{
	mat4 R;

	// NOTE(Justin): The 3 vectors constructed from P and Target are basis vectors that
	// are column vectors of the rotation matrix. The Mat4 function puts the
	// vectors in a mat4 as COLUMN vectors. I.e.
	//
	//	Mat4(X, Y, Z) = |X Y Z 0|
	//					|0 0 0 1|
	//
	// After constructing this matrix the final rotation is the inverse of this
	// matrix which is equivalent to its transpose.

	v3f CameraDirection = Normalize(P - Target);
	v3f CameraRight = Normalize(Cross(YAxis(), CameraDirection));
	v3f CameraUp = Normalize(Cross(CameraDirection, CameraRight));

	mat4 Rotate = Mat4TransposeMat3(Mat4(CameraRight, CameraUp, CameraDirection));

	mat4 Translate = Mat4Translation(-1.0f * P);

	R = Rotate * Translate;

	return(R);
}

// NOTE(Justin): Should the screen space map include the perspective divide
internal mat4
Mat4ScreenSpaceMap(int Width, int Height)
{
	mat4 R
	{
		{{Width / 2.0f, 0, 0, (Width - 1) / 2.0f},
		{0, Height / 2.0f, 0, (Height - 1) / 2.0f},
		{0, 0, 1, 0},
		{0, 0, 0, 1}},
	};

	return(R);
}

internal mat4
Mat4OrthographicProjection(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
	mat4 R =
	{
		{{(2.0f / (r - l)), 0, 0, (-1.0f * (r + l) / (r - l))},
		{0, (2.0f / (t - b)), 0, (-1.0f * (t + b) / (t - b))},
		{0, 0, (2.0f / (f - n)), (-1.0f * (f + n) / (f - n))},
		{0, 0, 0, 1}},
	};
	return(R);
}

internal v4f
Mat4Column(mat4 M, u32 ColumnIndex)
{
	Assert(ColumnIndex >= 0);
	Assert(ColumnIndex < 4);
	v4f Result = {};

	Result.x = M.e[0][ColumnIndex];
	Result.y = M.e[1][ColumnIndex];
	Result.z = M.e[2][ColumnIndex];
	Result.w = M.e[3][ColumnIndex];

	return(Result);
}

struct aabb 
{
	v3f Min;
	v3f Max;
};


inline aabb
AABBMinMax(v3f Min, v3f Max)
{
	aabb Result;

	Result.Min = Min;
	Result.Max = Max;

	return(Result);
}

inline aabb
AABBMinDim(v3f Min, v3f Dim)
{
	aabb Result;

	Result.Min = Min;
	Result.Max = Min + Dim;
}

inline aabb
AABBCenterHalfDim(v3f Center, v3f HalfDim)
{
	aabb Result;

	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return(Result);
}

inline aabb
AABBCenterDim(v3f Center, v3f Dim)
{
	aabb Result = AABBCenterHalfDim(Center, 0.5f * Dim);

	return(Result);
}

inline b32
IsInAABB(aabb AABB, v3f Test)
{
	b32 Result = ((AABB.Min.x <= Test.x) &&
				 (AABB.Min.y <= Test.y) &&
				 (AABB.Min.z <= Test.z) &&
				 (AABB.Max.x > Test.x) &&
				 (AABB.Max.y > Test.y) &&
				 (AABB.Max.z > Test.z));

	return(Result);



}

#define APP_MATH_H
#endif
