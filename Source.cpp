#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

// Enum and Mapper for all the token types
enum token_type
{
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    PUNCTUATION,
    KEYWORD,
    DEFAULT_FAIL
};

const char* token_type_mapper[] = {
    "IDENTIFIER",
    "NUMBER",
    "OPERATOR",
    "PUNCTUATION",
    "KEYWORD",
    "DEFAULT_FAIL" };

// Enum and Mapper for all state types
enum state_type
{
    ACCEPTING,
    NON_ACCEPTING,
    ERROR
};

const char* state_type_mapper[] =
{
    "ACCEPTING",
    "NON_ACCEPTING",
    "ERROR" };

// Main token type data structure
struct Token
{
    char* lexeme;
    token_type t_type;
};

// Main state type data structure
struct State
{
    int state_no;
    state_type s_type;
    token_type accepting_class;
    char* error_msg;
};

// Main error type data structure
struct Error
{
    int line_no;
    int col_no;
    char* causing_token;
    char* error_msg;
};

const int CHARACTER_MAPPER_SIZE = 93;
int CHARACTER_MAPPER[CHARACTER_MAPPER_SIZE];

const int NUMBER_OF_STATES = 29, NUMBER_OF_VALID_CHARS_CLASSES = 25;
int TRANSITION_TABLE[NUMBER_OF_STATES][NUMBER_OF_VALID_CHARS_CLASSES];

int INITIAL_STATE = 1, NEXT_MACHINE_STATE = 21;

const int NUMBER_OF_STATE_TABLE_COLUMNS = 3;
State States[NUMBER_OF_STATES];

int TOKEN_COUNT = 0, ERROR_COUNT = 0;
Token* TOKEN_LIST;
Error* ERROR_LIST;

// Resource files readers
void getCharacterMappingTable();
void getTransitionTable();
void getStateTable();

// Output files writers
void writeTokensFile(ofstream&);
void writeErrorsFile(ofstream&);

// Main Scanner Functions
void LexicalAnalyzer(ifstream&);
void Tokenizer(ifstream&, char*, int);

// Utility Functions
int* regrow(int*, int, int);
char* regrow(char*, int, char);
Token* regrow_token_arr(Token*, int, Token*);
Error* regrow_error_arr(Error*, int, Error*);
bool isKeyword(char*);

int main()
{
    // Get resources
    getCharacterMappingTable();
    getTransitionTable();
    getStateTable();

    // Opening source code file
    ifstream f_source;
    f_source.open("Sourcecode.txt", ios::in);

    if (!f_source.is_open())
    {
        cout << "ERROR: Could not open Source Code file!\n";
        return 0;
    }

    // Initializing the token and error arrays
    TOKEN_LIST = new Token[TOKEN_COUNT];
    ERROR_LIST = new Error[ERROR_COUNT];

    // Passing the source file to scanner. It will populate into globally made token and error arrays
    LexicalAnalyzer(f_source);

    f_source.close();

    // Writing to Output Files

    // Writing to tokens file
    ofstream f_token;
    f_token.open("Token.txt", ios::out);

    if (!f_token.is_open())
    {
        cout << "ERROR: Could not open/create Token file!\n";
        return 0;
    }

    writeTokensFile(f_token);
    cout << "Successfully Written to Tokens File!\n";

    f_token.close();

    // Writing to errors file
    ofstream f_error;
    f_error.open("Error.txt", ios::out);

    if (!f_error.is_open())
    {
        cout << "ERROR: Could not open/create Error file!\n";
        return 0;
    }

    writeErrorsFile(f_error);
    cout << "Successfully Written to Errors File!\n";

    f_error.close();

    // Cout some information
    cout << "\nNumber of Tokens Written : " << TOKEN_COUNT << endl;
    cout << "Number of Errors Written : " << ERROR_COUNT << endl;

    return 0;
}

void getCharacterMappingTable()
{
    ifstream file;
    file.open("Character Mapping Table.csv", ios::in);
    if (!file.is_open())
    {
        cout << "ERROR: Could not open Character Mapping Table file!\n";
        return;
    }

    int i = 0;
    while (!file.eof())
    {
        // Get the first column of file
        char line[10];
        file.getline(line, 10, ',');

        // Exception for comma within a field
        if (i == 10)
        {
            file.getline(line, 10, ',');
        }

        // Stores the second column of file
        file >> CHARACTER_MAPPER[i];
        i++;
    }

    file.close();
}

