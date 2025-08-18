#define _GNU_SOURCE
#include <api/menu.h>
#include <api/uart.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include "audioctl2.h"

#define MAX_AUDIO2_MENU_DEPTH     2
static WINDOW *pr_win_audio2[MAX_AUDIO2_MENU_DEPTH];
static int pr_win_audio2_depth = 0;

static int audio2_exec(const char *cmd)
{
    pr_win(pr_win_audio2[pr_win_audio2_depth], "Executing command: %s\n\nPress ctrl-c to stop\n", cmd);
    system(cmd);
    pr_win(pr_win_audio2[pr_win_audio2_depth], "Command execution finished.\n");
    return 0;
}

static int play_music_video(void)
{
    menu_args_t play_music_video_menu[] = {
        {audio2_exec, "VOL 1%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=0.01 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {audio2_exec, "VOL 5%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=0.05 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {audio2_exec, "VOL 10%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=0.1 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {audio2_exec, "VOL 25%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=0.25 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {audio2_exec, "VOL 50%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=0.5 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {audio2_exec, "VOL 100%", "gst-launch-1.0 playbin uri=file:///home/root/MIDA-039.mp4 volume=1.0 audio-sink=\"pulsesink device=alsa_output.platform-sound-max98357a.stereo-fallback\" > /dev/null 2>&1"},
        {back, "back", ""}
    };

    pr_win_audio2_depth++;
    menu_args_exec(play_music_video_menu, sizeof(play_music_video_menu) / sizeof(menu_args_t), "PLAY MUSIC VIDEO MENU", &pr_win_audio2[pr_win_audio2_depth]);
    pr_win_audio2_depth--;
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
    menu_args_exec(play_tone_menu, sizeof(play_tone_menu) / sizeof(menu_args_t), "PLAY TONE AUDIO MENU", &pr_win_audio2[pr_win_audio2_depth]);
    pr_win_audio2_depth--;
    return 0;
}

static int stream_audio(void)
{
    pr_win(pr_win_audio2[pr_win_audio2_depth], "Audio streaming started...\nPress ctrl-c to stop streaming.\n");
    system("gst-launch-1.0 alsasrc device=plughw:dir9001audio,0 ! audio/x-raw,rate=96000,channels=1,format=S16LE ! volume volume=0.5 ! alsasink device=plughw:max98357aaudio,0 sync=false > /dev/null 2>&1");
    pr_win(pr_win_audio2[pr_win_audio2_depth], "Audio streaming stopped.\n");
    return 0;
}

static menu_t audio2_menus[] = {
    {play_music_video, "PLAY MUSIC VIDEO"},
    {play_tone, "PLAY TONE AUDIO"},
    {stream_audio, "AUDIO STREAMING"},
    {back, "back"},
};

int audio2_ctl(void)
{
    char *des = "AUDIO TEST MENU";
    menu_exec(audio2_menus, sizeof(audio2_menus) / sizeof(menu_t), des, &pr_win_audio2[pr_win_audio2_depth]);
    return 0;
}

void audio2_init(void)
{
}