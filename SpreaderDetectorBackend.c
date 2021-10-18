/**
* @file SpreaderDetectorBackend.c
* @author Aviel Shtern <aviel.shtern@mail.huji.ac.il>
* @version 1.0
* @date 3 Aug
* @brief A system that calculates the probability of infection in the Covid-19 for people who are
 * in the chain of infection
* @section DESCRIPTION
* Input :  Input file containing a list of people and details about them. And an input file
* containing details about the meetings during the paste chain
* Process: Goes through all the sessions and according to the duration of the session and the
* distance between the two people calculates the person's chance of infection
* Output : A output file containing a list of all the people as well as further instructions
* (isolation, hospitalization, etc.) according to the chance of them being infected.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SpreaderDetectorParams.h"

/**
 * @def VALID_ARG 3
 * @brief The number of arguments required to run the program (The name of the program, the
 * people file and the meeting file)
 */
#define VALID_ARG 3

/**
 * @def PLACE_OF_PEOPLES_FILE 1
 * @brief the place of peoples file in argv[]
 */
#define PLACE_OF_PEOPLS_FILE 1

/**
 * @def PLACE_OF_MEETINGS_FILE 2
 * @brief the place of meetings file in argv[]
 */
#define PLACE_OF_MEETINGS_FILE 2

/**
 * @def TYPE_ARG_ERROR 0
 * @brief In case not enough arguments were received the error will be represented by 0
 */
#define TYPE_ARG_ERROR 0

/**
 * @def ARGS_ERR_MSG "Usage: ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>\n"
 * @brief This message should be printed to stderr when a invalid error occurs.
 */
#define ARGS_ERR_MSG "Usage: ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>\n"

/**
 * @def TYPE_OPEN_INFILE_ERROR 1
 * @brief In case the program fails to open a file (input!) the error will be represented
 * by 1
 */
#define TYPE_OPEN_INFILE_ERROR 1

/**
 * @def OPEN_IN_FILE_ERR_MSG "Error in input files.\n"
 * @brief This message should be printed to stderr when a open file error occurs (in input file).
 */
#define OPEN_IN_FILE_ERR_MSG "Error in input files.\n"

/**
 * @def TYPE_LIBRARY_ERROR 2
 * @brief In case there is a directory error for example: The operating system failed to allocate
 * memory, the error will be represented by 2
 */
#define TYPE_LIBRARY_ERROR 2

/**
 * @def TYPE_OPEN_OUTFILE_ERROR 3
 * @brief In case the program fails to open a file (output!) the error will be represented
 * by 1
 */
#define TYPE_OPEN_OUTFILE_ERROR 3

/**
 * @def OPEN_OUT_FILE_ERR_MSG "Error in output file.\n"
 * @brief This message should be printed to stderr when a open file error occurs (in output file).
 */
#define OPEN_OUT_FILE_ERR_MSG "Error in output file.\n"

/**
 * @def MAX_LINE_SIZE 1025
 * @brief It can be assumed that the maximum line length (in the input file or in the output file)
 * is 1024 but since we capture each line as a string we will need 1025 space in memory because '\0'
 */
#define MAX_LINE_SIZE 1025

/**
 * @def INIT_PROB 0.0f
 * @brief For each person absorbed from the people file we will initialize the probability that he
 * is sick to be 0, then if he is also in the meeting file (he will not necessarily be there)
 * we will change the probability accordingly)
 */
#define INIT_PROB 0.0f

/**
 * @def IRRELEVANT_LEN 0
 * @brief The function that handles errors should have the array that contains the people and its
 * length. (To release resources before exiting) but sometimes the length of the list is irrelevant
 * (for example: before we even allocated memory) so 0 is given in the parameter of "list length"
 */
#define IRRELEVANT_LEN 0

/**
 * @def INIT_COUNTER 0
 * @brief init counter
 */
#define INIT_COUNTER 0

/**
 * @def READING_MODE "r"
 * @brief Will indicate to the function "fopen" that we want to read only from the file (Of the two
 * input files we only read)
 */
