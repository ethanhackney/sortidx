#include <algorithm>
#include <string>
#include <vector>

using std::pair;
using std::string;
using std::vector;

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <sysexits.h>

static FILE *e_fopen(const char *path, const char *mode);
static void e_fclose(const char *path, FILE *fp);

static string read_line(const char *path, FILE *fp);

static vector<string> fields(const string &line);

int main(int argc, char **argv)
{
        if (argc != 4)
                errx(EX_USAGE, "usage: a.out dbpath idxpath key");

        // open the database
        auto dbpath = argv[1];
        auto dbfp = e_fopen(dbpath, "r");

        // first line of database has attribute names
        auto attrline = read_line(dbpath, dbfp);
        auto attrnames = fields(attrline);

        // make sure this database has attribute we are keying on
        // and get the column number
        auto key = argv[3];
        auto p = std::find(attrnames.begin(), attrnames.end(), key);
        if (p == attrnames.end())
                errx(EX_USAGE, "%s does not have attribute %s", dbpath, key);
        auto column = p - attrnames.begin();

        // create the index
        auto idxpath = argv[2];
        auto idxfp = e_fopen(idxpath, "w");

        // build up index
        vector<pair<string,long>> index;
        string line;
        long curoff = ftell(dbfp);
        while ((line = read_line(dbpath, dbfp)).size() > 0) {
                auto attrvalues = fields(line);
                auto keyvalue = attrvalues[column];

                index.push_back({keyvalue, curoff});
                curoff = ftell(dbfp);
        }

        // now sort the index and write the contents to index file.
        // now other programs can use binary search when doing
        // queries for this key and can iterate through the
        // database in sorted order by using the index. however, if
        // the source database were to change, the index will have to be rebuilt
        std::sort(index.begin(), index.end());
        for (auto p : index)
                fprintf(idxfp, "%s %ld\n", p.first.c_str(), p.second);

        e_fclose(idxpath, idxfp);
        e_fclose(dbpath, dbfp);
}

static FILE *e_fopen(const char *path, const char *mode)
{
        auto fp = fopen(path, mode);
        if (fp == NULL)
                err(EX_SOFTWARE, "could not open %s", path);
        return fp;
}

static void e_fclose(const char *path, FILE *fp)
{
        if (fclose(fp) < 0)
                err(EX_SOFTWARE, "could not close %s", path);
}

static string read_line(const char *path, FILE *fp)
{
        string line = "";
        int c;

        while ((c = fgetc(fp)) != EOF) {
                line += c;
                if (c == '\n')
                        break;
        }
        if (ferror(fp))
                err(EX_SOFTWARE, "could not read %s", path);

        return line;
}

static vector<string> fields(const string &line)
{
        vector<string> f;
        string cur;
        cur = "";

        for (auto c : line) {
                if (!isspace(c)) {
                        cur += c;
                } else if (cur.size() > 0) {
                        f.push_back(cur);
                        cur = "";
                }
        }

        return f;
}
