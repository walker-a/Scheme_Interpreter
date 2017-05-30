//by shiny-morning (Adam Klein, Kerim Celik, Alex Walker)
//May 3, 2017

#include "tokenizer.h"
#include "talloc.h"
#include "linkedlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *init = "!$%&*/:<=>?~_^";
char *subs = "!$%&*/:<=>?~_^0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.+-";
char *lett = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *digi = "0123456789";

// adds a character to a string using strcat
void addCharToStr(char *string, char c) {
    char charStr[2] = {c, '\0'};
    strcat(string, charStr);
}

// prints an error message then exits
void handleError(int i) {
    if (i == CONS_TYPE) {
        printf("Problem with linked list.\n");
    } else {
        printf("Syntax error: untokenizable.\n");
    }
    if (i == STR_TYPE) {
        printf("Error in tokenizing string.\n");
    }
    if (i == BOOL_TYPE) {
        printf("Error in tokenizing boolean.\n");
    }
    if (i == SYMBOL_TYPE) {
        printf("Error in tokenizing symbol.\n");
    }
    if (i == OPEN_TYPE) {
        printf("Error in tokenizing open paren.\n");
    }
    if (i == CLOSE_TYPE) {
        printf("Error in tokenizing closed paren.\n");
    }
    if (i == INT_TYPE || i == DOUBLE_TYPE) {
        printf("Error in tokenizing number.\n");
    }
    tfree();
    exit(0);
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize() {
    int canStartNewToken = 1; // this is just a boolean
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);

    // while loop that builds 1 token until end of file
    while (charRead != EOF) {
        
        // skip over whitespace and newline chars
        if (charRead == ' ' || charRead == '\n') {
            canStartNewToken = 1;
            charRead = fgetc(stdin);
        }
        else {           
            //sets up a new node holding a string for the token
            Value *newNode = makeNull();
            newNode->s = talloc(1000 * sizeof(char));
            for (int i = 0; i < 999; i++) {
                newNode->s[i] = '\0';
            }
        
            // accounts for boolean case
            if (charRead == '#' && canStartNewToken) {
                newNode->type = BOOL_TYPE;           
                charRead = fgetc(stdin);

//                if (charRead == 't' || charRead == 'f') {
//                    addCharToStr(newNode->s, charRead);
//                    charRead = fgetc(stdin);
//                }
                
                if (charRead == 't'){
                    newNode->i = 1;
                    charRead = fgetc(stdin);
                }
                else if (charRead == 'f'){
                    newNode->i = 0;
                    charRead = fgetc(stdin);
                }
                else {
                    handleError(BOOL_TYPE);
                }
                
                // stores t/f values as 0/1 in int i of Value
//                if (!strcmp(newNode->s, "#t")) {
//                    newNode->i = 1;
//                }
//                else if (!strcmp(newNode->s, "#f")) {
//                    newNode->i = 0;
//                }
//                else {
//                    handleError(BOOL_TYPE);
//                }

                canStartNewToken = 0;
            }

            // accounts for digit case (float and int)
            else if (canStartNewToken && 
                     (isdigit(charRead) || charRead == '.' || 
                      charRead == '+' || charRead == '-')) {

                if (charRead == '.') {
                    addCharToStr(newNode->s, charRead);
                    newNode->type = DOUBLE_TYPE;
                    charRead = fgetc(stdin);
                    if (!isdigit(charRead)) {
                        handleError(INT_TYPE);
                    }
                }
                else if (charRead == '+' || charRead == '-') {
                    addCharToStr(newNode->s, charRead);
                    charRead = fgetc(stdin);
                    //does next if statement work?
                    if (!isdigit(charRead) && charRead != '.') {
                        newNode->type = SYMBOL_TYPE;
                    }
                    else if (charRead == '.') {
                        newNode->type = DOUBLE_TYPE;
                        addCharToStr(newNode->s, charRead);
                        charRead = fgetc(stdin);
                        if (!isdigit(charRead)) {
                            handleError(INT_TYPE);
                        }
                    }
                    else if (isdigit(charRead)) {
                        newNode->type = INT_TYPE;
                    }
                    else {
                        handleError(BOOL_TYPE);
                    }
                }
                else {
                    newNode->type = INT_TYPE;
                }

                // no matter what at this point, we currently have a number
                // where charRead is a digit that has not been added to
                // the token yet
                while (isdigit(charRead) || (charRead == '.' && 
                                             newNode->type == INT_TYPE)) {
                    if (charRead == '.') {
                        if (newNode->type == INT_TYPE) {
                            newNode->type = DOUBLE_TYPE;
                        }
                        else {
                            handleError(INT_TYPE);
                        }
                    }

                    addCharToStr(newNode->s, charRead);
                    charRead = fgetc(stdin);
                }
                if (newNode->type == INT_TYPE) {
                    newNode->i = atoi(newNode->s);
                }
                else if (newNode->type == DOUBLE_TYPE) {
                    newNode->d = atof(newNode->s);
                }

                canStartNewToken = 0;
            } 

            // accounts for symbol case
            else if (canStartNewToken && (strchr(init, charRead) || 
                                          strchr(lett, charRead))) {
                newNode->type = SYMBOL_TYPE;
                addCharToStr(newNode->s, charRead);
                charRead = fgetc(stdin);
                while (strchr(subs, charRead)) {
                    addCharToStr(newNode->s, charRead);
                    charRead = fgetc(stdin);
                }
                canStartNewToken = 0;
            }

            // accounts for open paren case
            else if (charRead == '(') {
                canStartNewToken = 1;
                newNode->type = OPEN_TYPE;
                addCharToStr(newNode->s, charRead);
                charRead = fgetc(stdin);
            }

            // accounts for closed paren case
            else if (charRead == ')') {
                canStartNewToken = 1;
                newNode->type = CLOSE_TYPE;
                addCharToStr(newNode->s, charRead);
                charRead = fgetc(stdin);
            }
            
            // accounts for string case
            else if (charRead == '"') {
                canStartNewToken = 1;
                newNode->type = STR_TYPE;
                addCharToStr(newNode->s, charRead);
                charRead = fgetc(stdin);
                while (charRead != '"' && charRead != EOF && charRead != '\n') {
                    addCharToStr(newNode->s, charRead);
                    charRead = fgetc(stdin);
                }
                if (charRead != '"') {
                    handleError(STR_TYPE);
                }
                addCharToStr(newNode->s, charRead);
                charRead = fgetc(stdin);
            }

            // accounts for comment case
            else if (charRead == ';'){
                canStartNewToken = 1;
                charRead = fgetc(stdin);
                while (charRead != '\n' && charRead != EOF) {
                    charRead = fgetc(stdin);
                }
                // no error to handle here b/c no close syntax for comments
                // this exits the loop if an EOF is read, you could also ungetc()
                if (charRead  == EOF){
                    break;
                }
            }

            else {
                handleError(-1);
            }

            if (newNode->type != NULL_TYPE) {
                Value *consCell = cons(newNode, list);
                list = consCell;
            }
        }
    }

    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    if (list->type != CONS_TYPE) {
        handleError(CONS_TYPE);
    }
    while (list->type == CONS_TYPE) {
        if (car(list)->type == INT_TYPE) {
            printf("%i", car(list)->i);
            printf(":integer\n");
        }
        else if (car(list)->type == DOUBLE_TYPE) {
            printf("%f", car(list)->d);
            printf(":float\n");
        }
        else if (car(list)->type == STR_TYPE) {
            printf("%s", car(list)->s);
            printf(":string\n");
        }
        else if (car(list)->type == OPEN_TYPE) {
            printf("%s", car(list)->s);
            printf(":open\n");
        }
        else if (car(list)->type == CLOSE_TYPE) {
            printf("%s", car(list)->s);
            printf(":close\n");
        }
        else if (car(list)->type == BOOL_TYPE) {
            if (car(list)->i == 1) {
                printf("#t");
            }
            else {
                printf("#f");
            }
            printf(":boolean\n");
        }
        else if (car(list)->type == SYMBOL_TYPE) {
            printf("%s", car(list)->s);
            printf(":symbol\n");
        }
        else {
            handleError(-1);
        }
        list = list->c.cdr;
    }
}