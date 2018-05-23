#include <map>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

// Global variables
// Map with words: <word, number>
map<string, int> *wordsFreqMap;

// Vector with words: keys from map
vector<string> *wordsVector;

// Counter of created threads
int createThreads;

// Counter of finish threads
int finishThreads;

//Mutex
pthread_mutex_t lock;

// Frame size
long frameSize;

// Size of file
long fileSize;

// Number of threads
int threadNumber;


// Functions for counting frequency of words in text
// Add word frequency in one frame into global map and vector
void addFreqForFrame(map<string, int> *newFreqMap,
	vector<string> *newKeysVector);

// Counting words frequency in one text frame
void *countFreqForFrame(void *arg);

// Generation text frames for each threads
void generateWordsFreq(const char *inputString);

void printResult();


int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Not enough arguments!\n");
		return 1;
	}

	wordsFreqMap = new map<string, int>;
	wordsVector = new vector<string>;
	createThreads = 0;
	finishThreads = 0;

	threadNumber = atoi(argv[1]);
	
	FILE *file = fopen(argv[2], "r");

	if (file == NULL) {
		perror("File error");
		return 2;
	}

	fseek(file, 0, SEEK_END);
	fileSize = (size_t)ftell(file);
	frameSize = fileSize / threadNumber + 1;
	rewind(file);

	char *buffer = (char *)malloc((size_t)fileSize);
	fread(buffer, 1, fileSize, file);

	// init mutex
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}

	struct timeval tvStart;
	struct timeval tvFinish;

	// Start time
	gettimeofday(&tvStart, NULL);
	
        // Count frequency of words
        generateWordsFreq(buffer);

	// Finish time
	gettimeofday(&tvFinish, NULL);
        
        // Operating time (in milisecond)
	long int msStart = tvStart.tv_sec * 1000 + tvStart.tv_usec / 1000;
	long int msFinish = tvFinish.tv_sec * 1000 + tvFinish.tv_usec / 1000;

	printf("%ld\n", msFinish - msStart);
	printResult();

	// Delete mutex and other variables
	pthread_mutex_destroy(&lock);
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
			wordsFreqMap->insert(pair<string, int>(*it, frqNew));
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

void *countFreqForFrame(void *arg) {
	char *frame = (char *)arg;

	map<string, int> *wMap = new map<string, int>;
	vector<string> *wVector = new vector<string>;

	char *ptr;

	char *word = strtok_r(frame, " ,.:; \"!?()\n", &ptr);

	int i = 0;
	while (word != NULL) {
		i++;
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
		word = strtok_r(NULL, " ,.:; \"!?()\n", &ptr);
	}

	// on mutex
	pthread_mutex_lock(&lock);
	addFreqForFrame(wMap, wVector);
	finishThreads++;
	// off mutex
	pthread_mutex_unlock(&lock);

	delete (wMap);
	delete (wVector);
	delete (frame);
	delete (word);
}

void generateWordsFreq(const char *inputString) {
	if (inputString == NULL)
		return;

	long from = 0;
	long to = 0;

	int i = 0;
	while (to < (fileSize - 1)) {
		if (to == (fileSize - 1))
			break;

		to += frameSize;
		while (inputString[to] != ' ' && to < fileSize)
			to++;

		if (to > fileSize)
			to = fileSize - 1;

		char *frame = new char[to - from + 1];

		for (int i = 0; i < to - from + 1; i++) {
			frame[i] = 0;
		}
		
		strncpy(frame, inputString + from, to - from);
		from = to + 1;

		pthread_t thread;
		// create threads
		createThreads++;
                // counting words frequency for frame
		pthread_create(&thread, NULL, countFreqForFrame, (void *)frame);
		// detach threads
		pthread_detach(thread);
	}

	while (finishThreads < createThreads)
		usleep(10);
}

void printResult() {
	for (vector<string>::iterator it = wordsVector->begin(); it != wordsVector->end(); ++it) {
		string bufName = *it;
		int bufCount = wordsFreqMap->at(*it);
		printf("%s %d\n", bufName.c_str(), bufCount);
	}
}
