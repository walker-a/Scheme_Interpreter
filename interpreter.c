// by shiny-morning (Adam Klein, Kerim Celik, and Alex Walker)
// for Programming Languages; 05/15/2017

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "interpreter.h"
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"

// prints error message and exits
void handleInterpError() {
    printf("An error occurred during interpretation.\n");
    texit(0);
}

// creates the starting frame in the interpret function and initializes
// parent to null
// only meant to be used once
Frame *makeFirstFrame() {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    return newFrame;
}

// creates a new frame, with its parent as a parameter
Frame *makeNewFrame(Frame *parent) {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = parent;
    return newFrame;
}

// adds a binding consisting of a symbol and result to the current
// list of bindings, which is the parameter 'binding'
// returns the new list of bindings
Value *addBinding(Value *symbol, Value *result, Value *bindingList) {
    Value *cell1 = cons(symbol, result);
    Value *cell2 = cons(cell1, bindingList);
    return cell2;
}

// looks up a symbol in the frame bindings and returns the value
// associated with that symbol
Value *lookUpSymbol(Value *expr, Frame *frame) {
    assert(expr->type == SYMBOL_TYPE);
    assert(frame);
    Value *bindings;
    while(frame != NULL) {
        bindings = frame->bindings;
        while (bindings->type != NULL_TYPE) {
            if (!strcmp(car(car(bindings))->s, expr->s)) {
                expr = cdr(car(bindings));
                return expr;
            }
            bindings = cdr(bindings);
        }
        frame = frame->parent;
    }
    handleInterpError(); //couldnt find symbol
    return NULL;
}

// prints a value, provided that it is an int, double, boolean, string,
// or symbol
void printVal(Value *val) {
    switch (val->type) {
     case INT_TYPE: {
        printf("%i", val->i);
        break;
     }
     case DOUBLE_TYPE: {
        printf("%f", val->d);
        break;
     }
     case BOOL_TYPE: {
        if (val->i == 1){
            printf("#t");
        }
        else {
            printf("#f");
        }
        break;
     }
     case STR_TYPE: {
        printf("%s", val->s);
        break;
     }
     case SYMBOL_TYPE: {
        printf("%s", val->s);
        break;
     }
     case CONS_TYPE: {
        printf("(");
        int firstItem = 1;
        while (val->type != NULL_TYPE) {
            //adds a space before all but the first item
            if (firstItem == 0) {
                printf(" ");
            }
            else {
                firstItem = 0;
            }
            printVal(car(val));
            val = cdr(val);
        }
        printf(")");
        break;
     }
     case NULL_TYPE: {
        break;
     }
     default: {
        //otherwise throw an error
        handleInterpError();
     }
    }
}

// interprets scheme tree as code
void interpret(Value *tree) {
    Frame *newFrame = makeFirstFrame();
    while (tree->type != NULL_TYPE) {
        printVal(eval(car(tree), newFrame));
        printf("\n");
        tree = cdr(tree);
    }
}

// evaluates an if statement based on the arguments 'expr'
Value *evalIf(Value *expr, Frame *frame) {
    Value *tExpr;
    Value *fExpr;
    if (expr->type != CONS_TYPE) {
        handleInterpError();
    }
    if (cdr(expr)->type == CONS_TYPE) {
        tExpr = car(cdr(expr));
        if (cdr(cdr(expr))->type == CONS_TYPE) {
            if(cdr(cdr(cdr(expr)))->type != NULL_TYPE) {
                handleInterpError();
            }
            fExpr = car(cdr(cdr(expr)));
        }
        else {
            handleInterpError();
        }
    }
    else {
        handleInterpError();
    }
    Value *check = eval(car(expr), frame);
    if (check->i) {
        return eval(tExpr, frame);
    }
    else {
        return eval(fExpr, frame);
    }
}

// evaluates a let expression in scheme code
Value *evalLet(Value *expr, Frame *frame) {
    if (expr == NULL || expr->type != CONS_TYPE ||
        car(expr)->type != CONS_TYPE || cdr(cdr(expr))->type != NULL_TYPE) {
        handleInterpError();
    }
    
    Frame *newFrame = makeNewFrame(frame);
    
    Value *assignList = car(expr);
    while (assignList->type != NULL_TYPE) {
        // error checking for assignList
        if (assignList->type != CONS_TYPE) {
            handleInterpError();
        }
        Value *assign = car(assignList); 
        
        // error checking for assign
        if (assign->type != CONS_TYPE) {
            handleInterpError();
        }
        else if (cdr(assign)->type != CONS_TYPE) {
            handleInterpError();
        }
        else if (cdr(cdr(assign))->type != NULL_TYPE) {
            handleInterpError();
        }
        else if (car(assign)->type != SYMBOL_TYPE) {
            handleInterpError();
        }
        
        Value *symbol = car(assign);
        if (car(cdr(assign))->type == CONS_TYPE) {
            if (car(car(cdr(assign))) == NULL) {
                handleInterpError();
            }
            else if (car(car(cdr(assign)))->type != SYMBOL_TYPE) {
                handleInterpError();
            }
        }
        Value *result = eval(car(cdr(assign)), newFrame);
        newFrame->bindings = addBinding(symbol, result, newFrame->bindings);
        assignList = cdr(assignList);
    }

    return eval(car(cdr(expr)), newFrame);
}

// evaluates a quote expression in scheme code
Value *evalQuote(Value *expr, Frame *frame) {
    if (expr->type != CONS_TYPE || car(expr) == NULL || cdr(expr) == NULL
        || cdr(expr)->type != NULL_TYPE) {
        handleInterpError();
    }
    return car(expr);
}

// evaluates an expression in scheme code
Value *eval(Value *expr, Frame *frame) {
    Value *result;
    switch (expr->type) {
     case INT_TYPE: {
        return expr;
        break;
     }
     case DOUBLE_TYPE: {
        return expr;
        break;
     }
     case BOOL_TYPE: {
        return expr;
        break;
     }
     case STR_TYPE: {
        return expr;
        break;
     } 
     case SYMBOL_TYPE: {
        return lookUpSymbol(expr, frame);
        break;
     }
     case CONS_TYPE: {
        Value *first = car(expr);
        Value *args = cdr(expr);

        if (first->type == NULL_TYPE) {
            result = expr;
        }
        
        // Sanity and error checking on first...
        else if (!strcmp(first->s, "if")) {
            result = evalIf(args, frame);
        }
        
        else if (!strcmp(first->s, "let")) {
            result = evalLet(args, frame);
        }
        
        else if (!strcmp(first->s, "quote")) {
            result = evalQuote(args, frame);
        }

        else {
           // not a recognized special form
           handleInterpError();
        }
        break;
     }
     default: {
        // otherwise throw an error
        handleInterpError();
     }    
    }    
    return result;
}