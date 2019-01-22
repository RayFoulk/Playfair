//------------------------------------------------------------------------|
// Copyright (c) 2019 by Raymond M. Foulk IV
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//------------------------------------------------------------------------|

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>

//------------------------------------------------------------------------|
#define KEYBLOCK_WIDTH		5
#define KEYBLOCK_HEIGHT		5
#define KEYBLOCK_SIZE		(KEYBLOCK_WIDTH * KEYBLOCK_HEIGHT)

//------------------------------------------------------------------------|
typedef struct
{
    char keyblock[KEYBLOCK_WIDTH][KEYBLOCK_HEIGHT];

    // Playfair must omit 1 letter (Typically J or Q)
    // And optionally map it to another character (Typically J to I)
    // It must also consider a specific character as a nonce (Typically X)
    char omit;
    char mapto;
    char nonce;

    // Pointers to the passphrase and message to encrypt/decrypt
    char * passphrase;
    char * message;

    // Options
    bool verbose;
    bool encode;
}
playfair_t;

//------------------------------------------------------------------------|
static playfair_t pf;

//------------------------------------------------------------------------|
static void init()
{
    memset(pf.keyblock, 0, KEYBLOCK_SIZE);

    pf.omit = 'J';
    pf.mapto = 'I';
    pf.nonce = 'X';

    pf.passphrase = "";
    pf.message = "";

    pf.verbose = false;
    pf.encode = true;
}

//------------------------------------------------------------------------|
static void quit(int error)
{
    // Cleanup...

    exit(error);
}

//------------------------------------------------------------------------|
static void help(const char * opts)
{
    printf("opts: '%s'\n", opts);
}

//------------------------------------------------------------------------|
static void parse(int argc, char *argv[])
{
    const char * opts = "vqn:p:m:edh";
    int option;
    extern char * optarg;

    // Show help and exit if no arguments
    if (argc < 2)
    {
        help(opts);
        quit(2);
    }

    // Process command line
    while ((option = getopt (argc, argv, opts)) > 0)
    {
        switch (option)
        {
            case 'v':
                pf.verbose = true;
                break;

            case 'q':
                // Drop 'Q' rather than mapping 'J' to 'I'
                pf.omit = 'Q';
                pf.mapto = '\0';
                break;

            case 'n':
                // Change the nonce from 'X' to something else
                pf.nonce = (char) toupper(optarg[0]);
                break;

            case 'p':
                // Set the passphrase pointer
                pf.passphrase = optarg;
                break;

            case 'm':
                // Set the message pointer
                pf.passphrase = optarg;
                break;

            case 'e':
                // Encode (Default)
                pf.encode = true;
                break;

            case 'd':
                // Decode
                pf.encode = false;
                break;

            case 'h':
            default:
                help(opts);
                quit(3);
                break;
        }
    }
}


//------------------------------------------------------------------------|
int main(int argc, char *argv[])
{
    init();
    parse(argc, argv);




    quit(0);
    return 0;
}
