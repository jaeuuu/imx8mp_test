#include "stoctl.h"

static int sto_init(dir_info_t *dirs, int size)
{
    int i, j = 0;
    char path[32];

    // existing storage check.
    // path /run/media/sdx1
    // x is a, b, c, d .. etc.
    for (i = 0; i < size; i++) {
        memset(path, 0x00, sizeof(path));
        sprintf(path, PATH_STORAGE"%c1", i+0x61);
        if (access(path, F_OK) < 0)
            continue;

        memcpy(dirs[j].path, path, sizeof(path));
        dirs[j].dp = opendir(dirs[j].path);
        if (!dirs[j].dp)
            continue;
        j++;
    }
    return j;
}

static int sto_file_list(DIR *dp)
{
    struct dirent *ent;
    int i, file_num, key_num;
    char c, key[4];

    if (!dp)
        return -1;

    while(1) {
        memset(key, 0x00, sizeof(key));
        printf ("\n***********************************************\n");
        printf ("\n\t\t  File List\n");
        printf ("\n***********************************************\n");

        rewinddir(dp);
        file_num = 0;
        while ((ent = readdir(dp)) != NULL) {
            printf("| %2d: %s\n", file_num+1, ent->d_name);
            file_num++;
        }
        printf("|  0: Exit\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            scanf("%c", &c);
            if (c == 0x0a)
                break;
            if (i < sizeof(key))
                key[i++] = c;
        }

        system("clear");

        // selection key check
        key_num = check_ascii_num(key, i);
        if (key_num == 0)
            return -1;
        if (key_num < 0 || key_num > file_num)
            continue;

        // key_num is '1' start, index is '0' start. So, level 1 decrease.
        return --key_num;
    }
}

static int sto_list(dir_info_t *dirs, int size)
{
    int i, key_num;
    char c, key[4];

    while (1) {
        memset(key, 0x00, sizeof(key));
        printf ("\n***********************************************\n");
        printf ("\n\t\tStorage List\n");
        printf ("\n***********************************************\n");
        for (i = 0; i < size; i++) {
            printf ("| %2d: %s\n", i+1, dirs[i].path);
        }
        printf("|  0: Exit\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        // selection key input check
        i = 0;
        for (;;) {
            scanf("%c", &c);
            if (c == 0x0a)
                break;
            if (i < sizeof(key))
                key[i++] = c;
        }
        system("clear");

        // selection key check
        key_num = check_ascii_num(key, i);
        if (key_num == 0)
            return -1;
        if (key_num < 0 || key_num > size)
            continue;

        // key_num is '1' start, index is '0' start. So, level 1 decrease.
        return --key_num;
    }
}

static void sto_print(void)
{
    int i, ret, dir_num, file_num;
    dir_info_t dirs[MAX_STO_NUM];

    // existing storage check
    memset(dirs, 0x00, sizeof(dirs));
    ret = sto_init(dirs, MAX_STO_NUM);
    if (!ret) {
        printf("\nNo search storage.\n");
        return;
    }

    // storage list check
    dir_num = sto_list(dirs, ret);
    if (dir_num < 0)
        goto err;

    // target storage file list check
    file_num = sto_file_list(dirs[dir_num].dp);

err:
    // close dir pointer opend
    for (i = 0; i < MAX_STO_NUM; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);
}

static void sto_write(void)
{
    int i, ret, dir_num, file_num, num;
    dir_info_t dirs[MAX_STO_NUM];
    struct dirent *ent;
    char c, tmp[5192], txt[4096];

    // existing storage check
    memset(dirs, 0x00, sizeof(dirs));
    ret = sto_init(dirs, MAX_STO_NUM);
    if (!ret) {
        printf("\nNo search storage.\n");
        return;
    }

    // storage list check
    dir_num = sto_list(dirs, ret);
    if (dir_num < 0)
        goto err;

    // target storage file list check
    file_num = sto_file_list(dirs[dir_num].dp);
    if (file_num < 0)
        goto err;

    // dir pointer set to first offset
    rewinddir(dirs[dir_num].dp);

    // write to selection file
    i = 0;
    while ((ent = readdir(dirs[dir_num].dp)) != NULL) {
        if (i == file_num) {
            while (1) {
                memset(txt, 0x00, sizeof(txt));
                printf("\n\n***********************************************\n");
                printf("\n\tInput format: [text]\n\n");
                printf("   (ex) >> abcdefghijklmnopqrstuvwxyz12345...\n");
                printf("\n***********************************************\n");
                printf("|  0: Exit\n");
                printf("+---------------------------------------------+\n");
                printf(">> ");

                i = 0;
                for (;;) {
                    scanf("%c", &c);
                    if (c == 0x0a)
                        break;
                    if (i < sizeof(txt))
                        txt[i++] = c;
                }

                system("clear");

                // selection key check
                num = check_ascii_num(txt, i);
                if (num == 0)
                    return;

                // write to file
                memset(tmp, 0x00, sizeof(tmp));
                sprintf(tmp, "echo %s >> %s/%s", txt, dirs[dir_num].path, ent->d_name);
                printf ("\n***********************************************\n");
                printf ("\n\t  %s/%s\n", dirs[dir_num].path, ent->d_name);
                printf ("\n***********************************************\n");
                system(tmp);
                system("sync");
                break;
            }
            break;
        }
        i++;
    }

err:
    for (i = 0; i < MAX_STO_NUM; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);
}

static void sto_read(void)
{
    int i, ret, dir_num, file_num;
    dir_info_t dirs[MAX_STO_NUM];
    struct dirent *ent;
    char tmp[512];

    // existing storage check
    memset(dirs, 0x00, sizeof(dirs));
    ret = sto_init(dirs, MAX_STO_NUM);
    if (!ret) {
        printf("\nNo search storage.\n");
        return;
    }

    // storage list check
    dir_num = sto_list(dirs, ret);
    if (dir_num < 0)
        goto err;

    // file list check
    file_num = sto_file_list(dirs[dir_num].dp);
    if (file_num < 0)
        goto err;

    // dir pointer set to first offset
    rewinddir(dirs[dir_num].dp);

    // read file
    i = 0;
    while ((ent = readdir(dirs[dir_num].dp)) != NULL) {
        if (i == file_num) {
            memset(tmp, 0x00, sizeof(tmp));
            sprintf(tmp, "cat %s/%s", dirs[dir_num].path, ent->d_name);
            printf ("\n***********************************************\n");
            printf ("\n\t  %s/%s\n", dirs[dir_num].path, ent->d_name);
            printf ("\n***********************************************\n");
            system(tmp);
            break;
        }
        i++;
    }

err:
    for (i = 0; i < MAX_STO_NUM; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);

}

static void sto_create(void)
{
    int i, ret, dir_num, num;
    dir_info_t dirs[MAX_STO_NUM];
    char c, fname[256], tmp[512];

    // existing storage check
    memset(dirs, 0x00, sizeof(dirs));
    ret = sto_init(dirs, MAX_STO_NUM);
    if (!ret) {
        printf("\nNo search storage.\n");
        return;
    }

    // storage list check
    dir_num = sto_list(dirs, ret);
    if (dir_num < 0)
        goto err;

    // create file on target stoarge
    while (1) {
        memset(fname, 0x00, sizeof(fname));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [file name]\n\n");
        printf("\t   (ex) >> hello.txt\n");
        printf("\n***********************************************\n");
        printf("|  0: Exit\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            scanf("%c", &c);
            if (c == 0x0a)
                break;
            if (i < sizeof(fname))
                fname[i++] = c;
        }

        system("clear");

        num = check_ascii_num(fname, i);
        if (num == 0)
            return;

        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "touch %s/%s", dirs[dir_num].path, fname);
        system(tmp);
        system("sync");
        break;
    }
err:
    for (i = 0; i < MAX_STO_NUM; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);
}

