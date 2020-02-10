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
#define ENV_STEP_WAITING				500
#define TOUCHED_GEOMETRY				0b0001
#define TOUCHED_CONTENT					0b0010
#define TOUCHED_NAME					0b0100
#define TOUCHED_GROUP					0b1000
#define NO_CLIENT						-1
// Коды изменяемых полей.
#define SCH_ELEMENT_BIT_BUSY            0b00000001
#define SCH_ELEMENT_BIT_BKG_COLOR       0b00000010
#define SCH_ELEMENT_BIT_FRAME           0b00000100
#define SCH_ELEMENT_BIT_POS             0b00001000
#define SCH_ELEMENT_BIT_GROUP           0b00010000
#define SCH_ELEMENT_BIT_ZPOS            0b00100000
//
#define SCH_LINK_BIT_SCR_PORT_POS       0b00000001
#define SCH_LINK_BIT_DST_PORT_POS       0b00000010
#define SCH_LINK_BIT_INIT_ERROR         0b11111111
//
#define SCH_GROUP_BIT_BUSY              0b00000001
#define SCH_GROUP_BIT_BKG_COLOR         0b00000010
#define SCH_GROUP_BIT_FRAME             0b00000100
#define SCH_GROUP_BIT_ZPOS              0b00001000
#define SCH_GROUP_BIT_ELEMENTS_SHIFT    0b00010000

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
const char m_chGroups[] = "Groups";
const char m_chGroup[] = "Group";
const char m_chID[] = "ID";
const char m_chName[] = "Name";
const char m_chBkgColor[] = "BkgColor";
const char m_chFrame[] = "Frame";
const char m_chElements[] = "Elements";
const char m_chZ[] = "Z";
const char m_chElement[] = "Element";
const char m_chPos[] = "Pos";
const char m_chGroupID[] = "GroupID";
const char m_chLinks[] = "Links";
const char m_chLink[] = "Link";
const char m_chSrcID[] = "SrcID";
const char m_chSrcPortID[] = "SrcPortID";
const char m_chSrcPortPos[] = "SrcPortPos";
const char m_chDstID[] = "DstID";
const char m_chDstPortID[] = "DstPortID";
const char m_chDstPortPos[] = "DstPortPos";

#endif // ZHUBDEFS_H
