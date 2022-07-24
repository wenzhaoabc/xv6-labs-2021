#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[])
{
    char input[32]; // record the input from previous command
    char buf[320];  // buf for the char of all token
    char *bufP = buf;
    int charBufSize = 0;

    char *commandToken[32];   // record the token from input spilt by space(' ')
    int tokenSize = argc - 1; // record token number(initial is argc - 1,because xargs will bot be execute)
    int inputSize = -1;

    // first copy initial argv argument to commandToken
    for (int tokenIdx = 0; tokenIdx < tokenSize; tokenIdx++)
        commandToken[tokenIdx] = argv[tokenIdx + 1];

    while ((inputSize = read(0, input, sizeof(input))) > 0)
    {
        for (int i = 0; i < inputSize; i++)
        {
            char curChar = input[i];
            if (curChar == '\n')
            {                         // if read '\n', execute the command
                buf[charBufSize] = 0; // set '\0' to end of token
                commandToken[tokenSize++] = bufP;
                commandToken[tokenSize] = 0; // set nullptr in the end of array

                if (fork() == 0)
                { // open child to execute
                    exec(argv[1], commandToken);
                }
                wait(0);
                tokenSize = argc - 1; // initialize
                charBufSize = 0;
                bufP = buf;
            }
            else if (curChar == ' ')
            {
                buf[charBufSize++] = 0; // mark the end of string
                commandToken[tokenSize++] = bufP;
                bufP = buf + charBufSize; // change to the start of new string
            }
            else
            {
                buf[charBufSize++] = curChar;
            }
        }
    }
    exit(0);
}