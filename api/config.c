#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sqlite3.h>
#include <api/debug.h>
#include <api/db.h>
#include <api/config.h>
#include <api/utils.h>

static sqlite3 *cfg_db;
static cfg_t cfg;

void *get_ptr_cfg(cfg_idx_t idx)
{
    switch (idx) {
    case CFG_BROAD_AL1_REAL:
        return &cfg.al1.real;
    case CFG_BROAD_AL1_TRAIN:
        return &cfg.al1.train;
    case CFG_BROAD_AL1_TEST:
        return &cfg.al1.test;
    case CFG_BROAD_AL2_REAL:
        return &cfg.al2.real;
    case CFG_BROAD_AL2_TRAIN:
        return &cfg.al2.train;
    case CFG_BROAD_AL2_TEST:
        return &cfg.al2.test;
    case CFG_BROAD_AL3_REAL:
        return &cfg.al3.real;
    case CFG_BROAD_AL3_TRAIN:
        return &cfg.al3.train;
    case CFG_BROAD_AL3_TEST:
        return &cfg.al3.test;
    case CFG_BROAD_DPA_REAL:
        return &cfg.dpa.real;
    case CFG_BROAD_DPA_TRAIN:
        return &cfg.dpa.train;
    case CFG_BROAD_DPA_TEST:
        return &cfg.dpa.test;
    case CFG_BROAD_CBR_REAL:
        return &cfg.cbr.real;
    case CFG_BROAD_CBR_TRAIN:
        return &cfg.cbr.train;
    case CFG_BROAD_CBR_TEST:
        return &cfg.cbr.test;
    case CFG_BROAD_CLR_REAL:
        return &cfg.clr.real;
    case CFG_BROAD_CLR_TRAIN:
        return &cfg.clr.train;
    case CFG_BROAD_CLR_TEST:
        return &cfg.clr.test;
    case CFG_BROAD_ACL_REAL:
        return &cfg.acl.real;
    case CFG_BROAD_ACL_TRAIN:
        return &cfg.acl.train;
    case CFG_BROAD_ACL_TEST:
        return &cfg.acl.test;
    case CFG_BROAD_MSR_REAL:
        return &cfg.msr.real;
    case CFG_BROAD_MSR_TRAIN:
        return &cfg.msr.train;
    case CFG_BROAD_MSR_TEST:
        return &cfg.msr.test;
    case CFG_BROAD_ADR_REAL:
        return &cfg.adr.real;
    case CFG_BROAD_ADR_TRAIN:
        return &cfg.adr.train;
    case CFG_BROAD_ADR_TEST:
        return &cfg.adr.test;
    case CFG_BROAD_A00_REAL:
        return &cfg.a00.real;
    case CFG_BROAD_A00_TRAIN:
        return &cfg.a00.train;
    case CFG_BROAD_A00_TEST:
        return &cfg.a00.test;
    case CFG_BROAD_A01_REAL:
        return &cfg.a01.real;
    case CFG_BROAD_A01_TRAIN:
        return &cfg.a01.train;
    case CFG_BROAD_A01_TEST:
        return &cfg.a01.test;
    case CFG_BROAD_A02_REAL:
        return &cfg.a02.real;
    case CFG_BROAD_A02_TRAIN:
        return &cfg.a02.train;
    case CFG_BROAD_A02_TEST:
        return &cfg.a02.test;
    case CFG_BROAD_A03_REAL:
        return &cfg.a03.real;
    case CFG_BROAD_A03_TRAIN:
        return &cfg.a03.train;
    case CFG_BROAD_A03_TEST:
        return &cfg.a03.test;
    case CFG_BROAD_A04_REAL:
        return &cfg.a04.real;
    case CFG_BROAD_A04_TRAIN:
        return &cfg.a04.train;
    case CFG_BROAD_A04_TEST:
        return &cfg.a04.test;
    case CFG_BROAD_A05_REAL:
        return &cfg.a05.real;
    case CFG_BROAD_A05_TRAIN:
        return &cfg.a05.train;
    case CFG_BROAD_A05_TEST:
        return &cfg.a05.test;
    case CFG_BROAD_A06_REAL:
        return &cfg.a06.real;
    case CFG_BROAD_A06_TRAIN:
        return &cfg.a06.train;
    case CFG_BROAD_A06_TEST:
        return &cfg.a06.test;
    case CFG_BROAD_A07_REAL:
        return &cfg.a07.real;
    case CFG_BROAD_A07_TRAIN:
        return &cfg.a07.train;
    case CFG_BROAD_A07_TEST:
        return &cfg.a07.test;
    case CFG_BROAD_A08_REAL:
        return &cfg.a08.real;
    case CFG_BROAD_A08_TRAIN:
        return &cfg.a08.train;
    case CFG_BROAD_A08_TEST:
        return &cfg.a08.test;
    case CFG_BROAD_A09_REAL:
        return &cfg.a09.real;
    case CFG_BROAD_A09_TRAIN:
        return &cfg.a09.train;
    case CFG_BROAD_A09_TEST:
        return &cfg.a09.test;
    case CFG_BROAD_A10_REAL:
        return &cfg.a10.real;
    case CFG_BROAD_A10_TRAIN:
        return &cfg.a11.train;
    case CFG_BROAD_A10_TEST:
        return &cfg.a10.test;
    case CFG_BROAD_A11_REAL:
        return &cfg.a11.real;
    case CFG_BROAD_A11_TRAIN:
        return &cfg.a11.train;
    case CFG_BROAD_A11_TEST:
        return &cfg.a11.test;
    case CFG_BROAD_A12_REAL:
        return &cfg.a12.real;
    case CFG_BROAD_A12_TRAIN:
        return &cfg.a12.train;
    case CFG_BROAD_A12_TEST:
        return &cfg.a12.test;
    case CFG_BROAD_A13_REAL:
        return &cfg.a13.real;
    case CFG_BROAD_A13_TRAIN:
        return &cfg.a13.train;
    case CFG_BROAD_A13_TEST:
        return &cfg.a13.test;
    case CFG_BROAD_A14_REAL:
        return &cfg.a14.real;
    case CFG_BROAD_A14_TRAIN:
        return &cfg.a14.train;
    case CFG_BROAD_A14_TEST:
        return &cfg.a14.test;
    case CFG_BROAD_MANUAL:
        return &cfg.manual;
    case CFG_VOLUME:
        return &cfg.volume;
    case CFG_ETH0:
        return &cfg.eth0;
    case CFG_ETH1:
        return &cfg.eth1;
    case CFG_WWAN:
        return &cfg.wwan;
    case CFG_CERT:
        return &cfg.cert;
    case CFG_SECURITY:
        return &cfg.security;
    case CFG_VPN:
        return &cfg.vpn;
    case CFG_SERV1:
        return &cfg.serv1;
    case CFG_SERV2:
        return &cfg.serv2;
    case CFG_SERV3:
        return &cfg.serv3;
    case CFG_LTE_GENERAL:
        return &cfg.lte_gen;
    case CFG_LTE_ARS:
        return &cfg.ars;
    case CFG_AMP_GENERAL:
        return &cfg.amp_gen;
    case CFG_TERM_GENERAL:
        return &cfg.term_gen;
    default:
        dlp_warn("unknown config index! (index: [%d])", idx);
        return NULL;
    }
}

