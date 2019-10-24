#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

struct StringBuffer
{
    int Num;
    const char** Indexer;
    const char* Buffer;
};

struct IndexResult
{
    int Num;
    int* Indices;
};

/// Takes an array of strings and outputs a Morse coded version for each input
/// Morse codes are generated in the "Smoosh" fashion (no spaces between letters)
///
/// The strings are allocated in a contiguous buffer but are accessible through an array of strings which means we only need
/// an allocation for the buffer and the index and similarly only 2 deallocations rather than an allocation per string
///
/// The work is distributed over a number of worker threads
///
StringBuffer MorseEncode(const char** inputStrings, int numInputs, const char** morseAlphabet, int maxAlphabetCharLen) noexcept
{
    const char** indexer = (const char**)malloc(numInputs * sizeof(const char*));

    // Setup for the worker threads based on the number of supported cores
    const unsigned int numWorkers = std::thread::hardware_concurrency();
    const int numStringsPerWorker = numInputs/numWorkers;

    std::thread* workers = new std::thread[numWorkers];
    int* workerBufferOffsets = (int*)malloc(sizeof(int) * numWorkers);
    workerBufferOffsets[0] = 0;
    int workBuffIdx = 0;

    // Need to calculate how much memory is required to store all morse encoded strings in one chunk
    // Then we can allocate a single buffer at that size to hold the characters contiguously
    // Rather than going through the conversion fully to find the actual size we will just allocate
    // for the max size
    size_t totalOutputLen = numInputs; //Including null terminators

    int currentWorkloadCount = 0;
    for(int i=0; i<numInputs; ++i, ++currentWorkloadCount)
    {
        const char* input = inputStrings[i];
        size_t maxEncodedLen = strlen(input) * maxAlphabetCharLen;
        totalOutputLen += maxEncodedLen;

        if(currentWorkloadCount > numStringsPerWorker)
        {
            workerBufferOffsets[++workBuffIdx] = totalOutputLen + 1;
            currentWorkloadCount = 0;
        }
    }
    char* buffer = (char*)malloc(totalOutputLen * sizeof(char));

    // Now actually perform the conversion using the morseAlphabet as a lookup table to convert from ASCII to Morse
    // Each character is stored in the buffer but we use the indexer to point to the start of each string allowing
    // us to iterate over the buffer per input.
    // This is performed as a parallel job spread across a number of workers
    struct EncodeJob
    {
        void operator()(const char** inputStrings, int numInputs, const char** morseAlphabet, char* bufferStart, const char** indexerStart) const noexcept
        {
            char* next = bufferStart;
            for(int i=0; i<numInputs; ++i)
            {
                *indexerStart = next; //Start of this output string
                ++indexerStart;

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
        }
    };

    //Distribute the work evenly across the workers
    int numStringsAssigned = 0;
    for(int i=0; i<numWorkers-1; ++i)
    {
        workers[i] = std::thread(EncodeJob(), inputStrings + numStringsAssigned, numStringsPerWorker, morseAlphabet, buffer + workerBufferOffsets[i], indexer + numStringsAssigned);
        numStringsAssigned += numStringsPerWorker;
    }

    //Handle remainder
    int remainingWork = numInputs - numStringsAssigned;
    workers[numWorkers-1] = std::thread(EncodeJob(), inputStrings + numStringsAssigned, remainingWork, morseAlphabet, buffer + workerBufferOffsets[numWorkers-1], indexer + numStringsAssigned);

    //Wait for all the workers to finish
    for(int i=0; i<numWorkers; ++i)
    {
        workers[i].join();
    }

    delete[] workers;
    free(workerBufferOffsets);

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
StringBuffer ReadInputFile(const char* inputFileName) noexcept
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
int FindIndexWithContiguousChar(char contigChar, int minNumContiguous, const char** strings, int numStrings) noexcept
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

/// Search throught the array of strings and find the first string that occurs the given number of times
///
/// NOTE: I measured the peformance of this using a reserved unordered_map to store the counts of each (stopping when we reached the target) but
/// this method of sorting the array was much quicker
///
const char* FindReoccuringString(int minNumOccurrences, const char** strings, int numStrings) noexcept
{
    struct
    {
        int operator()(const char* a, const char* b) const
        {   
            return strcmp(a, b) < 0;
        }   
    } SortComp;

    const char** sortedStrings = (const char**)malloc(sizeof(char*) * numStrings);
    memcpy(sortedStrings, strings, sizeof(char*) * numStrings);
    std::sort(sortedStrings, sortedStrings + numStrings, SortComp);

    const char* result = nullptr;
    const char* last = sortedStrings[0];
    int count = 1;

    for(int i=1; i<numStrings; ++i)
    {
        const char* current = sortedStrings[i];
        if(strcmp(current, last) != 0)
        {
            last = current;
            count = 1;
        }
        else
        {
            ++count;
            if(count == minNumOccurrences)
            {
                result = current;
                break;
            }
        }
    }

    free(sortedStrings);
    return result;
}

/// Search throught the array of strings and find the N strings thats unencoded length is at least the min given and thats encoded strings have the same number of dots and dashes
///
IndexResult FindBalancedStrings(int maxToFind, int minLength, const char** unencodedStrings, const char** encodedStrings, int numStrings) noexcept
{
    int* resultIndices = (int*)malloc(sizeof(int) * maxToFind);
    int numFound = 0;
    int counts[2] = {0, 0};

    for(int i=0; i<numStrings; ++i)
    {
        int len = strlen(unencodedStrings[i]);
        if(len < minLength)
            continue;

        const char* string = encodedStrings[i];
        while(*string != '\0')
        {
            // Dash is 45 in ascii so will map to 0, dot is 46 so will map to 1
            int countIndex = *string - '-';
            counts[countIndex]++;
            ++string;
        }

        if(counts[0] == counts[1])
        {
            resultIndices[numFound++] = i;
            if(numFound == maxToFind)
                break;
        }

        counts[0] = counts[1] = 0;
    }

    IndexResult result;
    result.Indices = resultIndices;
    result.Num = numFound;
    return result;
}

/// Really easy challenge of generating the "smooshed" Morse code for a given word. This one forms the basis
/// of future harder challenges
///
int main() noexcept
{
    const char* morseAlphabet[26] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};

    auto start = std::chrono::system_clock::now();
    StringBuffer inputs = ReadInputFile("input.txt");
    StringBuffer encodedOutputs = MorseEncode(inputs.Indexer, inputs.Num, morseAlphabet, 4);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end - start;
    printf("Time Taken to encode %d strings: %f\n", inputs.Num, duration.count());

    //BP 1: The sequence -...-....-.--. is the code for four different words (needing, nervate, niding, tiling). Find the only sequence that's the code for 13 different words.
    start = std::chrono::system_clock::now();
    const char* reoccurringEncoding = FindReoccuringString(13, encodedOutputs.Indexer, encodedOutputs.Num);
    end = std::chrono::system_clock::now();
    duration = end - start;
    printf("Time Taken to find reoccurring string %s: %f\n", reoccurringEncoding, duration.count());

    //BP 2: autotomous encodes to .-..--------------..-..., which has 14 dashes in a row. Find the only word that has 15 dashes in a row.
    start = std::chrono::system_clock::now();
    int contigIndex = FindIndexWithContiguousChar('-', 15, encodedOutputs.Indexer, encodedOutputs.Num);
    end = std::chrono::system_clock::now();
    duration = end - start;
    printf("Time Taken to find contiguous string %s(%s): %f\n", inputs.Indexer[contigIndex], encodedOutputs.Indexer[contigIndex], duration.count());

    //BP 3: Call a word perfectly balanced if its code has the same number of dots as dashes. counterdemonstrations is one of two 21-letter words that's perfectly balanced. Find the other one.
    start = std::chrono::system_clock::now();
    IndexResult balancedIndices = FindBalancedStrings(2, 21, inputs.Indexer, encodedOutputs.Indexer, encodedOutputs.Num);
    end = std::chrono::system_clock::now();
    duration = end - start;
    printf("Time Taken to find perfectly %d balanced strings: %f\n", balancedIndices.Num, duration.count());
    for(int i=0; i<balancedIndices.Num; ++i)
    {
        printf("\tBalanced string %s(%s)\n", inputs.Indexer[balancedIndices.Indices[i]], encodedOutputs.Indexer[balancedIndices.Indices[i]]);
    }

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
    // free(balancedIndices.Indices);

    return 0;
}
