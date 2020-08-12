/////////////////////////////////////////////////////
///     Seth Jaksik
///       ID: 1001541359
///     Jason Bernard Lim
///       ID: 1001640993
///     3320-003
///     Page Faults Assignment
///
///     Compilation: g++ pf.cpp -o pf
/////////////////////////////////////////////////////


// The MIT License (MIT)
//
// Copyright (c) 2020 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/////////////////////////////////////////////////////
///
/// INCLUDES
///
/////////////////////////////////////////////////////
#include <iostream>
#include <queue>
#include <deque>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <vector>

using namespace std;

/////////////////////////////////////////////////////
///
/// CONSTANTS
///
/////////////////////////////////////////////////////
#define MAX_LINE 1024

/////////////////////////////////////////////////////
///
/// FUNCTIONS
///
/////////////////////////////////////////////////////
void FIFO(char *line);
void LRU(char *line);
void MFU(char *line);
void Optimal(char *line);
int findOptimal(vector<int> remaining, vector<int> working_set);
vector<int> vectorize(char *temp);
int findLRU(map<int,int> working_set);
int findMFU(map<int,int> working_set);

/////////////////////////////////////////////////////
///
///     int main(int argc, char *argv[])
///
/////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  char *line = NULL;
  size_t line_length = MAX_LINE;
  char *filename;

  FILE *file;

  if (argc < 2)
  {
    printf("Error: You must provide a datafile as an argument.\n");
    printf("Example: ./pf datafile.txt\n");
    exit(EXIT_FAILURE);
  }

  filename = (char *)malloc(strlen(argv[1]) + 1);
  line = (char *)malloc(MAX_LINE);

  memset(filename, 0, strlen(argv[1] + 1));
  strncpy(filename, argv[1], strlen(argv[1]));

  printf("Opening file %s\n", filename);
  file = fopen(filename, "r");

  if (file)
  {
    //While there is a new page string print out the various counts of the page fault algorithms
    while (fgets(line, line_length, file))
    {
      cout << line << endl;
      FIFO(line);
      LRU(line);
      MFU(line);
      Optimal(line);
      cout << endl;
    }

    free(line);
    fclose(file);
  }

  return 0;
}

/////////////////////////////////////////////////////
///     void FIFO (char *line)
///
///     Purpose: this function counts the number of
///               page faults using the FIFO algorithm
///
///     Inputs: char *line -- the page string
///
///     Return: void
/////////////////////////////////////////////////////

void FIFO(char *line)
{
  char *token;
  //create a copy of the page string so you do not mess up the original pointer
  char *temp = (char *)malloc(MAX_LINE);
  strcpy(temp, line);
  token = strtok(temp, " ");
  //create the working set size
  int working_set_size = atoi(token);
  deque<int> working_set;
  //count is the number of page faults
  int count = 0;

  while (token != NULL)
  {
    //while the page string has not ended request the next page
    token = strtok(NULL, " ");
    if (token != NULL)
    {
      //printf("Request: %d \n", atoi(token));
      //find to see if the requested page is in the working set
      auto it = find(working_set.begin(), working_set.end(), atoi(token));
      //if yes do nothing but go to the next request
      if (it != working_set.end())
      {
      }
      else
      {
        //else, increment the page fault counter
        count++;
        //if the working set is full pop the front while pushing the newest value in the back
        if (working_set.size() == working_set_size)
        {
          working_set.pop_front();
          working_set.push_back(atoi(token));
        }
        else
        {
          //otherwise, just push the value
          working_set.push_back(atoi(token));
        }
      }
    }
  }
  //free the temp string and print out the number of page faults
  free(temp);
  printf("Page faults of FIFO: %d\n", count);
}

/////////////////////////////////////////////////////
///     void LRU (char *line)
///
///     Purpose: this function counts the number of
///               page faults using the LRU algorithm
///
///     Inputs: char *line -- the page string
///
///     Return: void
/////////////////////////////////////////////////////

