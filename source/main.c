typedef struct
{
    size_t count;
    char* data;
} String;

String
read_entire_file(const char* filename)
{
    String str = { 0 };

    FILE* file_handle = fopen(filename, "r");
    if (!file_handle) return str;

    size_t size_ok = fseek(file_handle, 0, SEEK_END);
    if (size_ok == -1) return str;

    size_t length = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);

    void* data = malloc(length + 1);
    if (!data) return str;

    auto read_length = fread(data, 1, length, file_handle);
    if (read_length != length) {
        free(data);
        return str;
    }

    str.count = length;
    str.data = data;

    return str;
}

void
free_string(String str)
{
    free(str.data);
}

char*
to_cstring(String str)
{
    char* c_string = str.data;
    c_string[str.count + 1] = '\0';
    return c_string;
}

String
to_string(char* c_string)
{
    String str = { 0 };
    str.count = strlen(c_string);
    str.data = c_string;
    return str;
}

int
main(int argc, char** argv)
{
    String program_name = to_string(argv[0]);

    int slash_index = 0;
    for (int i = 0; i < program_name.count - 1; i++) {
        if (program_name.data[i] == '/') slash_index = i + 1;
    }

    program_name.data  += slash_index;
    program_name.count -= slash_index;

    if (argc <= 1) {
        printf("Usage: %s <filename>\n", program_name.data);
        return 1;
    }

    String contents = read_entire_file(argv[1]);
    if (!contents.data) {
        printf("Unable to read file '%s'\n", argv[1]);
        return 1;
    }

    printf("Contents:\n%s", contents.data);
    free_string(contents);

    return 0;
}