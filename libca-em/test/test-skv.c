#include "skv.h"
#include <stdio.h>

skv_kv_t kvs[10];
skv_t    skv;
char     buf[1024];
char     key[SKV_MAX_KEY_LEN];

int main()
{
    skv_init(&skv, kvs, array_size(kvs));
    skv_put(&skv, "name", "skv");


    skv_to_str(&skv, buf);
    // 写到文件里面看看
    FILE* f = fopen("skv.txt", "w");
    fputs(buf, f);
    fclose(f);

    skv_from_str(&skv, buf);

    skv_get(&skv, "name", key);

    return 0;
}
