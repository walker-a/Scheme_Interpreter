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

// Prints error message and exits
void handleInterpError() {
    printf("An error occurred during interpretation.\n");
    texit(0);
}

Frame *makeFirstFrame() {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    return newFrame;
}

Frame *makeNewFrame(Frame *parent) {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = parent;
    return newFrame;
}

Value *addBinding(Value *symbol, Value *result, Value *binding) {
    Value *cell1 = cons(symbol, result);
    Value *cell2 = cons(cell1, binding);
    return cell2;
}

Value *lookUpSymbol(Value *expr, Frame *frame) {
    assert(expr->type == SYMBOL_TYPE);
    assert(frame);
    Value *bindings;
    while(frame != NULL) {
        bindings = frame->bindings;
        while (bindings->type != NULL_TYPE) {
            // might break car assert if frame badly done
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
        printf("symbol?");
        break;
     }
     default: {
        //otherwise throw an error
        handleInterpError();
     }
    }
}

void interpret(Value *tree) {
    Frame *newFrame = makeFirstFrame();
    while (tree->type != NULL_TYPE) {
        printVal(eval(car(tree), newFrame));
        printf("\n");
        tree = cdr(tree);
    }
}

Value *evalIf(Value *expr, Frame *frame) {
    Value *tExpr;
    Value *fExpr;
    if (expr->type != CONS_TYPE) {
        handleInterpError();
    }
    if (cdr(expr)->type == CONS_TYPE) {
        tExpr = car(cdr(expr));
        if (cdr(cdr(expr))->type == CONS_TYPE) {
            if(cdr(cdr(cdr(expr)))->type != NULL_TYPE){
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
    assert(check->type == BOOL_TYPE);
    if (check->i) {
        return eval(tExpr, frame);
    }
    else {
        return eval(fExpr, frame);
    }
}

Value *evalLet(Value *expr, Frame *frame) {
//    if (expr == NULL) {
//        handleInterpError();
//    }
    if (expr == NULL || expr->type != CONS_TYPE ||
        car(expr)->type != CONS_TYPE || cdr(cdr(expr))->type != NULL_TYPE) {
        handleInterpError();
    }
    
    Frame *newFrame = makeNewFrame(frame);
    
    Value *assignList = car(expr);
    while (assignList->type != NULL_TYPE) {
        if (assignList->type != CONS_TYPE) { //error checking for assignList
            handleInterpError();
        }
        Value *assign = car(assignList); 
        
        //error checking for assign
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

        // Sanity and error checking on first...
        if (!strcmp(first->s, "if")) {
            result = evalIf(args, frame);
        }

        // .. other special forms here...
        
        else if (!strcmp(first->s, "let")) {
            result = evalLet(args, frame);
        }

        else {
           // not a recognized special form
           handleInterpError();
        }
        break;
     }
     default: {
        //otherwise throw an error
        handleInterpError();
     }    
    }    
    return result;
}