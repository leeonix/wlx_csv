#ifndef  ___csv_INC___
#define  ___csv_INC___

#define TK_ERR      (-1)
#define TK_EOF      (-2)
#define TK_EOL      (-3)
#define TK_Comma    (-4)
#define TK_String   (-5)
#define TK_Quote    (-6)

typedef void (*csv_fn_t)(int token, int record_no, const char *field, int field_len);
extern void csv_fread(csv_fn_t fn, FILE *f);
extern void csv_read_file(csv_fn_t fn, const char *name);
extern void csv_read_buf(csv_fn_t fn, const char *buf, size_t len);

#endif   // ----- #ifndef ___csv_INC___  ----- 