#define READING_MODE "r"

/**
 * @def WRITING_MODE "w"
 * @brief Will indicate to the function "fopen" that we will just write into the file (In the
 * case of the output file)
 */
#define WRITING_MODE "w"

/**
 * @def NUM_OF_FILDS_PEOPLE_FILE 3
 * @brief The number of fields in each row in the people file should be 3: name, ID number and age
 */
#define NUM_OF_FILDS_PEOPLE_FILE 3

/**
 * @def FORMAT_LINE_IN_PEOPLES_FILE "%s %d %f"
 * @brief Each line in the people file is as follows: <Person Name> <Person ID> <Person age>\n
 * Therefore we will read from it in the following format - "%s %d %f" - Into the variables
 */
#define FORMAT_LINE_IN_PEOPLES_FILE "%s %lu %f"

/**
 * @def NUM_OF_FILDS_FIRST_LINE_MEETING_FILE 1
 * @brief The number of fields in first line in the meetings file should be 1: id of spreader
 * infector-id, infected-id, distance, time
 */
#define NUM_OF_FILDS_FIRST_LINE_MEETING_FILE 1


/**
 * @def FORMAT_FIRST_LINE_IN_MEETINGS_FILE "%d"
 * @brief the first line in the meeting file is as follows: <spreader_id>\n
 * Therefore we will read from it in the following format - "%d" - Into the variable
 */
#define FORMAT_FIRST_LINE_IN_MEETINGS_FILE "%lu"

/**
 * @def NUM_OF_FILDS_MEETING_FILE 4
 * @brief The number of fields in each row (except first line) in the meetings file should be 4:
 * infector-id, infected-id, distance, time
 */
#define NUM_OF_FILDS_MEETING_FILE 4

/**
 * @def FORMAT_LINE_IN_MEETINGS_FILE "%d %d %f %f"
 * @brief Each line in the meeting file is as follows: <infector_id> <infected_id> <distance> <time>
 * Therefore we will read from it in the following format - "%d %d %f %f" - Into the variables
 */
#define FORMAT_LINE_IN_MEETINGS_FILE "%lu %lu %f %f"

/**
 * @def LEFT_BEFOR_RIGHT -1
 * @brief In the comparison functions to be passed to q-sort -1 will indicate that the left value
 * should be according to the right value
 */
#define LEFT_BEFOR_RIGHT -1

/**
 * @def PROBABILITY_IS_ONE 1
 * @brief The first person received in the file is sick for sure
 */
#define PROBABILITY_IS_ONE 1

/**
 * @def ELEMENT_NOT_FOUND -1
 * @brief If no element is found in the binary search, -1 will be returned. Let us note that in this
 * program if we look for someone in the array of people he will inevitably be there.
 * (It can be assumed that all the people in the meeting file are in the people file)
 */
#define ELEMENT_NOT_FOUND -1

/**
 * @def NO_PEOPLE_IN_FIRST_FILE 0
 * @brief Indicates that the people file is empty
 */
#define NO_PEOPLE_IN_FIRST_FILE 0


/**
 * @struct PersonDetailsNode
 * @brief Represents a person received in the first input file
 */
typedef struct PersonDetailsNode
{
	size_t id;
	float age;
	char name[MAX_LINE_SIZE];
	float probInfected;
} PersonDetailsNode;

/**
 * @struct MeetingInfo
 * @brief Represents a meeting between two people received in the second input file
 */
typedef struct MeetingInfo
{
	size_t infectorId;
	size_t infectedId;
	float distance;
	float time;
} MeetingInfo;


/**
 * Creates an array that contains all the people from the first file
 * @param path Path to the file of the people
 * @param numOfPeople Pointer to counter of people. So that at the end of the run the variable will
 * be updated. (And it also represented the length of the array)
 * @return pointer to Array of pointers to "PersonDetailsNode". That is, an array that contains all
 * the people
 */
PersonDetailsNode **createArrayOfPeopleFromFirstFile(const char *path, size_t * numOfPeople);

