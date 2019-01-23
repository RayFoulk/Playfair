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
    pf.passphrase = NULL;
    pf.message = NULL;
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
static void help(const char * prog, const char * opts)
{
    // Keep it simple.  No need to introduce opts module just yet...
    printf("usage: %s [%s]\n\n"
        "-v               Verbose mode.  Show intermediate info\n"
        "-q               Drop Q rather than mapping J to I\n"
        "-n <nonce>       Change nonce character from X to something else\n"
        "-p <passphrase>  Set the passphrase\n"
        "-e <message>     Encode a message: plaintext to ciphertext\n"
        "-d <message>     Decode a message: ciphertext to plaintext\n"
        "-h               Help.  Show this screen and exit\n"
        "\n", prog, opts);

    quit(1);
}

//------------------------------------------------------------------------|
static void parse(int argc, char *argv[])
{
    const char * opts = "vqn:p:e:d:h";
    int option;
    extern char * optarg;

    // Show help and exit if no arguments
    if (argc < 2)
    {
        help(argv[0], opts);
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

            case 'e':
                // Encode a message
                pf.encode = true;
                pf.message = optarg;
                break;

            case 'd':
                // Decode a message
                pf.encode = false;
                pf.message = optarg;
                break;

            case 'h':
            default:
                help(argv[0], opts);
                break;
        }
    }

    // Require at least a passphrase and a message
    if (pf.passphrase == NULL)
    {
        printf("ERROR: No passphrase was given\n");
        help(argv[0], opts);
    }

    if (pf.message == NULL)
    {
        printf("ERROR: No message was given\n");
        help(argv[0], opts);
    }
}

// remove duplicate chars (passphrase only)
// insert nonces between repeated chars (message only - encrypt) 

// remove non-alpha/whitespace chars (passphrase and message)
// uppercase all letters (passphrase and message)
// remove ommitted letter and substitue with mapto (passphrase and message)


//------------------------------------------------------------------------|
// Common filter for passphrase or message prior to encode or decode.
// This removes non-alpha characters, forces all to uppercase, and removes
// the ommitted letter, optionally substituting with the mapped letter.
// This operates on the string in-place assuming we are using memory given
// to this process by the environment (the command line itself) 
static bool filter(char * str)
{
    size_t i = 0;
    size_t j = 0;

    if(pf.verbose)
    {
        printf("raw:      \'%s\'\n", str);
    }

    while(str[i] != '\0')
    {
        printf("%c\t", str[i]);

        j = 0;
        while(str[j] != '\0')
        {
            printf("%c ", str[j]);

            j++;
        }
        printf("\n");

        i++;
    }
    printf("\n");

    if(pf.verbose)
    {
        printf("filtered: \'%s\'\n", str);
    }    

    return true;
}

//------------------------------------------------------------------------|
static void setup()
{
    // Prepare the passphrase
    if (!filter(pf.passphrase))
    {
        printf("ERROR: Filter passphrase failed\n");
        quit(2);
    }

    // Prepare the message
    if (!filter(pf.message))
    {
        printf("ERROR: Filter message failed\n");
        quit(3);
    }

    // populate keyblock
       

}

//------------------------------------------------------------------------|
static void cipher()
{
    if (pf.encode)
    {

    }
    else
    {

    }
}

//------------------------------------------------------------------------|
int main(int argc, char * argv[])
{
    init();
    parse(argc, argv);

    setup();
    cipher();

    quit(0);
    return 0;
}