void getTransitionTable()
{
    ifstream file;
    file.open("Transition Table.csv", ios::in);

    if (!file.is_open())
    {
        cout << "ERROR: Could not open Transition Table file!\n";
        return;
    }

    // Skip the 1st row and 1st column of each row because it's the header
    for (int i = 0; i < NUMBER_OF_STATES; i++)
    {
        if (i == 0)
        {
            char* buffer = new char[256];
            file.getline(buffer, 256);
            delete[] buffer;
        }
        for (int j = 0; j < NUMBER_OF_VALID_CHARS_CLASSES; j++)
        {
            if (j == 0)
            {
                char buffer[20];
                file.getline(buffer, 20, ',');
            }

            file >> TRANSITION_TABLE[i][j];
            char buffer;
            file >> buffer;
        }
    }

    file.close();
}

void getStateTable()
{
    ifstream file;
    file.open("State Table.csv", ios::in);

    if (!file.is_open())
    {
        cout << "ERROR: Could not open State Table file!\n";
        return;
    }

    // Skip the 1st row and 1st column of each row because it's the header
    for (int i = 0; i < NUMBER_OF_STATES; i++)
    {
        if (i == 0)
        {
            char* buffer = new char[256];
            file.getline(buffer, 256);
            delete[] buffer;
        }

        // Setting the state number as current iteration
        States[i].state_no = i + 1;

        char buffer_arr[20];
        file.getline(buffer_arr, 20, ',');

        int type, accepting_class;
        char buffer;
        char* error_str = new char[256];
        file >> type >> buffer >> accepting_class >> buffer;
        file.getline(error_str, 256);
        States[i].error_msg = error_str;
        switch (type)
        {
        case 1:
            States[i].s_type = ACCEPTING;
            switch (accepting_class)
            {
            case 0:
                States[i].accepting_class = IDENTIFIER;
                break;
            case 1:
                States[i].accepting_class = NUMBER;
                break;
            case 2:
                States[i].accepting_class = OPERATOR;
                break;
            case 3:
                States[i].accepting_class = PUNCTUATION;
                break;
            case 4:
                States[i].accepting_class = KEYWORD;
                break;
            case -1:
                States[i].accepting_class = DEFAULT_FAIL;
                break;
            }
            break;
        case 0:
            States[i].s_type = NON_ACCEPTING;
            States[i].accepting_class = DEFAULT_FAIL;
            break;
        case -1:
            States[i].s_type = ERROR;
            States[i].accepting_class = DEFAULT_FAIL;
            break;
        }
    }

    file.close();
}

void writeTokensFile(ofstream& file)
{
    // Writing in format Token: <lexeme,type>
    for (int i = 0; i < TOKEN_COUNT; i++)
    {
        file << "< \"" << TOKEN_LIST[i].lexeme << "\" , " << token_type_mapper[TOKEN_LIST[i].t_type]
            << " >\n";
    }
}

void writeErrorsFile(ofstream& file)
{
    // Writing in format Error: Error at Line _ Column _ : Causing Token -> Error Msg
    for (int i = 0; i < ERROR_COUNT; i++)
    {
        file << "Error at Line " << ERROR_LIST[i].line_no << " Column " << ERROR_LIST[i].col_no << ": \"" << ERROR_LIST[i].causing_token << "\" " << ERROR_LIST[i].error_msg << endl;
    }
}

void LexicalAnalyzer(ifstream& f_source)
{
    // 1: Reading the file line by line
    for (int line_number = 1; !f_source.eof(); line_number++)
    {
        // Getting line of sourcecode
        char* curr_line = new char[256];
        f_source.getline(curr_line, 256);

        // 2: removing whitespaces from code line
        int count = 0, i = 0;
        char* curr_line_cleaned = new char[strlen(curr_line)];
        while (i < strlen(curr_line))
        {
            if (curr_line[i] != ' ')
            {
                curr_line_cleaned[count] = curr_line[i];
                count++;
            }

            i++;
        }
        curr_line_cleaned[count] = '\0';
        delete[] curr_line;

        // 3: Tokenization + 4: token validation + 5: token classification
        Tokenizer(f_source, curr_line_cleaned, line_number);
    }
}