/**
 * creates one "PersonDetailsNode" from one line in the people file
 * @param line line from the people file
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because if there
 * is a directory error within the function we can free up resources and change (!) the pointer to
 * an array to be a NULL)
 * @param lenOfArray num of peoples
 * @return pointer to "PersonDetailsNode" that Represents people
 */
PersonDetailsNode *createPersonDetailsNodeFromLine(const char *line, PersonDetailsNode
                                                   ***arrayPeople, size_t lenOfArray);

/**
 * Reads the meeting file and updates the probability of each person who is there accordingly
 * @param path Path to the meeting file
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because if there
 * is a directory error within the function we can free up resources and change (!) the pointer to
 * an array to be a NULL)
 * @param lenOfArray num of peoples
 */
void readMeetingsFile(const char *path, PersonDetailsNode ***arrayPeople, size_t lenOfArray);

/**
 * Given a line in the meeting file (starting with the second line) we will create a "MeetingInfo"
 * that represents a one meeting
 * @param line line from meeting file
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because if there
 * is a directory error within the function we can free up resources and change (!) the pointer to
 * an array to be a NULL)
 * @param lenOfArray num of peoples
 * @return "MeetingInfo" that represents a one meeting
 */
MeetingInfo createMeetingInfoFromLine(const char *line, PersonDetailsNode ***arrayPeople, size_t
                                      lenOfArray);

/**
 * Standard binary search (O(logn)).
 * After creating the array of people - the progrem sort them by ID number. So when two people
 * meet and we want to update the probability that they are infected we can do so quickly
 * by binary search.
 * @param arr array of people
 * @param left The beginning of the array we are looking for
 * @param right The end of the array we are looking for
 * @param The ID number of the person we are looking for in the array
 * @return The location in the array where the person is
 */
size_t standardBinarySearch(PersonDetailsNode **arr, size_t left, size_t right, size_t id);

/**
 * Given infector and infected, As well as the meeting time and the distance between them. We will
 * update the probability that the second is infected according to the "crna" function and the
 * probability that the first is sick
 * @param distance The distance between the two people during the meeting
 * @param time How long did the meeting take
 * @param first the infector
 * @param sec the infected
 */
void updateProbAccordingCrnaAndProbOfInfector(float distance, float time, const PersonDetailsNode
                                              *first, PersonDetailsNode *sec);

/**
 * After processing the data and updating the probabilities of all the people.
 * We will print to the output file all the people and instructions regarding isolation or
 * hospitalization, etc. according to the probability in which they were infected.
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because if there
 * is a directory error within the function we can free up resources and change (!) the pointer to
 * an array to be a NULL)
 * @param lenOfArray num of peoples
 */
void printToOutputFile(PersonDetailsNode ***peopleArray, size_t lenOfArray);

/**
 * Given one person. The function will decide what to print to the output file.
 * (Is hospitalization, isolation needed or is it clean)
 * @param fdOutputFile  the output file
 * @param spreader the cur person
 */
void manageToOutputFile(FILE *fdOutputFile, const PersonDetailsNode *spreader);

/**
 * Handles any case of program error. (arguments. Error opening files, directory errors, etc.)
 * Releases resources and exits the program with exit code 1
 * @param typeError Error type
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because we
 * want change (!) the pointer to an array to be a NULL)
 * attention!! In errors that occurred before assignment we will pass null instead of the pointer
 * And 0 as the length of the array
 * @param lenOfArray num of peoples
 */
void errorCase(int typeError, PersonDetailsNode ***arrayPeople, size_t lenOfArray);

/**
 * Releases the allocated resources. And turns the pointers we released to a NULL
 * @param arrayPeople pointer to pointer to array of people (We want a pointer, because we
 * want change (!) the pointer to an array to be a NULL)
 * @param lenOfArray num of peoples
 */
void freeResources(PersonDetailsNode ***arrayPeople, size_t lenOfArray);

/**
 * A comparison function to q-sort that decides which value is greater than the other according to
 * the ID number. This way we can sort the array by ID (the array will be sorted from the smallest to
 * the largest)
 * @param first
 * @param sec
 * @return -1 if less. 1 if grater and 0 if equal
 */