void LRU(char *line)
{
  char *token;
  char * temp = (char *)malloc(MAX_LINE);
  strcpy(temp, line);
  token = strtok(temp, " ");
  int working_set_size = atoi(token);
  //the map holds the page value as the main attribute 
  //  and the index of when the value was last used
  map<int,int> working_set;
  int count = 0;
  int use_cnt = 1;

  while (token != NULL)
  {
    token = strtok(NULL, " ");
    if (token != NULL)
    {
      //printf("Request: %d \n", atoi(token));
      auto it = working_set.find(atoi(token));
      //if the value was found in the working set, increment the iterator and increment the use_cnt
      if (it != working_set.end())
      {
        it->second = use_cnt;
        use_cnt++;
      }
      else
      {
        //if it wasn't found increment page fault counter
        count++;
        if (working_set.size() == working_set_size)
        {
          //find the value to erase then push the new value in
          int temp = findLRU(working_set);
          working_set.erase(working_set.find(temp));
          working_set.emplace(pair<int, int>(atoi(token),use_cnt));
        }
        else
        {
          working_set.emplace(pair<int, int>(atoi(token),use_cnt));
        }
        use_cnt++;
      }
    }
  }
  printf("Page faults of LRU: %d\n", count);
  free(temp);
}

/////////////////////////////////////////////////////
///     void MFU (char *line)
///
///     Purpose: this function counts the number of
///               page faults using the MFU algorithm
///
///     Inputs: char *line -- the page string
///
///     Return: void
/////////////////////////////////////////////////////

void MFU(char *line)
{
  char *token;
  char * temp = (char *)malloc(MAX_LINE);
  strcpy(temp, line);
  token = strtok(temp, " ");
  int working_set_size = atoi(token);
  //create a map that holds the page value and the number of times it has been used
  map<int,int> working_set;
  int count = 0;

  while (token != NULL)
  {
    token = strtok(NULL, " ");
    if (token != NULL)
    {
      //printf("Request: %d \n", atoi(token));
      auto it = working_set.find(atoi(token));
      //if the value was found in the working set, increment the iterator
      if (it != working_set.end())
      {
        it->second++;
      }
      else
      {
        //else, increment the page fault counter
        count++;
        if (working_set.size() == working_set_size)
        {
          //find the value to erase then push the new value in
          int temp = findMFU(working_set);
          working_set.erase(working_set.find(temp));
          working_set.emplace(pair<int, int>(atoi(token),1));
        }
        else
        {
          working_set.emplace(pair<int, int>(atoi(token),1));
        }
      }
    }
  }
  printf("Page faults of MFU: %d\n", count);
  free(temp);
}

/////////////////////////////////////////////////////
///     void Optimal (char *line)
///
///     Purpose: this function counts the number of
///               page faults using the Optimal algorithm
///
///     Inputs: char *line -- the page string
///
///     Return: void
/////////////////////////////////////////////////////
void Optimal(char *line)
{
  char *token;
  char *temp = (char *)malloc(MAX_LINE);
  char *temp_cpy = (char *)malloc(MAX_LINE);
  strcpy(temp, line);
  strcpy(temp_cpy, line);
  //take the page string and copy it into a vector to use for optimal
  vector<int> remaining = vectorize(temp_cpy);
  token = strtok(temp, " ");
  int working_set_size = atoi(token);
  vector<int> working_set;
  int count = 0;

  while (token != NULL)
  {
    token = strtok(NULL, " ");
    if (token != NULL)
    {
      //printf("Request: %d \n", atoi(token));
      auto it = find(working_set.begin(), working_set.end(), atoi(token));
      //if the value is found in the working set just delete the value in the remaining vector
      if (it != working_set.end())
      {
        remaining.erase(remaining.begin());
      }
      else
      {
        //else increment the page fault counter
        count++;
        if (working_set.size() == working_set_size)
        {
          //erase the current value in the remaining then find the index to erase
          remaining.erase(remaining.begin());
          //find the index to remove then remove it from the working set then push the new value
          int remove = findOptimal(remaining, working_set);
          working_set.erase(working_set.begin() + remove);
          working_set.push_back(atoi(token));
        }
        else
        {
          remaining.erase(remaining.begin());
          working_set.push_back(atoi(token));
        }
      }
    }
  }
  printf("Page faults of Optimal: %d\n", count);
  free(temp);
  free(temp_cpy);
}