void Tokenizer(ifstream& f_source, char* input_line, int curr_line_no)
{
    int curr_state = INITIAL_STATE;
    int prev_state = 0;

    // Reading line char by char
    int curr_token_size = 1;
    char* token = new char[curr_token_size];
    *token = '\0';

    // Looping an extra time to make token of last char
    for (int i = 0; i <= strlen(input_line); i++)
    {
        // Do according to transition table

        // Change states according to transition table
        prev_state = curr_state;
        // Subtracting 34 from curr char because charachter mapping table is
        // starting from assci 33 and one because of index 0
        if (i == strlen(input_line))
            // Exception case if new line comes then refer to other columns of transition table
            curr_state = TRANSITION_TABLE[curr_state - 1][24];
        else
            curr_state = TRANSITION_TABLE[curr_state - 1][CHARACTER_MAPPER[input_line[i] - 34] - 1];

        // When a token is completed or end of line is reached
        if (curr_state == NEXT_MACHINE_STATE || States[curr_state - 1].s_type == ERROR || i == strlen(input_line))
        {
            // Next Machine State now we have to reset everything

            // 1: Make a token

            // Token Validation + Classification
            // Classifying the token according to state table by using the previous state accepting class
            if (States[prev_state - 1].s_type == ACCEPTING)
            {
                // Accepting state add it to token array
                Token* new_token = new Token;
                char* curr_lexeme = new char[strlen(token)];
                strcpy_s(curr_lexeme, strlen(token) + 1, token);
                new_token->lexeme = curr_lexeme;
                new_token->t_type = States[prev_state - 1].accepting_class;

                TOKEN_LIST = regrow_token_arr(TOKEN_LIST, TOKEN_COUNT, new_token);
                TOKEN_COUNT++;
            }
            else
            {
                // error state first check if it's a keyword if not add it to error array

                // Checking if it's a keyword
                if ((curr_state == 22 || curr_state == 2) && isKeyword(token))
                {

                    // Add it to token array
                    Token* new_token = new Token;
                    char* curr_lexeme = new char[strlen(token)];
                    strcpy_s(curr_lexeme, strlen(token) + 1, token);
                    new_token->lexeme = curr_lexeme;
                    new_token->t_type = KEYWORD;

                    TOKEN_LIST = regrow_token_arr(TOKEN_LIST, TOKEN_COUNT, new_token);
                    TOKEN_COUNT++;
                }
                else
                {
                    // Not a keyword add to error arr
                    Error* new_error = new Error;
                    new_error->line_no = curr_line_no;
                    new_error->col_no = i;
                    char* curr_lexeme = new char[strlen(token)];
                    strcpy_s(curr_lexeme, strlen(token) + 1, token);
                    new_error->causing_token = curr_lexeme;
                    new_error->error_msg = States[curr_state - 1].error_msg;

                    ERROR_LIST = regrow_error_arr(ERROR_LIST, ERROR_COUNT, new_error);
                    ERROR_COUNT++;
                }
            }
            /*token_list = regrow(token_list, token_list_size, token);
            token_list_size++;*/

            // Exception case if new line is reached becauae in that case we don't need last 3 steps
            if (i != strlen(input_line))
            {
                // 2: Moving pointer one step back
                // f_source.seekg(-1, ios_base::cur);
                i--;

                // 3: Reset the current state
                curr_state = INITIAL_STATE;

                // 4: Reset current token
                delete[] token;
                curr_token_size = 1;
                token = new char[curr_token_size];
                *token = '\0';
            }
        }

        // Add the current char to current token
        else
        {
            token = regrow(token, curr_token_size, input_line[i]);
            curr_token_size++;
        }
    }
}

int* regrow(int* old_arr, int old_size, int new_data)
{
    // Make a new array with one size greater
    int new_size = old_size + 1;
    int* new_arr = new int[new_size];

    // Copy old array elements to new array
    for (int i = 0; i < old_size; i++)
    {
        new_arr[i] = old_arr[i];
    }

    // Add new value to last index of new array
    new_arr[old_size] = new_data;

    // Delete old array
    delete old_arr;

    return new_arr;
}

