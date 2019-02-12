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
#define MSG_SIZE_MAX       2048
#define KEY_WIDTH          5
#define KEY_HEIGHT         5
#define KEY_SIZE           (KEY_WIDTH * KEY_HEIGHT)

#define MAX(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a > _b ? _a : _b; })

#define MIN(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a < _b ? _a : _b; })

//------------------------------------------------------------------------|
typedef struct
{
    // Playfair must omit 1 letter (Typically J or Q)
    // And optionally map it to another character (Typically J to I)
    // It must also consider a specific character as a nonce (Typically X)
    char omit;
    char mapto;
    char nonce;

    // Pointers to the passphrase and message to encrypt/decrypt
    char * key;
    size_t keysize;
    char * msg;
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
    pf.omit = 'J';
    pf.mapto = 'I';
    pf.nonce = 'X';
    pf.key = NULL;
    pf.keysize = 0;
    pf.msg = NULL;
    pf.msgsize = 0;
    pf.verbose = false;
    pf.encode = true;
}

//------------------------------------------------------------------------|
static void quit(int error)
{
    // Cleanup...
    if (pf.msg != NULL)
    {
        free(pf.msg);
        pf.msg = NULL;
    }

    if (pf.key != NULL)
    {
        free(pf.key);
        pf.key = NULL;
    }

    exit(error);
}

//------------------------------------------------------------------------|
// Allocate a passphrase/key buffer based on the command-line passphrase
// given
static void allockey(const char * key)
{
    size_t len = strlen(key);

    // Avoid double-allocating in case of multiple passphrases provided
    // use the last one given.
    if (pf.key != NULL)
    {
        free(pf.key);
        pf.key = NULL;
    }

    // The passphrase given will most likely shrink from what is provided
    // as duplicate characters are cancelled out.  This buffer is also
    // used later directly as the keyblock, so it cannot go below the
    // keyblock size because it will get populated with the remainder
    // of the alphabet.  Pad the length a little for safety.
    pf.keysize = MAX(KEY_SIZE, len) + 2;
    pf.key = (char *) malloc(pf.keysize);

    if (pf.key == NULL)
    {
        fprintf(stderr, "\nERROR: Could not allocate passphrase/key buffer!\n");
        quit(4);
    }

    memset(pf.key, 0, pf.keysize);
    strncpy(pf.key, key, len + 1);
}

//------------------------------------------------------------------------|
// Allocate a message buffer based on the command-line message given
static void allocmsg(const char * msg)
{
    size_t len = strlen(msg);

    // This could be a problem if encode and decode options are both
    // specified.  The last once given will be used.
    if (pf.msg != NULL)
    {
        free(pf.msg);
        pf.msg = NULL;
    }

    // The cipher message length may be shorter or longer than the
    // original message length for various reasons.  The theoretical
    // worst case scenario is an odd number of repeated characters,
    // resulting in ciphertext twice as long as the original message.
    // add a little padding just to be on the safe side.
    pf.msgsize = MIN(len * 2 + 2, MSG_SIZE_MAX);
    pf.msg = (char *) malloc(pf.msgsize);

    if (pf.msg == NULL)
    {
        fprintf(stderr, "\nERROR: Could not allocate message buffer!\n");
        quit(5);
    }

    memset(pf.msg, 0, pf.msgsize);
    strncpy(pf.msg, msg, len + 1);
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
                // Set the passphrase
                allockey(optarg);
                break;

            case 'e':
                // Encode a message
                pf.encode = true;
                allocmsg(optarg);
                break;

            case 'd':
                // Decode a message
                pf.encode = false;
                allocmsg(optarg);
                break;

            case 'h':
            default:
                help(argv[0], opts);
                break;
        }
    }

    // Require at least a passphrase and a message
    if (pf.key == NULL)
    {
        fprintf(stderr, "\nERROR: No passphrase was given\n");
        help(argv[0], opts);
    }

    if (pf.msg == NULL)
    {
        fprintf(stderr, "\nERROR: No message was given\n");
        help(argv[0], opts);
    }
}

