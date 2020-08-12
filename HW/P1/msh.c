/*
  Name: Seth Jaksik
  ID:   1001541359
  File: msh.c
*/


// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
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

#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports ten arguments

// functions to add pids and cmds to arrays 
// these will help with rolling over commands if more than 15 exist
void add_pid(int* pid_array, int pid, int *cur_num);
void add_cmd(char** cmd_array, char *cmd, int *cur_num);

int main()
{
  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
  char **cmd_history = (char **)malloc(sizeof(char *) * 15);
  int *pid_history = (int *)malloc(sizeof(int) * 15);
  int curr_cmd = 0, curr_pid = 0;

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // if cmd contains a !, parse the command to load in
    // the specific command from history so that it is executed
    // instead of the number. 
    if (strstr(cmd_str, "!") != NULL)
    {
      char *cmd_num = strtok(cmd_str, "!");
      int cmd_int = atoi(cmd_num);
      // If trying to call a command that doesnt exist yet,
      // give warning and skip
      if(cmd_int > curr_cmd-1)
      {
        printf("Command not in history.\n");
        strcpy(cmd_str, "");
      }
      // load the requested command from history to be executed
      else
      {
        cmd_str = cmd_history[cmd_int];
      }
    }

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // if there is a command
    if (token[0] != NULL)
    {
      // exit shell commands
      // break out of loop, which will end the shell
      if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
      {
        break;
      }
      // change directory command
      // call the chdir command with the requested location from the 
      // parent since the child would end and not make a difference
      else if (strcmp(token[0], "cd") == 0)
      {
        add_cmd(cmd_history, cmd_str, &curr_cmd);
        chdir(token[1]);
      }
      // showpids command
      // print out the stored pids from the pid array
      else if (strcmp(token[0], "showpids") == 0)
      {
        add_cmd(cmd_history, cmd_str, &curr_cmd);
        for (int i = 0; i < curr_pid; i++)
        {
          printf("%d: %d\n", i, pid_history[i]);
        }
      }
      // history command
      // first store the history call in the command history
      // array, then print out the array with the list of
      // commands
      else if (strcmp(token[0], "history") == 0)
      {
        add_cmd(cmd_history, cmd_str, &curr_cmd);
        for (int i = 0; i < curr_cmd; i++)
        {
          printf("%d: %s", i, cmd_history[i]);
        }
      }
      else
      {
        // if a command that requires fork, fork then call execvp in child
        pid_t pid = fork();

        if (pid == 0) // this is the child process
        {
          // pass in the command and arguments into execvp which have 
          // already been parsed earlier with a null terminating
          // the end of the array
          // save the return value to check for bad calls
          int retVal = execvp(token[0], token);
          if (retVal == -1) // if a bad call, show command not found
          {
            cmd_str[strlen(cmd_str)-1] = 0;
            printf("%s : Command not found.\n", cmd_str);
          }
          return 0;
        }
        else // in the parent process, so add pids and cmd history into here
             // after the child is done executing
        {
          int status;
          wait(&status);
          add_cmd(cmd_history, cmd_str, &curr_cmd);
          add_pid(pid_history, pid, &curr_pid);
        }
      }
    }
    // free the current command string so we can recieve input again
    free(working_root);
  }
  // free the command strings that we have alloc'd to avoid memory leaks
  free(cmd_str);
  free(pid_history);
  for(int i = 0; i < curr_cmd; i++)
  {
    free(cmd_history[i]);
  }
  free(cmd_history);
  return 0;
}

// add pid to the pid history, and if over 15 pids exist, 
// move pids and add current pid at the end of the array
void add_pid(int* pid_array, int pid, int *cur_num)
{
  if(*cur_num < 15)
  {
    pid_array[(*cur_num)++] = pid;
  }
  else
  {
    for(int i = 0; i < 14; i++)
    {
      pid_array[i] = pid_array[i+1];
    }
    pid_array[14] = pid;
  }
}

// add cmd to the cmd history, and if over 15 cmds exist, 
// move cmds and add current cmd at the end of the array
void add_cmd(char** cmd_array, char* cmd, int *cur_num)
{
  if(*cur_num < 15)
  {
    cmd_array[*cur_num] = (char *)malloc(sizeof(char *));
    strcpy(cmd_array[(*cur_num)++], cmd);
  }
  else
  {
    for(int i = 0; i < 14; i++)
    {
      strcpy(cmd_array[i], cmd_array[i+1]);
    }
    strcpy(cmd_array[14], cmd);
  }
}