static void sto_remove(void)
{
    int i, ret, dir_num, file_num;
    dir_info_t dirs[MAX_STO_NUM];
    struct dirent *ent;
    char tmp[512];

    // existing storage check
    memset(dirs, 0x00, sizeof(dirs));
    ret = sto_init(dirs, MAX_STO_NUM);
    if (!ret) {
        printf("\nNo search storage.\n");
        return;
    }

    // storage list check
    dir_num = sto_list(dirs, ret);
    if (dir_num < 0)
        goto err;

    // file list check
    file_num = sto_file_list(dirs[dir_num].dp);
    if (file_num < 0)
        goto err;

    // dir pointer set to first offset
    rewinddir(dirs[dir_num].dp);

    // remove file
    i = 0;
    while ((ent = readdir(dirs[dir_num].dp)) != NULL) {
        if (i == file_num) {
            memset(tmp, 0x00, sizeof(tmp));
            sprintf(tmp, "rm %s/%s", dirs[dir_num].path, ent->d_name);
            printf ("\n***********************************************\n");
            printf ("\n\t  %s/%s\n", dirs[dir_num].path, ent->d_name);
            printf ("\n***********************************************\n");
            system(tmp);
            system("sync");
            break;
        }
        i++;
    }


err:
    for (i = 0; i < MAX_STO_NUM; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);
}

static menu_t sto_menus [] = {
    {sto_print, "LIST FILE"},
    {sto_create, "CREATE FILE"},
    {sto_read, "READ FILE"},
    {sto_write, "WRITE FILE"},
    {sto_remove, "DELETE FILE"},
};

void sto_control(void)
{
    char *des = "\t     Storage Control Menu";
    menu_print(sto_menus, sizeof(sto_menus)/sizeof(menu_t), des);
}
