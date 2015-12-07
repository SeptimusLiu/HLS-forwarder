//
//  utils.c
//  p2p
//
//  Created by septimus on 15/7/5.
//  Copyright (c) 2015年 SEPTIMUS. All rights reserved.
//

#include "utils.h"

#include <stdio.h>

void insert_substring(char *a, char *b, int position)
{
    char *f, *e;
    int length;
    
    length = strlen(a);

    f = substring(a, 1, position - 1 );
    e = substring(a, position, length-position+1);
    
    strcpy(a, "");
    strcat(a, f);
    free(f);
    strcat(a, b);
    strcat(a, e);
    free(e);
}

char *substring(char *string, int position, int length)
{
    char *pointer;
    int c;
    
    pointer = malloc(length+1);
    
    if( pointer == NULL )
        exit(EXIT_FAILURE);
    
    for( c = 0 ; c < length ; c++ )
        *(pointer+c) = *((string+position-1)+c);
    
    *(pointer+c) = '\0';
    
    return pointer;
}