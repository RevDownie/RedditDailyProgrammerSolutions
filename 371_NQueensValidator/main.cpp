#include <stdio.h>

///  Interesting linear approach to detect duplicates in a list I found here https://www.geeksforgeeks.org/find-duplicates-in-on-time-and-constant-extra-space/
///  It does involve reorganising the coord array but that doesn't really matter here. It also only works for integers
///
bool check_row(int* coords, int length)
{
    for(int i=0; i<length; ++i)
    {
        int absVal = abs(coords[i]);
        if(coords[absVal] < 0)
        {
            //Duplicate row so invalid result
            return false;
        }

        coords[absVal] = -coords[absVal];
    }

    return true;
}

/// Standard N^2 check to see if any of the given coords share a pos or neg diagonal
///
bool check_diagonal(const int* coords, int length)
{
    for(int i=0; i<length; ++i)
    {
        for(int j=i+1; j<length; ++j)
        {
            //Check positive diagonal and negative diagonal
            //y2-y1 = x2-x1 / y2-y1 = x1-x2
            float dy = coords[j] - coords[i];
            if(dy == j - i || dy == i - j)
            {
                return false;
            }
        }
    }

    return true;
}

/// Given an array of 8 integers between 1 and 8 where the index represents the column and the value the row
/// determine whether a queen at each of those locations on an 8x8 chess board would be safe from all other queens.
///
/// Essentially we need to check if any 2 queens share a col, row or diagonal
///
/// * Because there is a queen at each index (column) we can guarantee that no 2 queens will share a column
/// * If any of the values in the array are the same then the queens share a row
/// * Both positive and negative diagonals will have to be checked - ended up just doing a N^2 check for this.
///
int main()
{
    const int k_numTestCases = 5;
    const int k_numQueens = 8;

    int testCases[k_numTestCases][k_numQueens] =
    {
        //Valid
        {4, 2, 7, 3, 6, 8, 5, 1},
        {2, 5, 7, 4, 1, 8, 6, 3},

        //Invalid
        {1, 2, 3, 1, 4, 4, 5, 6},
        {5, 3, 1, 4, 2, 8, 6, 3},
        {5, 8, 2, 4, 7, 1, 3, 6}
    };

    for(int i=0; i<k_numTestCases; ++i)
    {
        bool row_result = check_row(testCases[i], k_numQueens);
        bool diag_result = false;
        if(row_result == true)
        {
            diag_result = check_diagonal(testCases[i], k_numQueens);
        }
        printf("%s\n", row_result && diag_result ? "valid" : "invalid");
    }

    return 0;
}
