# FAT 32 Assignmnet

## Description

This assignment will familiarize you with the FAT32 file system.  You will become familiar with file allocation tables, endieness, as well as file access.  You will implement a user space shell application that is capable of interpreting a FAT32 file system image. The utility must not corrupt the file system image and should be robust.  No existing kernel code or any other FAT 32 utility code  may be used in your program. 

You may complete this assignment in groups of two or by yourself.  If you wish to be in a group of two the group leader must email me your group member’s names by April 24, 2020.   Your email must have the subject line “3320 [Section #] Assignment 4 Group” where section number is 002 or 003. ( 003 is the 5:30pm class, 002 is 7:00pm ) 

The code you submit for this assignment will be verified against a database consisting of kernel source, github code, stackoverflow, previous student’s submissions and other internet resources.  Code that is not 100% your own code will result in a grade of 0 and referral to the Office of Student Conduct. 

You can find a FAT32 file image, fat32.img, on Canvas under Assignment 4.  Also under Assignment 4 you can find the fat 32 specification, fatspec.pdf.  You will need this specification to interpret the file system image correctly.

## Administrative

The assignment will be graded out of 100 points. Compiler warnings are there to tell you something is not correct.  Pay attention to them and you will save yourself a lot of late nights debugging code.  Code that does not compile will earn 0.   

Your code will be compiled and graded on omega.uta.edu .  Your code must compile as: 

g++ mfs.c -o mfs or gcc mfs.c -o mfs