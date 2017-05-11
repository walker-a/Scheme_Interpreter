#include <stdio.h>
#include "interpreter.h"
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"

void evalationError() {
    texit(0);
}

Value *lookUpSymbol(Value *expr, Frame *frame) {

    return expr;
}

void interpret(Value *tree) {
    
}

Value *evalIf(Value *expr, Frame *frame) {
    Value *tExpr;
    Value *fExpr;
    if (cdr(expr)->type == CONS_TYPE) {
        tExpr = car(cdr(expr));
        if (cdr(expr)->type == CONS_TYPE) {
            fExpr = car(cdr(cdr(expr)));
        }
        else () {
            handleError();
        }
    }
    else {
        handleError();
    }
    if (car(expr)->i) {
        return eval(tExpr, frame);
    }
    else {
        return (fExpr, frame);
    }
}

Value *evalLet(Value *expr, Frame *frame) {
    return expr;
}

Value *eval(Value *expr, Frame *frame) {
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
     case STRING_TYPE: {
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
            result = evalIf(args,frame);
        }

        // .. other special forms here...
        
        if (!strcmp(first->s, "let")) {
            result = evalLet(args,frame);
        }

        else {
           // not a recognized special form
           evalationError();
        }
        break;
     }

//      ....
    }    
//    ....
}