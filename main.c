#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    Value *list = tokenize(stdin);
    //displayTokens(list);
    Value *tree = parse(list);
    //printTree(tree);
    interpret(tree);
    tfree();
    return 0;
}