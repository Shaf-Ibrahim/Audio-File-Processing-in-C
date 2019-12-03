/*
**
** Opens a file and allocates memory for it. After confirming the file is
** a WAV and PCM file, it then checks the header and fixes any calculated values
** that are missing or incorrect. Then the user can select a filter to apply
** and the program then saves the modified WAV file to a new user-specified file.
**
** Shafieen Ibrahim
** 4/20/2017
**
**/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "atcs.h"

//#define min(a,b) ((a) < (b) ? (a) : (b))

struct wavChunk {LONG chunkID;
                 LONG chunkSize;
                 LONG format;};

struct wavSubchunk1 {LONG subchunk1ID;
                     LONG subchunk1Size;
                     short audioFormat;
                     short numChannels;
                     LONG sampleRate;
                     LONG byteRate;
                     short blockAlign;
                     short bitsPerSample;};

struct wavSubchunk2 {LONG subchunk2ID;
                     LONG subchunk2Size;
                     BYTE data[1];};

struct WAVE {struct wavChunk     chunk;
             struct wavSubchunk1 subchunk1;
             struct wavSubchunk2 subchunk2;};

struct mem {off_t size;
            struct WAVE wave;};

union values {char strs[4];
              LONG val;};

void smartFree(struct mem* pointer);

off_t fileLength(off_t fileHandle);

struct mem* openFile(char* fileName);

int isWavFile(struct mem* memPointer);

int isPCMFile(struct mem* memPointer);

int checkBlockAlign(struct mem* memPointer);

void determineCalculatedFields(struct mem* memPointer);

int updateBytesPerSample(struct mem* memPointer);

void saveFile(char* fileName, struct mem* memPointer);

void applyFilters(struct mem* memPointer, int filter, double rate);

void changeSampleRate(struct mem* memPointer, double rate);

void reverseSoundBlocks(struct mem* memPointer);

void swapAndReverse(struct mem* memPointer);

/*
** Function to free memory if the pointer exists
**
** args:
** struct mem* pointer - pointer in memory
**
*/
void smartFree(struct mem* pointer)
   {
   if (pointer != (struct mem*) NIL) free(pointer);

   return;
   }

/*
** Function to determine length of file, leaving the seek-arm unchanged.
**
** args:
** int fileHandle - handle of the file
**
** returns off_t
**
*/
off_t fileLength(off_t fileHandle)
   {
   off_t length, curPos;

   curPos = lseek(fileHandle, 0, SEEK_CUR);
   length = lseek(fileHandle, 0, SEEK_END);
   lseek(fileHandle, curPos, SEEK_SET);

   return(length);
   }

/*
** Function to open file and allocate memory for the file. Calls other functions
** to determine if file is WAV, PCM, and has correct calculated fields.
**
** args:
** char* fileName - name of the file
**
** returns struct* memPointer
**
*/
struct mem* openFile(char* fileName)
   {
   int fileHandle;
   off_t length;
   struct mem* memPointer = NULL;
   off_t numBytes;

   fileHandle = open(fileName, O_BINARY | O_RDONLY);

   if (fileHandle != -1) //checks if file was opened correctly
      {
      length = fileLength(fileHandle);

      memPointer = (struct mem*)malloc(length + sizeof(struct mem));

      if (memPointer != (struct mem*)NIL)
         {
         numBytes = read(fileHandle, &memPointer->wave.chunk.chunkID, length); //read file into memory

         if (numBytes != length) //checks if there was error reading in file
            {
            free(memPointer); //free memory pointer and reset memory pointer to null
            memPointer = (struct mem*)NIL;
            }
         else
            {
            memPointer->size = length;
            }
         }
      close(fileHandle);
      }
  else
      printf("File did not open.");

   return(memPointer);
   }

/*
** Function to determine if the file is a WAV file by testing the
** chunkID, format, subchunk1ID, and subchunk2ID.
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** returns int - returns 1 if it is a WAV file; otherwise, -1
*/
int isWavFile(struct mem* memPointer)
   {
   int returnValue;
   union values fields;

   memcpy(fields.strs, "RIFF", 4);

   if (memPointer->wave.chunk.chunkID != fields.val)
      {
      printf("\nChunkID is incorrect, the file is not a WAV file.");
      returnValue = -1;
      }
   else
      {
      memcpy(fields.strs, "WAVE", 4);

      if (memPointer->wave.chunk.format != fields.val)
         {
         printf("\nFormat is incorrect, the file is not a WAV file.");
         returnValue = -1;
         }
      else
         {
         memcpy(fields.strs, "fmt ", 4);

         if (memPointer->wave.subchunk1.subchunk1ID != fields.val)
            {
            printf("\nSubchunk1ID is incorrect, the file is not a WAV file.");
            returnValue = -1;
            }
         else
            {
            memcpy(fields.strs, "data", 4);

            if (memPointer->wave.subchunk2.subchunk2ID != fields.val)
               {
               printf("\nSubchunk2ID is incorrect, the file is not a WAV file.");
               returnValue = -1;
               }
            else
               {
               if (memPointer->wave.subchunk1.bitsPerSample > 32)
                   {
                   printf("\n Files greater than 32 bits per sample are not accepted.");
                   returnValue = -1;
                   }
               else
                   {
                   returnValue = 1;
                   }
               }
            }
         }
      }
   return(returnValue);
   }

