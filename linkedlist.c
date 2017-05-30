//by shiny-morning (Adam Klein, Kerim Celik, Alex Walker)
//April 26th 2017

#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include <assert.h>

// Create a new NULL_TYPE value node.
Value *makeNull() {
  Value *new = talloc(sizeof(Value));
  new -> type = NULL_TYPE;
  return new;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr) {
  Value *new = talloc(sizeof(Value));
  new->type = CONS_TYPE;
  new->c.car = car;
  new->c.cdr = cdr;
  return new;
}

// Helper function that displays the items of the list
void display2(Value *list) {
  if (list->type == INT_TYPE) {
    printf("%i", list->i);
  }
  else if (list->type == DOUBLE_TYPE){
    printf("%f", list->d);
  }
  else if (list->type == STR_TYPE){
    printf("%s", list->s);
  }
  else if (list->type == NULL_TYPE){
    printf(")");
  }
  else {
    if (list->c.car->type == CONS_TYPE) {
      printf("(");
    }
    display2(list->c.car);
    if (list->c.cdr->type != NULL_TYPE) {
      printf(" ");
    }
    display2(list->c.cdr);
  }
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list) {
  if (list->type == CONS_TYPE || list->type == NULL_TYPE) {
    printf("(");
  }
  display2(list);
  printf("\n");
}

// tallocs a copy of Value *list
//Value *copyValue(Value *list) {
//  Value *copy = talloc(sizeof(Value));
//  copy->type = list->type;
//  if (list->type == INT_TYPE) {
//    copy->i = list->i;
//  }
//  else if (list->type == DOUBLE_TYPE) {
//    copy->d = list->d;
//  }
//  else if (list->type == STR_TYPE) {
//    copy->s = list->s;
//  }
//  else if (list->type == NULL_TYPE) {
//  }
//  else {
//    copy->c.car = copyValue(list->c.car);
//    copy->c.cdr = copyValue(list->c.cdr);
//  }
//  return copy;
//}

// Helps implement reverse
Value *helper(Value *list, Value *pointer) {

  Value *newcons = talloc(sizeof(Value));
  newcons->type = CONS_TYPE;
  newcons->c.car = list->c.car;
  newcons->c.cdr = pointer;

  if (list->c.cdr->type == NULL_TYPE) {
    return newcons;
  }
  else {
    return helper(list->c.cdr, newcons);
  }
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list) {
  if (list->type != CONS_TYPE){
    return list;
  }
  if (list->c.cdr->type == NULL_TYPE) {
      return list;
  }
  Value *first = talloc(sizeof(Value));
  first->type = CONS_TYPE;
  first->c.car = list->c.car;
  first->c.cdr = makeNull();
  return helper(list->c.cdr, first);
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
  assert(list);
  assert(list->type == CONS_TYPE);
  assert(list->c.car);
  return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
  assert(list);
  assert(list->type == CONS_TYPE);
  assert(list->c.cdr);
  return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
  assert(value);
  assert(value->type);
  if (value->type == NULL_TYPE) {
    return true;
  }
  return false;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
  assert(value);
  int leng = 0;
  Value *curr = value;
  while(curr->type == CONS_TYPE){
    leng++;
    curr = curr->c.cdr;
  }
  //assert(curr->type==NULL_TYPE);
  return leng;
}
