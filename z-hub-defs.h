#ifndef ZHUBDEFS_H
#define ZHUBDEFS_H

//== МАКРОСЫ.
#define H_MAINWINDOW_UI_CONF_PATH       "../Z-Hub/settings/mainwindow_ui.ini"
#define S_BANS_CAT_PATH					"../Z-Hub/settings/bans.xml"
#define S_CONF_PATH                     "../Z-Hub/settings/server.xml"
#define E_CONF_PATH                     "../Z-Hub/settings/environment.xml"
#define ENVS_DIR						"../Z-Hub/environments/"
#define SERVER_WAITING_ATTEMPTS			128
#define ENV_NAME_LEN					64

//== КОНСТАНТЫ.
// Многократно используемые строки.
const char m_chStatusWorking[] = "Включён";
const char m_chClientLabelWaiting[] = "Ожидается";
const char m_chLogServerUpdated[] = "Server configuration is updated.";
const char m_chLogBansCorrupt[] = "Bans catalogue file is corrupt. ";
const char m_chLogEnvUpdated[] = "Environment configuration is updated.";

#endif // ZHUBDEFS_H
