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
#define MESSAGE_SIZE_MAX        2048
#define KEYBLOCK_WIDTH          5
#define KEYBLOCK_HEIGHT         5
#define KEYBLOCK_SIZE           (KEYBLOCK_WIDTH * KEYBLOCK_HEIGHT)

#define MAX(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a > _b ? _a : _b; })

#define MIN(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a < _b ? _a : _b; })

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
    size_t msgsize;

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
    pf.msgsize = 0;
    pf.verbose = false;
    pf.encode = true;
}

//------------------------------------------------------------------------|
static void quit(int error)
{
    // Cleanup...
    if (pf.message != NULL)
    {
        free(pf.message);
        pf.message = NULL;
    }

    exit(error);
}

//------------------------------------------------------------------------|
// Allocate a message buffer based on the command-line message given
static void message(const char * msg)
{
    size_t len = strlen(msg);

    // This could be a problem if encode and decode options are both
    // specified.  The last once given will be used.
    if (pf.message != NULL)
    {
        free(pf.message);
        pf.message = NULL;
    }

    // The cipher message length may be shorter or longer than the
    // original message length for various reasons.  The theoretical
    // worst case scenario is an odd number of repeated characters,
    // resulting in ciphertext twice as long as the original message.
    // add a little padding just to be on the safe side.
    pf.msgsize = MIN(len * 2 + 2, MESSAGE_SIZE_MAX);
    pf.message = (char *) malloc(pf.msgsize);

    if (pf.message == NULL)
    {
        printf("ERROR: Could not allocate message buffer!\n");
        quit(4);
    }

    memset(pf.message, 0, pf.msgsize);
    strncpy(pf.message, msg, len + 1);
}

//------------------------------------------------------------------------|
// Show a help screen and quit the program
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
// Parse command line parameters and setup globals
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
                message(optarg);
                break;

            case 'd':
                // Decode a message
                pf.encode = false;
                message(optarg);
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

//------------------------------------------------------------------------|
// Keep only alphabetic characters, discarding the rest.  This will be
// used for the message and the passphrase.
static void alpha(char * str, size_t len)
{
    size_t i = 0;  // source
    size_t j = 0;  // destination

    while((i < len) && (str[i] != '\0'))
    {
        if (isalpha(str[i]))
        {
            if (i != j)
            {
                str[j] = str[i];
            }

            j++;
        }

        i++;
    }

    str[j] = '\0';
}

//------------------------------------------------------------------------|
// Convert all letters to uppercase.  This will be used to the message
// and the passphrase.
static void upper(char * str, size_t len)
{
    size_t i = 0;
    while((i < len) && (str[i] != '\0'))
    {
        if (isalpha(str[i]))
        {
            str[i] = toupper(str[i]);
        }

        i++;
    }
}

//------------------------------------------------------------------------|
// Remove duplicate alpha characters, keeping only uniquely occuring ones.
// This will be used for the passphrase only.
static void unique(char * str, size_t len)
{
    size_t i = 0;
    size_t j = 0;

    while((i < len) && (str[i] != '\0'))
    {
        j = i + 1;
        while((j < len) && (str[j] != '\0'))
        {

            // Simply mark the char non-alpha then call alpha() later
            if (str[j] == str[i])
            {
                str[j] = ' ';
            }

            j++;
        }

        i++;
    }

    alpha(str, len);
}

// insert nonces between repeated chars (message only - encrypt) 

// remove ommitted letter and substitue with mapto (passphrase and message)


//------------------------------------------------------------------------|
// Common filter for passphrase or message prior to encode or decode.
// This removes non-alpha characters, forces all to uppercase, and removes
// the ommitted letter, optionally substituting with the mapped letter.
// This operates on the string in-place assuming we are using memory given
// to this process by the environment (the command line itself) 
static bool filter(char * str)
{
    //size_t i = 0;
    //size_t j = 0;
    size_t len = strlen(str);
    //size_t slen = 0;

    if(pf.verbose)
    {
        printf("raw:      \'%s\'\n", str);
    }

    alpha(str, len);
    upper(str, len);
    unique(str, len);

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