//------------------------------------------------------------------------|
// Keep only alphabetic characters, discarding the rest.  This will be
// used for the message and the passphrase.
static void alpha(char * str)
{
    size_t i = 0;  // source
    size_t j = 0;  // destination

    while (str[i] != '\0')
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
static void upper(char * str)
{
    size_t i = 0;

    while (str[i] != '\0')
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
static void unique(char * str)
{
    size_t i = 0;
    size_t j = 0;

    while (str[i] != '\0')
    {
        j = i + 1;
        while (str[j] != '\0')
        {
            // Simply mark the char non-alpha then call alpha() later.
            // Alternatively we could call memmove() multiple times,
            // but this is much simpler.
            if (str[j] == str[i])
            {
                str[j] = ' ';
            }

            j++;
        }

        i++;
    }

    alpha(str);
}

//------------------------------------------------------------------------|
// Remove ommitted letter and optionally substitue with mapped character.
// This should be done both for the passphrase and the message.
static void mapchar(char * str)
{
    size_t i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == pf.omit)
        {
            // Use the same strategy as unique() here
            str[i] = (pf.mapto != '\0') ? pf.mapto : ' ';
        }

        i++;
    }

    alpha(str);
}

//------------------------------------------------------------------------|
// Insert nonces between repeated characters.  This will be used for the
// message only.
static void nonces(char * str)
{
    size_t len = strlen(str);
    size_t i = 1;

    while (str[i] != '\0')
    {
        // Only need to insert nonces if repeated char is within
        // a pair of letter (i is odd? - check least bit)
        if ((str[i - 1] == str[i]) && (i & 1))
        {
            // memmove is overlap-safe.  move the remainder back
            memmove(str + i + 1, str + i, len - i);
            str[i] = pf.nonce;
            len++; 
        }

        i++;
    }

    // Require an even number of characters by appending a nonce.
    // Check for case of intentional nonces, and refuse to encode
    // Such a message: We could perhaps work around this case
    // programmatically, but here we just leave it to the user
    if (len & 1)
    {
        if (str[len - 1] == pf.nonce)
        {
            fprintf(stderr, "\nERROR: Cannot encode message with badly"
                " intentional nonces\n");
            quit(7);
        }

        str[len++] = pf.nonce;
        str[len] = '\0';
    }
}

//------------------------------------------------------------------------|
// Treating the passphrase/key buffer as the keyblock, fill in the
// remainder of the (restricted) alphabet.
static void fillkey(char * str)
{
    size_t len = strlen(str);
    char letter;
    int i;

    // Ensure that every letter other than the omitted leter is present
    for (letter = 'A'; letter <= 'Z'; letter++)
    {
        // Skip the ommitted letter.
        if (letter == pf.omit)
        {
            continue;
        }

        // Walk through looking for the current letter
        i = 0;
        len = strlen(str);
        while ((i < len) && (str[i] != '\0') && (letter != str[i]))
        {
            i++;
        }

        // If the letter was not found, append it to the end
        if (letter != str[i])
        {
            str[len++] = letter;
            str[len] = '\0';
        }
    }

    // Sanity check that the key block is correctly sized
    if (len != KEY_SIZE)
    {
        fprintf(stderr, "\nERROR: Invalid key block size: %zu\n", len);
        quit(6);
    }
}

//------------------------------------------------------------------------|
static void filterkey(char * str)
{
    size_t i = 0;

    if (pf.verbose)
    {
        printf("%s\n", __FUNCTION__);
        printf("    raw:      \'%s\'\n", str);
    }

    alpha(str);
    upper(str);
    mapchar(str);
    unique(str);
    fillkey(str);

    if (pf.verbose)
    {
        printf("    filtered: \'%s\'\n", str);

        printf("\n");
        while (str[i] != '\0')
        {
            printf("%c%c ", str[i], (str[i] == pf.mapto) ? pf.omit : ' ');

            if (i % KEY_WIDTH == (KEY_WIDTH - 1))
            {
                printf("\n");
            }

            i++;
        }
        printf("\n");
    }
}

//------------------------------------------------------------------------|
static void filtermsg(char * str)
{
    if (pf.verbose)
    {
        printf("%s\n", __FUNCTION__);
        printf("    raw:      \'%s\'\n", str);
    }

    alpha(str);
    upper(str);
    mapchar(str);

    // Should only need to insert nonces for encode
    if (pf.encode)
    {
        nonces(str);
    }

    if (pf.verbose)
    {
        printf("    filtered: \'%s\'\n\n", str);
    }
}

//------------------------------------------------------------------------|
// Given keyblock coordinates, get the letter
static char keyletter(size_t col, size_t row)
{
    return pf.key[KEY_HEIGHT * row + col];
}

