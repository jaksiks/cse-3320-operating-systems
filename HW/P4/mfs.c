/////////////////////////////////////////////////////
///     Seth Jaksik
///       ID: 1001541359
///     Jason Bernard Lim
///       ID: 1001640993
///     3320-003
///     FAT32 Assignment
///
///     Compilation: gcc mfs.c -o mfs
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
///
/// INCLUDES
///
/////////////////////////////////////////////////////
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////////////////////////////////
///
/// CONSTANTS
///
/////////////////////////////////////////////////////
#define EOF32 = 0x0FFFFFF8;
#define MaxFileName = 100;
#define ClnShutBitMask = 0x08000000;
#define HrdErrBitMask = 0x0400000;
#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 10
#define True 1
#define False 0

/////////////////////////////////////////////////////
///
/// FUNCTIONS DECLARATIONS
///
/////////////////////////////////////////////////////

// msf functions
void open(char *filename);
void close();
void info();
void stat(char *filename);
void get();
void cd(char *dir);
void ls(char *token);
void read();

// FAT manipulations
int LBAToOffset(int32_t sector);
int16_t NextLB(uint32_t sector);

/////////////////////////////////////////////////////
///
/// GLOBALS
///
/////////////////////////////////////////////////////
struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

FILE *fp;
int fileOpen = False;
struct DirectoryEntry dir[16];
char BS_OMEName[8];
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluseter = 0;

/////////////////////////////////////////////////////
///
///     int main(int argc, char *argv[])
///
///     Function Desc:
///         This programs runs the simulated shell
///         to parse a fat32 file sysyem.
/////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
    char **cmd_history = (char **)malloc(sizeof(char *) * 15);
    int *pid_history = (int *)malloc(sizeof(int) * 15);
    int curr_cmd = 0, curr_pid = 0;

    while (1)
    {
        // Print out the mfs prompt
        printf("mfs> ");

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
        if (token[0] != NULL)
        {
            //give the various cases given the user input (i.e open, close, info, etc)
            if (strcmp(token[0], "open") == 0)
            {
                if (token[1] != NULL)
                    open(token[1]);
                else
                    printf("Error: Improper format. Please put in format open <filename>\n");
            }
            else if (strcmp(token[0], "info") == 0)
            {
                info();
            }
            else if (strcmp(token[0], "close") == 0)
            {
                close();
            }
            else if (strcmp(token[0], "stat") == 0)
            {
                if (token[1] != NULL)
                    stat(token[1]);
                else
                    printf("Error: Improper format. Please put in format stat <filename> or <directory name>\n");
            }
            else if (strcmp(token[0], "get") == 0)
            {
                if (token[1] != NULL)
                    get(token[1]);
                else
                    printf("Error: Improper format. Please put in format get <filename>\n");
            }
            else if (strcmp(token[0], "cd") == 0)
            {
                if (token[1] != NULL)
                    cd(token[1]);
                else
                    printf("Error: Improper format. Please put in format cd <directory>\n");
            }
            else if (strcmp(token[0], "ls") == 0)
            {
                if (token[1] != NULL)
                {
                    ls(token[1]);
                }
                else
                {
                    ls("");
                }
            }
            else if (strcmp(token[0], "read") == 0)
            {
                if (token[1] != NULL && token[2] != NULL && token[3] != NULL)
                    read(token[1], atoi(token[2]), atoi(token[3]));
                else
                    printf("Error: Improper format. Please put in format read <filename> <position> <number of bytes>\n");
            }
            else if (strcmp(token[0], "exit") == 0)
            {
                exit(0);
            }
            else
            {
                printf("Error: Invalid Command\n");
            }
        }
    }
    // ROOT DIR ADDRESS(BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);
    // FROM FAT32 SPEC
    // RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytesPerSec - 1)) / BPB_BytesPerSec;
    // TmpVal1 = DskSize – (BPB_ResvdSecCnt + RootDirSectors);
    // TmpVal2 = (256 * BPB_SecPerClus) + BPB_NumFATs;
    // TmpVal2 = TmpVal2 / 2;
    // FATSz = (TMPVal1 + (TmpVal2 - 1)) / TmpVal2;
    // BPB_FATSz32 = FATSz;
}

