typedef unsigned long long poly_t;

// Multiplication

poly_t reduceOnly(poly_t A, poly_t B);
poly_t polyMul(poly_t A, poly_t B);

// Random polynomials

void initRandomPolys();
poly_t randomPoly();

