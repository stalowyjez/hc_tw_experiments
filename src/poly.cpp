#include <iostream>
#include <bitset>

#include "poly.h"

using namespace std;

poly_t polyMul(poly_t A, poly_t B) {
	unsigned long long result;
	asm (
		"movq %1, %%xmm0;"
		"movq %2, %%xmm1;"

		"pclmulqdq $0, %%xmm0, %%xmm1;"
		"movhlps %%xmm1, %%xmm0;"

		"movq %%xmm0, %%rdx;"
		"movq %%xmm1, %%rax;"

		"movq %%rdx, %%rcx;"

		"shr $60, %%rcx;"
		"xor %%rcx, %%rdx;"
		"shr $1, %%rcx;"
		"xor %%rcx, %%rdx;"
		"shr $2, %%rcx;"
		"xor %%rcx, %%rdx;"

		"xor %%rdx, %%rax;"
		"shl $1, %%rdx;"
		"xor %%rdx, %%rax;"
		"shl $2, %%rdx;"
		"xor %%rdx, %%rax;"
		"shl $1, %%rdx;"
		"xor %%rdx, %%rax;"

		"movq %%rax, %0;"
		: "=r" (result)
		: "r" (A), "r" (B)
	);
	return result;
}

poly_t reduceOnly(poly_t A, poly_t B) {
	unsigned long long result;
	asm (
		"movq %1, %%rax;"
		"movq %2, %%rdx;"

		"movq %%rdx, %%rcx;"

		"shr $60, %%rcx;"
		"xor %%rcx, %%rdx;"
		"shr $1, %%rcx;"
		"xor %%rcx, %%rdx;"
		"shr $2, %%rcx;"
		"xor %%rcx, %%rdx;"

		"xor %%rdx, %%rax;"
		"shl $1, %%rdx;"
		"xor %%rdx, %%rax;"
		"shl $2, %%rdx;"
		"xor %%rdx, %%rax;"
		"shl $1, %%rdx;"
		"xor %%rdx, %%rax;"

		"movq %%rax, %0;"
		: "=r" (result)
		: "r" (A), "r" (B)
	);
	return result;
}

void initRandomPolys() {
	srand(time(NULL));
}

poly_t randomPoly() {
	poly_t low_a = (poly_t)(unsigned int)rand();
	poly_t low_b = (poly_t)(unsigned int)rand();
	poly_t high_a = (poly_t)(unsigned int)rand();
	poly_t high_b = (poly_t)(unsigned int)rand();

	auto low = low_a ^ (low_b<<1);
	auto high = high_a ^ (high_b<<1);

	return (high<<32)+low;
}