//------------------------------------------------------------------------|
// Given a letter, find its keyblock coordinates
static void lookup(char c, size_t * col, size_t * row)
{
    // Hack: If the lookup char is NULL zero, it probably means we were
    // given an odd number of ciphertext characters.  fudge it in with
    // a nonce character instead of throwing an error.
    if (c == '\0')
    {
        c = pf.nonce;
    }

    for (*col = 0; *col < KEY_WIDTH; (*col)++)
    {
        for (*row = 0; *row < KEY_HEIGHT; (*row)++)
        {
            if (keyletter(*col, *row) == c)
            {
                return;
            }
        }
    }

    fprintf(stderr, "\nERROR: lookup(0x%02X) \'%c\' failed\n", c, isprint(c) ? c : ' ');
    quit(8);
}

//------------------------------------------------------------------------|
static void encodepair(char first, char second)
{
    size_t col[2] = { 0, 0 };
    size_t row[2] = { 0, 0 };

    // disallow this on encode, which shouldn't happen anyway since the
    // message has been filtered through nonce().
    if (first == second)
    {
        fprintf(stderr, "\nERROR: %s(%c, %c) Invalid pair\n", __FUNCTION__,
            first, second);
        quit(9);
    }

    lookup(first, &col[0], &row[0]);
    lookup(second, &col[1], &row[1]);

    if (pf.verbose)
    {
        printf("\n%c%c %zu,%zu %zu,%zu -> ", first, second,
            col[0], row[0], col[1], row[1]);
    }

    // Two letters in the same column
    if (col[0] == col[1])
    {
        printf("%c%c", keyletter(col[0], (row[0] + 1) % KEY_HEIGHT),
                       keyletter(col[1], (row[1] + 1) % KEY_HEIGHT));
    }

    // Two letters in the same row
    else if (row[0] == row[1])
    {
        printf("%c%c", keyletter((col[0] + 1) % KEY_WIDTH, row[0]),
                       keyletter((col[1] + 1) % KEY_WIDTH, row[1]));
    }

    // Two letters form the corners of a rectangle
    else
    {
        printf("%c%c", keyletter(col[1], row[0]),
                       keyletter(col[0], row[1]));
    }
}

//------------------------------------------------------------------------|
static void decodepair(char first, char second)
{
    size_t col[2] = { 0, 0 };
    size_t row[2] = { 0, 0 };

    lookup(first, &col[0], &row[0]);
    lookup(second, &col[1], &row[1]);

    if (pf.verbose)
    {
        printf("\n%c%c %zu,%zu %zu,%zu -> ", first, second,
            col[0], row[0], col[1], row[1]);
    }

    // Allow plaintext doublets (identical pair) in decode, since
    // JFK test case contains incorrectly nonced plaintext pair.
    if (first == second)
    {
        printf("%c%c", first, second);
    }

    // Two letters in the same column
    else if (col[0] == col[1])
    {
        // modulo trick is not possible due to potential underflow
        printf("%c%c", keyletter(col[0], (row[0] == 0) ?
                                 (KEY_HEIGHT - 1) : (row[0] - 1)),
                       keyletter(col[1], (row[1] == 0) ?
                                 (KEY_HEIGHT - 1) : (row[1] - 1)));
    }

    // Two letters in the same row
    else if (row[0] == row[1])
    {
        printf("%c%c", keyletter((col[0] == 0) ? (KEY_WIDTH - 1) :
                                 (col[0] - 1), row[0]),
                       keyletter((col[1] == 0) ? (KEY_WIDTH - 1) :
                                 (col[1] - 1), row[1]));
    }

    // Two letters form the corners of a rectangle
    else
    {
        printf("%c%c", keyletter(col[1], row[0]),
                       keyletter(col[0], row[1]));
    }

}

//------------------------------------------------------------------------|
static void encodemsg()
{
    size_t len = strlen(pf.msg);
    size_t i = 0;

    for (i = 0; i < len; i += 2)
    {
        encodepair(pf.msg[i], pf.msg[i + 1]);
    }

    printf("\n");
}

//------------------------------------------------------------------------|
static void decodemsg()
{
    size_t len = strlen(pf.msg);
    size_t i = 0;

    for (i = 0; i < len; i += 2)
    {
        decodepair(pf.msg[i], pf.msg[i + 1]);
    }

    printf("\n");

}

//------------------------------------------------------------------------|
int main(int argc, char * argv[])
{
    init();
    parse(argc, argv);

    filterkey(pf.key);
    filtermsg(pf.msg);

    if (pf.encode)
    {
        encodemsg();
    }
    else
    {
        decodemsg();
    }

    quit(0);
    return 0;
}
