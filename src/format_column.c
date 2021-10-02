#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include "format_column.h"
#include "common.h"
#include "format_long.h"
#include "list.h"
#include "display.h"
#include "types.h"

void column_display(linklist l, int *ls, int *lm, _file_info *files_info, int index, int *ts, int *tm, int cl, int ln)
{
    int x;
    _file t;

    for (int i = 0; i < ln; i++)
    {
        for (int j = 0; j < cl; j++)
        {
            x = j * ln + i;
            if (x < l->count)
            {
                // if options -s, -p, -m, -u or -g is set
                t = (_file)lget(l, x);
                if (files_info)
                {
                    long_print(files_info[x].bfr[index], tm[j] - 1, 1);
                }
                display(t->name, &t->st.st_mode, _false);
                x = lm[j] - ls[x];
                for (int k = 0; k < x; k++)
                { // the +1 is for the last space between columns.
                    printf(" ");
                }
                if (j != cl - 1)
                { // don't add space at the last column
                    printf("  ");
                }
            }
        }
        printf("\n");
    }
}

void column_main(linklist l, _file_info *files_info, int index)
{
    struct winsize w;
    int cl = l->count, ln = 1, winsiz, ok = 0;
    unsigned long int cnt;
    int *ls;        // list sizes
    int *lm;        // list max sizes
    int *ts = NULL; // array sizes of "tb"
    int *tm = NULL; // array max sizes of "tb"
    int x, y = 0;
    iterator it;
    _file file;
    int k;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    winsiz = w.ws_col;

    ls = (int *)_alloc(sizeof(int) * l->count);
    lm = (int *)_alloc(sizeof(int) * l->count);
    if (files_info)
    {
        ts = (int *)_alloc(sizeof(int) * l->count);
        tm = (int *)_alloc(sizeof(int) * l->count);
    }
    // length of each name.
    it = lat(l, LFIRST);
    int nb_spaces;
    for (int i = 0; i < l->count; i++)
    {
        file = (_file)it->data;
        ls[i] = str_width(file->name);
        nb_spaces = has_space(file->name);
        if (nb_spaces)
        {
            if (_opt.nl->b)
            {
                ls[i] += nb_spaces;
            }
            if (_opt.nl->q)
            {
                ls[i] += 2;
            }
        }
        if (_opt.nl->i && (S_ISDIR(file->st.st_mode) ||
                           S_ISLNK(file->st.st_mode) ||
                           S_ISSOCK(file->st.st_mode) ||
                           S_ISFIFO(file->st.st_mode) ||
                           (S_IEXEC & file->st.st_mode)))
        {
            ls[i] += 1;
        }
        if (files_info)
        {
            ts[i] = strlen(files_info[i].bfr[index]) + 1; // +1 for space between "tb" and "l"
        }
        linc(&it);
    }
    while (!ok)
    {
        cnt = 0;
        for (int i = 0; i < cl; i++)
        { // for each column "i", calculates the maximum of that column
            x = ls[i * ln];
            if (files_info)
            {
                y = ts[i * ln];
            }
            for (int j = 1; j < ln; j++)
            {
                k = i * ln + j;
                if (k < l->count)
                {
                    if (ls[k] > x)
                    {
                        x = ls[k];
                    }
                    if (files_info && (ts[k] > y))
                    {
                        y = ts[k];
                    }
                }
            }
            if (files_info)
            {
                cnt += x + y;
                tm[i] = y;
            }
            else
            {
                cnt += x;
            }
            lm[i] = x;
        }
        cnt += 2 * (cl - 1); // space between columns
        if (cnt <= winsiz)
        {
            ok = 1;
        }
        else
        {
            cl--;
            if (l->count % cl)
            {
                ln = l->count / cl + 1;
            }
            else
            {
                ln = l->count / cl;
            }
        }
    }
    column_display(l, ls, lm, files_info, index, ts, tm, cl, ln);
    free(ls);
    free(ts);
    free(lm);
    free(tm);
}
