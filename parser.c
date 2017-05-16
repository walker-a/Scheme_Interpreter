// Alex Walker, May 7, 2017

#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"

// handles Errors in parser.c
void handleParseError(int i) {
    if (i == 0) {
        printf("Syntax Error: error in stack.\n");
    }
    else if (i == 1) {
        printf("Syntax Error: mismatched parentheses--too many ')'.\n");
    }
    else if (i == 2) {
        printf("Syntax Error: mismatched parentheses--too many '('.\n");
    }
    tfree();
    exit(0);
}

// stack function, tells whether the stack is empty
int empty(Value *stack) {
    if (stack->type == CONS_TYPE) {
        return 0;
    }
    else if (stack->type == NULL_TYPE) {
        return 1;
    }
    else {
        handleParseError(0);
        return 1;
    }
}

// stack function, pushes a token onto the stack, returns stack
Value *push(Value *stack, Value *token) {
    if (token -> type == NULL_TYPE) {
        return stack;
    }
    else {
        Value *newStack = cons(token, stack);
        return newStack;
    }
}

// stack function, returns the top item on the stack
Value *peek(Value *stack) {
    if (empty(stack)) {
        handleParseError(0);
    }
    return car(stack);
}

// stack function, pops top item off the stack and returns it
Value *pop(Value *stack) {
    if (empty(stack)) {
        handleParseError(0);
    }
    Value *popped = car(stack);
    *stack = *cdr(stack);
    return popped;
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    Value *stack = makeNull();
    Value *finalParseTree = makeNull();
    Value *curToken;
    Value *poppedToken;
    Value *newParseTree;
    int depth = 0;  
    
    if (tokens->type != CONS_TYPE) {
        handleParseError(0);
    }
    
    while (tokens->type == CONS_TYPE) {
        curToken = car(tokens);
        if (curToken->type == OPEN_TYPE) {
            depth++;
        }
         
        if (curToken->type == CLOSE_TYPE) {
            depth--;
            newParseTree = makeNull();
            if (empty(stack)) {
                handleParseError(1);
            }
            poppedToken = pop(stack);
            while (poppedToken->type != OPEN_TYPE) {
                if (empty(stack)) {
                    handleParseError(1);
                }
                newParseTree = push(newParseTree, poppedToken);
                poppedToken = pop(stack);
            }
            stack = push(stack, newParseTree);
        }
        else {
            stack = push(stack, curToken);
        }
        tokens = cdr(tokens);
    }
    
    if (depth != 0) {
        handleParseError(2);
    }
    
    while (!empty(stack)) {
        finalParseTree = push(finalParseTree, pop(stack));
    }
    
    return finalParseTree;
}

// Displays the value stored in a given token, provided
// it's not a cons cell
void displayValue(Value *value) {
    if (value->type == CONS_TYPE) {
        handleParseError(0);
    }
    if (value->type == INT_TYPE) {
        printf("%i", value->i);
    }
    else if (value->type == DOUBLE_TYPE) {
        printf("%f", value->d);
    }
    else if (value->type == STR_TYPE) {
        printf("%s", value->s);
    }
    else if (value->type == OPEN_TYPE) {
        printf("%s", value->s);
    }
    else if (value->type == CLOSE_TYPE) {
        printf("%s", value->s);
    }
    else if (value->type == BOOL_TYPE) {
        if (value->i == 1) {
                printf("#t");
            }
        else {
            printf("#f");
        }
    }
    else if (value->type == SYMBOL_TYPE) {
        printf("%s", value->s);
    }
    else {
        handleParseError(0);
    }
}

// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree2(Value *tree) {
    Value *curToken;
    while (!empty(tree)) {
        curToken = car(tree);
        if (curToken->type == CONS_TYPE) {
            printf("(");
            printTree2(curToken);
            printf(")");
            tree = cdr(tree);
            if (!empty(tree)) {
                printf(" ");
            }
        } else {
            displayValue(curToken);
            if (!empty(cdr(tree))) {
                printf(" ");
            }
            tree = cdr(tree);
        }
    }
}

void printTree(Value *tree) {
    printTree2(tree);
    printf("\n");
}