#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Making a Simple REPL

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

void clear_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(){
    InputBuffer* input_buffer = create_new_buffer();
    while(true){
        cl_default_print();
        read_input(input_buffer);

        if(strcmp(input_buffer->buffer, ".exit") == 0){
            clear_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }

        printf("Command not found: `%s`\n", input_buffer->buffer);
    }
}