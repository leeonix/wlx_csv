/*
 * usage:
 * ragel -G2 csv.rl
 * gcc -o csv.exe csv.c -D__TEST__
 */

#include <stdio.h>
#include <string.h>
#include "csv.h"

#define BUFSIZE 4096

struct scanner {
    int cs;
    int act;
    char *ts;
    char *te;
    char *p;
    char *pe;
    char *eof;

    int done;
    int curline;
    char *data;
    int len;
    char cache[BUFSIZE];

    union {
        FILE *file;
        struct csv_strbuf {
            const char *buf;
            size_t len;
            size_t pos;
        } str;
    } u;

    int (*do_read)(struct scanner *s, size_t space);
};

static int csv_file_read(struct scanner *s, size_t space)
{
    return fread(s->p, 1, space, s->u.file);
}

static int csv_buf_read(struct scanner *s, size_t space)
{
    size_t left = s->u.str.len - s->u.str.pos;
    if (left > 0) {
        if (space > left) {
            space = left;
        }
        memcpy(s->p, &s->u.str.buf[s->u.str.pos], space);
        s->u.str.pos += space;
        return space;
    }
    return EOF;
}

%%{
    machine csv;

    variable p s->p;
    variable pe s->pe;
    variable eof s->eof;
    access s->;

    EOF = 0;
    EOL = [\r\n];
    comma = [,];
    string = [^,"\r\n\0]*;
    quote = '"' [^"\0]* '"';

    csv_scan := |*

    string => {
        return_token(TK_String);
        fbreak;
    };

    quote => {
        return_token(TK_Quote);
        s->data += 1;
        fbreak;
    };

    comma => {
        return_token(TK_Comma);
        fbreak;
    };

    EOL => {
        s->curline += 1;
        return_token(TK_EOL);
        fbreak;
    };

    EOF => {
        return_token(TK_EOF);
        fbreak;
    };

    *|;
}%%

%% write data nofinal;

static void scan_init(struct scanner *s)
{
    memset(s, 0, sizeof(struct scanner));
    s->curline = 1;
    %% write init;
}

static int check_buf(struct scanner *s)
{
    size_t have, space, readlen;

    if (s->p == s->pe) {
        if (s->ts == 0) {
            have = 0;
        } else {
            have = s->pe - s->ts;
            memmove(s->cache, s->ts, have);
            s->te -= (s->ts - s->cache);
            s->ts = s->cache;
        }

        s->p = s->cache + have;
        space = BUFSIZE - have;

        if (space == 0) {
            return TK_ERR;
        }

        if (s->done) {
            s->p[0] = 0;
            readlen = 1;
        } else {
            readlen = s->do_read(s, space);
            if (readlen < space) {
                s->done = 1;
            }
        }

        s->pe = s->p + readlen;
    }
    return 0;
}

#define return_token(t) token = t; s->data = s->ts

static int scan(struct scanner *s)
{
    int token = 0;

    while (1) {
        if (check_buf(s) == TK_ERR) {
            return TK_ERR;
        }

        %% write exec;

        if (s->cs == csv_error) {
            return TK_ERR;
        }

        if (token < 0) {
            switch (token) {
            case TK_Quote:
                s->len = s->p - s->data - 1;
                break;
            case TK_String:
                s->len = s->p - s->data;
                break;
            }               // -----  end switch  -----
            return token;
        }
    }
}

static void csv_read(struct scanner *s, csv_fn_t fn)
{
    while (1) {
        int token = scan(s);
        switch (token) {
        case TK_EOL:
            fn(token, s->curline - 1, NULL, 0);
            break;
        case TK_Quote:
        case TK_String:
            fn(token, s->curline, s->data, s->len);
            break;
        default:
            fn(token, s->curline, NULL, 0);
            if (token == TK_ERR || token == TK_EOF) {
                return;
            }
        }               // -----  end switch  -----
    }
}

void csv_fread(csv_fn_t fn, FILE *f)
{
    struct scanner s;
    scan_init(&s);
    s.u.file = f;
    s.do_read = csv_file_read;
    csv_read(&s, fn);
}

void csv_read_file(csv_fn_t fn, const char *name)
{
    FILE *f = fopen(name, "r");
    if (f != NULL) {
        csv_fread(fn, f);
        fclose(f);
    }
}

void csv_read_buf(csv_fn_t fn, const char *buf, size_t len)
{
    struct scanner s;
    scan_init(&s);
    s.u.str.buf = buf;
    s.u.str.len = len;
    s.do_read = csv_buf_read;
    csv_read(&s, fn);
}