int cmpFuncId(const void *first, const void *sec);

/**
 *  A comparison function to q-sort that decides which value is greater than the other according to
 * the The probability of infection. This way we can sort the array of people by probability of
 * infection (Note!: the array will be sorted from the largest to the smallest)
 * @param first
 * @param sec
 * @return -1 if grater. 1 if less and 0 if equal
 */
int cmpFuncProb(const void *first, const void *sec);


int main(int argc, char *argv[])
{
	if (argc != VALID_ARG)
	{
		errorCase(TYPE_ARG_ERROR, NULL, IRRELEVANT_LEN); // len of array not relevant
	}
	char *pathToPeopleFile = argv[PLACE_OF_PEOPLS_FILE];
	char *pathToMeetings = argv[PLACE_OF_MEETINGS_FILE];
	size_t numOfPeople = INIT_COUNTER; // len of array!!
	PersonDetailsNode **ArrayOfPeople = createArrayOfPeopleFromFirstFile(pathToPeopleFile,
																		 &numOfPeople);
	if (numOfPeople ==
		NO_PEOPLE_IN_FIRST_FILE) // The people file is empty so surely (by assumptions)
		// the meeting file is empty. So we will print a empty file,(We will first check the
		// correctness of the second file, because it may not open at all and then this is an error.
		// If it opens it is guaranteed to be empty) free resources And we'll get
		// out of the program
	{
		readMeetingsFile(pathToMeetings, &ArrayOfPeople, numOfPeople);
		printToOutputFile(&ArrayOfPeople, numOfPeople);
		freeResources(&ArrayOfPeople, numOfPeople);
		return EXIT_SUCCESS;
	}
	qsort(ArrayOfPeople, numOfPeople, sizeof(PersonDetailsNode *), cmpFuncId); //Sort the array
	// by ID
	readMeetingsFile(pathToMeetings, &ArrayOfPeople, numOfPeople);
	qsort(ArrayOfPeople, numOfPeople, sizeof(PersonDetailsNode *), cmpFuncProb); //Sort the array
	// according to the probability of infection
	printToOutputFile(&ArrayOfPeople, numOfPeople);
	freeResources(&ArrayOfPeople, numOfPeople);
	return EXIT_SUCCESS;
}

PersonDetailsNode **createArrayOfPeopleFromFirstFile(const char *path, size_t * numOfPeople)
{
	FILE *inputFile = fopen(path, READING_MODE);
	if (inputFile == NULL)
	{
		errorCase(TYPE_OPEN_INFILE_ERROR, NULL, IRRELEVANT_LEN);
	}
	PersonDetailsNode **arrayOfPeople = (PersonDetailsNode **) malloc(
			sizeof(PersonDetailsNode *));
	if (arrayOfPeople == NULL)
	{
		errorCase(TYPE_LIBRARY_ERROR, NULL, IRRELEVANT_LEN);
	}
	char currentRow[MAX_LINE_SIZE];
	while (fgets(currentRow, sizeof(currentRow), inputFile))
	{
		size_t placeNeeded = ((*numOfPeople) + 1) * sizeof(PersonDetailsNode *);
		PersonDetailsNode **tmpArrayOfPeople = (PersonDetailsNode **) realloc(arrayOfPeople,
																			  placeNeeded);
		if (tmpArrayOfPeople == NULL)
		{
			errorCase(TYPE_LIBRARY_ERROR, &arrayOfPeople, *numOfPeople);
		}
		arrayOfPeople = tmpArrayOfPeople;
		arrayOfPeople[*numOfPeople] = createPersonDetailsNodeFromLine(currentRow,
																	  &arrayOfPeople,
																	  *numOfPeople);
		(*numOfPeople)++;
	}
	fclose(inputFile);
	return arrayOfPeople;

}

