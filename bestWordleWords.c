/*
  Program 3: Best Wordle Words
  CS 211, UIC, Fall 2022
  System: Replit
  Authors: Faaizuddin Farooqui, Dylen Greenenwald

  Wordle is a fun online word-guessing game. This program finds the
  "best" starting word(s) (where best is defined as the valid dictionary word(s) found
  in either the guesses or answers file that has the most letter matches when compared
  to all the answers words) and their corresponding second best word(s).
  Link to wordle dictionary words at:
    https://www.reddit.com/r/wordle/comments/s4tcw8/a_note_on_wordles_word_list/
*/

#include <stdio.h>
#include <stdlib.h>   // for exit( -1)
#include <string.h>

// Defines global constants so as not to have various "magic numbers" floating around
#define WORD_LENGTH 5
#define MAX_FILENAME_LENGTH 81

// Defines two struct and corresponding synonyms to contain data relating to the tota 
// list of words and an arbitrary file to be manipulated
typedef struct word wordStruct;
typedef struct file fileStruct;

struct word
{
    char word[WORD_LENGTH + 1];  // the word including the '\0'
    int score;                   // the score
};

struct file
{
    char name[MAX_FILENAME_LENGTH];
    int length;
};

//-----------------------------------------------------------------------------------------
// Comparator for use in built-in qsort(..) function.  Parameters are declared to be a
// generic type, so they will match with anything.
// This is a two-part comparison.  First the scores are compared.  If they are the same,
// then the words themselves are also compared, so that the results are in descending
// order by score, and within score they are in alphabetic order.
int compareFunction(const void *a, const void *b)
{
    // Before using parameters we have cast them into the actual type they are in our program
    // and then extract the numerical value used in comparison
    int firstScore = ((wordStruct *) a)->score;
    int secondScore = ((wordStruct *) b)->score;

    // If scores are different, then that's all we need for our comparison.
    if(firstScore != secondScore)
    {
        // We reverse the values, so the result is in descending vs. the otherwise ascending order
        // return firstScore - secondScore;   // ascending order
        return secondScore - firstScore;      // descending order
    }
    else
    {
      // Scores are equal, so check the words themselves to put them in alphabetical order
      return strcmp(((wordStruct *) a)->word, ((wordStruct *) b)->word);
    }
} //end compareFunction(..)


///-----------------------------------------------------------------------------------------
// Accepts a single file name as input and returns the number of words in that file.
int getFileWordCount(char *fileName)
{ 
    int count = 0;

    FILE *inFilePtr  = fopen(fileName, "r");
    char wordToCopy[WORD_LENGTH + 1];

    // Sanity check: ensure file open worked correctly
    if(inFilePtr == NULL)
    {
        printf("Error: could not open %s for reading\n", fileName);
        exit(-1);
    }
  
    // Increments the word count for every line read (assumes 1 word per line)
    while(fscanf(inFilePtr, "%s", wordToCopy) != EOF)
    {
        count++;
    }
  
    return count;
} //end getFileWordCount(...)


//-----------------------------------------------------------------------------------------

void initializeLists(fileStruct answers, fileStruct guesses,
                     wordStruct **wordList, char **answerList)
{
    // Declare variables to be used in file operations
    FILE *inFilePtr = NULL;
    char wordToCopy[WORD_LENGTH + 1];
          
    // ANSWERS FILE
    // Reads in answer file
    inFilePtr  = fopen(answers.name, "r");

    int i;
    for(i = 0; fscanf(inFilePtr, "%s", wordToCopy) != EOF; i++)
    {
        // Copies word from answers file to wordList and answerList
        strcpy((*wordList + i)->word, wordToCopy);
        strcpy(answerList[i], wordToCopy);
    }

    // GUESSES FILE
    // Reads in guesses file
    inFilePtr = fopen(guesses.name, "r");

    // Loop uses a null statement to reuse the counter from the previous loop
    for(; fscanf(inFilePtr, "%s", wordToCopy) != EOF; i++)
    {
        // Copies word from guesses file to wordList (starting at last answer word in wordList)
        strcpy((*wordList + i)->word, wordToCopy);
    }
          
    // Close the file
    fclose(inFilePtr);
} //end initializeLists(...)

///-----------------------------------------------------------------------------------------
// Accepts two words as input and returns the score for a single guess and answer pair.
int calculateSingleScore(char guessWord[], char answerWord[])
{
    int score = 0;

    // Compares words in search of exact position character matches 
    for(int i = 0; i < WORD_LENGTH; i++)
    {
        // Variables for legibility: Why does using these variables change the score?
        // char answerLetter = answerWord[i];
        // char guessLetter = guessWord[i];
      
        if(guessWord[i] == answerWord[i])
        {
            answerWord[i] = '*';   // replaces matched answer letter with a symbol to avoid re-matches
            guessWord[i] = '_';    // replaces matched guess letter with a symbol to avoid re-checks
            score += 3;            // increments score by 3 for an exact match
        }
    }

    // Compares words in search of offset position character matches
    for(int i = 0; i < WORD_LENGTH; i++)
    {
        // Variables for legibility
        // char guessLetter = guessWord[i];
      
        // Pointer to letter that exists within the answer, if non-NULL is an offset match
        char *pMatchLetter = NULL;
        pMatchLetter = strchr(answerWord, guessWord[i]);
      
        if(pMatchLetter != NULL)
        {               
            (*pMatchLetter) = '*';  // replaces matched letter with a symbol to avoid re-matches
            score += 1;             // increments score by 1 for an offset match
        }
    }
  return score;
} //end calculateSingleScore(...)


