#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define TABLE_MAX_PAGES 100

typedef enum {
  META_SUCCESS,
  META_UNKNOWN_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNKNOWN_COMMAND,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_INVALID_ID_TYPE
} PrepareCommandResult;

typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef enum {
  EXECUTE_TABLE_FULL,
  EXECUTE_SUCCESS
} ExecuteResult;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;
  
typedef struct {
  StatementType type;
  Row row_to_insert;
} Statement;

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096; // same size as a page used in the virtual memory systems
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

void serialize_row(Row* source, void* destination) {
  memcpy((char*)destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy((char*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy((char*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), (char*)source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), (char*)source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), (char*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_slot(Table* table, uint32_t row_num) {
  const uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];
  if(page == NULL) {
    page = table->pages[page_num] = malloc(PAGE_SIZE); // allocated 4kb page
  }
  const uint32_t row_offset = row_num % ROWS_PER_PAGE;
  const uint32_t byte_offset = row_offset * ROW_SIZE;
  return (char*)page + byte_offset;
}

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if(strcmp(input_buffer->buffer, ".exit") == 0) {
    printf("Quitting...");
    exit(EXIT_SUCCESS);
  } else {
    return META_UNKNOWN_COMMAND;
  }
}

PrepareCommandResult prepare_insert(InputBuffer* input_buffer, Statement* statement){
    statement->type = STATEMENT_INSERT;

    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_string == NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);

    if(id < 0){
        return PREPARE_INVALID_ID_TYPE;
    }

    if(strcmp(id_string, "0") != 0 && id == 0){ // user entered a character instead of a number
        // srand(time(NULL));
        // id = rand() % 100;
        return PREPARE_INVALID_ID_TYPE;
    }

    if(strlen(username) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    if(strlen(email) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

PrepareCommandResult prepare_command(InputBuffer* input_buffer, Statement* statement) {
  if(strncmp(input_buffer->buffer, "insert", 6) == 0) {
    // statement->type = STATEMENT_INSERT;
    // int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);

    // if(args_assigned < 3) {
    //   return PREPARE_SYNTAX_ERROR;
    // }
    // return PREPARE_SUCCESS;

    // call the function prepare_insert
    return prepare_insert(input_buffer, statement);
  }

  if(strcmp(input_buffer->buffer, "select") == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNKNOWN_COMMAND;
}

ExecuteResult execute_insert(Statement* statement, Table* table) {
  if(table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row_to_insert = &(statement->row_to_insert);

  serialize_row(row_to_insert, row_slot(table, table->num_rows));
  table->num_rows += 1;

  return EXECUTE_SUCCESS;
}

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  for(uint32_t i = 0; i < table->num_rows; i++) {
    deserialize_row(row_slot(table, i), &row);
    print_row(&row);
  }
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table) {
  switch (statement->type)
  {
  case STATEMENT_INSERT:
    // printf("Insertion will be done here.\n");
    // break;
    return execute_insert(statement, table);

  case STATEMENT_SELECT:
    // printf("Select statement will be executed here.\n");
    // break;
    return execute_select(statement, table);
  }
}

void print_prompt() { 
  printf("db > "); 
  fflush(stdout);
}

void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

Table* create_new_table() {
  Table* table = (Table*)malloc(sizeof(Table));
  table->num_rows = 0;
  for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    table->pages[i] = NULL;
  }
  return table;
}

void free_table(Table* table) {
  for(uint32_t i = 0; table->pages[i]; i++) {
    free(table->pages[i]);
  }
  free(table);
}

int main(int argc, char* argv[]) {
  Table* table = create_new_table();
  InputBuffer* input_buffer = new_input_buffer();
  while (true) {
    print_prompt();
    read_input(input_buffer);

    if(input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer))
      {
      case META_SUCCESS:
        continue;
      
      case META_UNKNOWN_COMMAND:
        printf("Unrecognized command '%s'\n", input_buffer->buffer);
        continue;
      }
    }

    Statement statement;
    switch (prepare_command(input_buffer, &statement))
    {
    case PREPARE_SUCCESS:
      break;

    case PREPARE_STRING_TOO_LONG:
        printf("Insertion failed: Provided string exceeds the maximum supported length\n");
        continue;

    case PREPARE_INVALID_ID_TYPE:
        printf("ID must be a valid positive integer\n");
        continue;

    case (PREPARE_SYNTAX_ERROR):
      printf("Syntax error. Could not parse statement.\n");
      continue;
    
    case PREPARE_UNKNOWN_COMMAND:
      printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
      continue;
    }

    // execute_statement(&statement);
    
    switch (execute_statement(&statement, table))
    {
      case EXECUTE_SUCCESS:
        printf("Operation successful.\n");
        continue;
      
      case EXECUTE_TABLE_FULL:
        printf("Error: Table full.\n");
        continue;
    }
  }
}