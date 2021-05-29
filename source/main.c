#define BUFFER_SIZE (32 << 20)
#define MAX_POSTS 1024

#define RAW_ROOT "_pages/"
#define SITE_ROOT "docs/"

#define NO_TEMPLATE ""

#define TAG_TITLE "Title"
#define TAG_DATE "Date"
#define TAG_UPDATED "Updated"
#define TAG_CONTENT "Content"
#define TAG_TEMPLATE "Template"
#define TAG_KIND "Kind"
#define TAG_POSTS "Posts"
#define TAG_STYLE "Style"
#define TAG_INCLUDE "Include"

#define panic(...) (                        \
    fprintf(stderr, "Error: " __VA_ARGS__), \
    fprintf(stderr, "\n"),                  \
    exit(1)                                 \
)

static void* memory = 0;
static int   memory_index = 0;

typedef enum {
    KindUnknown = 0b00000001,
    KindOther   = 0b00000010,
    KindBlog    = 0b00000100,
    KindPage    = 0b00001000,
} PostKind;

typedef struct
{
    char* filename;
    char* title;
    char* url; // Only set after processing
    char* date;
    char* updated;
    PostKind kind;
    char* template;
    char* text;
} Post;

void*
alloc(size_t bytes)
{
    if (memory_index + bytes > BUFFER_SIZE) {
        panic("Out of memory after allocation of %zukb at %dkb\n", bytes >> 10, memory_index >> 10);
    }

    void* ptr = memory + memory_index;
    memory_index += bytes;
    return ptr;
}

void
push_char(char chr)
{
    char* value = alloc(1);
    *value = chr;
}

char* 
push_string(char* str, bool include_terminator)
{
    char* value = str;
    char* res = alloc(0);

    char chr = 0;
    while((chr = *value++)) {
        push_char(chr);
    }

    if (include_terminator)
        push_char('\0');

    return res;
}

