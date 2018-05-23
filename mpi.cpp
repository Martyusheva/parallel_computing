#include <map>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <mpi.h>
#include <stdlib.h>

using namespace std;

// Global variables
// Map with words: <word, number>
map<string, int> *wordsFreqMap;

// Vector with words: keys from map
vector<string> *wordsVector;

// Frame size
long frameSize;

// Size of file
long fileSize;

int rank, size;

MPI::Status status;


// Functions for counting frequency of words in text
// Add word frequency in one frame into global map and vector
void addFreqForFrame(map<string, int> *newFreqMap,
	vector<string> *newKeysVector);

// Counting words frequency in one text frame
void countFreqForFrame(char *workCharArr);

// Generation text frames for each threads
void generateWordsFreq(const char *inputString);

void printResult();



int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Please write filename in parameter\n");
		return 1;
	}

	wordsFreqMap = new map<string, int>;
	wordsVector = new vector<string>;

	FILE *file = fopen(argv[1], "r");

	if (file == NULL) {
		perror("File error");
		return 2;
	}

	fseek(file, 0, SEEK_END);
	fileSize = (size_t)ftell(file);
	rewind(file);

	char *buffer = (char *)malloc((size_t)fileSize);
	fread(buffer, 1, fileSize, file);

	struct timeval tvStart;
	struct timeval tvFinish;

	// Get time of start programm
	gettimeofday(&tvStart, NULL);

	MPI_Init(&argc, &argv);                      // starts MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);        // get current process id
	MPI_Comm_size(MPI_COMM_WORLD, &size);        // get number of processes

	frameSize = fileSize / (size - 1);

        // Count frequency of words
	generateWordsFreq(buffer);
	
        if (rank == 0) {
		// Get time of finish programm
		gettimeofday(&tvFinish, NULL);
		long int msStart = tvStart.tv_sec * 1000 + tvStart.tv_usec / 1000;
		long int msFinish = tvFinish.tv_sec * 1000 + tvFinish.tv_usec / 1000;

		printf("%ld\n", msFinish - msStart);

		printResult();
	}

	MPI_Finalize();

	fclose(file);


	delete (buffer);
	delete (wordsVector);
	delete (wordsFreqMap);
	return 0;
}

void addFreqForFrame(map<string, int> *newFreqMap, vector<string> *newKeysVector) {
	if (wordsFreqMap == NULL) {
		wordsFreqMap = new map<string, int>;
		wordsVector = new vector<string>;

		for (vector<string>::iterator it = newKeysVector->begin(); it != newKeysVector->end(); ++it) {
			int freqNew = newFreqMap->at(*it);
			wordsFreqMap->insert(pair<string, int>(*it, freqNew));
			wordsVector->push_back(*it);
		}
		return;
	}

	if (newFreqMap == NULL)
		return;

	if (newKeysVector == NULL)
		return;

	for (vector<string>::iterator it = newKeysVector->begin(); it != newKeysVector->end(); ++it) {
		if (wordsFreqMap->count(*it)) {
			int freq = wordsFreqMap->at(*it);
			int freqNew = newFreqMap->at(*it);
			map<string, int>::iterator itMap;
			itMap = wordsFreqMap->find(*it);
			wordsFreqMap->erase(itMap);
			wordsFreqMap->insert(pair<string, int>(*it, freq + freqNew));
		}
		else {
			int freqNew = newFreqMap->at(*it);
			wordsFreqMap->insert(pair<string, int>(*it, freqNew));
			wordsVector->push_back(*it);
		}
	}
}

void countFreqForFrame(char *workCharArr) {

	map<string, int> *wMap = new map<string, int>;
	vector<string> *wVector = new vector<string>;

	char *word = strtok(workCharArr, " ,:. \"!?;()\n");

	while (word != NULL) {
		if (wMap->count(word)) {
			int bufCount = wMap->at(word);
			map<string, int>::iterator itMap = wMap->find(word);
			wMap->erase(itMap);
			wMap->insert(pair<string, int>(word, ++bufCount));
		}
		else {
			wMap->insert(pair<string, int>(word, 1));
			wVector->push_back(word);
		}
		word = strtok(NULL, " ,:. \"!?()\n");
	}

	string sendString = "";
	for (vector<string>::iterator it = wVector->begin(); it != wVector->end(); ++it) {
		string bufString = *it;
		char bufNumber[20];
		int bufNum = wMap->at(*it);
		sprintf(bufNumber, "%d", bufNum);
		string intString(bufNumber);
		sendString = sendString + ' ' + bufString + ' ' + intString;
	}

	int string_lenght = sendString.size() + 1;
	MPI::COMM_WORLD.Send(&string_lenght, 1, MPI::INT, 0, 0);
	MPI::COMM_WORLD.Send(sendString.c_str(), string_lenght, MPI::CHAR, 0, 1);

	delete (wMap);
	delete (wVector);
}

void generateWordsFreq(const char *inputString) {
	if (inputString == NULL)
		return;

	long from = 0;
	long to = 0;

	if (rank == 0) {
		int i = 1;
		while (i < size) {

			to += frameSize;
			while (inputString[to] != ' ' && to < fileSize)
				to++;

			if (to > fileSize)
				to = fileSize - 1;

			char *workArray = new char[to - from + 1];

			for (int i = 0; i < to - from + 1; i++)
				workArray[i] = 0;

			strncpy(workArray, inputString + from, to - from);

			int string_lenght = to - from + 1;
			MPI::COMM_WORLD.Send(&string_lenght, 1, MPI::INT, i, 0);
			MPI::COMM_WORLD.Send(workArray, string_lenght, MPI::CHAR, i, 1);

			from = to + 1;
			i++;

			delete (workArray);
		}
	}
	else {
		int frameLenght;
		MPI::COMM_WORLD.Recv(&frameLenght, 1, MPI::INT, 0, 0, status);
		char *i_buffer = new char[frameLenght];
		MPI::COMM_WORLD.Recv(i_buffer, frameLenght, MPI::CHAR, 0, 1, status);
		int count = status.Get_count(MPI::CHAR);
		countFreqForFrame(i_buffer);
		delete(i_buffer);
	}

	if (rank == 0) {
		for (int i = 1; i < size; i++) {
			map<string, int> *wMap = new map<string, int>;
			vector<string> *wVector = new vector<string>;

			// Get text string
			int frameLenght;
			MPI::COMM_WORLD.Recv(&frameLenght, 1, MPI::INT, i, 0, status);
			char *i_buffer = new char[frameLenght];
			MPI::COMM_WORLD.Recv(i_buffer, frameSize, MPI::CHAR, i, 1, status);

			char *word = strtok(i_buffer, " ,. \"!?;()\n");
			while (word != NULL) {
				string b(word);
				wVector->push_back(b);
				word = strtok(NULL, " ,. \"!?()\n");

				string buf = wVector->back();
				wMap->insert(pair<string, int>(buf, atoi(word)));
				word = strtok(NULL, " ,. \"!?()\n");
			}

			addFreqForFrame(wMap, wVector);

			delete(i_buffer);
			delete(wMap);
			delete(wVector);
		}
	}
}

void printResult() {
	for (vector<string>::iterator it = wordsVector->begin(); it != wordsVector->end(); ++it) {
		string bufName = *it;
		int bufCount = wordsFreqMap->at(*it);
		printf("%s %d\n", bufName.c_str(), bufCount);
	}
}