/////////////////////////////////////////////////////
///
/// FUNCTIONS DEFINES
///
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
///
///     void open(char *filename)
///
///     Params:
///     char *filename - file system img to be opened
///
///     Function Desc:
///         This functions opens the file system img
/////////////////////////////////////////////////////
void open(char *filename)
{
    char buffer[100];
    fp = fopen(filename, "r+");
    //check if the file system image file exists
    if (fp == NULL)
    {
        printf("Error: File system image not found.\n");
    }
    else if (fileOpen == True)
    {
        printf("Error: File system image already open.\n");
    }
    else
    {
        fileOpen = True;
        // if it does initialize the global vars from spec
        fseek(fp, 3, SEEK_SET);
        fread(buffer, 8, 1, fp);
        memcpy(BS_OMEName, buffer, 8);

        fseek(fp, 11, SEEK_SET);
        fread(buffer, 2, 1, fp);
        memcpy(&BPB_BytesPerSec, buffer, 2);

        fseek(fp, 13, SEEK_SET);
        fread(buffer, 1, 1, fp);
        memcpy(&BPB_SecPerClus, buffer, 1);

        fseek(fp, 14, SEEK_SET);
        fread(buffer, 2, 1, fp);
        memcpy(&BPB_RsvdSecCnt, buffer, 2);

        fseek(fp, 16, SEEK_SET);
        fread(buffer, 1, 1, fp);
        memcpy(&BPB_NumFATs, buffer, 1);

        fseek(fp, 17, SEEK_SET);
        fread(buffer, 2, 1, fp);
        memcpy(&BPB_RootEntCnt, buffer, 2);

        fseek(fp, 36, SEEK_SET);
        fread(buffer, 4, 1, fp);
        memcpy(&BPB_FATSz32, buffer, 4);

        fseek(fp, 43, SEEK_SET);
        fread(buffer, 11, 1, fp);
        memcpy(&BS_VolLab, buffer, 11);

        fseek(fp, 44, SEEK_SET);
        fread(buffer, 4, 1, fp);
        memcpy(&BPB_RootClus, buffer, 4);

        BPB_RootClus = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);
        int i;
        for (int i = 0; i < 16; i++)
        {
            int startAdd = BPB_RootClus + (i * 32);

            fseek(fp, startAdd, SEEK_SET);
            fread(&dir[i], 32, 1, fp);
        }
        FirstDataSector = BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32) + RootDirSectors;
    }
}

/////////////////////////////////////////////////////
///
///     void close()
///
///
///     Function Desc:
///         This functions closes the file system img
/////////////////////////////////////////////////////
void close()
{
    if (fileOpen == False)
    {
        printf("Error: File system not open.\n");
    }
    else
    {
        fileOpen = False;
        memset(&BS_OMEName, 0, sizeof(char) * 8);
        memset(&BPB_BytesPerSec, 0, sizeof(int16_t));
        memset(&BPB_SecPerClus, 0, sizeof(int8_t));
        memset(&BPB_RsvdSecCnt, 0, sizeof(int16_t));
        memset(&BPB_NumFATs, 0, sizeof(int8_t));
        memset(&BPB_RootEntCnt, 0, sizeof(int16_t));
        memset(&BS_VolLab, 0, sizeof(char) * 11);
        memset(&BPB_FATSz32, 0, sizeof(int32_t));
        memset(&BPB_RootClus, 0, sizeof(int32_t));
    }
}

