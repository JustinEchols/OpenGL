#if !defined(APPLICATION_MATH_H)

inline glm::vec3
operator *(f32 c, glm::vec3 V)
{
	glm::vec3 Result;

	Result.x  = c * V.x;
	Result.y  = c * V.y;
	Result.z  = c * V.z;

	return(Result);
}

inline glm::vec3
operator *(glm::vec3 V, f32 c)
{
	glm::vec3 Result = c * V;
	return(Result);
}
#define APPLICATION_MATH_H
#endif