char* regrow(char* old_arr, int old_size, char new_data)
{
    // Make a new array with one size greater
    int new_size = old_size + 1;
    char* new_arr = new char[new_size];

    // Copy old array elements to new array
    for (int i = 0; i < old_size; i++)
    {
        new_arr[i] = old_arr[i];
    }

    // Add new value to last index of new array
    new_arr[old_size - 1] = new_data;
    new_arr[old_size] = '\0';

    // Delete old array
    delete[] old_arr;

    return new_arr;
}

Token* regrow_token_arr(Token* old_arr, int old_size, Token* new_data)
{
    int new_size = old_size + 1;
    Token* new_arr = new Token[new_size];

    // Copy old array elements to new array
    for (int i = 0; i < old_size; i++)
    {
        new_arr[i].lexeme = old_arr[i].lexeme;
        new_arr[i].t_type = old_arr[i].t_type;
    }

    // Add new value to last index of new array
    new_arr[old_size].lexeme = new_data->lexeme;
    new_arr[old_size].t_type = new_data->t_type;

    // Delete old array
    delete[] old_arr;

    return new_arr;
}

Error* regrow_error_arr(Error* old_arr, int old_size, Error* new_data)
{
    int new_size = old_size + 1;
    Error* new_arr = new Error[new_size];

    // Copy old array elements to new array
    for (int i = 0; i < old_size; i++)
    {
        new_arr[i].line_no = old_arr[i].line_no;
        new_arr[i].col_no = old_arr[i].col_no;
        new_arr[i].error_msg = old_arr[i].error_msg;
        new_arr[i].causing_token = old_arr[i].causing_token;
    }

    // Add new value to last index of new array
    new_arr[old_size].line_no = new_data->line_no;
    new_arr[old_size].col_no = new_data->col_no;
    new_arr[old_size].error_msg = new_data->error_msg;
    new_arr[old_size].causing_token = new_data->causing_token;

    // Delete old array
    delete[] old_arr;

    return new_arr;
}

bool isKeyword(char* str)
{
    if (!strcmp(str, "while") || !strcmp(str, "do-while") || !strcmp(str, "for") || !strcmp(str, "if") ||
        !strcmp(str, "if-else") || !strcmp(str, "else") || !strcmp(str, "cin") || !strcmp(str, "cout") ||
        !strcmp(str, "asm") || !strcmp(str, "new") || !strcmp(str, "this") || !strcmp(str, "auto") ||
        !strcmp(str, "enum") || !strcmp(str, "operator") || !strcmp(str, "throw") || !strcmp(str, "bool") ||
        !strcmp(str, "explicit") || !strcmp(str, "private") || !strcmp(str, "true") ||
        !strcmp(str, "break") || !strcmp(str, "export") || !strcmp(str, "protected") || !strcmp(str, "try") ||
        !strcmp(str, "case") || !strcmp(str, "extern") || !strcmp(str, "public") || !strcmp(str, "typedef") ||
        !strcmp(str, "catch") || !strcmp(str, "false") || !strcmp(str, "register") || !strcmp(str, "typeid") ||
        !strcmp(str, "char") || !strcmp(str, "float") || !strcmp(str, "typename") || !strcmp(str, "class") ||
        !strcmp(str, "for") || !strcmp(str, "return") || !strcmp(str, "union") || !strcmp(str, "cost") ||
        !strcmp(str, "friend") || !strcmp(str, "short") || !strcmp(str, "unsigned") || !strcmp(str, "goto") ||
        !strcmp(str, "signed") || !strcmp(str, "using") || !strcmp(str, "continue") || !strcmp(str, "if") ||
        !strcmp(str, "sizeof") || !strcmp(str, "virtual") || !strcmp(str, "default") ||
        !strcmp(str, "inline") || !strcmp(str, "static") || !strcmp(str, "void") ||
        !strcmp(str, "delete") || !strcmp(str, "int") || !strcmp(str, "volatile") ||
        !strcmp(str, "do") || !strcmp(str, "long") || !strcmp(str, "struct") ||
        !strcmp(str, "double") || !strcmp(str, "mutable") || !strcmp(str, "switch") ||
        !strcmp(str, "while") || !strcmp(str, "namespace") || !strcmp(str, "template"))
        return true;
    return false;
}
