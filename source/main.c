typedef struct
{
    const char* filename;
    const char* title;
    const char* date;
    const char* kind;
    const char* text;
} Post;

char*
read_entire_file(const char* filename)
{
    char* str = 0;

    FILE* file_handle = fopen(filename, "r");
    if (!file_handle) return str;

    size_t size_ok = fseek(file_handle, 0, SEEK_END);
    if (size_ok == -1) return str;

    size_t length = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);

    void* data = malloc(length + 1);
    if (!data) return str;

    size_t read_length = fread(data, 1, length, file_handle);
    if (read_length != length) {
        free(data);
        return str;
    }

    str = data;
    return str;
}

bool
ends_with(char* haystack, char* needle)
{
    char* haystack_copy = haystack;
    char* needle_copy = needle;

    size_t haystack_length = strlen(haystack_copy);
    size_t needle_length = strlen(needle_copy);
    if (haystack_length < needle_length) return false;

    haystack_copy += haystack_length - needle_length;

    bool ok = false;
    while (*haystack_copy++ && *needle_copy++) {
        ok = (haystack_copy[0] == needle_copy[0]);
        if (!ok) break;
    }

    return ok;
}

void
consume_tags(Post* post, char** contents)
{
    char* contents_copy = *contents;

    char current = 0;
    while (( current = *contents_copy++ )) {
        switch (current) {
            // Start of tag
            case ':': {
                char* tag   = contents_copy;
                char* value = 0;

                int i = 0;
                while (( current = *contents_copy++ )) {
                    i++;

                    if (current == '=') {
                        *(tag + i - 1) = '\0';

                        i = 0;
                        value = contents_copy;
                        while (( current = *contents_copy++ )) {
                            if (current == '\n') break;
                            i++;
                        }

                        *(value + i - 1) = '\0';
                        break;
                    }
                }

                if (strcmp("Title", tag) == 0) {
                    post->title = value;
                }
                else if (strcmp("Date", tag) == 0) {
                    post->date = value;
                }
                else if (strcmp("Kind", tag) == 0) {
                    post->kind = value;
                }
            } break;

            // End of tags section
            case '-': {
                while (( current = *contents_copy++ )) {
                    if (current != '-') break;
                }

                *contents = contents_copy;
            } break;
        }
    }

    // @Todo(Judah): Default values?
    /*
    if (!post->title) post->title = "Something";
    if (!post->kind) post->kind = "Blog";
    if (!post->date) post->date = "";
    */
}

int
main(int argc, char* argv[])
{
    DIR* pages_directory = opendir("_pages");
    if (!pages_directory) {
        printf("Couldn't find '_pages' directory! Does it exist?\n");
        return 1;
    }

    int i = 0;
    Post posts[1024] = { {0} };

    struct dirent* directory = { 0 };
    while ((directory = readdir(pages_directory))) {
        char* filename = directory->d_name;
        if (!ends_with(filename, ".text")) continue;

        char* path = xstrcat("_pages/", filename);
        char* contents = read_entire_file(path);
        free(path);

        if (!contents) {
            printf("Unable to open file '%s'! Skipping...\n", filename);
            continue;
        }

        Post* post = &posts[i++];

        consume_tags(post, &contents);
        post->filename = filename;
        post->text = contents;
    }

    for (int i = 0; i < 1023; i++) {
        Post post = posts[i];
        if (!post.title) continue;

        printf("POST:\n");
        printf("\tTitle: %s\n", post.title);
        printf("\tKind: %s\n", post.kind);
        printf("\tDate: %s\n", post.date);
        printf("\tFilename: %s\n", post.filename);
        printf("\tContents: %s\n", post.text);
        printf("\n");
    }

    closedir(pages_directory);

    return 0;
}
