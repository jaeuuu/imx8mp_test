#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "audioctl2.h"

#define MAX_AUDIO2_MENU_DEPTH     3
static WINDOW *pr_win_media[MAX_AUDIO2_MENU_DEPTH];
static int pr_win_audio2_depth = 0;

static int audio2_exec(const char *cmd)
{
    pr_win(pr_win_media[pr_win_audio2_depth], "Executing command: %s\n\nPress ctrl-c to stop\n", cmd);
    system(cmd);
    pr_win(pr_win_media[pr_win_audio2_depth], "Command execution finished.\n");
    return 0;
}

static int play_music(const char *fname)
{
    char cmd_1per[256];
    char cmd_5per[256];
    char cmd_10per[256];
    char cmd_25per[256];
    char cmd_50per[256];
    char cmd_100per[256];

    snprintf(cmd_1per, sizeof(cmd_1per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=0.01 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);
    snprintf(cmd_5per, sizeof(cmd_5per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=0.05 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);
    snprintf(cmd_10per, sizeof(cmd_10per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=0.1 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);
    snprintf(cmd_25per, sizeof(cmd_25per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=0.25 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);
    snprintf(cmd_50per, sizeof(cmd_50per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=0.5 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);
    snprintf(cmd_100per, sizeof(cmd_100per), "gst-launch-1.0 playbin uri=file:///home/root/Musics/%s volume=1.0 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1", fname);

    menu_args_t play_music_vol_menu[] = {
        {audio2_exec, "VOL 1%", cmd_1per},
        {audio2_exec, "VOL 5%", cmd_5per},
        {audio2_exec, "VOL 10%", cmd_10per},
        {audio2_exec, "VOL 25%", cmd_25per},
        {audio2_exec, "VOL 50%", cmd_50per},
        {audio2_exec, "VOL 100%", cmd_100per},
        {back, "back", ""}
    };

    pr_win_audio2_depth++;
    menu_args_exec(play_music_vol_menu, sizeof(play_music_vol_menu) / sizeof(menu_args_t), "PLAY MUSIC VOLUME MENU", &pr_win_media[pr_win_audio2_depth]);
    pr_win_audio2_depth--;
    return 0;
}

static int name_filter(const struct dirent *entry)
{
    return (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."));
}

static int play_music_list(void)
{
    int i, nr_list;
    struct dirent **namelist;

    nr_list = scandir("/home/root/Musics", &namelist, name_filter, alphasort);

    if (nr_list < 0) {
        pr_win(pr_win_media[pr_win_audio2_depth], "Failed to read music directory.\n");
        return 0;
    }

    menu_args_t play_music_menu[nr_list + 1];
    for (i = 0; i < nr_list; i++) {
        play_music_menu[i].func = play_music;
        play_music_menu[i].func_des = namelist[i]->d_name;
        play_music_menu[i].args = namelist[i]->d_name;
    }
    play_music_menu[nr_list].func = back2;
    play_music_menu[nr_list].func_des = "back";
    play_music_menu[nr_list].args = NULL;

    pr_win_audio2_depth++;
    menu_args_exec(play_music_menu, nr_list + 1, "PLAY MUSIC LIST MENU", &pr_win_media[pr_win_audio2_depth]);
    pr_win_audio2_depth--;

    for (i = 0; i < nr_list; i++) {
        free(namelist[i]);
    }
    free(namelist);

    return 0;
}

static int play_tone(void)
{
    menu_args_t play_tone_menu[] = {
        {audio2_exec, "100 HZ", "speaker-test -D plughw:max98357aaudio,0 -t sine -f 100 > /dev/null 2>&1"},
        {audio2_exec, "250 HZ", "speaker-test -D plughw:max98357aaudio,0 -t sine -f 250 > /dev/null 2>&1"},
        {audio2_exec, "500 HZ", "speaker-test -D plughw:max98357aaudio,0 -t sine -f 500 > /dev/null 2>&1"},
        {audio2_exec, "1000 HZ", "speaker-test -D plughw:max98357aaudio,0 -t sine -f 1000 > /dev/null 2>&1"},
        {back, "back", ""}
    };

    pr_win_audio2_depth++;
    menu_args_exec(play_tone_menu, sizeof(play_tone_menu) / sizeof(menu_args_t), "PLAY TONE AUDIO MENU", &pr_win_media[pr_win_audio2_depth]);
    pr_win_audio2_depth--;
    return 0;
}

static int stream_audio(void)
{
    pr_win(pr_win_media[pr_win_audio2_depth], "Audio streaming started...\nPress ctrl-c to stop streaming.\n");
    system("gst-launch-1.0 alsasrc device=plughw:dir9001audio,0 ! audio/x-raw,rate=96000,channels=2,format=S32LE ! volume volume=0.5 ! alsasink device=plughw:max98357aaudio,0 sync=false > /dev/null 2>&1");
    pr_win(pr_win_media[pr_win_audio2_depth], "Audio streaming stopped.\n");
    return 0;
}

static menu_t audio2_menus[] = {
    {play_music_list, "PLAY MUSIC LIST"},
    {play_tone, "PLAY TONE AUDIO"},
    {stream_audio, "AUDIO STREAMING"},
    {back, "back"},
};

int audio2_ctl(void)
{
    char *des = "AUDIO TEST MENU";
    menu_exec(audio2_menus, sizeof(audio2_menus) / sizeof(menu_t), des, &pr_win_media[pr_win_audio2_depth]);
    return 0;
}

void audio2_init(void)
{
}