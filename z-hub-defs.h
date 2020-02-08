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
#define SCH_NEXT_Z_SHIFT				1.0
#define SCH_LINK_Z_SHIFT				0.01

//== КОНСТАНТЫ.
// Многократно используемые строки.
const char m_chStatusWorking[] = "Включён";
const char m_chClientLabelWaiting[] = "Ожидается";
const char m_chLogServerUpdated[] = "Server configuration is updated.";
const char m_chLogBansCorrupt[] = "Bans catalogue file is corrupt. ";
const char m_chLogEnvUpdated[] = "Environment configuration is updated.";
const char m_chLogCurrentEnv[] = "Current environment: ";
const char m_chLogEnvFileCorrupt[] = "Environment file is corrupt. ";
const char m_chLogEnvNodeAbsend[] = " node is absend.";
const char m_chLogEnvNodeFormatIncorrect[] = " node format incorrect - " ;
const char m_chLogEnvGroup[] = "'Group'";
const char m_chLogEnvElement[] = "'Element'";
const char m_chLogEnvLink[] = "'Link'";
const char m_chLogMissing[] = "missing ";
const char m_chLogWrong[] = "wrong ";
const char m_chLogNode[] = " node.";
const char m_chLogID[] = "'ID'";
const char m_chLogName[] = "'Name'";
const char m_chLogBkgColor[] = "'BkgColor'";
const char m_chLogFrame[] = "'Frame'";
const char m_chLogZ[] = "'Z'";
const char m_chLogPos[] = "'Pos'";
const char m_chLogSrcID[] = "'SrcID'";
const char m_chLogDstID[] = "'DstID'";
const char m_chLogSrcPortID[] = "'SrcPortID'";
const char m_chLogSrcPortPos[] = "'SrcPortPos'";
const char m_chLogDstPortID[] = "'DstPortID'";
const char m_chLogDstPortPos[] = "'DstPortPos'";
const char m_chXML[] = ".xml";

#endif // ZHUBDEFS_H