/////////////////////////////////////////////////////
///
/// void info()
///
///     Prints out information of the file system in
///     both hex and base 10
///
/////////////////////////////////////////////////////
void info()
{
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else
    {
        printf("-----FILE SYSTEM INFO------\n");
        printf("--------------------DEC\tHEX\n");
        printf("- BPB_BytesPerSec:  %d\t%x\n", BPB_BytesPerSec, BPB_BytesPerSec);
        printf("- BPB_SecPerClus:   %d\t%x\n", BPB_SecPerClus, BPB_SecPerClus);
        printf("- BPB_RsvdSecCnt:   %d\t%x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
        printf("- BPB_NumFATs:      %d\t%x\n", BPB_NumFATs, BPB_NumFATs);
        printf("- BPB_FATSz32:      %d\t%x\n", BPB_FATSz32, BPB_FATSz32);
        printf("----------------------------\n");
    }
}

/////////////////////////////////////////////////////
///
///     void stat(char *filename)
///
///     Params:
///     char *filename - file system img to be opened
///
///     Function Desc:
///         This command shall print the attributes and starting cluster number 
///         of the file or directory name. If the parameter is a directory name then the size shall be 0. 
///         If the file or directory does not exist
///        then your program shall output “Error: File not found”. 
/////////////////////////////////////////////////////
void stat(char *filename)
{
    //check the various stats for a file or directory
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else if (strcmp(filename, ".") == 0) 
    {
        int i;
        for (i = 0; i < 16; i++)
        {
            if (dir[i].DIR_Name[0] == 0x2e)
            {
                char temp[11];
                strncpy(temp, dir[i].DIR_Name, sizeof(temp));
                temp[11] = '\0';
                printf("File Name: %s\n", temp);
                printf("Attrib Byte: %x\n", dir[i].DIR_Attr);
                printf("First Cluster High: %d\n", dir[i].DIR_FirstClusterHigh);
                printf("First Cluster Low: %d\n", dir[i].DIR_FirstClusterLow);
                if (dir[i].DIR_Attr == 0x10)
                {
                    printf("File Size: 0\n");
                }
                else
                {
                    printf("File Size: %x\n", dir[i].DIR_FileSize);
                }
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
    else if (strcmp(filename, "..") == 0)
    {
        int i;
        for (i = 0; i < 16; i++)
        {
            if (dir[i].DIR_Name[0] == 0x2e && dir[i].DIR_Name[1] == 0x2e)
            {
                char temp[11];
                strncpy(temp, dir[i].DIR_Name, sizeof(temp));
                temp[11] = '\0';
                printf("File Name: %s\n", temp);
                printf("Attrib Byte: %x\n", dir[i].DIR_Attr);
                printf("First Cluster High: %d\n", dir[i].DIR_FirstClusterHigh);
                printf("First Cluster Low: %d\n", dir[i].DIR_FirstClusterLow);
                //if the file is a directory set the file size = 0
                if (dir[i].DIR_Attr == 0x10)
                {
                    printf("File Size: 0\n");
                }
                else
                {
                    printf("File Size: %x\n", dir[i].DIR_FileSize);
                }
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
    else
    {
        //for a file find the name for the file and check the stats
        char expanded_name[12];
        memset(expanded_name, ' ', 12);
        char *token = strtok(filename, ".");
        strncpy(expanded_name, token, strlen(token));
        token = strtok(NULL, ".");
        if (token)
        {
            strncpy((char *)(expanded_name + 8), token, strlen(token));
        }
        expanded_name[11] = '\0';
        int i;
        for (i = 0; i < 11; i++)
        {
            expanded_name[i] = toupper(expanded_name[i]);
        }

        for (i = 0; i < 16; i++)
        {
            if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
            {
                //print out the various stats needed
                char temp[11];
                strncpy(temp, dir[i].DIR_Name, sizeof(temp));
                temp[11] = '\0';
                printf("File Name: %s\n", temp);
                printf("Attrib Byte: %x\n", dir[i].DIR_Attr);
                printf("First Cluster High: %d\n", dir[i].DIR_FirstClusterHigh);
                printf("First Cluster Low: %d\n", dir[i].DIR_FirstClusterLow);
                if (dir[i].DIR_Attr == 0x10)
                {
                    printf("File Size: 0\n");
                }
                else
                {
                    printf("File Size: %x\n", dir[i].DIR_FileSize);
                }
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
}

/////////////////////////////////////////////////////
///
///     void get(char *filename)
///
///     Params:
///     char *filename - file name to be gotten
///
///     Function Desc:
///         This functions gets a file specified
///         and puts it in your current working directory
/////////////////////////////////////////////////////
void get(char *filename)
{
    FILE *writeTo;
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else if (strcmp(filename, "..") == 0)
    {
        printf("Error: Cannot get a directory.\n");
    }
    else
    {
        //find the file name to get and put it in your current directory
        char *use = (char *)malloc(sizeof(filename));
        strcpy(use, filename);
        char expanded_name[12];
        memset(expanded_name, ' ', 12);
        char *token = strtok(use, ".");
        strncpy(expanded_name, token, strlen(token));
        token = strtok(NULL, ".");
        if (token)
        {
            strncpy((char *)(expanded_name + 8), token, strlen(token));
        }
        expanded_name[11] = '\0';
        int i;
        for (i = 0; i < 11; i++)
        {
            expanded_name[i] = toupper(expanded_name[i]);
        }

        for (i = 0; i < 16; i++)
        {
            if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
            {
                //get open the file
                char output;
                writeTo = fopen(filename, "w");
                int sect = dir[i].DIR_FirstClusterLow;
                int add = LBAToOffset(sect);
                int readSoFar;
                for (readSoFar = 0; readSoFar < dir[i].DIR_FileSize; readSoFar++)
                {
                    //if it goes past the current LB 
                    //find the next LB
                    if (readSoFar % 512 == 0 && readSoFar != 0)
                    {
                        sect = NextLB(sect);
                        add = LBAToOffset(sect);
                    }
                    fseek(fp, add++, SEEK_SET);
                    fread(&output, 1, 1, fp);
                    //put the file into directory
                    fputc(output, writeTo);
                }
                //close the file
                fclose(writeTo);
                //break as to not constantly repeat
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
}

/////////////////////////////////////////////////////
///
///     void cd(char *filename)
///
///     Params:
///     char *filename - directory to be cd'd into
///
///     Function Desc:
///         This function cd's into the specified directory
/////////////////////////////////////////////////////
void cd(char *filename)
{
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else
    {
        //if there is more than one directory in the path
        //  strtok by the '/' then search for that directory then cd into it
        char *tok = strtok(filename, "/");
        while (tok != NULL)
        {
            if (strcmp(tok, "..") == 0)
            {
                int i;
                for (i = 0; i < 16; i++)
                {
                    if (dir[i].DIR_Name[0] == 0x2e && dir[i].DIR_Name[1] == 0x2e)
                    {
                        int offset;
                        if (dir[i].DIR_FirstClusterLow == 0)
                        {
                            offset = LBAToOffset(2);
                        }
                        else
                        {
                            offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
                        }
                        int j;
                        for (j = 0; j < 16; j++)
                        {
                            //calculate the start address then cd from there
                            int startAdd = offset + (j * 32);
                            fseek(fp, startAdd, SEEK_SET);
                            fread(&dir[j], 32, 1, fp);
                        }
                        break;
                    }
                }
                if (i == 16)
                {
                    printf("Error: file not found\n");
                }
            }
            else
            {
                int i;
                char *use = (char *)malloc(sizeof(tok));
                strcpy(use, tok);

                for (i = 0; i < sizeof(use); i++)
                {
                    use[i] = toupper(use[i]);
                }
                for (i = strlen(use); i < 12; i++)
                {
                    strcat(use, " ");
                }
                use[11] = '\0';

                for (i = 0; i < 16; i++)
                {
                    if (dir[i].DIR_Attr == 0x10 && strncmp(use, dir[i].DIR_Name, 11) == 0)
                    {
                        int offset;
                        if (dir[i].DIR_FirstClusterLow == 0)
                        {
                            offset = LBAToOffset(2);
                        }
                        else
                        {
                            offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
                        }
                        int j;
                        for (j = 0; j < 16; j++)
                        {
                            int startAdd = offset + (j * 32);
                            fseek(fp, startAdd, SEEK_SET);
                            fread(&dir[j], 32, 1, fp);
                        }
                        break;
                    }
                }
            }
            tok = strtok(NULL, "/");
        }
    }
}

/////////////////////////////////////////////////////
///
///     void ls(char *token)
///
///     Params:
///     char *token - if you want to ls the parent directory
///
///     Function Desc:
///         This function lists the files/subdirectories 
///         in the current directory.
/////////////////////////////////////////////////////
void ls(char *token)
{
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else if (strcmp(token, "..") == 0)
    {
        //if asking for the parent directory
        //find it then print it
        int i;
        for (i = 0; i < 16; i++)
        {
            if (dir[i].DIR_Name[0] == 0x00)
            {
                break;
            }
            else if (dir[i].DIR_Name[0] == 0x2e && dir[i].DIR_Name[1] == 0x2e)
            {
                int offset;
                if (dir[i].DIR_FirstClusterLow == 0)
                {
                    offset = LBAToOffset(2);
                }
                else
                {
                    offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
                }
                int j;
                struct DirectoryEntry temp[16];
                for (j = 0; j < 16; j++)
                {
                    int startAdd = offset + (j * 32);
                    fseek(fp, startAdd, SEEK_SET);
                    fread(&temp[j], 32, 1, fp);
                }
                for (j = 0; j < 16; j++)
                {
                    if (temp[j].DIR_Name[0] == 0x00)
                    {
                        break;
                    }
                    //check if the files are to be printed out
                    else if ((temp[j].DIR_Attr == 0x01 || temp[j].DIR_Attr == 0x10 || temp[j].DIR_Attr == 0x20 || temp[j].DIR_Attr == 0x30) && (unsigned char)temp[j].DIR_Name[0] != 0xE5)
                    {
                        char tok[11];
                        strncpy(tok, temp[j].DIR_Name, sizeof(tok));
                        tok[11] = '\0';
                        printf("%s\n", tok);
                    }
                }
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
    else
    {
        //else just list the files in the current directory
        int i;
        for (i = 0; i < 16; i++)
        {
            if (dir[i].DIR_Name[0] == 0x00)
            {
                break;
            }
            //check if the files are to be printed out
            else if ((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20 || dir[i].DIR_Attr == 0x30) && (unsigned char)dir[i].DIR_Name[0] != 0xe5)
            {
                char temp[11];
                strncpy(temp, dir[i].DIR_Name, sizeof(temp));
                temp[11] = '\0';
                printf("%s\n", temp);
            }
        }
    }
}

/////////////////////////////////////////////////////
///
///     void read(char *filename, int position, int numBytes)
///
///     Params:
///     char *filename - file system img to be opened
///     int position - where to start in the file         
///     int numBytes - how many bytes to read
///
///     Function Desc:
///         This functions, given the starting position,
///         reads a specified number of bytes in a file.
/////////////////////////////////////////////////////
void read(char *filename, int position, int numBytes)
{
    if (fileOpen == False)
    {
        printf("Error: File system image must be opened first.\n");
    }
    else if (strcmp(filename, "..") == 0)
    {
        printf("Error: Cannot read a directory.\n");
    }
    else
    {
        //find the name if the file to be read
        char *use = (char *)malloc(sizeof(filename));
        strcpy(use, filename);
        char expanded_name[12];
        memset(expanded_name, ' ', 12);
        char *token = strtok(use, ".");
        strncpy(expanded_name, token, strlen(token));
        token = strtok(NULL, ".");
        if (token)
        {
            strncpy((char *)(expanded_name + 8), token, strlen(token));
        }
        expanded_name[11] = '\0';
        int i;
        for (i = 0; i < 11; i++)
        {
            expanded_name[i] = toupper(expanded_name[i]);
        }

        for (i = 0; i < 16; i++)
        {
            if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
            {
                char output;
                int sect = dir[i].DIR_FirstClusterLow;
                int add = LBAToOffset(sect);
                int readSoFar;
                for (readSoFar = 0; readSoFar < dir[i].DIR_FileSize; readSoFar++)
                {
                    //when the name is found, set the char to the position to start
                    //  then read for the number of bytes
                    if (readSoFar % 512 == 0 && readSoFar != 0)
                    {
                        sect = NextLB(sect);
                        add = LBAToOffset(sect);
                    }
                    fseek(fp, add++, SEEK_SET);
                    fread(&output, 1, 1, fp);
                    if (readSoFar >= position && readSoFar < (position + numBytes))
                    {
                        printf("%x ", output);
                    }
                }
                printf("\n");
                break;
            }
        }
        if (i == 16)
        {
            printf("Error: file not found\n");
        }
    }
}

/////////////////////////////////////////////////////
///
///     int LBAToOffset(int32_t sector)
///
///     Params:
///     int32_t sector - the sector
///
///     Function Desc:
///         This function calculates the logial block
///         address to the offset
/////////////////////////////////////////////////////
int LBAToOffset(int32_t sector)
{
    return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

/////////////////////////////////////////////////////
///
///     int16_t NextLB(uint32_t sector)
///
///     Params:
///     uint32_t sector - the sector
///
///     Function Desc:
///         This function calculates the 
///         next logial block
/////////////////////////////////////////////////////
int16_t NextLB(uint32_t sector)
{
    uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
}
