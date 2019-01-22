#include <stdio.h>

/// Simple ascii to int conversion using ascii table 48 => 0
///
int AsciiToInt(char ascii)
{
    assert(ascii >= '0' && ascii <= '9');
    return ascii - '0';
}

/// Calculate UPC check digits
///
/// Essentially perform a series of maths functions on a list of integers to obtain a result
/// 1. Sum even-indexed digits
/// 2. Multiply by 3
/// 3. Sum odd-indexed digits and add to current
/// 4. Modulo by 10
/// 4. If 0 then result is zero, else 10 - M
///
int CalculateCheckDigit(const char* number, int length)
{
    int evenSum = 0;
    for(int i=0; i<length; i+=2)
    {
        int val = AsciiToInt(number[i]);
        evenSum += val;
    }

    int oddSum = 0;
    for(int i=1; i<length; i+=2)
    {
        int val = AsciiToInt(number[i]);
        oddSum += val;
    }

    int checkDigit = (evenSum * 3 + oddSum) % 10;
    return checkDigit > 0 ? (10 - checkDigit) : 0;
}

///
int main()
{
    const int k_numTestCases = 4;
    const char* testCases[k_numTestCases] =
    {
        "04210000526",
        "03600029145",
        "12345678910",
        "00001234567"
    };

    for(int i=0; i<k_numTestCases; ++i)
    {
        int length = strlen(testCases[i]);
        assert(length == 11);
        printf("%d\n", CalculateCheckDigit(testCases[i], length));
    }

    return 0;
}
