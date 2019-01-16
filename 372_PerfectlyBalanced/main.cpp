#include <string.h>
#include <stdio.h>

/// Given a list of strings of lowercase ascii alphabet characters check if each
/// character in each string appears the same number of times and assign the result
/// to the preallocated results array
///
void CheckBalanced(int numStrings, const char** strings, bool* out_results)
{
    int* charTotals = (int*)calloc(26, sizeof(int));

    for(int stringIdx=0; stringIdx<numStrings; ++stringIdx)
    {
        const char* characters = strings[stringIdx];
        int length = strlen(characters);

        //Count the number of each character
        int lastTotal = 0;
        for(int charIdx=0; charIdx<length; ++charIdx)
        {
            int mapIndex = characters[charIdx] - 97;
            charTotals[mapIndex]++;
            lastTotal = charTotals[mapIndex];
        }

        //Check for differences in number of each characters
        bool result = true;
        for(int i=0; i<26; ++i)
        {
            if(charTotals[i] > 0 && charTotals[i] != lastTotal)
            {
                result = false;
                break;
            }
        }

        out_results[stringIdx] = result;
        memset(charTotals, 0, sizeof(int) * 26);
    }

    free(charTotals);
}

/// Tests
///
int main()
{
    const int k_numTestCases = 8;

    const char* testCases[k_numTestCases] =
    {
        //Balanced
        "xxxyyy",
        "abcdz",
        "",
        "a",

        //Unbalanced
        "aac",
        "xxyyy",
        "ppppwwee",
        "xxxyyyz"
    };

    bool* results = new bool[k_numTestCases]; //Don't need to delete
    CheckBalanced(k_numTestCases, testCases, results);
    for(int i=0; i<k_numTestCases; ++i)
    {
        printf("%s\n", results[i] ? "balanced" : "unbalanced");
    }

    return 0;
}