PersonDetailsNode *
createPersonDetailsNodeFromLine(const char *line, PersonDetailsNode ***arrayPeople, size_t
                                lenOfArray)
{
	PersonDetailsNode *curPerson = (PersonDetailsNode *) malloc(sizeof(PersonDetailsNode));
	if (curPerson == NULL)
	{
		errorCase(TYPE_LIBRARY_ERROR, arrayPeople, lenOfArray);
	}
	size_t id;
	float age;
	char name[MAX_LINE_SIZE];
	if (sscanf(line, FORMAT_LINE_IN_PEOPLES_FILE, name, &id, &age) != NUM_OF_FILDS_PEOPLE_FILE)
	{
		errorCase(TYPE_LIBRARY_ERROR, arrayPeople, lenOfArray);
	}
	strcpy(curPerson->name, name); // we need chack?? (ERROR LIBRARY..)
	curPerson->id = id;
	curPerson->age = age;
	curPerson->probInfected = INIT_PROB; // We initialize the probability to 0
	// ("The person is innocent until proven otherwise")
	return curPerson;
}

void readMeetingsFile(const char *path, PersonDetailsNode ***arrayPeople, size_t lenOfArray)
{
	FILE *inputFile = fopen(path, READING_MODE);
	if (inputFile == NULL)
	{
		errorCase(TYPE_OPEN_INFILE_ERROR, arrayPeople, lenOfArray);
	}
	else if (lenOfArray == NO_PEOPLE_IN_FIRST_FILE) // The first file may be empty
	{
		fclose(inputFile);
		return;
	}
	char currentRow[MAX_LINE_SIZE];
	//first line. get the id of the first infector. (The first line is different from the rest!)
	size_t idOfFirstInfector;
	if (fgets(currentRow, sizeof(currentRow), inputFile) == NULL)
	{
		fclose(inputFile);
		return;
	}
	if (sscanf(currentRow, FORMAT_FIRST_LINE_IN_MEETINGS_FILE, &idOfFirstInfector) !=
		NUM_OF_FILDS_FIRST_LINE_MEETING_FILE)
	{
		errorCase(TYPE_LIBRARY_ERROR, arrayPeople, lenOfArray);
	}
	size_t idxOfFirstInfectorInArray = standardBinarySearch(*arrayPeople, 0, lenOfArray - 1,
															idOfFirstInfector);
	PersonDetailsNode *firstInfector = (*arrayPeople)[idxOfFirstInfectorInArray];
	firstInfector->probInfected = PROBABILITY_IS_ONE; //he sick for sure!!

	// we get the rest of lines. Each line represents a meeting We will look for the people in the
	// array (O(logn)!!) and update the probability of the infected according to the probability
	// of the infector and the function "crna"
	while (fgets(currentRow, sizeof(currentRow), inputFile))
	{
		MeetingInfo curMeeting = createMeetingInfoFromLine(currentRow, arrayPeople, lenOfArray);
		size_t idxInfectorInArray = standardBinarySearch(*arrayPeople, 0, lenOfArray - 1,
														 curMeeting.infectorId);
		size_t idxInfectedInArray = standardBinarySearch(*arrayPeople, 0, lenOfArray - 1,
														 curMeeting.infectedId);
		updateProbAccordingCrnaAndProbOfInfector(curMeeting.distance, curMeeting.time,
												 (*arrayPeople)[idxInfectorInArray],
												 (*arrayPeople)[idxInfectedInArray]);
	}
	fclose(inputFile);
}

MeetingInfo
createMeetingInfoFromLine(const char *line, PersonDetailsNode ***arrayPeople, size_t lenOfArray)
{
	size_t firstId, secondId;
	float distance, time;
	if (sscanf(line, FORMAT_LINE_IN_MEETINGS_FILE, &firstId, &secondId, &distance, &time) !=
		NUM_OF_FILDS_MEETING_FILE)
	{
		errorCase(TYPE_LIBRARY_ERROR, arrayPeople, lenOfArray);

	}
	MeetingInfo curMeeting;
	curMeeting.infectorId = firstId;
	curMeeting.infectedId = secondId;
	curMeeting.distance = distance;
	curMeeting.time = time;
	return curMeeting;
}

