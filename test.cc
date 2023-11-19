// this program is using an index that was previously
// built. it iterates through the database in sorted
// order on the NAME attribute.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
        FILE *dbfp = fopen("db", "r");
        FILE *idxfp = fopen("nameidx", "r");
        char buf[BUFSIZ];

        while (fgets(buf, sizeof(buf), idxfp)) {
                char *bufp = buf;
                char *key = strsep(&bufp, " ");
                char *offset = strsep(&bufp, "\n");
                long off = strtol(offset, NULL, 10);
                fseek(dbfp, off, SEEK_SET);
                fgets(buf, sizeof(buf), dbfp);
                printf("%s", buf);
        }

        fclose(dbfp);
        fclose(idxfp);
}