static int cb_load_cfg_term_gen(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_term_gen_t *gen = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_TERM_KIND:
        gen->kind = atoi(argv[0]);
        break;
    case CFG_VAL_TERM_MEDIA:
        gen->media = atoi(argv[0]);
        break;
    case CFG_VAL_SIREN_BULB:
        gen->siren_bulb = atoi(argv[0]);
        break;
    case CFG_VAL_ELEC_DISP:
        gen->elec_disp = atoi(argv[0]);
        break;
    case CFG_VAL_PA_SYSTEM:
        gen->pa_system = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_amp_gen(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_amp_gen_t *gen = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_DELAY_TIME:
        gen->delay = atoi(argv[0]);
        break;
    case CFG_VAL_AMP_KIND:
        gen->kind = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_act(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_broad_act_t *act = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_CTRL:
        act->ctrl = atoi(argv[0]);
        break;
    case CFG_VAL_LINE_MANUAL:
        act->line_manual = atoi(argv[0]);
        break;
    case CFG_VAL_SMS_MANUAL:
        act->sms_manual = atoi(argv[0]);
        break;
    case CFG_VAL_BROAD_TEXT:
        act->broad_text = atoi(argv[0]);
        break;
    case CFG_VAL_BOARD_TEXT:
        act->board_text = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_manual(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_broad_manual_t *manual = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_TIMEOUT:
        manual->timeout = atoi(argv[0]);
        break;
    case CFG_VAL_TIMEOUT_ACT:
        manual->timeout_act = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_vol(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_vol_t *volume = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_INPUT_VOL_STO:
        volume->input.sto = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_AOIP:
        volume->input.aoip = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_TTS:
        volume->input.tts = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_ARS:
        volume->input.ars = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_XROSHOT:
        volume->input.xroshot = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_SPARE:
        volume->input.spare = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_FM:
        volume->input.fm = atoi(argv[0]);
        break;
    case CFG_VAL_INPUT_VOL_UHD:
        volume->input.uhd = atoi(argv[0]);
        break;
    case CFG_VAL_OUTPUT_VOL_LTEMIC:
        volume->output.lte_mic = atoi(argv[0]);
        break;
    case CFG_VAL_OUTPUT_VOL_AMP:
        volume->output.amp = atoi(argv[0]);
        break;
    case CFG_VAL_OUTPUT_VOL_SPDIF:
        volume->output.spdif = atoi(argv[0]);
        break;
    case CFG_VAL_OUTPUT_VOL_SPARE:
        volume->output.spare = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_eth(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_eth_t *eth = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_USE:
        eth->use = atoi(argv[0]);
        break;
    case CFG_VAL_DHCP:
        eth->dhcp = atoi(argv[0]);
        break;
    case CFG_VAL_IP:
        strcpy(eth->ip, argv[0]);
        break;
    case CFG_VAL_GW:
        strcpy(eth->gw, argv[0]);
        break;
    case CFG_VAL_SUB:
        strcpy(eth->sub, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_wwan(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_wwan_t *wwan = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_USE:
        wwan->use = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_cert(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_cert_t *cert = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_IP:
        strcpy(cert->ip, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_secu(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_secu_t *secu = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_SECURITY:
        secu->security = atoi(argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_vpn(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_vpn_t *vpn = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_USE:
        vpn->use = atoi(argv[0]);
        break;
    case CFG_VAL_PORT:
        vpn->port = atoi(argv[0]);
        break;
    case CFG_VAL_IP:
        strcpy(vpn->ip, argv[0]);
        break;
    case CFG_VAL_ID:
        strcpy(vpn->id, argv[0]);
        break;
    case CFG_VAL_PW:
        strcpy(vpn->pw, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_serv(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_serv_t *serv = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_USE:
        serv->use = atoi(argv[0]);
        break;
    case CFG_VAL_PORT:
        serv->port = atoi(argv[0]);
        break;
    case CFG_VAL_IP:
        strcpy(serv->ip, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_lte_gen(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_lte_gen_t *gen = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_USE:
        gen->use = atoi(argv[0]);
        break;
    case CFG_VAL_RESP_NUM1:
        strcpy(gen->resp_num1, argv[0]);
        break;
    case CFG_VAL_RESP_NUM2:
        strcpy(gen->resp_num2, argv[0]);
        break;
    case CFG_VAL_XROSHOT_NUM:
        strcpy(gen->xroshot_num, argv[0]);
        break;
    case CFG_VAL_ADMIN_NUM:
        strcpy(gen->admin_num, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static int cb_load_cfg_ars(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    cfg_lte_ars_t *ars = get_ptr_cfg(cmd->idx);

    if (argc != 1) {
        dlp_warn("why column is not one? please check sqlite DB");
        return -1;
    }

    dlp_dbg("\"%s = %s\"", col_name[0], argv[0]);

    switch (cmd->val_idx) {
    case CFG_VAL_NUM1:
        strcpy(ars->num1, argv[0]);
        break;
    case CFG_VAL_NUM2:
        strcpy(ars->num2, argv[0]);
        break;
    case CFG_VAL_NUM3:
        strcpy(ars->num3, argv[0]);
        break;
    case CFG_VAL_NUM4:
        strcpy(ars->num4, argv[0]);
        break;
    case CFG_VAL_NUM5:
        strcpy(ars->num5, argv[0]);
        break;
    case CFG_VAL_NUM6:
        strcpy(ars->num6, argv[0]);
        break;
    case CFG_VAL_NUM7:
        strcpy(ars->num7, argv[0]);
        break;
    case CFG_VAL_NUM8:
        strcpy(ars->num8, argv[0]);
        break;
    case CFG_VAL_NUM9:
        strcpy(ars->num9, argv[0]);
        break;
    case CFG_VAL_NUM10:
        strcpy(ars->num10, argv[0]);
        break;
    default:
        dlp_warn("unknown config value index! (index: [%d])", cmd->val_idx);
        return -1;
    }
    return 0;
}

static void *get_ptr_cb_load(cfg_idx_t idx)
{
    switch (idx) {
    case CFG_BROAD_AL1_REAL:
    case CFG_BROAD_AL1_TRAIN:
    case CFG_BROAD_AL1_TEST:
    case CFG_BROAD_AL2_REAL:
    case CFG_BROAD_AL2_TRAIN:
    case CFG_BROAD_AL2_TEST:
    case CFG_BROAD_AL3_REAL:
    case CFG_BROAD_AL3_TRAIN:
    case CFG_BROAD_AL3_TEST:
    case CFG_BROAD_DPA_REAL:
    case CFG_BROAD_DPA_TRAIN:
    case CFG_BROAD_DPA_TEST:
    case CFG_BROAD_CBR_REAL:
    case CFG_BROAD_CBR_TRAIN:
    case CFG_BROAD_CBR_TEST:
    case CFG_BROAD_CLR_REAL:
    case CFG_BROAD_CLR_TRAIN:
    case CFG_BROAD_CLR_TEST:
    case CFG_BROAD_ACL_REAL:
    case CFG_BROAD_ACL_TRAIN:
    case CFG_BROAD_ACL_TEST:
    case CFG_BROAD_MSR_REAL:
    case CFG_BROAD_MSR_TRAIN:
    case CFG_BROAD_MSR_TEST:
    case CFG_BROAD_ADR_REAL:
    case CFG_BROAD_ADR_TRAIN:
    case CFG_BROAD_ADR_TEST:
    case CFG_BROAD_A00_REAL:
    case CFG_BROAD_A00_TRAIN:
    case CFG_BROAD_A00_TEST:
    case CFG_BROAD_A01_REAL:
    case CFG_BROAD_A01_TRAIN:
    case CFG_BROAD_A01_TEST:
    case CFG_BROAD_A02_REAL:
    case CFG_BROAD_A02_TRAIN:
    case CFG_BROAD_A02_TEST:
    case CFG_BROAD_A03_REAL:
    case CFG_BROAD_A03_TRAIN:
    case CFG_BROAD_A03_TEST:
    case CFG_BROAD_A04_REAL:
    case CFG_BROAD_A04_TRAIN:
    case CFG_BROAD_A04_TEST:
    case CFG_BROAD_A05_REAL:
    case CFG_BROAD_A05_TRAIN:
    case CFG_BROAD_A05_TEST:
    case CFG_BROAD_A06_REAL:
    case CFG_BROAD_A06_TRAIN:
    case CFG_BROAD_A06_TEST:
    case CFG_BROAD_A07_REAL:
    case CFG_BROAD_A07_TRAIN:
    case CFG_BROAD_A07_TEST:
    case CFG_BROAD_A08_REAL:
    case CFG_BROAD_A08_TRAIN:
    case CFG_BROAD_A08_TEST:
    case CFG_BROAD_A09_REAL:
    case CFG_BROAD_A09_TRAIN:
    case CFG_BROAD_A09_TEST:
    case CFG_BROAD_A10_REAL:
    case CFG_BROAD_A10_TRAIN:
    case CFG_BROAD_A10_TEST:
    case CFG_BROAD_A11_REAL:
    case CFG_BROAD_A11_TRAIN:
    case CFG_BROAD_A11_TEST:
    case CFG_BROAD_A12_REAL:
    case CFG_BROAD_A12_TRAIN:
    case CFG_BROAD_A12_TEST:
    case CFG_BROAD_A13_REAL:
    case CFG_BROAD_A13_TRAIN:
    case CFG_BROAD_A13_TEST:
    case CFG_BROAD_A14_REAL:
    case CFG_BROAD_A14_TRAIN:
    case CFG_BROAD_A14_TEST:
        return cb_load_cfg_act;
    case CFG_BROAD_MANUAL:
        return cb_load_cfg_manual;
    case CFG_VOLUME:
        return cb_load_cfg_vol;
    case CFG_ETH0:
    case CFG_ETH1:
        return cb_load_cfg_eth;
    case CFG_WWAN:
        return cb_load_cfg_wwan;
    case CFG_CERT:
        return cb_load_cfg_cert;
    case CFG_SECURITY:
        return cb_load_cfg_secu;
    case CFG_VPN:
        return cb_load_cfg_vpn;
    case CFG_SERV1:
    case CFG_SERV2:
    case CFG_SERV3:
        return cb_load_cfg_serv;
    case CFG_LTE_GENERAL:
        return cb_load_cfg_lte_gen;
    case CFG_LTE_ARS:
        return cb_load_cfg_ars;
    case CFG_AMP_GENERAL:
        return cb_load_cfg_amp_gen;
    case CFG_TERM_GENERAL:
        return cb_load_cfg_term_gen;
    default:
        return NULL;
    }
}
/*
static int cb_save_cfg_code(void *data, int argc, char **argv, char **col_name)
{
    cfg_cmd_t *cmd = data;
    return 9;
}

static int cb_save_cfg_manual(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_vol(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_eth(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_wwan(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_cert(void *data, int argc, char **argv, char **col_name)
{
    return 9;
}

static int cb_save_cfg_secu(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_vpn(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_serv(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_lte_gen(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}

static int cb_save_cfg_ars(void *data, int argc, char **argv, char **col_name)
{
    return 0;
}
*/

static int cb_dummy(void *data, int argc, char **argv, char **col_name)
{
    int i = 0;

    for (i = 0; i < argc; i++) {
        dlp_dbg("col_name: [%s], argv: [%s]", col_name[i], argv[i]);
    }

    return 0;
}
/*
static void *get_ptr_cb_save(cfg_idx_t idx)
{
    switch (idx) {
    case CFG_BROAD_AL1:
    case CFG_BROAD_AL2:
    case CFG_BROAD_AL3:
    case CFG_BROAD_DPA:
    case CFG_BROAD_CBR:
    case CFG_BROAD_CLR:
    case CFG_BROAD_ACL:
    case CFG_BROAD_MSR:
    case CFG_BROAD_ADR:
    case CFG_BROAD_A00:
    case CFG_BROAD_A01:
    case CFG_BROAD_A02:
    case CFG_BROAD_A03:
    case CFG_BROAD_A04:
    case CFG_BROAD_A05:
    case CFG_BROAD_A06:
    case CFG_BROAD_A07:
    case CFG_BROAD_A08:
    case CFG_BROAD_A09:
    case CFG_BROAD_A10:
    case CFG_BROAD_A11:
    case CFG_BROAD_A12:
    case CFG_BROAD_A13:
    case CFG_BROAD_A14:
        return cb_save_cfg_code;
    case CFG_BROAD_MANUAL:
        return cb_save_cfg_manual;
    case CFG_BROAD_VOLUME:
        return cb_save_cfg_vol;
    case CFG_LINE_ETH0:
    case CFG_LINE_ETH1:
        return cb_save_cfg_eth;
    case CFG_LINE_WWAN:
        return cb_save_cfg_wwan;
    case CFG_LINE_CERT:
        return cb_save_cfg_cert;
    case CFG_LINE_SECURITY:
        return cb_save_cfg_secu;
    case CFG_LINE_VPN:
        return cb_save_cfg_vpn;
    case CFG_LINE_SERV1:
    case CFG_LINE_SERV2:
    case CFG_LINE_SERV3:
        return cb_save_cfg_serv;
    case CFG_LTE_GENERAL:
        return cb_save_cfg_lte_gen;
    case CFG_LTE_ARS:
        return cb_save_cfg_ars;
    default:
        return NULL;
    }
}
*/

static char *cfg_idx_to_tb(cfg_idx_t idx)
{
    switch (idx) {
    case CFG_BROAD_AL1_REAL:
        return CFG_NAME_BROAD_AL1_REAL;
    case CFG_BROAD_AL1_TRAIN:
        return CFG_NAME_BROAD_AL1_TRAIN;
    case CFG_BROAD_AL1_TEST:
        return CFG_NAME_BROAD_AL1_TEST;
    case CFG_BROAD_AL2_REAL:
        return CFG_NAME_BROAD_AL2_REAL;
    case CFG_BROAD_AL2_TRAIN:
        return CFG_NAME_BROAD_AL2_TRAIN;
    case CFG_BROAD_AL2_TEST:
        return CFG_NAME_BROAD_AL2_TEST;
    case CFG_BROAD_AL3_REAL:
        return CFG_NAME_BROAD_AL3_REAL;
    case CFG_BROAD_AL3_TRAIN:
        return CFG_NAME_BROAD_AL3_TRAIN;
    case CFG_BROAD_AL3_TEST:
        return CFG_NAME_BROAD_AL3_TEST;
    case CFG_BROAD_DPA_REAL:
        return CFG_NAME_BROAD_DPA_REAL;
    case CFG_BROAD_DPA_TRAIN:
        return CFG_NAME_BROAD_DPA_TRAIN;
    case CFG_BROAD_DPA_TEST:
        return CFG_NAME_BROAD_DPA_TEST;
    case CFG_BROAD_CBR_REAL:
        return CFG_NAME_BROAD_CBR_REAL;
    case CFG_BROAD_CBR_TRAIN:
        return CFG_NAME_BROAD_CBR_TRAIN;
    case CFG_BROAD_CBR_TEST:
        return CFG_NAME_BROAD_CBR_TEST;
    case CFG_BROAD_CLR_REAL:
        return CFG_NAME_BROAD_CLR_REAL;
    case CFG_BROAD_CLR_TRAIN:
        return CFG_NAME_BROAD_CLR_TRAIN;
    case CFG_BROAD_CLR_TEST:
        return CFG_NAME_BROAD_CLR_TEST;
    case CFG_BROAD_ACL_REAL:
        return CFG_NAME_BROAD_ACL_REAL;
    case CFG_BROAD_ACL_TRAIN:
        return CFG_NAME_BROAD_ACL_TRAIN;
    case CFG_BROAD_ACL_TEST:
        return CFG_NAME_BROAD_ACL_TEST;
    case CFG_BROAD_MSR_REAL:
        return CFG_NAME_BROAD_MSR_REAL;
    case CFG_BROAD_MSR_TRAIN:
        return CFG_NAME_BROAD_MSR_TRAIN;
    case CFG_BROAD_MSR_TEST:
        return CFG_NAME_BROAD_MSR_TEST;
    case CFG_BROAD_ADR_REAL:
        return CFG_NAME_BROAD_ADR_REAL;
    case CFG_BROAD_ADR_TRAIN:
        return CFG_NAME_BROAD_ADR_TRAIN;
    case CFG_BROAD_ADR_TEST:
        return CFG_NAME_BROAD_ADR_TEST;
    case CFG_BROAD_A00_REAL:
        return CFG_NAME_BROAD_A00_REAL;
    case CFG_BROAD_A00_TRAIN:
        return CFG_NAME_BROAD_A00_TRAIN;
    case CFG_BROAD_A00_TEST:
        return CFG_NAME_BROAD_A00_TEST;
    case CFG_BROAD_A01_REAL:
        return CFG_NAME_BROAD_A01_REAL;
    case CFG_BROAD_A01_TRAIN:
        return CFG_NAME_BROAD_A01_TRAIN;
    case CFG_BROAD_A01_TEST:
        return CFG_NAME_BROAD_A01_TEST;
    case CFG_BROAD_A02_REAL:
        return CFG_NAME_BROAD_A02_REAL;
    case CFG_BROAD_A02_TRAIN:
        return CFG_NAME_BROAD_A02_TRAIN;
    case CFG_BROAD_A02_TEST:
        return CFG_NAME_BROAD_A02_TEST;
    case CFG_BROAD_A03_REAL:
        return CFG_NAME_BROAD_A03_REAL;
    case CFG_BROAD_A03_TRAIN:
        return CFG_NAME_BROAD_A03_TRAIN;
    case CFG_BROAD_A03_TEST:
        return CFG_NAME_BROAD_A03_TEST;
    case CFG_BROAD_A04_REAL:
        return CFG_NAME_BROAD_A04_REAL;
    case CFG_BROAD_A04_TRAIN:
        return CFG_NAME_BROAD_A04_TRAIN;
    case CFG_BROAD_A04_TEST:
        return CFG_NAME_BROAD_A04_TEST;
    case CFG_BROAD_A05_REAL:
        return CFG_NAME_BROAD_A05_REAL;
    case CFG_BROAD_A05_TRAIN:
        return CFG_NAME_BROAD_A05_TRAIN;
    case CFG_BROAD_A05_TEST:
        return CFG_NAME_BROAD_A05_TEST;
    case CFG_BROAD_A06_REAL:
        return CFG_NAME_BROAD_A06_REAL;
    case CFG_BROAD_A06_TRAIN:
        return CFG_NAME_BROAD_A06_TRAIN;
    case CFG_BROAD_A06_TEST:
        return CFG_NAME_BROAD_A06_TEST;
    case CFG_BROAD_A07_REAL:
        return CFG_NAME_BROAD_A07_REAL;
    case CFG_BROAD_A07_TRAIN:
        return CFG_NAME_BROAD_A07_TRAIN;
    case CFG_BROAD_A07_TEST:
        return CFG_NAME_BROAD_A07_TEST;
    case CFG_BROAD_A08_REAL:
        return CFG_NAME_BROAD_A08_REAL;
    case CFG_BROAD_A08_TRAIN:
        return CFG_NAME_BROAD_A08_TRAIN;
    case CFG_BROAD_A08_TEST:
        return CFG_NAME_BROAD_A08_TEST;
    case CFG_BROAD_A09_REAL:
        return CFG_NAME_BROAD_A09_REAL;
    case CFG_BROAD_A09_TRAIN:
        return CFG_NAME_BROAD_A09_TRAIN;
    case CFG_BROAD_A09_TEST:
        return CFG_NAME_BROAD_A09_TEST;
    case CFG_BROAD_A10_REAL:
        return CFG_NAME_BROAD_A10_REAL;
    case CFG_BROAD_A10_TRAIN:
        return CFG_NAME_BROAD_A10_TRAIN;
    case CFG_BROAD_A10_TEST:
        return CFG_NAME_BROAD_A10_TEST;
    case CFG_BROAD_A11_REAL:
        return CFG_NAME_BROAD_A11_REAL;
    case CFG_BROAD_A11_TRAIN:
        return CFG_NAME_BROAD_A11_TRAIN;
    case CFG_BROAD_A11_TEST:
        return CFG_NAME_BROAD_A11_TEST;
    case CFG_BROAD_A12_REAL:
        return CFG_NAME_BROAD_A12_REAL;
    case CFG_BROAD_A12_TRAIN:
        return CFG_NAME_BROAD_A12_TRAIN;
    case CFG_BROAD_A12_TEST:
        return CFG_NAME_BROAD_A12_TEST;
    case CFG_BROAD_A13_REAL:
        return CFG_NAME_BROAD_A13_REAL;
    case CFG_BROAD_A13_TRAIN:
        return CFG_NAME_BROAD_A13_TRAIN;
    case CFG_BROAD_A13_TEST:
        return CFG_NAME_BROAD_A13_TEST;
    case CFG_BROAD_A14_REAL:
        return CFG_NAME_BROAD_A14_REAL;
    case CFG_BROAD_A14_TRAIN:
        return CFG_NAME_BROAD_A14_TRAIN;
    case CFG_BROAD_A14_TEST:
        return CFG_NAME_BROAD_A14_TEST;
    case CFG_BROAD_MANUAL:
        return CFG_NAME_BROAD_MANUAL;
    case CFG_VOLUME:
        return CFG_NAME_VOLUME;
    case CFG_ETH0:
        return CFG_NAME_LINE_ETH0;
    case CFG_ETH1:
        return CFG_NAME_LINE_ETH1;
    case CFG_WWAN:
        return CFG_NAME_LINE_WWAN;
    case CFG_CERT:
        return CFG_NAME_LINE_CERT;
    case CFG_SECURITY:
        return CFG_NAME_LINE_SECURITY;
    case CFG_VPN:
        return CFG_NAME_LINE_VPN;
    case CFG_SERV1:
        return CFG_NAME_LINE_SERV1;
    case CFG_SERV2:
        return CFG_NAME_LINE_SERV2;
    case CFG_SERV3:
        return CFG_NAME_LINE_SERV3;
    case CFG_LTE_GENERAL:
        return CFG_NAME_LTE_GENERAL;
    case CFG_LTE_ARS:
        return CFG_NAME_LTE_ARS;
    case CFG_AMP_GENERAL:
        return CFG_NAME_AMP_GENERAL;
    case CFG_TERM_GENERAL:
        return CFG_NAME_TERM_GENERAL;
    default:
        return NULL;
    }
}

static char *cfg_val_idx_to_col(cfg_val_idx_t idx)
{
    switch (idx) {
    case CFG_VAL_CTRL:
        return CFG_NAME_VAL_CTRL;
    case CFG_VAL_LINE_MANUAL:
        return CFG_NAME_VAL_LINE_MANUAL;
    case CFG_VAL_SMS_MANUAL:
        return CFG_NAME_VAL_SMS_MANUAL;
    case CFG_VAL_BROAD_TEXT:
        return CFG_NAME_VAL_BROAD_TEXT;
    case CFG_VAL_BOARD_TEXT:
        return CFG_NAME_VAL_BOARD_TEXT;
    case CFG_VAL_TIMEOUT:
        return CFG_NAME_VAL_MANUAL_TIMEOUT;
    case CFG_VAL_TIMEOUT_ACT:
        return CFG_NAME_VAL_MANUAL_TIMEOUT_ACT;
    case CFG_VAL_INPUT_VOL_STO:
        return CFG_NAME_VAL_INPUT_VOL_STO;
    case CFG_VAL_INPUT_VOL_AOIP:
        return CFG_NAME_VAL_INPUT_VOL_AOIP;
    case CFG_VAL_INPUT_VOL_TTS:
        return CFG_NAME_VAL_INPUT_VOL_TTS;
    case CFG_VAL_INPUT_VOL_ARS:
        return CFG_NAME_VAL_INPUT_VOL_ARS;
    case CFG_VAL_INPUT_VOL_XROSHOT:
        return CFG_NAME_VAL_INPUT_VOL_XROSHOT;
    case CFG_VAL_INPUT_VOL_SPARE:
        return CFG_NAME_VAL_INPUT_VOL_SPARE;
    case CFG_VAL_INPUT_VOL_FM:
        return CFG_NAME_VAL_INPUT_VOL_FM;
    case CFG_VAL_INPUT_VOL_UHD:
        return CFG_NAME_VAL_INPUT_VOL_UHD;
    case CFG_VAL_OUTPUT_VOL_LTEMIC:
        return CFG_NAME_VAL_OUTPUT_VOL_LTEMIC;
    case CFG_VAL_OUTPUT_VOL_AMP:
        return CFG_NAME_VAL_OUTPUT_VOL_AMP;
    case CFG_VAL_OUTPUT_VOL_SPDIF:
        return CFG_NAME_VAL_OUTPUT_VOL_SPDIF;
    case CFG_VAL_OUTPUT_VOL_SPARE:
        return CFG_NAME_VAL_OUTPUT_VOL_SPARE;
    case CFG_VAL_USE:
        return CFG_NAME_VAL_USE;
    case CFG_VAL_IP:
        return CFG_NAME_VAL_IP;
    case CFG_VAL_PORT:
        return CFG_NAME_VAL_PORT;
    case CFG_VAL_GW:
        return CFG_NAME_VAL_GW;
    case CFG_VAL_SUB:
        return CFG_NAME_VAL_SUB;
    case CFG_VAL_ID:
        return CFG_NAME_VAL_ID;
    case CFG_VAL_PW:
        return CFG_NAME_VAL_PW;
    case CFG_VAL_SECURITY:
        return CFG_NAME_VAL_SECURITY;
    case CFG_VAL_DHCP:
        return CFG_NAME_VAL_DHCP;
    case CFG_VAL_RESP_NUM1:
        return CFG_NAME_VAL_RESP_NUM1;
    case CFG_VAL_RESP_NUM2:
        return CFG_NAME_VAL_RESP_NUM2;
    case CFG_VAL_XROSHOT_NUM:
        return CFG_NAME_VAL_XROSHOT_NUM;
    case CFG_VAL_ADMIN_NUM:
        return CFG_NAME_VAL_ADMIN_NUM;
    case CFG_VAL_NUM1:
        return CFG_NAME_VAL_NUM1;
    case CFG_VAL_NUM2:
        return CFG_NAME_VAL_NUM2;
    case CFG_VAL_NUM3:
        return CFG_NAME_VAL_NUM3;
    case CFG_VAL_NUM4:
        return CFG_NAME_VAL_NUM4;
    case CFG_VAL_NUM5:
        return CFG_NAME_VAL_NUM5;
    case CFG_VAL_NUM6:
        return CFG_NAME_VAL_NUM6;
    case CFG_VAL_NUM7:
        return CFG_NAME_VAL_NUM7;
    case CFG_VAL_NUM8:
        return CFG_NAME_VAL_NUM8;
    case CFG_VAL_NUM9:
        return CFG_NAME_VAL_NUM9;
    case CFG_VAL_NUM10:
        return CFG_NAME_VAL_NUM10;
    case CFG_VAL_DELAY_TIME:
        return CFG_NAME_VAL_DELAY_TIME;
    case CFG_VAL_AMP_KIND:
        return CFG_NAME_VAL_AMP_KIND;
    case CFG_VAL_TERM_KIND:
        return CFG_NAME_VAL_TERM_KIND;
    case CFG_VAL_TERM_MEDIA:
        return CFG_NAME_VAL_TERM_MEDIA;
    case CFG_VAL_SIREN_BULB:
        return CFG_NAME_VAL_SIREN_BULB;
    case CFG_VAL_ELEC_DISP:
        return CFG_NAME_VAL_ELEC_DISP;
    case CFG_VAL_PA_SYSTEM:
        return CFG_NAME_VAL_PA_SYSTEM;
    default:
        return NULL;
    }
}

void load_cfg(cfg_cmd_t cmd)
{
    int (*func)(void *, int, char **, char **) = get_ptr_cb_load(cmd.idx);
    char str[MAX_SQL_SIZE * 2];

    if (!func) {
        dlp_warn("maybe invalid config command! (index: [%d], value: [%d])", cmd.idx, cmd.val_idx);
        return;
    }

    memset(str, 0x00, sizeof(str));
    SQL_READ(str, cfg_idx_to_tb(cmd.idx), cfg_val_idx_to_col(cmd.val_idx));

    if (strlen(str) > MAX_SQL_SIZE) {
        dlp_warn("sql max string size over! (len: [%d])", strlen(str));
        return;
    }

    if (exec_db(cfg_db, str, func, &cmd) < 0) {
        dlp_warn("exec_db() fail!");
        return;
    }
}

void save_cfg(cfg_cmd_t cmd, const char *data)
{
    int (*func)(void *, int, char **, char **) = get_ptr_cb_load(cmd.idx);
    char str[MAX_SQL_SIZE * 2];

    if (!func) {
        dlp_warn("maybe invalid config command! will do not apply this change after restart! (index: [%d], value: [%d])", cmd.idx, cmd.val_idx);
        return;
    }

    memset(str, 0x00, sizeof(str));
    SQL_UPDATE(str, cfg_idx_to_tb(cmd.idx), cfg_val_idx_to_col(cmd.val_idx), data);

    if (strlen(str) > MAX_SQL_SIZE) {
        dlp_warn("sql max string size over! (len: [%d])", strlen(str));
        return;
    }

    if (exec_db(cfg_db, str, cb_dummy, &cmd) < 0) {
        dlp_warn("exec_db() fail, will do not apply this change after restart!");
        return;
    }
    load_cfg(cmd);
}

int open_cfg(const char *filename)
{
    if (open_db(&cfg_db, filename) < 0) {
        dlp_err("config db open fail! please check db file");
        return -1;
    }
    return 0;
}

static bool select_cfg_file(void)
{
    if (access(CFG_DB_FILE, F_OK) == 0) {
        return true;
    } else if (access(CFG_DB_BACKUP_FILE, F_OK) == 0) {
        char tmp[256];
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "cp %s %s", CFG_DB_BACKUP_FILE, CFG_DB_FILE);
        if (system(tmp) < 0) {
            dlp_warn("copying from backup fail!");
            return false;
        }
        sync();
        return true;
    } else {
        return false;
    }
}

static void early_load_cfg_act(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    for (idx = CFG_BROAD_AL1_REAL; idx <= CFG_BROAD_A14_TEST; idx++) {
        cmd.idx = idx;
        for (val_idx = CFG_VAL_CTRL; val_idx <= CFG_VAL_BOARD_TEXT; val_idx++) {
            cmd.val_idx = val_idx;
            load_cfg(cmd);
        }
    }
}

static void early_load_cfg_manual(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_BROAD_MANUAL;
    for (val_idx = CFG_VAL_TIMEOUT; val_idx <= CFG_VAL_TIMEOUT_ACT; val_idx++) {
        cmd.val_idx = val_idx;
        load_cfg(cmd);
    }
}

static void early_load_cfg_volume(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_VOLUME;
    for (val_idx = CFG_VAL_INPUT_VOL_STO; val_idx <= CFG_VAL_OUTPUT_VOL_SPARE; val_idx++) {
        cmd.val_idx = val_idx;
        load_cfg(cmd);
    }
}

static void early_load_cfg_eth(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    for (idx = CFG_ETH0; idx <= CFG_ETH1; idx++) {
        cmd.idx = idx;
        cmd.val_idx = CFG_VAL_USE;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_DHCP;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_IP;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_GW;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_SUB;
        load_cfg(cmd);
    }
}

static void early_load_cfg_wwan(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_WWAN;
    cmd.val_idx = CFG_VAL_USE;
    load_cfg(cmd);
}

static void early_load_cfg_secu(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_SECURITY;
    cmd.val_idx = CFG_VAL_SECURITY;
    load_cfg(cmd);
}

static void early_load_cfg_vpn(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_VPN;
    cmd.val_idx = CFG_VAL_USE;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_PORT;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_IP;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_ID;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_PW;
    load_cfg(cmd);
}

static void early_load_cfg_serv(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    for (idx = CFG_SERV1; idx <= CFG_SERV3; idx++) {
        cmd.idx = idx;
        cmd.val_idx = CFG_VAL_USE;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_PORT;
        load_cfg(cmd);
        cmd.val_idx = CFG_VAL_IP;
        load_cfg(cmd);
    }
}

static void early_load_cfg_lte_gen(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_LTE_GENERAL;
    cmd.val_idx = CFG_VAL_USE;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_RESP_NUM1;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_RESP_NUM2;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_XROSHOT_NUM;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_ADMIN_NUM;
    load_cfg(cmd);
}

static void early_load_cfg_ars(void)
{
    int idx = 0, val_idx = 0;
    cfg_cmd_t cmd;

    cmd.idx = CFG_LTE_ARS;
    for (val_idx = CFG_VAL_NUM1; val_idx <= CFG_VAL_NUM10; val_idx++) {
        cmd.val_idx = val_idx;
        load_cfg(cmd);
    }
}

static void early_load_cfg_amp_gen(void)
{
    cfg_cmd_t cmd;

    cmd.idx = CFG_AMP_GENERAL;
    cmd.val_idx = CFG_VAL_DELAY_TIME;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_AMP_KIND;
    load_cfg(cmd);
}

static void early_load_cfg_term_gen(void)
{
    cfg_cmd_t cmd;

    cmd.idx = CFG_TERM_GENERAL;
    cmd.val_idx = CFG_VAL_TERM_KIND;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_TERM_MEDIA;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_SIREN_BULB;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_ELEC_DISP;
    load_cfg(cmd);
    cmd.val_idx = CFG_VAL_PA_SYSTEM;
    load_cfg(cmd);
}

static void early_load_cfg(void)
{
    early_load_cfg_act();
    early_load_cfg_manual();
    early_load_cfg_volume();
    early_load_cfg_eth();
    early_load_cfg_wwan();
    early_load_cfg_secu();
    early_load_cfg_vpn();
    early_load_cfg_serv();
    early_load_cfg_lte_gen();
    early_load_cfg_ars();
    early_load_cfg_amp_gen();
    early_load_cfg_term_gen();
}

static void early_init_cfg_act(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'0\', \'0\', \'0\', \'0\'";
    /* (미제어, 유선 자동, SMS 자동, 단말 저장메시지, 단말 문안) */

    for (idx = CFG_BROAD_AL1_REAL; idx <= CFG_BROAD_A14_TEST; idx++) {
        memset(sql, 0x00, sizeof(sql));
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp,
            "%s BOOLEAN, %s BOOLEAN, %s BOOLEAN, %s TINYINT, %s TINYINT",
            cfg_val_idx_to_col(CFG_VAL_CTRL),
            cfg_val_idx_to_col(CFG_VAL_LINE_MANUAL),
            cfg_val_idx_to_col(CFG_VAL_SMS_MANUAL),
            cfg_val_idx_to_col(CFG_VAL_BROAD_TEXT),
            cfg_val_idx_to_col(CFG_VAL_BOARD_TEXT));
        SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
        exec_db(cfg_db, sql, NULL, NULL);
    }

    for (idx = CFG_BROAD_AL1_REAL; idx <= CFG_BROAD_A14_TEST; idx++) {
        memset(sql, 0x00, sizeof(sql));
        SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
        exec_db(cfg_db, sql, cb_dummy, NULL);
    }
}

static void early_init_cfg_manual(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'1\', \'0\'";
    /* (타임아웃 1분, 타임아웃 시 자동 수행) */

    idx = CFG_BROAD_MANUAL;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TINYINT, %s BOOLEAN",
        cfg_val_idx_to_col(CFG_VAL_TIMEOUT),
        cfg_val_idx_to_col(CFG_VAL_TIMEOUT_ACT));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_volume(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\', \'75\'";
    /* (저장메시지, AOIP, TTS, ARS, XROSHOT, SPARE, FM, UHD, LTEMIC, AMP, SPDIF, SPARE)*/

    idx = CFG_VOLUME;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT, %s TINYINT",
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_STO),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_AOIP),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_TTS),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_ARS),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_XROSHOT),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_SPARE),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_FM),
        cfg_val_idx_to_col(CFG_VAL_INPUT_VOL_UHD),
        cfg_val_idx_to_col(CFG_VAL_OUTPUT_VOL_LTEMIC),
        cfg_val_idx_to_col(CFG_VAL_OUTPUT_VOL_AMP),
        cfg_val_idx_to_col(CFG_VAL_OUTPUT_VOL_SPDIF),
        cfg_val_idx_to_col(CFG_VAL_OUTPUT_VOL_SPARE));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_eth(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'0\', \'127.0.0.1\', \'127.0.0.1\', \'255.255.255.0\'";
    /* (미사용, 미사용, IP, GW, SUBNET) */

    for (idx = CFG_ETH0; idx <= CFG_ETH1; idx++) {
        memset(sql, 0x00, sizeof(sql));
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp,
            "%s BOOLEAN, %s BOOLEAN, %s TEXT, %s TEXT, %s TEXT",
            cfg_val_idx_to_col(CFG_VAL_USE),
            cfg_val_idx_to_col(CFG_VAL_DHCP),
            cfg_val_idx_to_col(CFG_VAL_IP),
            cfg_val_idx_to_col(CFG_VAL_GW),
            cfg_val_idx_to_col(CFG_VAL_SUB));
        SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
        exec_db(cfg_db, sql, NULL, NULL);
    }

    for (idx = CFG_ETH0; idx <= CFG_ETH1; idx++) {
        memset(sql, 0x00, sizeof(sql));
        SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
        exec_db(cfg_db, sql, cb_dummy, NULL);
    }
}

static void early_init_cfg_wwan(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\'";
    /* (미사용) */

    idx = CFG_WWAN;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s BOOLEAN",
        cfg_val_idx_to_col(CFG_VAL_USE));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_cert(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'127.0.0.1\'";
    /* (IP) */

    idx = CFG_CERT;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TEXT",
        cfg_val_idx_to_col(CFG_VAL_IP));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_secu(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\'";
    /* (미사용) */

    idx = CFG_SECURITY;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TINYINT",
        cfg_val_idx_to_col(CFG_VAL_SECURITY));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_vpn(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'7591\', \'127.0.0.1\', \'abcdefg\', \'123456\'";
    /* (미사용, 포트번호, IP, ID, PW) */

    idx = CFG_VPN;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s BOOLEAN, %s INT, %s TEXT, %s TEXT, %s TEXT",
        cfg_val_idx_to_col(CFG_VAL_USE),
        cfg_val_idx_to_col(CFG_VAL_PORT),
        cfg_val_idx_to_col(CFG_VAL_IP),
        cfg_val_idx_to_col(CFG_VAL_ID),
        cfg_val_idx_to_col(CFG_VAL_PW));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_serv(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'7591\', \'127.0.0.1\'";
    /* (미사용, 포트번호, IP) */

    for (idx = CFG_SERV1; idx <= CFG_SERV3; idx++) {
        memset(sql, 0x00, sizeof(sql));
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp,
            "%s BOOLEAN, %s INT, %s TEXT",
            cfg_val_idx_to_col(CFG_VAL_USE),
            cfg_val_idx_to_col(CFG_VAL_PORT),
            cfg_val_idx_to_col(CFG_VAL_IP));
        SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
        exec_db(cfg_db, sql, NULL, NULL);
    }

    for (idx = CFG_SERV1; idx <= CFG_SERV3; idx++) {
        memset(sql, 0x00, sizeof(sql));
        SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
        exec_db(cfg_db, sql, cb_dummy, NULL);
    }
}

static void early_init_cfg_lte_gen(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'0\', \'0\', \'0\', \'0\'";
    /* (미사용, RESP_NUM1, RESP_NUM2, XROSHTO_NUM, ADMIN_NUM) */

    idx = CFG_LTE_GENERAL;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s BOOLEAN, %s TEXT, %s TEXT, %s TEXT, %s TEXT",
        cfg_val_idx_to_col(CFG_VAL_USE),
        cfg_val_idx_to_col(CFG_VAL_RESP_NUM1),
        cfg_val_idx_to_col(CFG_VAL_RESP_NUM2),
        cfg_val_idx_to_col(CFG_VAL_XROSHOT_NUM),
        cfg_val_idx_to_col(CFG_VAL_ADMIN_NUM));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_ars(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'0\', \'0\', \'0\', \'0\', \'0\', \'0\', \'0\', \'0\', \'0\'";
    /* (num1, num2, ..., num10) */

    idx = CFG_LTE_ARS;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT",
        cfg_val_idx_to_col(CFG_VAL_NUM1),
        cfg_val_idx_to_col(CFG_VAL_NUM2),
        cfg_val_idx_to_col(CFG_VAL_NUM3),
        cfg_val_idx_to_col(CFG_VAL_NUM4),
        cfg_val_idx_to_col(CFG_VAL_NUM5),
        cfg_val_idx_to_col(CFG_VAL_NUM6),
        cfg_val_idx_to_col(CFG_VAL_NUM7),
        cfg_val_idx_to_col(CFG_VAL_NUM8),
        cfg_val_idx_to_col(CFG_VAL_NUM9),
        cfg_val_idx_to_col(CFG_VAL_NUM10));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_amp_gen(void)
{
    cfg_idx_t idx;
    char sql[MAX_SQL_SIZE];
    char tmp[MAX_SQL_SIZE / 2];
    char *def_val = "\'0\', \'0\'";
    /* (지연시간, 종류) */

    idx = CFG_AMP_GENERAL;
    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TINYINT, %s TINYINT",
        cfg_val_idx_to_col(CFG_VAL_DELAY_TIME),
        cfg_val_idx_to_col(CFG_VAL_AMP_KIND));
    SQL_CREATE(sql, cfg_idx_to_tb(idx), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(idx), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg_term_gen(void)
{
    char sql[MAX_SQL_SIZE * 2];
    char tmp[MAX_SQL_SIZE];
    char *def_val = "\'0\', \'0\', \'0\', \'0\', \'0\'";
    /* (단말 종류, 방송 미디어, 경광등, 전광판 포트, 구내방송장치 포트) */

    memset(sql, 0x00, sizeof(sql));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp,
        "%s TINYINT, %s TINYINT, %s BOOLEAN, %s TINYINT, %s TINYINT",
        cfg_val_idx_to_col(CFG_VAL_TERM_KIND),
        cfg_val_idx_to_col(CFG_VAL_TERM_MEDIA),
        cfg_val_idx_to_col(CFG_VAL_SIREN_BULB),
        cfg_val_idx_to_col(CFG_VAL_ELEC_DISP),
        cfg_val_idx_to_col(CFG_VAL_PA_SYSTEM));
    SQL_CREATE(sql, cfg_idx_to_tb(CFG_TERM_GENERAL), tmp);
    exec_db(cfg_db, sql, NULL, NULL);

    memset(sql, 0x00, sizeof(sql));
    SQL_INSERT(sql, cfg_idx_to_tb(CFG_TERM_GENERAL), def_val);
    exec_db(cfg_db, sql, cb_dummy, NULL);
}

static void early_init_cfg(void)
{
    early_init_cfg_act();
    early_init_cfg_manual();
    early_init_cfg_volume();
    early_init_cfg_eth();
    early_init_cfg_wwan();
    early_init_cfg_cert();
    early_init_cfg_secu();
    early_init_cfg_vpn();
    early_init_cfg_serv();
    early_init_cfg_lte_gen();
    early_init_cfg_ars();
    early_init_cfg_amp_gen();
    early_init_cfg_term_gen();
}

void init_cfg(void)
{
    bool db_file = select_cfg_file();

    memset(&cfg, 0x00, sizeof(cfg_t));
    if (open_cfg(CFG_DB_FILE) < 0)
        exit(EXIT_FAILURE);

    if (!db_file)
        early_init_cfg();

    early_load_cfg();
    dlp_note("initialized config!");
}