/*
** Function to determine if the file is a PCM file by testing the
** audioFormat and subchunk1Size.
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** returns int - returns 1 if it is a PCM file; otherwise, -1
**
*/
int isPCMFile(struct mem* memPointer)
   {
   int returnValue;

   if (memPointer->wave.subchunk1.audioFormat != 1 || memPointer->wave.subchunk1.subchunk1Size != 16)
      {
      printf("This file is not a PCM file.");
      returnValue = -1;
      }
   else returnValue = 1;

   return(returnValue);
   }

int checkBlockAlign(struct mem* memPointer)
   {
   int returnValue;

   

   return(returnValue);
   }

/*
** Function to determine the calculated fields for the file, and correct any
** if they are missing or incorrect.
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** void function, no return value
*/
void determineCalculatedFields(struct mem* memPointer)
   {
   LONG cChunkSize;
   LONG cByteRate;
   short cBlockAlign;
   LONG cSubchunk2Size;

   int bytesPerSample;

   bytesPerSample = memPointer->wave.subchunk1.bitsPerSample/8;

   bytesPerSample = updateBytesPerSample(memPointer);

   cChunkSize = 4 + (8 + memPointer->wave.subchunk1.subchunk1Size) +
                (8 + memPointer->wave.subchunk2.subchunk2Size);

   cByteRate = memPointer->wave.subchunk1.sampleRate *
               memPointer->wave.subchunk1.numChannels * bytesPerSample;
   cBlockAlign = memPointer->wave.subchunk1.numChannels * bytesPerSample;

   cSubchunk2Size = memPointer->size - 44; //length of file - number of bytes up to the data array

   if (cChunkSize != memPointer->wave.chunk.chunkSize)
      {
      printf("The correct chunkSize is ");
      printf("%d\n", cChunkSize);
      }
   if (cByteRate != memPointer->wave.subchunk1.byteRate)
      {
      printf("The correct byteRate is ");
      printf("%d\n", cByteRate);
      }
   if (cBlockAlign != memPointer->wave.subchunk1.blockAlign)
      {
      printf("The correct blockAlign is ");
      printf("%d\n", cBlockAlign);
      }
   if (cSubchunk2Size != memPointer->wave.subchunk2.subchunk2Size)
      {
      printf("The correct subchunk2Size is ");
      printf("%d\n", cSubchunk2Size);
      }
   return;
   }

/*
** Function to update the bytes per sample using the bitsPerSample
** to digitize files that are not 8-bit or 16-bit files.
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** returns int - new bytesPerSample for the file
*/
int updateBytesPerSample(struct mem* memPointer)
   {
   int bytesPerSample;
   double logOfBitsPerSample;
   int powerOfTwo;

   //calculate next power of two
   logOfBitsPerSample = log(memPointer->wave.subchunk1.bitsPerSample) / log(2);
   powerOfTwo = (int)(pow(2, (int)(ceil(logOfBitsPerSample))));

   bytesPerSample = powerOfTwo / 8;

   return bytesPerSample;
   }

/*
** Function to save the given file
**
** args:
** char* fileName - pointer to the name of a fileName
** struct mem* memPointer - pointer to the memory space for the file
**
** void function, no return value
*/
void saveFile(char* fileName, struct mem* memPointer)
   {
   int fileHandle;

   fileHandle = open(fileName, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);
   if (fileHandle != -1)
      {
      //check if file written out is the same as original
      if (write(fileHandle, &memPointer->wave.chunk.chunkID, memPointer->size) != memPointer->size)
         {
         printf("Saving the file was unsuccessful.");
         }
      close(fileHandle);
      }
   else
      printf("File could not be opened.");

   return;
   }

/*
** Function to apply the filter based off the user-input filter.
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
** int filter - filter to be applied
** int rate - if using filter 1, this is the input rate
**
** void function, no return value
*/
void applyFilters(struct mem* memPointer, int filter, double rate)
   {
   if (filter == 1) changeSampleRate(memPointer, rate);
   else if (filter == 2) reverseSoundBlocks(memPointer);
   else if (filter == 3) swapAndReverse(memPointer);
   return;
   }

/*
** Function to alter the sample rate by an input
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
** int rate - the number to multiply the sampleRate by
**
** void function, no return value
*/
void changeSampleRate(struct mem* memPointer, double rate)
   {
   memPointer->wave.subchunk1.sampleRate = (int)(memPointer->wave.subchunk1.sampleRate * rate);
   determineCalculatedFields(memPointer);

   return;
   }