//-----------------------------------------------------------------------------------------
// Sets total score for each word in the word list.
void setTotalScore(wordStruct **wordList, int wordListLength,
                        char **answerList,
                        fileStruct guesses,
                        fileStruct answers)
{
    // Loop through entire word list to calculate all scores against all answer words
    for(int i = 0; i < wordListLength; i++)
    {
        int score = 0;

        // Calculates score for a given guess against all answers
        for(int j = 0; j < answers.length; j++)
        {
            // Copies guess and answer words into respective variables, using total word list as
            // source for guess words.
            char guessWord[WORD_LENGTH + 1];
            char answerWord[WORD_LENGTH + 1];
            strcpy(guessWord, (*wordList + i)->word);
            strcpy(answerWord, answerList[j]);

            // Increment score for a guess word by its score against a single answer word
            score += calculateSingleScore(guessWord, answerWord);
        }
      // Update the final score for a guess word
      (*wordList + i)->score = score;
    }
} //end setTotalScore(...)


//-----------------------------------------------------------------------------------------
int main()
{
    // Declares variables to be used in multiple menu options
    fileStruct answers;
    fileStruct guesses;
    wordStruct *wordList;
    int wordListLength;

    // Copies default filenames to respective variables, prints at program start
    strcpy(answers.name,"answersTiny.txt");
    strcpy(guesses.name,"guessesTiny.txt");
    printf("Default file names are %s and %s\n", answers.name, guesses.name);

    // Display menu to allow for partial credit
    int menuOption = 0;
    do
    {
        printf("\n");
        printf("Menu Options:\n");
        printf("  1. Display best first words only\n");
        printf("  2. Display best first and best second words\n");
        printf("  3. Change answers and guesses filenames\n");
        printf("  4. Exit\n");
        printf("Your choice: ");
        scanf("%d", &menuOption);
        if(menuOption == 1)
        {
            /* Calculate total number of words from both files to be used in dynamic allocation of 
               the final word list */
            answers.length = getFileWordCount(answers.name);
            guesses.length = getFileWordCount(guesses.name);
            int wordListLength = answers.length + guesses.length;

            // Print the chosen files and their respective sizes to standard output
            printf("%s has %d words\n", answers.name, answers.length);
            printf("%s has %d words\n\n", guesses.name, guesses.length);
          
            // Dynamically allocates space for a list containing all words from both files
            wordStruct *wordList = (wordStruct *) malloc(sizeof(wordStruct) * wordListLength);\

            // Dynamically allocates space for a list containing the answer words
            char **answerList = (char **) malloc(sizeof(char *) * answers.length);
            for(int i = 0; i < answers.length; i++)
            {
                answerList[i] = (char *) malloc(sizeof(char) * (WORD_LENGTH + 1));
            }

            // Copies file contents into lists and sets total score for best scoring words
            initializeLists(answers, guesses, &wordList, answerList);
            setTotalScore(&wordList, wordListLength, answerList, guesses, answers);

            // Sorts word list in descending order by score
            // Highest score will always be the first element
            qsort(wordList, wordListLength, sizeof(wordStruct), compareFunction);
            int highestScore = wordList[0].score;

            // Output score(s) for the best word(s)
            printf("Words and scores for top first words:\n");
            for(int i = 0; wordList[i].score == highestScore; i++)
            {
                printf("%s %d\n", wordList[i].word, wordList[i].score);
            }
            free(answerList);
            free(wordList);
            printf("Done");
        }
        else if(menuOption == 2)
        {
            /* Calculate total number of words from both files to be used in dynamic allocation of
               the final word list */
            answers.length = getFileWordCount(answers.name);
            guesses.length = getFileWordCount(guesses.name);
            int wordListLength = answers.length + guesses.length;

            // Print the chosen files and their respective sizes to standard output
            printf("%s has %d words\n", answers.name, answers.length);
            printf("%s has %d words\n\n", guesses.name, guesses.length);
          
            // Dynamically allocates space for a list containing all words from both files
            wordStruct *wordList = (wordStruct *) malloc(sizeof(wordStruct) * wordListLength);\

            // Dynamically allocates space for a list containing the answer words
            char **answerList = (char **) malloc(sizeof(char *) * answers.length);
            for(int i = 0; i < answers.length; i++)
            {
                answerList[i] = (char *) malloc(sizeof(char) * (WORD_LENGTH + 1));
            }

            // Copies file contents into lists and calculates best scoring words
            initializeLists(answers, guesses, &wordList, answerList);
            setTotalScore(&wordList, wordListLength, answerList, guesses, answers);

            // Sorts word list in order of descending score; first element will always be highest
            qsort(wordList, wordListLength, sizeof(wordStruct), compareFunction);
            int highestScore = wordList[0].score;

            // Calcualtes the size of the list that contains all the best first words
            int bestWordListSize = 0;
            for(int i = 0; wordList[i].score == highestScore; i++)
            {
                bestWordListSize++;
            }
          
            // Dynamically allocates space for and initializes a list containing the best words
            char **bestWordList = (char **) malloc(sizeof(char *) * bestWordListSize);
            for(int i = 0; i < answers.length; i++)
            {
                bestWordList[i] = (char *) malloc(sizeof(char) * (WORD_LENGTH + 1));
            }
            for(int i = 0; i < bestWordListSize; i++)
            {
                strcpy(bestWordList[i], wordList[i].word);
            }

            // Dynamically allocates space for a copy of the list containing all words from both files
            wordStruct* wordListCopy = (wordStruct*) malloc(sizeof(wordStruct) * wordListLength);
          
            // Dynamically allocates space for a list containing a copy of the answer words
            char **answerListCopy = (char **) malloc(sizeof(char *) * answers.length);
            for(int i = 0; i < answers.length; i++)
            {
                answerListCopy[i] = (char *) malloc(sizeof(char) * (WORD_LENGTH + 1));
            }

            // Possible call to initializeLists(...) to shave down some of the loop below?
            initializeLists(answers, guesses, &wordListCopy, answerListCopy);
          
            // Remove letters in the answers that exist in the best words
            printf("Words and scores for top first words and second words:\n");
            for(int i = 0; i < bestWordListSize; i++)
            {
                // Prints best word for each iteration of the best words list
                printf("%s %d\n", wordList[i].word, wordList[i].score);

                // Copies score from word list to word list copy
                for(int j = 0; j < wordListLength; j++)
                {
                    wordListCopy[j].score = wordList[j].score;
                }

                // Copies answer list to answer list copy
                // Why doesn't the above call to initialize lists eliminate the need for this loop?
                // When it's deleted, the scoring changes slightly...
                for(int j = 0; j < answers.length; j++)
                {
                    strcpy(answerListCopy[j], answerList[j]);
                }

                /* Declaration of variables to store word to be manipulated and the word to be
                   checked against, respectively */
                char bestWord[WORD_LENGTH + 1];
                char answerWordCopy[WORD_LENGTH + 1];

                // Loops through answers list and removes letters from answers contained in the best words
                for(int j = 0; j < answers.length; j++)
                {
                    /* Copies best word and answer word into their predefined variables for each iteration
                       of the answers list */
                    strcpy(bestWord, bestWordList[i]);
                    strcpy(answerWordCopy, answerListCopy[j]);

                    // Compares words in search of exact position character matches
                    for(int k = 0; k < WORD_LENGTH; k++)
                    {
                        if(bestWord[k] == answerWordCopy[k])
                        {
                            bestWord[k] = '*';
                            answerWordCopy[k] = ' ';
                        }
                    }

                    // Pointer to detect an offset position match via strchr(...)
                    char *pOffsetLetter = NULL; 
                
                    // Compares words in search of offset position character matches
                    for(int k = 0; k < WORD_LENGTH; k++)
                    {
                        pOffsetLetter = strchr(bestWord, answerWordCopy[k]);
                        if(pOffsetLetter != NULL)
                        {
                            *pOffsetLetter = '*';
                            pOffsetLetter = strchr(answerWordCopy, answerWordCopy[k]);
                            *pOffsetLetter = ' ';
                        }
                    }
                    /* Copies the answer word without the letters that exist in the best word
                       to the answer list copy */
                    strcpy(answerListCopy[j], answerWordCopy);
                }

                // Sets total score for best scoring words (without characters in the answer words)
                setTotalScore(&wordListCopy, wordListLength, answerListCopy, guesses, answers);

                // Sorts word list copy (containing second best words) in descending order by score
                // Highest score is always first the first element
                qsort(wordListCopy, wordListLength, sizeof(wordStruct), compareFunction);
                int nextHighestScore = wordListCopy[0].score;

                // Output score(s) for the second best word(s) for the current best word
                for(int j = 0; wordListCopy[j].score == nextHighestScore; j++)
                {
                    printf(" %s %d ", wordListCopy[j].word, wordListCopy[j].score);
                }
                printf("\n");
            }
            free(bestWordList);
            free(answerList);
            free(answerListCopy);
            free(wordListCopy);
            printf("Done");
        }
        else if(menuOption == 4)
        {
            exit(1); // Exit the program
        }
        else if(menuOption == 3)
        {
            // Change file names and reprompt
            printf("Enter new answers and guesses filenames: ");
            scanf("%s %s", &answers.name, &guesses.name);
        }
    } while( menuOption == 3);
  
    return 0;
} // end main()

