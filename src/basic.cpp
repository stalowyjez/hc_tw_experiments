#include "basic.h"

bool isDiffEmpty(std::vector<int> & A, std::vector<int> & B) {
	sort(A.begin(), A.end());
	sort(B.begin(), B.end());

	auto itrA = A.begin();
	auto itrB = B.begin();

	while (itrA != A.end()) {
		while(itrB != B.end() && *itrB < *itrA)
			itrB++;
		if (itrB == B.end() || *itrB > *itrA)
			return false;
		itrA++;
	}

	return true;
}

int firstInDiff(std::vector<int> & A, std::vector<int> & B) {
	auto itrA = A.begin();
	auto itrB = B.begin();

	while (itrA != A.end()) {
		while(itrB != B.end() && *itrB < *itrA)
			itrB++;
		if (itrB == B.end() || *itrB > *itrA)
			return *itrA;
		itrA++;
	}
	return -1;
}

std::vector<int> invPerm(std::vector<int> const & perm) {
	std::vector<int> result(perm.size());

	result[0] = 0;
	for (int i = 1; i < ((int) perm.size()); i++)
		result[perm[i]] = i;

	return result;
}

