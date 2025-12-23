#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    META_SUCCESS,
    META_UNKNOWN_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNKNOWN_COMMAND
} PrepareCommandResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct 
{
    StatementType type;
} Statement;

typedef struct 
{
    char* buffer;
    size_t buffer_length; //max buffer length
    ssize_t input_length; //current input length
} InputBuffer;

InputBuffer* create_new_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length=0;
    input_buffer->input_length=0;
    return input_buffer;
}

void clear_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer){
    if(strcmp(input_buffer->buffer, ".exit") == 0){
        clear_buffer(input_buffer);
        printf("Quitting...\n");
        exit(EXIT_SUCCESS);
    } else {
        return META_UNKNOWN_COMMAND;
    }
}

PrepareCommandResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
    if(strncmp(input_buffer->buffer, "insert", 6) == 0){
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }

    if(strcmp(input_buffer->buffer, "select") == 0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNKNOWN_COMMAND;
}

void execute_statement(Statement* statement){
    switch (statement->type)
    {
    case STATEMENT_SELECT:
        printf("Select statement has successfully run\n");
        break;
    case STATEMENT_INSERT:
        printf("Insert statement has successfully run\n");
        break;
    }
}

void cl_default_print(){
    printf("db > ");
}

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);

    if(bytes_read <= 0){
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1; 
    input_buffer->buffer[bytes_read - 1] = 0;
}

int main(){
    InputBuffer* input_buffer = create_new_buffer();
    while(true){
        cl_default_print();
        read_input(input_buffer);

        if(input_buffer->buffer[0] == '.'){
            switch (do_meta_command(input_buffer))
            {
                case META_SUCCESS:
                    continue;
                case META_UNKNOWN_COMMAND:
                    printf("Unrecognized comman %s", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
            case PREPARE_SUCCESS:
                break;
            
            case PREPARE_UNKNOWN_COMMAND:
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
        }

        execute_statement(&statement);
        printf("Executed.\n");

    }
}