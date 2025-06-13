#include "stoctl2.h"

static dir_info_t dirs[MAX_STO_NUM];
static int opend_dirs;

static int sto_init(void)
{
    int i;
    char path[32];

    // existing storage check.
    // path /run/media/sdx1
    // x is a, b, c, d .. etc.
    for (i = 0; i < MAX_STO_NUM; i++) {
        memset(path, 0x00, sizeof(path));
        sprintf(path, PATH_STORAGE"%c1", i+0x61);
        if (access(path, F_OK) < 0)
            continue;

        memcpy(dirs[opend_dirs].path, path, sizeof(path));
        dirs[opend_dirs].dp = opendir(dirs[opend_dirs].path);
        if (!dirs[opend_dirs].dp)
            continue;
        opend_dirs++;
    }
    return opend_dirs;
}

static void sto_destroy(void)
{
    int i;

    for (i = 0; i < opend_dirs; i++)
        if (dirs[i].dp)
            closedir(dirs[i].dp);

    opend_dirs = 0;
    memset(dirs, 0x00, sizeof(dirs));
}

static int sto_function(void *p)
{
    file_info_t *flp = (file_info_t *)p;
    struct dirent *ent;
    int i = 0;
    char tmp[MAX_TMP_SIZE];

    if (!flp)
        return -1;

    rewinddir(flp->dirp->dp);

    switch (flp->dirp->cmd) {
        case F_LIST:
            break;
        case F_READ:
            while ((ent = readdir(flp->dirp->dp)) != NULL) {
                if (i == flp->fnum && !strcmp(flp->fname, ent->d_name)) {
                    memset(tmp, 0x00, sizeof(tmp));
                    sprintf(tmp, "cat %s/%s", flp->dirp->path, flp->fname);
                    printf ("\n***********************************************\n");
                    printf ("\n      Read %s/%s\n", flp->dirp->path, flp->fname);
                    printf ("\n***********************************************\n");
                    if (system(tmp) < 0)
                        printf("%s command fail!\n", tmp);
                    break;
                }
                i++;
            }
            break;
        case F_CREAT:
            break;
        case F_REMOVE:
            while ((ent = readdir(flp->dirp->dp)) != NULL) {
                if (i == flp->fnum && !strcmp(flp->fname, ent->d_name)) {
                    memset(tmp, 0x00, sizeof(tmp));
                    sprintf(tmp, "rm %s/%s", flp->dirp->path, flp->fname);
                    printf ("\n***********************************************\n");
                    printf ("\n    Remove %s/%s\n", flp->dirp->path, flp->fname);
                    printf ("\n***********************************************\n");
                    if (system(tmp) < 0)
                        printf("%s command fail!\n", tmp);
                    sync();
                    break;
                }
                i++;
            }
            return -1;
        case F_WRITE:
            while ((ent = readdir(flp->dirp->dp)) != NULL) {
                if (i == flp->fnum && !strcmp(flp->fname, ent->d_name)) {
                    char c, txt[MAX_STRING_SIZE];
                    int num;
                    while (1) {
                        memset(txt, 0x00, sizeof(txt));
                        printf("\n\n***********************************************\n");
                        printf("\n\tInput format: [text]\n\n");
                        printf("   (ex) >> abcdefghijklmnopqrstuvwxyz12345...\n");
                        printf("\n***********************************************\n");
                        printf("|  0: Back\n");
                        printf("+---------------------------------------------+\n");
                        printf(">> ");

                        i = 0;
                        for (;;) {
                            if (scanf("%c", &c) < 0)
                                continue;
                            if (c == 0x0a)
                                break;
                            if (i < sizeof(txt))
                                txt[i++] = c;
                        }

                        if (system("clear") < 0)
                            printf("clear command fail\n");

                        // selection key check
                        num = check_ascii_num(txt, i);
                        if (num == 0)
                            return 0;

                        // write to file
                        memset(tmp, 0x00, sizeof(tmp));
                        sprintf(tmp, "echo %s >> %s/%s", txt, flp->dirp->path, flp->fname);
                        printf ("\n***********************************************\n");
                        printf ("\n    Write to %s/%s\n", flp->dirp->path, flp->fname);
                        printf ("\n***********************************************\n");
                        if (system(tmp) < 0)
                            printf("%s command fail!\n", tmp);
                        sync();
                        break;
                    }
                    break;
                }
                i++;
            }
            break;
        default:
            return -1;
    }
    return 0;
}