char*
concat(char* left, char* right)
{
    char* res = alloc(0);
    push_string(left, false);
    push_string(right, true);
    return res;
}

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

    void* data = alloc(length + 1);
    if (!data) return str;

    size_t read_length = fread(data, 1, length, file_handle);
    if (read_length != length) {
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

bool
contains(char* haystack, char* needle)
{
    int haystack_len = strlen(haystack);
    int needle_len   = strlen(needle);

    if (haystack_len < needle_len || needle_len <= 0) return false;

    bool found = false;
    for (int i = 0; i < haystack_len; i++) {
        if (haystack[i] == needle[i]) {
            for (int k = 0; k < needle_len; k++) {
                if (haystack[i + k] == needle[k]) {
                    found = true;
                }
                else {
                    found = false;
                }
            }

            if (found) break;
        }
    }

    return found;
}

void
consume_tags(Post* post, char** contents)
{
    char *contents_copy = *contents;

    bool end_of_tags = false;
    char current = 0;
    while (!end_of_tags && (current = *contents_copy++)) {
        switch (current) {
            // Start of tag
            case ':': {
                char* tag = contents_copy;
                char* value = 0;

                int i = 0;
                while ((current = *contents_copy++)) {
                    i++;

                    if (current == '=') {
                        *(tag + i - 1) = '\0';

                        i = 0;
                        value = contents_copy;
                        while ((current = *contents_copy++)) {
                            if (current == '\n') break;
                            i++;
                        }

                        *(value + i - 1) = '\0';
                        break;
                    }
                }

                if (strcmp(TAG_TITLE, tag) == 0) {
                    post->title = value;
                }
                else if (strcmp(TAG_DATE, tag) == 0) {
                    post->date = value;
                }
                else if (strcmp(TAG_UPDATED, tag) == 0) {
                    if (strlen(value) > 0) {
                        post->updated = value;
                    }
                }
                else if (strcmp(TAG_KIND, tag) == 0) {
                    char* kind_str = alloc(0);

                    int i = 0;
                    while (value[i]) {
                        if (value[i] == ',' || !value[i + 1]) {
                            if (!value[i + 1]) push_char(value[i]);
                            push_char(0);

                            if (strcmp("Blog", kind_str) == 0) {
                                post->kind |= KindBlog;
                            }
                            else if (strcmp("Page", kind_str) == 0) {
                                post->kind |= KindPage;
                            }
                            else if (strcmp("Other", kind_str) == 0) {
                                post->kind |= KindOther;
                            }
                            else {
                                printf("Unknown post kind '%s' in post '%s'\n", kind_str, post->filename);
                            }

                            i++;
                            kind_str = alloc(0);
                            continue;
                        }

                        push_char(value[i++]);
                    }
                }
                else if (strcmp(TAG_TEMPLATE, tag) == 0) {
                    if (strlen(value) <= 0) {
                        post->template = NO_TEMPLATE;
                    } else {
                        post->template = value;
                    }
                }
            } break;

            // End of tags section
            case '-': {
                int dashes = 1;
                while ((current = *contents_copy++)) {
                    if (current != '-') break;
                    dashes += 1;
                }

                if (dashes >= 3) {
                    end_of_tags = true;
                    *contents = contents_copy;
                }
            } break;
        }
    }

    // @Todo(Judah): Default values?
    // if (!post->title) post->title = "Something";
    // if (!post->kind) post->kind = "Blog";
    // if (!post->date) post->date = "";
    if (post->template == 0) {
        post->template = "template.html";
    }
}

char*
title_to_directory(char* title)
{
    char* new_string = alloc(0);

    int i = -1;
    while (title[++i]) {
        if (isalnum(title[i])) {
            push_char(tolower(title[i]));
        }
        else if (title[i] == ' ' || title[i] == '-' || title[i] == '_') {
            push_char('-');
        }
    }

    push_char(0);
    return new_string;
}

int
main(int argc, char** argv)
{
    memory = calloc(BUFFER_SIZE, 1);
    if (!memory) { panic("unable to allocate main buffer!"); }

    DIR *pages_directory = opendir(RAW_ROOT);
    if (!pages_directory) {
        panic("Couldn't find '%s' directory! Does it exist?", RAW_ROOT);
    }

    char* site_style  = 0;
    char* index_page  = 0;
    Post posts[MAX_POSTS] = {{0}};

    int i = MAX_POSTS - 1;
    struct dirent* directory = {0};
    while ((directory = readdir(pages_directory))) {
        char* filename = directory->d_name;
        char* path = concat(RAW_ROOT, filename);

        // Preload our template files
        if (ends_with(filename, ".html")) {
            if (contains(filename, "index")) {
                index_page = read_entire_file(path);
                continue;
            }
        }
        else if (ends_with(filename, ".css")) {
            if (contains(filename, "style")) {
                site_style = read_entire_file(path);
                continue;
            }
        }

        // Only process post files
        if (!ends_with(filename, ".text")) continue;

        char* contents = read_entire_file(path);
        if (!contents) {
            printf("Unable to open file '%s'! Skipping...\n", filename);
            continue;
        }

        Post* post = &posts[i--];
        post->filename = push_string(filename, true);
        post->text = contents;

        consume_tags(post, &post->text);
    }

    closedir(pages_directory);

    if (mkdir(SITE_ROOT, S_IRWXU) == -1) {
        panic("Unable to make site directory! Does it already exist?");
    }

    // Process posts
    for (int i = 0; i < MAX_POSTS; i++) {
        Post post = posts[i];
        if (!post.title) continue;

        char* directory_name = title_to_directory(post.title);
        char* post_directory = concat(SITE_ROOT, directory_name);
        char* index_file     = concat(post_directory, "/index.html");

        if (mkdir(post_directory, S_IRWXU) == -1) {
            printf("Unable to make post directory '%s'! Skipping...\n", post_directory);
            continue;
        }

        FILE* file = fopen(index_file, "w+");
        if (!file) {
            printf("Unable to create file '%s'! Skipping...\n", index_file);
            continue;
        }

        printf("Processing '%s' (%s -> %s)\n", post.title, post.filename, index_file);

        char* template = 0;

        if (post.template == NO_TEMPLATE) {
            template = post.text; // Use the post text as a template so the meta tags get filled
        }
        else {
            // @Todo(Judah): Intern template filenames so we don't read the same template multiple times
            template = read_entire_file(concat(RAW_ROOT, post.template));
        }

        if (!template) {
            printf("Unable to open template file '%s'! Does it exist?\n", post.template);
            fclose(file);
            remove(index_file);
            rmdir(post_directory);
            continue;
        }

        int c = -1;
        while (template[++c]) {
            // We found a tag
            if (template[c] == ':') {
                c++;

                char* tag = alloc(0);
                while (template[c] && isalnum(template[c])) {
                    push_char(template[c]);
                    c++;
                }
                push_char(0);

                if (strcmp(TAG_TITLE, tag) == 0) {
                    fputs(post.title, file);
                }
                else if (strcmp(TAG_DATE, tag) == 0) {
                    fputs(post.date, file);
                }
                else if (strcmp(TAG_CONTENT, tag) == 0) {
                    fputs(post.text, file);
                }
                else if (strcmp(TAG_STYLE, tag) == 0) {
                    fprintf(file, "<style>\n%s\n</style>", site_style);
                }
                else if (strcmp(TAG_INCLUDE, tag) == 0) {
                    if (template[c] != '(') {
                        printf("Malformed include in '%s'\n", post.filename);
                        continue;
                    }

                    char* filename = alloc(0);

                    while (template[++c] && template[c] != ')') {
                        push_char(template[c]);
                    }
                    push_char(0);
                    c++;

                    char* include_file = read_entire_file(concat("_pages/", filename));
                    if (!include_file) {
                        printf("Include for '%s' in '%s' was unable to be found! Does it exist?\n", filename, post.filename);
                        continue;
                    }

                    fputs(include_file, file);
                }
                else {
                    if (strlen(tag) <= 0) {
                        fputc(':', file);
                    } else {
                        printf("Unknown tag '%s' in '%s'\n", tag, post.filename);
                    }
                }
            }

            fputc(template[c], file);
        }

        posts[i].url = directory_name;
        fprintf(file, "\n<!-- Generated file -->");
        fclose(file);
    }

    // Process index page
    printf("Processing index page (%sindex.html -> %sindex.html)\n", RAW_ROOT, SITE_ROOT);

    if (!index_page) {
        panic("Unable to find index page! Does it exist?");
    }

    FILE* index_file = fopen(SITE_ROOT"index.html", "w+");
    if (!index_file) {
        panic("Unable to create main index file!");
    }

    i = -1;
    while (index_page[++i]) {
        if (index_page[i] == ':') {
            i++;

            char* tag = alloc(0);
            while (index_page[i] && isalnum(index_page[i])) {
                push_char(index_page[i]);
                i++;
            }
            push_char(0);

            if (strcmp(TAG_POSTS, tag) == 0) {
                for (int i = 0; i < MAX_POSTS; i++) {
                    Post post = posts[i];
                    if (!post.title || (post.kind & KindPage)) continue;
                    fprintf(index_file, "<li><a href=\"%s\">%s</a></li>\n", post.url, post.title);
                }
            }
            else if (strcmp(TAG_STYLE, tag) == 0) {
                fprintf(index_file, "<style>\n%s\n</style>", site_style);
            }
            else if (strcmp(TAG_INCLUDE, tag) == 0) {
                    if (index_page[i] != '(') {
                        printf("Malformed include in '%sindex.html'\n", RAW_ROOT);
                        continue;
                    }

                    char* filename = alloc(0);

                    while (index_page[++i] && index_page[i] != ')') {
                        push_char(index_page[i]);
                    }

                    push_char(0);
                    i++;

                    char* include_file = read_entire_file(concat("_pages/", filename));
                    if (!include_file) {
                        printf("Include for '%s' in '%sindex.html' was unable to be found! Does it exist?\n", filename, RAW_ROOT);
                        continue;
                    }

                    fputs(include_file, index_file);
            }
            else if (strlen(tag) <= 0) {
                fputc(':', index_file);
            }
        }

        fputc(index_page[i], index_file);
    }

    fprintf(index_file, "\n<!-- Generated file -->");
    fclose(index_file);

    printf("Done!\nMemory used: %dkb of %dkb\n", memory_index >> 10, BUFFER_SIZE >> 10);
    return 0;
}
