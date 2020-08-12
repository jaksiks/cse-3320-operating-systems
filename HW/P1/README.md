# Shell Assignment

## Description

In this assignment you will write your own shell program, Mav shell (msh), similar to bourne shell (bash), c-shell (csh), or korn shell (ksh). It will accept commands, fork a child process and execute those commands. The shell, like csh or bash, will run and accept commands until the user exits the shell. Your file must be named msh.c

## Administrative

This assignment must be coded in C. Any other language will result in 0 points. You programs will be compiled and graded on omega.uta.edu. Please make sure they compile and run on omega before submitting them.  Code that does not compile on omega with:

 gcc -Wall msh.c -o msh —std=c99
 
will result in a 0. Your program, msh.c is to be turned in via Canvas. Submission time is determined by the Canvas system time. You may submit your programs as often as you wish. Only your last submission will be graded.  

There are coding resources and working code you may use on Canvas and in the course github repository at: https://github.com/CSE3320/Shell-Assignment . You are free to use any of that code in your program if needed.  You may use no other outside code.

## Hints

Read the man pages for the following: fork, exec, exit, print, fgets, strtok, strsep, strcmp, wait, and pause. 

Use fork and one of the exec family as discussed in class to execute the command and call wait to wait for the child to complete. If the command is “cd” then use chdir() instead of exec. Note, chdir() must be called from the parent.  

If you see garbage in any of your commands or parameters, try using the functions memset() or bzero() to clear out your input string and token array before and/or after you are done using them. Also, verify you are NULL terminating your strings.  

There are examples on the course GitHub repository for this assignment that show how to use execl and execvp.