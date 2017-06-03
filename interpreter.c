// by shiny-morning (Adam Klein, Kerim Celik, and Alex Walker)
// for Programming Languages; 06/05/2017

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "interpreter.h"
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"

// prints error message and exits
void handleInterpError(int i) {
    printf("An error occurred during interpretation at: %i\n", i);
    texit(0);
}

Value *makeVoid() {
    Value *value = makeNull();
    value->type = VOID_TYPE;
    return value;
}

Value *makeClosure(Value *params, Value *fxnCode, Frame *fram){
    Value *value = makeNull();
    value->type = CLOSURE_TYPE;
    if (!(params) || !(fxnCode) || !(fram)){
        handleInterpError(1);
    }
    value->cl.paramNames = params;
    value->cl.functionCode = fxnCode;
    value->cl.frame = fram;
    return value;
}

Value *makeTrue() {
    Value *t = makeNull();
    t->type = BOOL_TYPE;
    t->i = 1;
    return t;
}

Value *makeFalse() {
    Value *t = makeNull();
    t->type = BOOL_TYPE;
    t->i = 0;
    return t;
}

// creates the starting frame in the interpret function and initializes
// parent to null
// only meant to be used once
Frame *makeFirstFrame() {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = NULL;
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
// list of bindings, which is the parameter 'bindingList'
// returns the new list of bindings
Value *addBinding(Value *symbol, Value *result, Value *bindingList) {
    Value *cell1 = cons(symbol, result);
    Value *cell2 = cons(cell1, bindingList);
    return cell2;
}

// prints a value, provided that it is an int, double, boolean, string,
// or symbol
void printVal(Value *val) {
    switch (val->type) {
     case VOID_TYPE: {
        break;
     }
     case CLOSURE_TYPE: {
        printf("#<procedure>");
        break;
    }
     case INT_TYPE: {
        printf("%i", val->i);
        break;
     }
     case DOUBLE_TYPE: {
        printf("%f", val->d);
        break;
     }
     case BOOL_TYPE: {
        if (val->i == 1) {
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
        while (val->type == CONS_TYPE) {
            printVal(car(val));
            //adds a space before all but the first item
            if (cdr(val)->type == CONS_TYPE) {
                printf(" ");
            }
            val = cdr(val);
        }
        if (val->type != NULL_TYPE) {
            printf(" . ");
            printVal(val);
        }
        printf(")");
        break;
     }
     case NULL_TYPE: {
        printf("()");
        break;
     }
     default: {
        //otherwise throw an error
        handleInterpError(3);
     }
    }
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
    printVal(expr);
    printf("\n");
    handleInterpError(2); //couldnt find symbol
    return NULL;
}

// checks if a symbol is assigned to a primitive or not
int isPrimitive(Value *symbol, Frame *frame) {
    assert(symbol); assert(symbol->type == SYMBOL_TYPE);
    while (frame->parent != NULL) {
        frame = frame->parent;
    }
    Value *bindings = frame->bindings;
    while (bindings->type != NULL_TYPE) {
        if (!strcmp(car(car(bindings))->s, symbol->s)) {
            if (cdr(car(bindings))->type == PRIMITIVE_TYPE) {
                return 1;
            }
            else {
                return 0;
            }
        }
        bindings = cdr(bindings);
    }
    return 0;
}

// binds a primitive symbols to its C code
void bindPrim(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *symbol = talloc(sizeof(Value));
    symbol->type = STR_TYPE;
    symbol->s = name;
    Value *cell1 = cons(symbol, value);
    Value *cell2 = cons(cell1, frame->bindings);
    frame->bindings = cell2;
}

/*** PRIMITIVE FUNCTION CODE ***/

Value *primitiveAdd(Value *args) {
    if (!(args) || args->type != CONS_TYPE) {
        if (args->type == NULL_TYPE) {
            args->type = DOUBLE_TYPE;
            args->d = 0;
            return args;
        }
        else {
            handleInterpError(7);
        }
    }
    else {
        Value *current = args;
        Value *sum = makeNull();
        sum->type = DOUBLE_TYPE;
        sum->d = 0.0;
        while (current->type != NULL_TYPE) {
            if (current->type != CONS_TYPE) {
                handleInterpError(8);
            }
            if (car(current)->type == INT_TYPE) {
                sum->d += car(current)->i;
            }
            else if (car(current)->type == DOUBLE_TYPE) {
                sum->d += car(current)->d;
            }
            else {
                handleInterpError(9);
            }
            current = cdr(current);
        }
        return sum;
    }
    return makeNull();
}

Value *primitiveNull(Value *args) {
    if (!args || !(args->type == CONS_TYPE) || !(cdr(args)->type == NULL_TYPE)) {
        handleInterpError(10);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    if (car(args)->type == NULL_TYPE) {
        ret->i = 1;
    }
    else {
        ret->i = 0;
    }
    return ret;
}

Value *primitiveCar(Value *args) {
    // are all cases problems?
    if (!args ||
        args->type != CONS_TYPE ||
        !(car(args)) || 
        car(args)->type != CONS_TYPE ||
        !(car(car(args))) ||
        car(car(args))->type == NULL_TYPE) {
        
        handleInterpError(11);
    }
    return car(car(args));
}

Value *primitiveCdr(Value *args) {
    if (!args || !(args->type == CONS_TYPE)) {
        handleInterpError(12);
    }
    Value *temp = cdr(car(args));
    return cdr(car(args));
}

Value *primitiveCons(Value *args) {
    if (!args || !(args->type == CONS_TYPE)) {
        handleInterpError(13);
    }
    if (length(args) != 2) {
        handleInterpError(14);
    }
    Value *c = car(args);
    Value *cd = car(cdr(args));
    return cons(c, cd);
}

Value *primitiveSub(Value *args) {
    if (!(args) || args->type != CONS_TYPE) {
        if (args->type == NULL_TYPE) {
            args->type = DOUBLE_TYPE;
            args->d = 0.0;
            return args;
        }
        else {
            handleInterpError(15);
        }
    }
    else {
        Value *current = args;
        Value *sum = makeNull();
        sum->type = DOUBLE_TYPE;
        sum->d = 0.0;
        // if more than one argument
        if (cdr(current)->type != NULL_TYPE) {
            if (current->type != CONS_TYPE) {
                handleInterpError(16);
            }
            if (car(current)->type == INT_TYPE) {
                sum->d += car(current)->i;
            }
            else if (car(current)->type == DOUBLE_TYPE) {
                sum->d += car(current)->d;
            }
            else {
                handleInterpError(17);
            }
            current = cdr(current);
        }
        while (current->type != NULL_TYPE) {
            if (current->type != CONS_TYPE) {
                handleInterpError(18);
            }
            if (car(current)->type == INT_TYPE) {
                sum->d -= car(current)->i;
            }
            else if (car(current)->type == DOUBLE_TYPE) {
                sum->d -= car(current)->d;
            }
            else {
                handleInterpError(19);
            }
            current = cdr(current);
        }
        return sum;
    }
    return makeNull();
}

Value *primitiveMult(Value *args) {
    if (!(args) || args->type != CONS_TYPE) {
        if (args->type == NULL_TYPE) {
            args->type = DOUBLE_TYPE;
            args->d = 1.0;
            return args;
        }
        else {
            handleInterpError(20);
        }
    }
    else {
        Value *current = args;
        Value *sum = makeNull();
        sum->type = DOUBLE_TYPE;
        sum->d = 1.0;
        while (current->type != NULL_TYPE) {
            if (current->type != CONS_TYPE) {
                handleInterpError(21);
            }
            if (car(current)->type == INT_TYPE) {
                sum->d *= car(current)->i;
            }
            else if (car(current)->type == DOUBLE_TYPE) {
                sum->d *= car(current)->d;
            }
            else {
                handleInterpError(22);
            }
            current = cdr(current);
        }
        return sum;
    }
    return makeNull();
}

Value *primitiveDiv(Value *args) {
    // errors if no arguments passed
    if (!(args) || args->type != CONS_TYPE) {
            handleInterpError(23);
    }
    else {
        Value *current = cdr(args);
        Value *sum = makeNull();
        sum->type = DOUBLE_TYPE;
        if (car(args)->type == INT_TYPE) {
            sum->d = (double)car(args)->i;
        }
        else if (car(args)->type == DOUBLE_TYPE) {
            sum->d = car(args)->d;
        }
        else {
            handleInterpError(24);
        }
        while (current->type != NULL_TYPE) {
            if (current->type != CONS_TYPE) {
                handleInterpError(24);
            }
            if (car(current)->type == INT_TYPE) {
                sum->d *= 1 / (double)(car(current)->i);
            }
            else if (car(current)->type == DOUBLE_TYPE) {
                sum->d *= 1 / (car(current)->d);
            }
            else {
                handleInterpError(25);
            }
            current = cdr(current);
        }
        return sum;
    }
    return makeNull();
}

Value *primitiveMod(Value *args) {
    if (length(args) != 2) {
        handleInterpError(26);
    }
    Value *val1 = car(args);
    Value *val2 = car(cdr(args));
    int num1;
    int num2;
    
    if (val1 == INT_TYPE) {
        num1 = val1->i;
    }
    else if (val1->type == DOUBLE_TYPE) {
        if (val1->d - floor(val1->d) == 0) {
            num1 = val1->d;
        }
        else {
            handleInterpError(27);
        }
    }
    else {
        handleInterpError(28);
    }
    
    if (val2->type == INT_TYPE) {
        num2 = val2->i;
    }
    else if (val2->type == DOUBLE_TYPE) {
        if (val2->d - floor(val2->d) == 0) {
            num2 = car(val2)->d;
        }
        else {
            handleInterpError(29);
        }
    }
    else {
        handleInterpError(30);
    }

    Value *result = makeNull();
    result->type = INT_TYPE;
    result->i = num1 % num2;
    return result;
}

Value *primitiveLess(Value *args) {
    if (length(args) < 2) {
        handleInterpError(31);
    }
    Value *first = car(args);
    double num1;
    if (first->type == INT_TYPE) {
        num1 = first->i;
    }
    else if (first->type == DOUBLE_TYPE) {
        num1 = first->d;
    }
    else {
        handleInterpError(32);
    }
    
    Value *comps = cdr(args);
    Value *num;
    double compNum;
    int boolean = 1;
    for (int i = 0; i < length(cdr(args)); i++) {
        num = car(comps);
        if (num->type == INT_TYPE) {
            compNum = num->i;
        }
        else if (num->type == DOUBLE_TYPE) {
            compNum = num->d;
        }
        else {
            handleInterpError(33);
        }
        if (num1 >= compNum) {
            boolean = 0;
        }
        comps = cdr(comps);
    }
        
    if (boolean == 0) {
        return makeFalse();
    }
    else {
        return makeTrue();
    }
}

Value *primitiveGreater(Value *args) {
    if (length(args) < 2) {
        handleInterpError(34);
    }
    Value *first = car(args);
    double num1;
    if (first->type == INT_TYPE) {
        num1 = first->i;
    }
    else if (first->type == DOUBLE_TYPE) {
        num1 = first->d;
    }
    else {
        handleInterpError(35);
    }
    
    Value *comps = cdr(args);
    Value *num;
    double compNum;
    int boolean = 1;
    for (int i = 0; i < length(cdr(args)); i++) {
        num = car(comps);
        if (num->type == INT_TYPE) {
            compNum = num->i;
        }
        else if (num->type == DOUBLE_TYPE) {
            compNum = num->d;
        }
        else {
            handleInterpError(36);
        }
        if (num1 <= compNum) {
            boolean = 0;
        }
        comps = cdr(comps);
    }
        
    if (boolean == 0) {
        return makeFalse();
    }
    else {
        return makeTrue();
    }
}

Value *primitiveEqual(Value *args) {
    if (length(args) < 2) {
        handleInterpError(37);
    }
    Value *first = car(args);
    double num1;
    if (first->type == INT_TYPE) {
        num1 = first->i;
    }
    else if (first->type == DOUBLE_TYPE) {
        num1 = first->d;
    }
    else {
        handleInterpError(38);
    }
    
    Value *comps = cdr(args);
    Value *num;
    double compNum;
    int boolean = 1;
    for (int i = 0; i < length(cdr(args)); i++) {
        num = car(comps);
        if (num->type == INT_TYPE) {
            compNum = num->i;
        }
        else if (num->type == DOUBLE_TYPE) {
            compNum = num->d;
        }
        else {
            handleInterpError(39);
        }
        if (num1 != compNum) {
            boolean = 0;
        }
        comps = cdr(comps);
    }
        
    if (boolean == 0) {
        return makeFalse();
    }
    else {
        return makeTrue();
    }
}

Value *primitiveLessEq(Value *args) {
    if (length(args) < 2) {
        handleInterpError(40);
    }
    Value *first = car(args);
    double num1;
    if (first->type == INT_TYPE) {
        num1 = first->i;
    }
    else if (first->type == DOUBLE_TYPE) {
        num1 = first->d;
    }
    else {
        handleInterpError(41);
    }
    
    Value *comps = cdr(args);
    Value *num;
    double compNum;
    int boolean = 1;
    for (int i = 0; i < length(cdr(args)); i++) {
        num = car(comps);
        if (num->type == INT_TYPE) {
            compNum = num->i;
        }
        else if (num->type == DOUBLE_TYPE) {
            compNum = num->d;
        }
        else {
            handleInterpError(42);
        }
        if (num1 > compNum) {
            boolean = 0;
        }
        comps = cdr(comps);
    }
        
    if (boolean == 0) {
        return makeFalse();
    }
    else {
        return makeTrue();
    }
}

Value *primitiveGrEq(Value *args) {
    if (length(args) < 2) {
        handleInterpError(43);
    }
    Value *first = car(args);
    double num1;
    if (first->type == INT_TYPE) {
        num1 = first->i;
    }
    else if (first->type == DOUBLE_TYPE) {
        num1 = first->d;
    }
    else {
        handleInterpError(44);
    }
    
    Value *comps = cdr(args);
    Value *num;
    double compNum;
    int boolean = 1;
    for (int i = 0; i < length(cdr(args)); i++) {
        num = car(comps);
        if (num->type == INT_TYPE) {
            compNum = num->i;
        }
        else if (num->type == DOUBLE_TYPE) {
            compNum = num->d;
        }
        else {
            handleInterpError(45);
        }
        if (num1 < compNum) {
            boolean = 0;
        }
        comps = cdr(comps);
    }
        
    if (boolean == 0) {
        return makeFalse();
    }
    else {
        return makeTrue();
    }
}

/*** EVALUATION CODE ***/

// interprets scheme tree as code
void interpret(Value *tree) {
    Frame *newFrame = makeFirstFrame();
    bindPrim("+", primitiveAdd, newFrame);
    bindPrim("null?", primitiveNull, newFrame);
    bindPrim("car", primitiveCar, newFrame);
    bindPrim("cdr", primitiveCdr, newFrame);
    bindPrim("cons", primitiveCons, newFrame);
    bindPrim("*", primitiveMult, newFrame);
    bindPrim("-", primitiveSub, newFrame);
    bindPrim("/", primitiveDiv, newFrame);
    bindPrim("modulo", primitiveMod, newFrame);
    bindPrim("<", primitiveLess, newFrame);
    bindPrim(">", primitiveGreater, newFrame);
    bindPrim("<=", primitiveLessEq, newFrame);
    bindPrim(">=", primitiveGrEq, newFrame);
    bindPrim("=", primitiveEqual, newFrame);
    
    while (tree->type != NULL_TYPE) {
        Value *val = eval(car(tree), newFrame);
        printVal(val);
        if (val->type != VOID_TYPE) {
            printf("\n");
        }
        tree = cdr(tree);
    }
}

// evaluates an if statement based on the arguments 'expr'
Value *evalIf(Value *expr, Frame *frame) {
    Value *tExpr;
    Value *fExpr;
    if (expr->type != CONS_TYPE) {
        handleInterpError(148);
    }
    if (cdr(expr)->type == CONS_TYPE) {
        tExpr = car(cdr(expr));
        if (cdr(cdr(expr))->type == CONS_TYPE) {
            if(cdr(cdr(cdr(expr)))->type != NULL_TYPE) {
                handleInterpError(149);
            }
            fExpr = car(cdr(cdr(expr)));
        }
        else {
            handleInterpError(150);
        }
    }
    else {
        handleInterpError(151);
    }
    Value *check = eval(car(expr), frame);
    if (check->i) {
        return eval(tExpr, frame);
    }
    else {
        return eval(fExpr, frame);
    }
}

int inFrame(Value *symbol, Frame *frame) {
    Value *temp = frame->bindings;
    while (temp->type == CONS_TYPE) {
        if (!strcmp(car(car(temp))->s, symbol->s)) {
            return 1;
        }
        temp = cdr(temp);
    }
    return 0;
}

Value *evalLetrec(Value *expr, Frame *frame) {
    
    if (expr == NULL || expr->type != CONS_TYPE ||
        car(expr)->type != CONS_TYPE || cdr(expr)->type == NULL_TYPE) {
        handleInterpError(152);
    }
    
    Frame *newFrame = makeNewFrame(frame);
    
    Value *assignList = car(expr);
    while (assignList->type != NULL_TYPE) {
        // error checking for assignList
        if (assignList->type != CONS_TYPE) {
            handleInterpError(153);
        }
        Value *assign = car(assignList); 
        
        // error checking for assign
        if (assign->type != CONS_TYPE) {
            handleInterpError(154);
        }
        else if (cdr(assign)->type != CONS_TYPE) {
            handleInterpError(155);
        }
        else if (cdr(cdr(assign))->type != NULL_TYPE) {
            handleInterpError(156);
        }
        else if (car(assign)->type != SYMBOL_TYPE) {
            handleInterpError(157);
        }
        
        Value *symbol = car(assign);
        if (inFrame(symbol, newFrame)) {
            handleInterpError(158);
        }

        
        Value *newBind = car(cdr(assign));
        newFrame->bindings = addBinding(symbol, newBind, newFrame->bindings);
        assignList = cdr(assignList);
    }
    assignList = newFrame->bindings;
    while (assignList->type != NULL_TYPE) {
        assignList->c.car = cons(car(car(assignList)),   
                                 eval(cdr(car(assignList)), newFrame));
        assignList = cdr(assignList);
    }
    
    Value *result;
    Value *cur = (cdr(expr));
    while (cur->type != NULL_TYPE){
        result = eval(car(cur), newFrame);
        cur = cdr(cur);
    }
    return result;
}

//evaluates a let expression in scheme code
Value *evalLet(Value *expr, Frame *frame, int star) {
    if (expr == NULL || expr->type != CONS_TYPE ||
        car(expr)->type != CONS_TYPE || cdr(expr)->type == NULL_TYPE) {
        handleInterpError(159);
    }
    
    Frame *newFrame = makeNewFrame(frame);
    
    Value *assignList = car(expr);
    while (assignList->type != NULL_TYPE) {
        // error checking for assignList
        if (assignList->type != CONS_TYPE) {
            handleInterpError(160);
        }
        Value *assign = car(assignList); 
        
        // error checking for assign
        if (assign->type != CONS_TYPE) {
            handleInterpError(161);
        }
        else if (cdr(assign)->type != CONS_TYPE) {
            handleInterpError(162);
        }
        else if (cdr(cdr(assign))->type != NULL_TYPE) {
            handleInterpError(163);
        }
        else if (car(assign)->type != SYMBOL_TYPE) {
            handleInterpError(164);
        }
        
        Value *symbol = car(assign);
        if (inFrame(symbol, newFrame)) {
            handleInterpError(165);
        }

        Value *newBind;
        //difference between let and let* is if it evals in frame or newFrame
        if (star) {
            newBind = eval(car(cdr(assign)), newFrame);
        }
        else {
            newBind = eval(car(cdr(assign)), frame);
        }
        newFrame->bindings = addBinding(symbol, newBind, newFrame->bindings);
        assignList = cdr(assignList);
    }
    Value *result;
    Value *cur = (cdr(expr));
    while (cur->type != NULL_TYPE){
        result = eval(car(cur), newFrame);
        cur = cdr(cur);
    }
    return result;
    //return eval(car(cdr(expr)), newFrame);
}

// evaluates a quote expression in scheme code
Value *evalQuote(Value *expr, Frame *frame) {
    if (expr->type != CONS_TYPE || car(expr) == NULL || cdr(expr) == NULL
        || cdr(expr)->type != NULL_TYPE) {
        handleInterpError(166);
    }
    if (car(expr)->type == CONS_TYPE && car(car(expr))->type == NULL_TYPE) {
        return car(car(expr));
    }
    return car(expr);
}

Value *evalDefine(Value *expr, Frame *frame) {
    if (length(expr) != 2) {
        handleInterpError(167);
    }
    if (car(expr) == NULL || car(cdr(expr)) == NULL) {
        handleInterpError(168);
    }
    if (car(expr)->type != SYMBOL_TYPE) {
        handleInterpError(169);
    }
    
    Value *result = eval(car(cdr(expr)), frame);
    
    frame->bindings = addBinding(car(expr), result, frame->bindings);
    return makeVoid();
}

Value *evalSetBang(Value *expr, Frame *frame) {
    if (length(expr) != 2) {
        handleInterpError(170);
    }  
    if (car(expr) == NULL || car(cdr(expr)) == NULL) {
        handleInterpError(171);
    }
    
    Value *result = eval(car(cdr(expr)), frame);
    Value *var = car(expr);
    
    Frame *tempFrame = frame;
    while (tempFrame != NULL) {
        Value *temp = tempFrame->bindings;
        while (temp->type == CONS_TYPE) {
            if (!strcmp(car(car(temp))->s, var->s)) {
                car(temp)->c.cdr = result;
                return makeVoid();
            }
            temp = cdr(temp);
        }
        tempFrame = tempFrame->parent;
    }
    handleInterpError(172);
    return makeVoid();
}

Value *evalBegin(Value *expr, Frame *frame) {
    Value *result = makeVoid();
    Value *temp = expr;
    for (int i = 0; i < length(expr); i++) {
        result = eval(car(temp), frame);
        temp = cdr(temp);
    }
    return result;
}

Value *evalLambda(Value *expr, Frame *frame) {
    Value *current = car(expr);
    if (current->type == CONS_TYPE && car(current)->type == NULL_TYPE) {
        current = cdr(current);
    }
    while (current->type != NULL_TYPE) {
        if (car(current)->type != SYMBOL_TYPE) {
            handleInterpError(174);
        }
        current = cdr(current);
    }
    Value *closure = makeClosure(car(expr), cdr(expr), frame);
    return closure;
}

Value *evalAnd(Value *args, Frame *frame) {
    Value *ret;
    if (!args) {
        handleInterpError(178);
    }
    // case: 0 args
    if (length(args) == 0) {
        ret = makeNull();
        ret->type = BOOL_TYPE;
        ret->i = 1;
        return ret;
    }
    Value *temp = args;
    Value *arg;
    for (int i = 0; i < length(args); i++) {
        arg = eval(car(temp), frame);
        if (arg->type == BOOL_TYPE && arg->i == 0) {
            return arg;
        }
        if (i == (length(args) - 1)) {
            ret = car(temp);
        }
        temp = cdr(temp);
    }
    return eval(ret, frame);
}

Value *evalOr(Value *args, Frame *frame) {
    Value *ret;
    if (!args) {
        handleInterpError(179);
    }
    // case: 0 args
    if (length(args) == 0) {
        ret = makeNull();
        ret->type = BOOL_TYPE;
        ret->i = 0;
        return ret;
    }
    Value *temp = args;
    Value *arg;
    for (int i = 0; i < length(args); i++) {
        arg = eval(car(temp), frame);
        if (arg->type != BOOL_TYPE) {
            ret = arg;
            return arg;
        }
        else if (arg->i == 1) {
            return arg;
        }
        ret = car(temp);
        temp = cdr(temp);
    }
    return eval(ret, frame);
}

Value *evalCond(Value *args, Frame *frame) {
    if (!args) {
        handleInterpError(180);
    }
    if (length(args) == 0) {
        return makeVoid();
    }
    Value *current = args;
    while (current->type != NULL_TYPE) {
        if (!car(current) || length(car(current)) == 0){
            handleInterpError(181);
        }
        if (length(car(current)) == 1) {
            if (car(car(current))->type == SYMBOL_TYPE) {
                if (!strcmp(car(car(current))->s, "else")) {
                    handleInterpError(182);
                }
            }
            Value *check = eval(car(car(current)), frame);
            if (check->type != BOOL_TYPE) {
                return eval(car(car(current)), frame);
            }
            if (check->type == BOOL_TYPE) {
                if (check->i == 1) {
                    return eval(car(car(current)), frame);
                }
            }
        }
        else {
            if (car(car(current))->type == SYMBOL_TYPE) {
                if (!strcmp(car(car(current))->s, "else")) {
                    Value *ret = cdr(car(current));
                    while (cdr(ret)->type != NULL_TYPE) {
                        ret = cdr(ret);
                    }
                    return eval(car(ret), frame);
                }
            }
            Value *check = eval(car(car(current)), frame);
            if (check->type != BOOL_TYPE) {
                Value *ret = cdr(car(current));
                    while (cdr(ret)->type != NULL_TYPE) {
                        ret = cdr(ret);
                    }
                    return eval(car(ret), frame);
            }
            if (check->type == BOOL_TYPE) {
                if (check->i == 1) {
                    Value *ret = cdr(car(current));
                    while (cdr(ret)->type != NULL_TYPE) {
                        ret = cdr(ret);
                    }
                    return eval(car(ret), frame);
                }
            }
        }
        current = cdr(current);
    }
    return makeVoid();
}

Value *evalEach(Value *expr, Frame *frame) {
    if (expr->type == NULL_TYPE) {
        return expr;
    }
    if (expr->type != CONS_TYPE) {
        handleInterpError(175);
    }
    Value *args = makeNull();
    Value *cur = expr;
    while (cur->type != NULL_TYPE) {
        args = cons(eval(car(cur), frame), args);
        cur = cdr(cur);
    }
    
    return reverse(args);
}

Value* evalPrim(Value *symbol, Value *args, Frame *frame) {
    if (!symbol || symbol->type != SYMBOL_TYPE) {
        handleInterpError(176);
    }
    Frame *tempFrame = frame;
    while (tempFrame->parent != NULL) {
        tempFrame = tempFrame->parent;
    }
    Value *bindings = tempFrame->bindings;
    while (bindings->type != NULL_TYPE) {
        if (!strcmp(car(car(bindings))->s, symbol->s)) {
            return (cdr(car(bindings))->pf)(evalEach(args, frame));
        }
        bindings = cdr(bindings);
    }
    handleInterpError(177);
    return NULL;   
}

Value *apply(Value *function, Value *args) {
    if (!(function) || function->type != CLOSURE_TYPE) {
        handleInterpError(4);
    }
    Frame *newFrame = makeNewFrame(function->cl.frame);
    Value *curr = function->cl.paramNames;
    if (car(curr)->type != NULL_TYPE) {
        Value *curr2 = args;
        while (curr->type != NULL_TYPE && curr2->type != NULL_TYPE) {
            newFrame->bindings = addBinding(car(curr), car(curr2), newFrame->bindings);
            curr = cdr(curr); 
            curr2 = cdr(curr2);
        }
        if (length(curr) != length(curr2)) {
            handleInterpError(5);
        }
    }
    else if (length(args) != 0) {
        handleInterpError(6);
    }
    
    // this assumes that the functionCode will be a list of bodies
    Value *evaled = function->cl.functionCode;
    Value *bodies = function->cl.functionCode;
    for (int i = 0; i < length(function->cl.functionCode); i++) {
        evaled = eval(car(bodies), newFrame);
        bodies = cdr(bodies);
    }
    return evaled;
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
         
        else if (first->type != SYMBOL_TYPE && first->type != CONS_TYPE) {
            handleInterpError(182);
        }
        
        // Sanity and error checking on first...
        else if (!strcmp(first->s, "if")) {
            result = evalIf(args, frame);
        }
        
        else if (!strcmp(first->s, "let")) {
            result = evalLet(args, frame, 0);
            //result = evalLet(args, frame, 0);
        }
        else if (!strcmp(first->s, "let*")) {
            result = evalLet(args, frame, 1);
            //result = evalLet(args, frame, 1);
        }
        else if (!strcmp(first->s, "letrec")) {
            result = evalLetrec(args, frame);
        }
        
        else if (!strcmp(first->s, "quote")) {
            result = evalQuote(args, frame);
        }
        
        else if (!strcmp(first->s, "define")) {
            result = evalDefine(args, frame);
        }
        
        else if (!strcmp(first->s, "lambda")) {
            result = evalLambda(args, frame);
        }
         
        else if (!strcmp(first->s, "and")) {
            result = evalAnd(args, frame);
        }
        
        else if (!strcmp(first->s, "or")) {
            result = evalOr(args, frame);
        }
        else if (!strcmp(first->s, "cond")) {
            result = evalCond(args, frame);
        }
         
        else if (!strcmp(first->s, "set!")) {
            result = evalSetBang(args, frame);
        }
         
        else if (!strcmp(first->s, "begin")) {
            result = evalBegin(args, frame);
        }
         
        // symbol is a primitive
        else if (first->type == SYMBOL_TYPE) {
            int check = isPrimitive(first, frame);
            if (check == 1) {
                result = evalPrim(first, args, frame);
            }
            else {
                // not a recognized special form or primitive
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator, evaledArgs);
            }
        }

        else {
            // not a recognized special form or primitive
            Value *evaledOperator = eval(first, frame);
            Value *evaledArgs = evalEach(args, frame);
            return apply(evaledOperator, evaledArgs);
        }
        break;
     }
     default: {
        // otherwise throw an error
        handleInterpError(183);
     }    
    }
    return result;
}