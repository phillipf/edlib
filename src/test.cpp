#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "myers.h"

using namespace std;


void runRandomTests(int numTests, int mode);
bool runTests();


int main() {
    srand(time(NULL));

    runRandomTests(100,  MYERS_MODE_HW);
    printf("\n");
    
    runRandomTests(10000,  MYERS_MODE_NW);
    printf("\n");

    printf("Specific tests:\n");
    if (runTests())
        printf("All specific tests passed!\n");
    else
        printf("Some specific tests failed\n");
    
    return 0;
}


int min(int x, int y) {
    return x < y ? x : y;
}

int min3(int x, int y, int z) {
    return min(x, min(y, z));
}

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength, int mode) {
    int* C = new int[queryLength];
    int* newC = new int[queryLength];

    int bestScore = -1;

    // set first column (column zero)
    for (int i = 0; i < queryLength; i++)
        C[i] = i+1;

      /*for (int i = 0; i < queryLength; i++)
        printf("%d ", C[i]);
        printf("\n");*/

    for (int c = 0; c < targetLength; c++) { // for each column
        newC[0] = min3((mode == MYERS_MODE_HW ? 0 : c + 1) + 1, // up
                       (mode == MYERS_MODE_HW ? 0 : c) + (target[c] == query[0] ? 0 : 1), // up left
                       C[0] + 1); // left
        for (int r = 1; r < queryLength; r++) {
            newC[r] = min3(newC[r-1] + 1, // up
                           C[r-1] + (target[c] == query[r] ? 0 : 1), // up left
                           C[r] + 1); // left
        }
        
        /*for (int i = 0; i < queryLength; i++)
            printf("%d ", newC[i]);
            printf("\n");*/

        if (mode == MYERS_MODE_HW || c == targetLength - 1) // For NW check only last column
            if (bestScore == -1 || newC[queryLength-1] < bestScore)
                bestScore = newC[queryLength-1];
        
        int *tmp = C;
        C = newC;
        newC = tmp;
    }

    delete[] C;
    delete[] newC;

    return bestScore;
}

void fillRandomly(unsigned char* seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = rand() % alphabetLength;
}

void runRandomTests(int numTests, int mode) {
    int alphabetLength = 10;
    int numTestsFailed = 0;
    clock_t start, finish;
    double timeMyers = 0;
    double timeSimple = 0;
    
    for (int i = 0; i < numTests; i++) {
        int queryLength = 10 + rand() % 2000;
        int targetLength = 100 + rand() % 2000;
        unsigned char query[queryLength];
        unsigned char target[targetLength];
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);

        start = clock();
        int score1 = myersCalcEditDistance(query, queryLength, target, targetLength,
                                           alphabetLength, -1, mode);
        timeMyers += clock() - start;
        
        start = clock();
        int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength,
                                            alphabetLength, mode);
        timeSimple += clock() - start;
        
        if (score1 != score2) {
            numTestsFailed++;
            printf("%d, %d\n", score1, score2);
        }
    }
    
    printf(mode == MYERS_MODE_HW ? "HW: " : "NW: ");
    printf(numTestsFailed == 0 ? "\x1B[32m" : "\x1B[31m");
    printf("%d/%d", numTests - numTestsFailed, numTests);
    printf("\x1B[0m");
    printf(" random tests passed!\n");
    double mTime = ((double)(timeMyers))/CLOCKS_PER_SEC;
    double sTime = ((double)(timeSimple))/CLOCKS_PER_SEC;
    printf("Time Myers: %lf\n", mTime);
    printf("Time Simple: %lf\n", sTime);
    printf("Times faster: %.2lf\n", sTime / mTime);
}


bool executeTest(const unsigned char* query, int queryLength,
                 const unsigned char* target, int targetLength,
                 int alphabetLength, int correct, int mode) {
    int score1 = myersCalcEditDistance(query, queryLength, target, targetLength,
                                       alphabetLength, -1, mode);
    int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength, alphabetLength, mode);
    bool pass = score1 == score2 && score2 == correct;

    printf(mode == MYERS_MODE_HW ? "HW: " : "NW: ");
    printf("Myers -> %d, simple -> %d, correct -> %d", score1, score2, correct);
    printf(pass ? "\x1B[32m OK \x1B[0m\n" : "\x1B[31m FAIL \x1B[0m\n");
    
    return score1 == score2 && score2 == correct;
}

bool test1() {
    int alphabetLength = 4;
    int queryLength = 4;
    int targetLength = 4;
    unsigned char query[4] = {0,1,2,3};
    unsigned char target[4] = {0,1,2,3};
    int correctHW = 0;
    int correctNW = 0;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, correctHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, correctNW, MYERS_MODE_NW);
    return r;
}

bool test2() {
    int alphabetLength = 9;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4}; // "match"
    unsigned char target[9] = {8,5,0,1,3,4,6,7,5}; // "remachine"
    int correctHW = 1;
    int correctNW = 6;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, correctHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, correctNW, MYERS_MODE_NW);
    return r;
}

bool test3() {
    int alphabetLength = 6;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4};
    unsigned char target[9] = {1,2,0,1,2,3,4,5,4};
    int correctHW = 0;
    int correctNW = 4;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, correctHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, correctNW, MYERS_MODE_NW);
    return r;
}

bool test4() {
    int alphabetLength = 2;
    int queryLength = 200;
    int targetLength = 200;
    unsigned char query[200] = {0};
    unsigned char target[200] = {1};
    int correctHW = 1;
    int correctNW = 1;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, correctHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, correctNW, MYERS_MODE_NW);
    return r;
}

bool test5() {
    int alphabetLength = 2;
    int queryLength = 64; // Testing for special case when queryLength == word size
    int targetLength = 64;
    unsigned char query[64] = {0};
    unsigned char target[64] = {1};
    int correctHW = 1;
    int correctNW = 1;

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, correctHW, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, correctNW, MYERS_MODE_NW);
    return r;
}

bool runTests() {
    bool t1 = test1();
    bool t2 = test2();
    bool t3 = test3();
    bool t4 = test4();
    bool t5 = test5();
    return t1 && t2 && t3 && t4 && t5;
}