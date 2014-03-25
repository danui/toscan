#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct buffer {
    char * txt;
    size_t len;
    size_t cap;
    char * file_name;
    int lineno;
};

/**
 * @return True if c is a whitespace character.
 */
static int is_white_space(char c)
{
    switch (c) {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            return 1;
    }
    return 0;
}

/**
 * @return True if S has nothing but whitespaces.
 */
static int is_blank_line(char * S)
{
    while (S[0] != (char)0) {
        if (!is_white_space(S[0])) {
            return 0;
        }
        S+=1;
    }
    return 1;
}

static void print_todo(char * file_name, int lineno, char * S, size_t n, FILE * out)
{
    char * sep = "\r\n\t ";
    char * tok = NULL;
    char * ctx = NULL;
    size_t ncol = 72;
    size_t linenocol=6;
    size_t cursor;
    size_t k;
    size_t ntok;
    if (S[0] != '[' || ']' != S[2])
        return;
    /* For now only include outstanding todo items. */
    if (S[1] != ' ') return;

    cursor = 0;
    ntok = 0;
    for (tok = strtok_r(S, sep, &ctx); tok; tok = strtok_r(NULL, sep, &ctx)) {
        k = strlen(tok);
        if ((cursor + 1 + k) > ncol) {
            fprintf(out, "\n    ");
            cursor = 4;
            ntok = 0;
        }
        if (ntok > 0) {
            fprintf(out, " ");
            cursor += 1;
        }
        fprintf(out, "%s", tok);
        ntok += 1;
        cursor += k;
    }
    fprintf(out, "\n%*s %*d\n", (int)ncol-(int)linenocol-1, file_name?file_name:"from line:", (int)linenocol, lineno);
    fprintf(out, "\n");
}

static void buffer_flush(struct buffer * buf, FILE * out)
{
    size_t b;
    size_t c;

    for (b=0; b<buf->len && is_white_space(buf->txt[b]); ++b);
    for (c=buf->len-b; c>0 && is_white_space(buf->txt[b+c-1]); --c);
    buf->txt[b+c] = (char)0;
    print_todo(buf->file_name, buf->lineno, buf->txt+b, c, out);
    buf->len = 0;
    buf->txt[buf->len] = (char)0;
}

static void buffer_store(struct buffer * buf, char * line, char * file_name, int lineno)
{
    size_t n, k;
    char * new_buf;
    n = strlen(line);
    k = buf->cap;
    while ((buf->len + n + 1) > k) {
        if (k == 0)
            k = 16;
        else
            k += k;
    }
    if (k > buf->cap) {
        new_buf = realloc(buf->txt, k);
        if (new_buf == NULL) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }
        buf->txt = new_buf;
        buf->cap = k;
    }
    if (buf->len == 0) {
        buf->file_name = file_name;
        buf->lineno = lineno;
    }
    memcpy(buf->txt + buf->len, line, n);
    buf->len += n;
    buf->txt[buf->len] = (char)0;
}

static void buffer_accept(struct buffer * buf, char * line, char * file_name, int lineno, FILE * out)
{
    if (is_blank_line(line))
        buffer_flush(buf, out);
    else
        buffer_store(buf, line, file_name, lineno);
}

static void scan(FILE * in, FILE * out, char * file_name)
{
    char * line_str = NULL;
    size_t line_len = 0;
    int line_number;
    struct buffer buf;

    buf.cap = 128;
    buf.txt = malloc(buf.cap);
    if (buf.txt == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    buf.len = 0;

    /*
     * todo items are separated by blank lines, so it makes sense to use
     * getline.
     */
    line_number = 0;
    while (-1 != getline(&line_str, &line_len, in)) {
        line_number += 1;
        buffer_accept(&buf, line_str, file_name, line_number, out);
    }
    buffer_flush(&buf, out);

    free(buf.txt);
    free(line_str);
}


int main(int argc, char ** argv)
{
    FILE * in = NULL;
    int i;
    if (argc == 1) {
        scan(stdin, stdout, NULL);
    } else {
        for (i=1; i < argc; ++i) {
            if (NULL == (in = fopen(argv[i], "r"))) {
                fprintf(stderr, "could not open input file %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            scan(in, stdout, argv[i]);
            fclose(in);
        }
    }
    return 0;
}
