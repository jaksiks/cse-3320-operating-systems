# Threading Assignment

## Description

In this assignment you will gain hands-on experience with parallel programming and the difficulty of building correct parallel programs. You are tasked with helping your professor schedule his office hours. The professor is teaching 2 classes this semester, class A and class B, and is holding shared office hours for both classes in his office. The professor can have a large number of students showing up for his office hours, so he decides to impose several restrictions.  

## Administrative

You have been provided a framework for the simulation, which creates a thread for the professor and a thread for each student who wants to attend the office hour. You will need to add synchronization to ensure the above restrictions. 

The source code for this assignment can be found on the course website at:  

https://github.com/CSE3320/Office-Hours-Assignment 

The student threads are implemented in the functions classa_student() and classb_student(), which simulate students from class A and class B, respectively. After being created a student from class A executes three functions: he enters the office (classa_enter()), he asks questions (ask_questions()) and then leaves the office (classa_leave()). A student from class B calls the corresponding functions classb_enter(), ask_questions() and classb_leave(). All synchronization between threads should be added in the ..._enter() and ..._leave() functions of the students and the function professorthread(), which implements the professor. 

The simulation framework is implemented in the file officehours.c. The program expects as an argument the name of an input file which controls the simulation. The input file specifies the arrival of students and the amount of time students spend in the professor’s office. More precisely, the file has one line for each student containing two numbers. The first number is the time (in seconds) between the arrival of this student and the previous 
student. The second number is the number of seconds the students needs to spend with the professor. 

The provided code implements all the functionality for the students and the professor, but does not implement any synchronization. To help you in developing your code a number of assert statements that help you check for correctness have been added. DO NOT delete those assert statements. Also, do not make any changes to the functions ask_questions and take_break. You might want to add additional assert statements, for example for ensuring that the number of students since the last break is less than the limit, or for ensuring that there are not class A and class B students in the office at the same time. 

Before you start your work, compile officehours.c and try to run it on this sample input file sample_input.txt. This will simulate 3 different students asking questions for 25, 10 and 15 seconds, respectively. The time between the first and the second student arriving is 10 seconds and the time between the second and the last student is 5 seconds. . 

This assignment must be coded in C. Any other language will result in 0 points. You programs will be compiled and graded on omega.uta.edu. Please make sure they compile and run on omega before submitting them.  Code that does not compile on omega with: 

gcc officehours.c -o officehours -lpthread 

will result in a 0. 

Your program, officehours.c is to be turned in via blackboard. Submission time is determined by the blackboard system time. You may submit your programs as often as you wish. Only your last submission will be graded.  

You may use no other outside code