#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>

struct StringBuffer
{
    int Num;
    const char** Indexer;
    const char* Buffer;
};

/// Takes an array of strings and outputs a Morse coded version for each input
/// Morse codes are generated in the "Smoosh" fashion (no spaces between letters)
///
/// The strings are allocated in a contiguous buffer but are accessible through an array of strings which means we only need
/// an allocation for the buffer and the index and similarly only 2 deallocations rather than an allocation per string
///
StringBuffer MorseEncode(const char** inputStrings, int numInputs, const char** morseAlphabet, int maxAlphabetCharLen)
{
    const char** indexer = (const char**)malloc(numInputs * sizeof(const char*));

    // Need to calculate how much memory is required to store all morse encoded strings in one chunk
    // Then we can allocate a single buffer at that size to hold the characters contiguously
    // Rather than going through the conversion fully to find the actual size we will just allocate
    // for the max size
    size_t totalOutputLen = numInputs; //Including null terminators
    for(int i=0; i<numInputs; ++i)
    {
        const char* input = inputStrings[i];
        size_t maxEncodedLen = strlen(input) * maxAlphabetCharLen;
        totalOutputLen += maxEncodedLen;
    }
    char* buffer = (char*)malloc(totalOutputLen * sizeof(char));

    // Now actually perform the conversion using the morseAlphabet as a lookup table to convert from ASCII to Morse
    // Each character is stored in the buffer but we use the indexer to point to the start of each string allowing
    // us to iterate over the buffer per input
    char* next = buffer;
    for(int i=0; i<numInputs; ++i)
    {
        indexer[i] = next; //Start of this output string

        const char* input = inputStrings[i];
        while(*input != '\0')
        {
            int index = *input - 'a';
            const char* morse = morseAlphabet[index];
            while(*morse != '\0')
            {
                *next = *morse;
                ++morse;
                ++next;
            }

            ++input;
        }

        *next = '\0';
        ++next;
    }

    // Convert from a single block to an array of strings
    StringBuffer output;
    output.Indexer = indexer;
    output.Buffer = buffer;
    output.Num = numInputs;
    return output;
}

/// Read a bunch of newline separated input strings from file that we can use to test the morse encoding
/// 
/// The strings are allocated in a contiguous buffer but are accessible through an array of strings which means we only need
/// an allocation for the buffer and the index and similarly only 2 deallocations rather than an allocation per string
///
StringBuffer ReadInputFile(const char* inputFileName)
{
    FILE* fp = fopen(inputFileName, "r");

    // Find the number of lines
    int numLines = 0;
    int numChars = 0;
    int ch = 0;
    do                                                                                                 
    {                                                                                                  
        ch = fgetc(fp);
        ++numChars;
        if (ch == '\n')                                                                                
            numLines++;

    } while (ch != EOF);                                                                               

    rewind(fp); 

    // Allocate a contiguous buffer to hold all strings
    char* buffer = (char*)malloc(numChars * sizeof(char));
    char* bufferNext = buffer;

    // Each line is an input
    char** inputs = (char**)malloc(numLines * sizeof(const char*));

    size_t lineBufferSize = 256;
    for(int i=0; i<numLines; ++i)
    {
        ssize_t lineSize = getline(&bufferNext, &lineBufferSize, fp) - 2; //Ignore new line chars \r\n
        inputs[i] = bufferNext;

        // Add a null terminator
        bufferNext[lineSize] = '\0';
        bufferNext += lineSize + 1;
    }

    fclose(fp);

    StringBuffer result;
    result.Num = numLines;
    result.Indexer = (const char**)inputs;
    result.Buffer = buffer;
    return result;
}

/// Search through the array of strings and find the index of the string that first meets the minimum contiguous number of the given character
/// e.g. ...-----... contains 5 contiguous dashes so would meet the criteria "contigChar = -" "minNumContiguous = 5"
///
/// Functon returns -1 if no strings found that match the criteria
///
int FindIndexWithContiguousChar(char contigChar, int minNumContiguous, const char** strings, int numStrings)
{
    for(int i=0; i<numStrings; ++i)
    {
        int numContig = 0;
        const char* string = strings[i];
        while(*string != '\0')
        {
            if(*string == contigChar)
            {
                ++numContig;
                if(numContig >= minNumContiguous)
                    return i;
            }
            else
            {
                numContig = 0;
            }

            ++string;
        }
    }

    return - 1;
}

/// Really easy challenge of outputting the "smooshed" Morse code for a given word. This one forms the basis
/// of future harder challenges
///
int main()
{
    const char* morseAlphabet[26] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};

    auto start = std::chrono::system_clock::now();
    StringBuffer inputs = ReadInputFile("input.txt");
    StringBuffer encodedOutputs = MorseEncode(inputs.Indexer, inputs.Num, morseAlphabet, 4);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end - start;
    printf("Time Taken to encode %d strings: %f\n", inputs.Num, duration.count());


    start = std::chrono::system_clock::now();
    int contigIndex = FindIndexWithContiguousChar('-', 15, encodedOutputs.Indexer, encodedOutputs.Num);
    end = std::chrono::system_clock::now();
    duration = end - start;
    printf("Time Taken to find contiguous string %s(%s): %f\n", inputs.Indexer[contigIndex], encodedOutputs.Indexer[contigIndex], duration.count());

    // for(int i=0; i<encodedOutputs.Num; ++i)
    // {
    //     const char* morseText = encodedOutputs.Indexer[i];
    //     printf("Input: %s Output: %s\n", inputs.Indexer[i], morseText);
    // }

    // Just ending the program so don't bother freeing the memory
    // free(inputs.Indexer);
    // free(inputs.Buffer);
    // free(encodedOutputs.Indexer);
    // free(encodedOutputs.Buffer);

    return 0;
}
