#include <vector>
#include <cstdio>

#include "gaussian.h"

using namespace std;

#define max(a,b) (a>b?a:b)

unsigned long long powMod(unsigned long long A, int d, unsigned long long p) {
	if(d == 0) return 1L;
	if(d == 1) return A % p;

	unsigned long long halfResult = powMod(A,d/2,p);

	if(d%2) return (halfResult*halfResult*A) % p;
	else return (halfResult*halfResult) % p;
}

struct SimpleMatrix {
	vector<vector<bool>> tab;
	int rows, cols;

	vector<int> rowsMap;
	vector<int> colsMap;

	SimpleMatrix(vector<vector<bool>> _tab) :
		tab(_tab),
		rows(_tab.size()),
		cols(_tab[0].size())
	{
		rowsMap = vector<int>(rows+1);
		colsMap = vector<int>(cols+1);

		for(int i = 0; i <= rows; i++)
			rowsMap[i] = i;
		for(int i = 0; i <= cols; i++)
			colsMap[i] = i;
	}

	int findNonZeroInRow(int row, int begin) {
		for(int i = begin; i<cols; i++) if(getVal(row,i)) return i;
		return -1;
	}

	void swapCols(int A, int B) {
		if (A == B)
			return;

		colsMap[A] ^= colsMap[B];
		colsMap[B] ^= colsMap[A];
		colsMap[A] ^= colsMap[B];
	}

	void swapRows(int A, int B) {
		if (A == B)
			return;

		rowsMap[A] ^= rowsMap[B];
		rowsMap[B] ^= rowsMap[A];
		rowsMap[A] ^= rowsMap[B];
	}

	void subRows(int A, int B, int begin) {
		for(int i = begin; i < cols; i++)
			setVal(B, i, getVal(B,i) ^ getVal(A,i));
	}

	void subRowFromAllBelow(int row, int begin) {
		for(int i = row+1; i<rows; i++)
			if (getVal(i,begin))
				for(int j = begin; j < cols; j++)
					setVal(i, j, getVal(i,j) ^ getVal(row,j));
	}

	unsigned long long getVal(int r, int c) {
		return tab[rowsMap[r]][colsMap[c]];
	}

	void setVal(int r, int c, unsigned long long val) {
		tab[rowsMap[r]][colsMap[c]] = val;
	}

	vector<int> gaussianElimination() {
		int currentRow = 0, currentCol = 0;
		while(currentRow < rows && currentCol < cols) {
			if(getVal(currentRow,currentCol)) {
				subRowFromAllBelow(currentRow,currentCol);

				currentRow++;
				currentCol++;
			}
			else {
				int zero = findNonZeroInRow(currentRow,currentCol);
				if(zero != -1) {
					swapCols(currentCol, zero);
				} else {
					swapRows(currentRow, rows-1);
					rows--;
				}
			}
		}

		vector<int> result;

		for (int i = 0; i < min(cols,rows); i++)
			result.push_back(rowsMap[i]);

		return result;
	}
};

vector<int> getSignificantRows(vector<vector<bool>> const & rows) {
	SimpleMatrix matrix(rows);
	return matrix.gaussianElimination();
}