static int sto_file_list(void *p)
{
    int i;
    struct dirent *ent;
    dir_info_t *dirp = (dir_info_t *)p;
    file_info_t flp[MAX_FILE_NUM];
    menu_args_t menu_sto_file[MAX_FILE_NUM];

    if (!dirp)
        return -1;

    rewinddir(dirp->dp);

    i = 0;
    while ((ent = readdir(dirp->dp)) != NULL) {
        flp[i].dirp = dirp;
        flp[i].fnum = i;
        strcpy(flp[i].fname, ent->d_name);
        menu_sto_file[i].func = sto_function;
        menu_sto_file[i].func_des = flp[i].fname;
        menu_sto_file[i].args = &flp[i];
        i++;
    }

    if (dirp->cmd == F_CREAT) {
        char c, fname[MAX_STRING_SIZE], tmp[MAX_TMP_SIZE];
        int num;
        while (1) {
            memset(fname, 0x00, sizeof(fname));
            printf("\n\n***********************************************\n");
            printf("\n\tInput format: [file name]\n\n");
            printf("\t   (ex) >> hello.txt\n");
            printf("\n***********************************************\n");
            printf("|  0: Back\n");
            printf("+---------------------------------------------+\n");
            printf(">> ");

            i = 0;
            for (;;) {
                if (scanf("%c", &c) < 0)
                    continue;
                if (c == 0x0a)
                    break;
                if (i < sizeof(fname))
                    fname[i++] = c;
            }

            if (system("clear") < 0)
                printf("clear command fail!\n");

            num = check_ascii_num(fname, i);
            if (num == 0)
                return 0;

            memset(tmp, 0x00, sizeof(tmp));
            sprintf(tmp, "touch %s/%s", dirp->path, fname);
            printf ("\n***********************************************\n");
            printf ("\n    Create %s/%s\n", dirp->path, fname);
            printf ("\n***********************************************\n");
            if (system(tmp) < 0)
                printf("%s command fail!\n", tmp);
            sync();
            break;
        }
        return 0;
    }

    char *des = "\t\tFile List";
    menu_args_print(menu_sto_file, i, des);
    return 0;
}

static void sto_list(void)
{
    int i;
    menu_args_t menu_sto_list[MAX_STO_NUM];

    // existing storage check
    memset(menu_sto_list, 0x00, sizeof(menu_sto_list));
    for (i = 0; i < opend_dirs; i++) {
        dirs[i].cmd = F_LIST;
        menu_sto_list[i].func = sto_file_list;
        menu_sto_list[i].func_des = dirs[i].path;
        menu_sto_list[i].args = &dirs[i];
    }

    char *des = "\t\tStoarge List";
    menu_args_print(menu_sto_list, opend_dirs, des);
}

static void sto_read(void)
{
    int i;
    menu_args_t menu_sto_list[MAX_STO_NUM];

    // existing storage check
    memset(menu_sto_list, 0x00, sizeof(menu_sto_list));
    for (i = 0; i < opend_dirs; i++) {
        dirs[i].cmd = F_READ;
        menu_sto_list[i].func = sto_file_list;
        menu_sto_list[i].func_des = dirs[i].path;
        menu_sto_list[i].args = &dirs[i];
    }

    char *des = "\t\tStoarge List";
    menu_args_print(menu_sto_list, opend_dirs, des);
}

static void sto_create(void)
{
    int i;
    menu_args_t menu_sto_list[MAX_STO_NUM];

    // existing storage check
    memset(menu_sto_list, 0x00, sizeof(menu_sto_list));
    for (i = 0; i < opend_dirs; i++) {
        dirs[i].cmd = F_CREAT;
        menu_sto_list[i].func = sto_file_list;
        menu_sto_list[i].func_des = dirs[i].path;
        menu_sto_list[i].args = &dirs[i];
    }

    char *des = "\t\tStoarge List";
    menu_args_print(menu_sto_list, opend_dirs, des);
}


static void sto_write(void)
{
    int i;
    menu_args_t menu_sto_list[MAX_STO_NUM];

    // existing storage check
    memset(menu_sto_list, 0x00, sizeof(menu_sto_list));
    for (i = 0; i < opend_dirs; i++) {
        dirs[i].cmd = F_WRITE;
        menu_sto_list[i].func = sto_file_list;
        menu_sto_list[i].func_des = dirs[i].path;
        menu_sto_list[i].args = &dirs[i];
    }

    char *des = "\t\tStoarge List";
    menu_args_print(menu_sto_list, opend_dirs, des);
}

static void sto_remove(void)
{
    int i;
    menu_args_t menu_sto_list[MAX_STO_NUM];

    // existing storage check
    memset(menu_sto_list, 0x00, sizeof(menu_sto_list));
    for (i = 0; i < opend_dirs; i++) {
        dirs[i].cmd = F_REMOVE;
        menu_sto_list[i].func = sto_file_list;
        menu_sto_list[i].func_des = dirs[i].path;
        menu_sto_list[i].args = &dirs[i];
    }

    char *des = "\t\tStoarge List";
    menu_args_print(menu_sto_list, opend_dirs, des);
}

static menu_t sto_menus[] = {
    {sto_list, "LIST FILE"},
    {sto_create, "CREATE FILE"},
    {sto_read, "READ FILE"},
    {sto_write, "WRITE FILE"},
    {sto_remove, "DELETE FILE"},
};

void sto_control(void)
{
    // sto_init program exit issue
    // none device
    if (sto_init() == 0) {
        printf("\n\nPlease insert any USB storage device.\n\n");
        return;
    }
    char *des = "\t     Storage Control Menu";
    menu_print(sto_menus, sizeof(sto_menus)/sizeof(menu_t), des);
    sto_destroy();
}
