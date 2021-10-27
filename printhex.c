void printhex(const unsigned char *p, size_t len, const char *descr)
{
    int dlen = descr ? (int) strlen(descr) + 3 : 0;
    if(dlen) printf("%s = %s", descr, len ? "" : "\n");
    while(len) {
        const unsigned char *q = p;
        for(int j = 0; j < 16; ++j) {
            if(len) {
                printf("%02x ", *q++);
                --len;
            } else {
                printf("   ");
            }
            if(j == 7) putchar(' ');
        }
        printf("   ");
        for(; p < q; ++p) putchar(isprint(*p) ? *p : '.');
        putchar('\n');
        if(len && dlen) printf("%*s", dlen, "");
    }
}
