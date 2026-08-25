#include <assert.h>
#include <stdlib.h>

#include "instructions.h"

static const size_t INITIAL_INSTRUCTIONS_CAPACITY = 64;
static const size_t INITIAL_LABELS_CAPACITY = 16;

void instruction_list_init(instruction_list_t* const list)
{
    assert(list);

    list->instructions = (instruction_t*) calloc(INITIAL_INSTRUCTIONS_CAPACITY,
                                                  sizeof(instruction_t));
    assert(list->instructions);
    list->count = 0;
    list->capacity = INITIAL_INSTRUCTIONS_CAPACITY;
}

void instruction_list_push_back(instruction_list_t* const list, instruction_t instruction)
{
    assert(list);

    if (list->count == list->capacity)
    {
        list->capacity *= 2;
        list->instructions = (instruction_t*) realloc(list->instructions,
                                                      list->capacity * sizeof(instruction_t));
        assert(list->instructions);
    }

    list->instructions[list->count++] = instruction;
}

void instruction_list_destroy(instruction_list_t* const list)
{
    assert(list);

    free(list->instructions);
    list->instructions = nullptr;
    list->count = 0;
    list->capacity = 0;
}

void label_list_init(label_list_t* const list)
{
    assert(list);

    list->labels = (label_t*) calloc(INITIAL_LABELS_CAPACITY, sizeof(label_t));
    assert(list->labels);
    list->count = 0;
    list->capacity = INITIAL_LABELS_CAPACITY;
}

void label_list_push_back(label_list_t* const list, label_t label)
{
    assert(list);

    if (list->count == list->capacity)
    {
        list->capacity *= 2;
        list->labels = (label_t*) realloc(list->labels, list->capacity * sizeof(label_t));
        assert(list->labels);
    }

    list->labels[list->count++] = label;
}

void label_list_destroy(label_list_t* const list)
{
    assert(list);

    free(list->labels);
    list->labels = nullptr;
    list->count = 0;
    list->capacity = 0;
}