/**
** Function to reverse the sound blocks of the file
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** void function, no return value
*/
void reverseSoundBlocks(struct mem* memPointer)
   {
   int numBlocks;
   int currIndex;
   int swapIndex;
   int subchunk2Size;
   BYTE temp;

   numBlocks = memPointer->wave.subchunk2.subchunk2Size / memPointer->wave.subchunk1.blockAlign;
   subchunk2Size = memPointer->wave.subchunk2.subchunk2Size;

   //reverse the samples in the data array
   for (int i = 0; i < numBlocks/2; i++)
      {
      //use i to calculate starting point of block
      //blockAlign includes numChannels
      //memPointer->wave.subchunk1.blockAlign * i
      for (int j = 0; j < memPointer->wave.subchunk1.blockAlign; j++)
         {
         currIndex = (memPointer->wave.subchunk1.blockAlign * i) + j;
         //locate the index to swap with by starting from the end of array
         swapIndex = subchunk2Size - (memPointer->wave.subchunk1.blockAlign * i) + j;

         temp = memPointer->wave.subchunk2.data[currIndex];
         memPointer->wave.subchunk2.data[currIndex] = memPointer->wave.subchunk2.data[swapIndex];
         memPointer->wave.subchunk2.data[swapIndex] = temp;
         }
      }

    return;
    }

/*
** Function that swaps the first and second half of the sound file
** and then reverses it.
**
**
** args:
** struct mem* memPointer - pointer to the memory space for the file
**
** void function, no return value
*/
void swapAndReverse(struct mem* memPointer)
   {
   int numBlocks;
   int currIndex;
   int swapIndex;
   BYTE temp;
   int subchunk2Size;

   numBlocks = memPointer->wave.subchunk2.subchunk2Size / memPointer->wave.subchunk1.blockAlign;
   subchunk2Size = memPointer->wave.subchunk2.subchunk2Size;

   //swap first half and second half of samples
   for (int i = 0; i < numBlocks/2; i++)
      {
      for (int j = 0; j < memPointer->wave.subchunk1.blockAlign; j++)
         {
         currIndex = (memPointer->wave.subchunk1.blockAlign * i) + j;
         swapIndex = (memPointer->wave.subchunk1.blockAlign * (numBlocks/2 + i)) + j;
         temp = memPointer->wave.subchunk2.data[currIndex];
         memPointer->wave.subchunk2.data[currIndex] = memPointer->wave.subchunk2.data[swapIndex];
         memPointer->wave.subchunk2.data[swapIndex] = temp;
         }
      }

      //reverse the samples
      for (int i = 0; i < numBlocks/2; i++)
         {
         for (int j = 0; j < memPointer->wave.subchunk1.blockAlign; j++)
            {
            currIndex = (memPointer->wave.subchunk1.blockAlign * i) + j;
            //printf("numBlocks/2: %d", numBlocks/2);
            swapIndex = subchunk2Size - (memPointer->wave.subchunk1.blockAlign * i) + j;
            temp = memPointer->wave.subchunk2.data[currIndex];
            memPointer->wave.subchunk2.data[currIndex] = memPointer->wave.subchunk2.data[swapIndex];
            memPointer->wave.subchunk2.data[swapIndex] = temp;
            }
         }

   return;
   }

/*
** main entry function
** Arguments are accepted and should be in the form of
** mycode inputfilename outputfilename filternumber [arguments]
** Arguments will follow the filternumber only if filter 1 is selected.
** Always exits with zero.
*/
int main(int argc, char* argv[])
   {
   int userInputCorrect = 1;
   double rate;
   struct mem* memPointer;

   //if filter 1 is selected, rate will be the argument
   //otherwise, it will be passed in as a dummy argument
   rate = 0;

   if (argc < 4 || (atoi(argv[3]) == 1 && argv[4] == NIL) || atoi(argv[3]) < 1 || atoi(argv[3]) > 3) userInputCorrect = 0;

   if (userInputCorrect == 1)
      {
      memPointer = openFile(argv[1]);

      if (memPointer == (struct mem*)NIL) printf("Memory was not allocated correctly.");
      else
         {
         if (isWavFile(memPointer) == 1) //check if the file is a WAV file
            {
            if (isPCMFile(memPointer) == 1) //check is he file is a PCM file
               {
               checkBlockAlign(memPointer);
               determineCalculatedFields(memPointer); //fix any calculated fields
               printf("Num channels: %d\n", memPointer->wave.subchunk1.numChannels);
               printf("Sample rate: %d\n", memPointer->wave.subchunk1.sampleRate);
               printf("Number of data samples %d\n", memPointer->wave.subchunk2.subchunk2Size / memPointer->wave.subchunk1.blockAlign);

               //argv[4] will exist if filter 1, so set rate to argv[4]
               if (argv[4] != NIL) rate = atof(argv[4]);

               applyFilters(memPointer, atoi(argv[3]), rate);
               saveFile(argv[2], memPointer);
               }
            }
         }
      //smart free that frees memory if pointer exists
      smartFree(memPointer);
      }
    else
       {
       printf("\nCorrect: mycode inputfilename outputfilename filternumber [arguments]\n");
       printf("\nFilter must be from 1 to 3\n");
       printf("\nFilter 1 will alter the sample rate, and requires an additional double (the rate) afterward\n");
       printf("\nFilter 2 reverses the order of the sound blocks. No additional arguments.\n");
       printf("\nFilter 3 swaps the first half and second half of the sound sample and reverses the first half. No additional arguments.\n");
       }

   exit(0);
   }
