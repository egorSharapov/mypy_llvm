#ifndef FILE_ANALIZE_HPP_INCLUDED
#define FILE_ANALIZE_HPP_INCLUDED

#include <stdio.h>

typedef struct String
{
    const char *point; // pointer to a start of string
    long long int len;    // len of string
} String;


typedef struct Text
{
    char *source;     // pointers massive
    size_t number_of_symbols;    // count of symbols in source file
    size_t number_of_strings;    // count of strings in source file
    String *string;
} Text;


enum ERRORS_CODE 
{
    SUCCESS         = 0, // succes
    FALL            = 1, // abnormal program termination
    ERRORS          = 2, // typical error
    INPUT_ERROR     = 3, // incorrect comand line input
    OPEN_FILE_ERROR = 4, // file cannot open
    NO_MEM_ERROR    = 5, // no mem for calloc 
};


void create_pointers  (Text * text);

void counting_strings (Text * text, const char str_end);
enum ERRORS_CODE count_and_read (int argc, const char *argv[], Text * text, const char str_end);

#endif