size_t standardBinarySearch(PersonDetailsNode **arr, size_t l, size_t r, size_t id)
{
	if (r >= l)
	{
		size_t mid = l + (r - l) / 2;
		if (arr[mid]->id == id)
		{
			return mid;
		}
		else if (arr[mid]->id > id)
		{
			return standardBinarySearch(arr, l, mid - 1, id);
		}
		// else: arr[mid]->id < id
		return standardBinarySearch(arr, mid + 1, r, id);
	}
	return ELEMENT_NOT_FOUND;
}

void updateProbAccordingCrnaAndProbOfInfector(float distance, float time, const PersonDetailsNode
                                              *first, PersonDetailsNode *sec) // need to be void!!
{
	float calCrna = (time * MIN_DISTANCE) / (distance * MAX_TIME);
	sec->probInfected = first->probInfected * calCrna;
}

void printToOutputFile(PersonDetailsNode ***peopleArray, size_t lenOfArray) // need to be void!!!
{
	FILE *outputFile = fopen(OUTPUT_FILE, WRITING_MODE);
	if (outputFile == NULL)
	{
		errorCase(TYPE_OPEN_OUTFILE_ERROR, peopleArray, lenOfArray);
	}

	for (size_t i = 0; i < lenOfArray; ++i)
	{
		manageToOutputFile(outputFile, (*peopleArray)[i]);
	}
	fclose(outputFile);
}

void manageToOutputFile(FILE *fdOutputFile, const PersonDetailsNode *spreader)
{
	if (spreader->probInfected >= MEDICAL_SUPERVISION_THRESHOLD)
	{
		fprintf(fdOutputFile, MEDICAL_SUPERVISION_THRESHOLD_MSG, spreader->name, spreader->id);
	}
	else if (spreader->probInfected >= REGULAR_QUARANTINE_THRESHOLD)
	{
		fprintf(fdOutputFile, REGULAR_QUARANTINE_MSG, spreader->name, spreader->id);
	}
	else // if (spreader->probInfected < REGULAR_QUARANTINE_THRESHOLD)
	{
		fprintf(fdOutputFile, CLEAN_MSG, spreader->name,
				spreader->id);
	}
}

void errorCase(int typeError, PersonDetailsNode ***arrayPeople, size_t lenOfArray)
{
	if (typeError == TYPE_ARG_ERROR)
	{
		fprintf(stderr, ARGS_ERR_MSG);
	}
	else if (typeError == TYPE_OPEN_INFILE_ERROR)
	{
		fprintf(stderr, OPEN_IN_FILE_ERR_MSG);
	}
	else if (typeError == TYPE_LIBRARY_ERROR)
	{
		fprintf(stderr, STANDARD_LIB_ERR_MSG);
	}
	else if (typeError == TYPE_OPEN_OUTFILE_ERROR)
	{
		fprintf(stderr, OPEN_OUT_FILE_ERR_MSG);
	}
	freeResources(arrayPeople, lenOfArray);
	exit(EXIT_FAILURE);
}

void freeResources(PersonDetailsNode ***arrayPeople, size_t lenOfArray)
{
	for (size_t i = 0; i < lenOfArray; i++)
	{
		free((*arrayPeople)[i]);
		(*arrayPeople)[i] = NULL;
	}
	if (arrayPeople)
	{
		free(*arrayPeople);
		*arrayPeople = NULL;
	}
}

int cmpFuncId(const void *first, const void *sec)
{
	size_t left = (*((PersonDetailsNode **) first))->id;
	size_t right = (*((PersonDetailsNode **) sec))->id;
	if (left < right)
	{
		return LEFT_BEFOR_RIGHT;
	}
	return left > right;
}

int cmpFuncProb(const void *first, const void *sec)
{
	float left = (*((PersonDetailsNode **) first))->probInfected;
	float right = (*((PersonDetailsNode **) sec))->probInfected;
	if (left > right) // We want to sort from big to small because the first on the list will have
		// the highest chance and so the order goes down to the end!
	{
		return LEFT_BEFOR_RIGHT;
	}
	return left < right;
}