/////////////////////////////////////////////////////
///     vector<int> vectorize(char *temp)
///
///     Purpose: this function copies the page string
///              and puts it into a vector to use for
///              optimal page replacement algorithm
///
///     Inputs: char *temp --a copy of the page string
///
///     Return: returns a vector version of the page 
///             string
/////////////////////////////////////////////////////
vector<int> vectorize(char *temp)
{
  vector<int> rtn;
  char *token;
  //remove the first value as it is the working set size
  strtok(temp, " ");
  //while there are still values in the page string push them back into the vector
  while (token = strtok(NULL, " "))
  {
    rtn.push_back(atoi(token));
  }
  return rtn;
}

/////////////////////////////////////////////////////
///     int findOptimal(vector<int> remaining, vector<int> working_set)
///
///     Purpose: this function find the index of the 
///               value to erase in the working set
///               for the optimal algorithm
///
///     Inputs: vector<int> remaining --the "future" page string
///             vector<int> working_set --the current working set
///
///     Return: returns the index of the value to be
///              erased
/////////////////////////////////////////////////////
int findOptimal(vector<int> remaining, vector<int> working_set)
{
  int index = 0, final_count = 0, count = 0;
  //if we are not at the end of the page string,
  //  look for where the next occurence of the value in the working set is and
  //  if the distance of that is greater than the maximum,
  //  set the index and set the distance as the largest one
  if (remaining.size() != 0)
  {
    for (int i = 0; i < working_set.size(); i++)
    {
      count = 0;
      while (remaining[count] != working_set[i] && count < remaining.size() - 1)
        count++;
      if (count > final_count)
      {
        index = i;
        final_count = count;
      }
    }
  }
  return index;
}

/////////////////////////////////////////////////////
///     int findLRU(map<int, int> working set)
///
///     Purpose: this function finds the value to be 
///              erased in the LRU algorithm
///
///     Inputs: map<int, int> working_set --the current working set
///
///     Return: returns the value to be erased in the 
///             working set
/////////////////////////////////////////////////////

int findLRU(map<int,int> working_set)
{
  int ret = 0;
  int curr_min = 9999;
  //for the values inside the working set, if the number of occurences 
  //  is smaller than the next value in the working set, set the first value
  //  as the value to erase then set it's number of occurences to the iterator
  for (auto it = working_set.begin(); it != working_set.end(); it++)
  {
    if(it->second < curr_min)
    {
      ret = it->first;
      curr_min = it->second;
    }
  }
  return ret;
}

/////////////////////////////////////////////////////
///     int findMFU(map<int, int> working set)
///
///     Purpose: this function finds the value to be 
///              erased in the MFU algorithm
///
///     Inputs: map<int, int> working_set --the current working set
///
///     Return: returns the value to be erased in the 
///             working set
/////////////////////////////////////////////////////
int findMFU(map<int,int> working_set)
{
  int ret = 0;
  int curr_max = 0;
  //for the values inside the working set, if the number of occurences 
  //  is greater than the next value in the working set, set the first value
  //  as the value to erase then set it's number of occurences to the iterator
  for (auto it = working_set.begin(); it != working_set.end(); it++)
  {
    if(it->second >= curr_max)
    {
      ret = it->first;
      curr_max = it->second;
    }
  }
  return ret;
}