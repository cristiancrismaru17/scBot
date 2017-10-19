/*
 * TJBot
 * Copyright (c) 2016 Annihil
 * github.com/Annihil/TJBot
 */

#ifndef __TJBOT_H__
#define __TJBOT_H__

#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <WinBase.h>
#include "cgame\cg_local.h"	
#include "game\g_local.h"
#include "sc_info.h"

#define varName(var)  #var

int syscall(int cmd, ...);

vec_t VectorLength(const vec3_t v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float VectorDistance(vec3_t v1, vec3_t v2) {
	vec3_t dir;

	VectorSubtract(v2, v1, dir);
	return VectorLength(dir);
}

 //void trap_UI_Popup( const char *arg0) {
void trap_UI_Popup(int arg0) {
	syscall(162, arg0);
}

static float AngleNormalize360(float angle) {
	return (float)((360.0 / 65536) * ((int)(angle * (65536 / 360.0)) & 65535));
}

static float AngleNormalize180(float angle) {
	angle = AngleNormalize360(angle);
	if (angle > 180.0)
		angle -= 360.0;
	return angle;
}

static float AngleDelta(float angle1, float angle2) {
	return AngleNormalize180(angle1 - angle2);
}

static inline float rad2deg(double a) {
	return (float)((a * 180.0f) / M_PI);
}

vec3_t	vec3_origin = { 0,0,0 };
vec3_t	axisDefault[3] = { { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 } };
vec4_t colz;
vec4_t g_color_table[32] =
{
	{ 0.0,  0.0,    0.0,    1.0 },      // 0 - black		0
	{ 1.0,  0.0,    0.0,    1.0 },      // 1 - red			1
	{ 0.0,  1.0,    0.0,    1.0 },      // 2 - green		2
	{ 1.0,  1.0,    0.0,    1.0 },      // 3 - yellow		3
	{ 0.0,  0.0,    1.0,    1.0 },      // 4 - blue			4
	{ 0.0,  1.0,    1.0,    1.0 },      // 5 - cyan			5
	{ 1.0,  0.0,    1.0,    1.0 },      // 6 - purple		6
	{ 1.0,  1.0,    1.0,    1.0 },      // 7 - white		7
	{ 1.0,  0.5,    0.0,    1.0 },      // 8 - orange		8
	{ 0.5,  0.5,    0.5,    1.0 },      // 9 - md.grey		9
	{ 0.75, 0.75,   0.75,   1.0 },      // : - lt.grey		10		// lt grey for names
	{ 0.75, 0.75,   0.75,   1.0 },      // ; - lt.grey		11
	{ 0.0,  0.5,    0.0,    1.0 },      // < - md.green		12
	{ 0.5,  0.5,    0.0,    1.0 },      // = - md.yellow	13
	{ 0.0,  0.0,    0.5,    1.0 },      // > - md.blue		14
	{ 0.5,  0.0,    0.0,    1.0 },      // ? - md.red		15
	{ 0.5,  0.25,   0.0,    1.0 },      // @ - md.orange	16
	{ 1.0,  0.6f,   0.1f,   1.0 },      // A - lt.orange	17
	{ 0.0,  0.5,    0.5,    1.0 },      // B - md.cyan		18
	{ 0.5,  0.0,    0.5,    1.0 },      // C - md.purple	19
	{ 0.0,  0.5,    1.0,    1.0 },      // D				20
	{ 0.5,  0.0,    1.0,    1.0 },      // E				21
	{ 0.2f, 0.6f,   0.8f,   1.0 },      // F				22
	{ 0.8f, 1.0,    0.8f,   1.0 },      // G				23
	{ 0.0,  0.4,    0.2f,   1.0 },      // H				24
	{ 1.0,  0.0,    0.2f,   1.0 },      // I				25
	{ 0.7f, 0.1f,   0.1f,   1.0 },      // J				26
	{ 0.6f, 0.2f,   0.0,    1.0 },      // K				27
	{ 0.8f, 0.6f,   0.2f,   1.0 },      // L				28
	{ 0.6f, 0.6f,   0.2f,   1.0 },      // M				29
	{ 1.0,  1.0,    0.75,   1.0 },      // N				30
	{ 1.0,  1.0,    0.5,    1.0 },      // O				31
};

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = (float)(angles[1] * (M_PI * 2 / 360));
	sy = (float)sin(angle);
	cy = (float)cos(angle);
	angle = (float)(angles[0] * (M_PI * 2 / 360));
	sp = (float)sin(angle);
	cp = (float)cos(angle);
	angle = (float)(angles[2] * (M_PI * 2 / 360));
	sr = (float)sin(angle);
	cr = (float)cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (right)
	{
		right[0] = (-1 * sr*sp*cy + -1 * cr*-sy);
		right[1] = (-1 * sr*sp*sy + -1 * cr*cy);
		right[2] = -1 * sr*cp;
	}
	if (up)
	{
		up[0] = (cr*sp*cy + -sr*-sy);
		up[1] = (cr*sp*sy + -sr*cy);
		up[2] = cr*cp;
	}
}

void AnglesToAxis(const vec3_t angles, vec3_t axis[3]) {
	vec3_t	right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors(angles, axis[0], right, axis[2]);
	VectorSubtract(vec3_origin, right, axis[1]);
}

static void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

#define PASSFLOAT( x ) ( *(int*)&x )

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

#define AUTO_ON_COMMAND "+sc_autojump"
#define AUTO_OFF_COMMAND "-sc_autojump"
#define BOT_ON_COMMAND "+sc_jumpbot"
#define BOT_OFF_COMMAND "-sc_jumpbot"
#define AUTO_BOT_R_ON "+sc_autobot_r"
#define AUTO_BOT_R_OFF "-sc_autobot_r"
#define AUTO_BOT_L_ON "+sc_autobot_l"
#define AUTO_BOT_L_OFF "-sc_autobot_l"
#define OB_ON "+sc_ob"
#define OB_OFF "-sc_ob"
#define CRAZY_ON "+sc_crazy"
#define CRAZY_OFF "-sc_crazy"
#define LONEBEAT_ON_COMMAND "+sc_lonebeat"
#define LONEBEAT_OFF_COMMAND "-sc_lonebeat"
#define ORIGIN_ON_COMMAND "+sc_origin"
#define ORIGIN_OFF_COMMAND "-sc_origin"
#define PS_PRINT_ON_COMMAND "+sc_ps_print"
#define PS_PRINT_OFF_COMMAND "-sc_ps_print"
#define RONEBEAT_ON_COMMAND "+sc_ronebeat"
#define RONEBEAT_OFF_COMMAND "-sc_ronebeat"
#define VIEW_HACK_ON_COMMAND "+sc_viewhack"
#define VIEW_HACK_OFF_COMMAND "-sc_viewhack"
#define ANGLE_COMMAND "sc_angle"
#define CHAT_COMMAND "sc_coloured_chat"
#define INFO_COMMAND "sc_drawcat"
#define LOG_COMMAND "sc_log"
#define MS_PRINT_COMMAND "sc_ms_print"
#define MS_RESET_COMMAND "sc_ms_reset"
#define ORIGIN_SET_COMMAND "sc_originset"
#define ORIGIN_GET_COMMAND "sc_originget"
#define PASSWORD_COMMAND "sc_password"
#define SAFE_START_COMMAND "sc_safestart"
#define SMOOTH_COMMAND "sc_smoothness"
#define AUTOSMOOTH_COMMAND "sc_smoothness_auto"
#define START_DUMP_COMMAND "sc_startdump"
#define STOP_DUMP_COMMAND "sc_stopdump"
#define STYLES_COMMAND "sc_styles"
#define CJ_COMMAND "sc_cj_info"
#define CGAZ_COMMAND "sc_drawCgaz"
#define JUMP_DELAY_COMMAND "sc_jump_delay"
#define RESET_COMMAND "sc_reset"
#define CHAT_COLOR "sc_coloured_chat_colors"
#define SMOOTHNESS_STYLE "sc_smoothness_style"
#define SMOOTHNESS_AGGRESSIVE "sc_smoothness_agressiveness"
#define SAY_COMMAND "sc_say"
#define SAY_BUFFER "cg_messageText"
#define CHAT_BOT "sc_chat_bot"
#define REC_START "sc_record"
#define REC_STOP "sc_stoprecord"
#define REC_FILE "sc_recfile"
#define PB_START "sc_startplayback"
#define PB_STOP "sc_stopplayback"

char *CMD_LIST[11] = { "sc_password", "sc_styles", "sc_coloured_chat_colors", "sc_coloured_chat", "sc_safestart", "sc_smoothness_auto", "sc_drawCgaz", "sc_jump_delay", "sc_smoothness_style", "sc_smoothness", "sc_smoothness_aggressiveness" };

#define ASM_JMP 0xe9
#define ASM_RETN 0xc3

#define MIN_SPEED 320

//hooked struct calls//
level_locals_t level;
typedef struct gentity_s gentity_t;
struct gclient_s   client;
gentity_t ent, g_entities[MAX_GENTITIES];
usercmd_t cmd;
playerState_t ps;
//////////////////////

// offsets
// client = 2.60b

//	etjump myserv + 999 0x01df0920   
//                 twc  0x1E33BD4
#define CG_OFFSET_999 0x01E39540
#define CG_OFFSET_TWC 0x01E33B20

//#define ETJUMP_OFFSET 0x01DF0978


#define VEL_SIZE_TWC 0x1E369FC //cg
#define VEL_SIZE_999 0xE7C41C //cg
#define CGAZ_ARG 0x003DDAE0
#define CGAZ_999 0x003DDADC
#define CGAZ_TWC 0x0388C76C
#define ARG1_UINFO_999 0x003BB138
#define ARG1_UINFO_TWC 0x001B0E110
#define ANGLE_VIEW 0x1DF0A2C //cg
#define MOUSE_YAW 0x00FEEA8C //et
#define KEY_FORWARD (0x00835A08 + 4 * 4)
#define KEY_BACK (0x00835A20 + 4 * 4)
#define KEY_MOVELEFT (0x00835A50 + 4 * 4)
#define KEY_MOVERIGHT (0x00835A68 + 4 * 4)
#define KEY_UP (0x00835AC8 + 4 * 4)
#define CONSOLE 0x128FB4C //client console chat line
#define CHCOUNT 0x128FB40 //client console nr of chars
#define SERVERCHAT_TWC 0x2A4542 // et + last console server non-voicechat line
#define SERVERCHAT_999 0x2DBFAC2 // cgame_mp_x86.dll+2DBFAC2
#define CHAT999 0x5C97DC
#define VOICELINE_TWC 0xE8B8BB // last voicechat command line (starts with [hh:mm:ss])
#define VOICECMD_TWC 0xE8B7B0 // CGAME vsay command
#define VOICECHATID_TWC 0xE8B7A0
#define VOICELINE_999 0x2DBB1DB // last voicechat command line (starts with [hh:mm:ss])
#define VOICECMD_999 0x2DBB0D0 // CGAME vsay command
#define VOICECHATID_999 0x2DBB0C0
#define ENTER_KEY 0x128FD30
#define PAD_HEIGHT 0x1CA6F0 //height of the pad you are on 999
#define IS_PRONED 0x1E343B0 //cgame 
#define FOLLOWING_NUM 0x616064 // et.exe +
#define CLIENT_NUM 0x11D69E0 // et.exe +

#define OB_INDICATOR 0x390351D//cgame_mp_x86.dll+390351D

#define CGS_XBIAS 0x1E60684 //cgame
#define CGS_PTR 0x1E59BA0 // cgame
#define CLIENT_0_ID 0x1E62FAC // ID 0 (int)
#define CLIENT_0_NAME 0x1E62FB0 // ID 0 name
#define CLIENT_TEAM 0x80 // NAME + 0x80
#define CLIENT_SIZE 0x1C0 // size between client data

///// ID - 4bytes  = (int)isconnected 0/1

#define FORWARD_KEY 0x435A18
#define BACK_KEY 0x435A30
#define MOVERIGHT_KEY 0x435A90
#define MOVELEFT_KEY 0x435A78
#define MOVEUP_KEY 0x435AD8
#define MOVEDOWN_KEY 0x435AF0
#define SPRINT_KEY 0x435B80
#define PRONE_KEY 0x435C70


#define SPEC_PRONE 0x1DEB04C  // prone == 128
#define SPEC_SPRINT 0x1DEB04D  // sprint == 160
#define SPEC_KEY 0x1DEB050  // 1 = forward 2 = backward 4 = left 5 = right

// cgame = etjump and other mods generaly use those values
//#define PREDICTED_PS_OFFSET 0x48C20
#define ORIGIN_X 5 * 4
#define ORIGIN_Y 6 * 4
#define ORIGIN_Z 7 * 4
#define VELOCITY_X 8 * 4
#define VELOCITYX 0xFE8D00 //et+
#define VELOCITY_Y 9 * 4
#define VELOCITYY 0xFE8D04 // et+
#define VELOCITY_Z 10 * 4
#define VELOCITYZ 0xFE8D08 // et+
#define VIEW_X 45 * 4
#define VIEW_Y 44 * 4
#define GROUND_ENTITY_NUM 20 * 4
#define MAX_STRING_CHARS 1024
#define	MAX_RENDER_STRINGS			8
#define	MAX_RENDER_STRING_LENGTH	32

int cmd_num;

struct tjbot_s {
    void *cgame_handle;
	unsigned int et_address;
	unsigned long frame_count;
	unsigned long info_count;
	unsigned int console_address;
    char cgame_filename[1024], rec_file[61];
	char curr_uinfo[5];
	char password[32];
	char styles[16];
	char colourS[32];
	char sayBuffer[1024];
	char *voiceCmd, *voiceLine, voicetime[11];
    void (*orig_cg_dllEntry)(int (*)(int, ...));
    int (*orig_cg_vmMain)(int, int, int, int, int, int, int, int, int, int, int, int, int);
    int (*orig_syscall)(int, ...);
    float *mouse_x;
    float *orig_x;
    float *orig_y;
    float *orig_z;
	float *vel_x;
	float *vel_y;
	float *vel_z;
	float *velx;
	float *vely;
	float *velz;
    float *view_x;
    float *view_y;
	float aggressiveness;
	float groundaccel, airaccel;
	float max_gndaccel, max_airaccel;
	float smooth;
	float origin_x;
	float origin_y;
	float max_speed;
	float cj_info;
	float pre;
	float vel_size;
	float velsize;
	float *view_angle;
	float *plspeed;
	byte *s_sprint, *s_key, *s_prone;
	int crazy;
	int clients[40];
	int *clientNum, *chatID, chatDelay[4];
	int rec, left_down, right_down, forward_down, back_down, moveup_down, pause;
	int ob_on;
	int playback;
	int movement;
	int *forward;
	int *back;
	int *moveleft;
	int *moveright;
	int *movedown;
	int *moveup;
	int *prone;
	int *sprint;
	int *ground_ent_num;
	int *cgaz_number;
	int chatBot;
	int draw_cgaz;
	int catspray;
	int isAvailable;
	int jumpdelay;
	int autosmooth;
	int safestart;
	int smoothstyle;
	int fb;
	int inv;
	int bw;
	int colorchat;
	int log;
	int offsetno;
	int ronebeat;
	int lonebeat;
	int l_autobot;
	int r_autobot;
    int jumpbot_enable;
    int autojump_enable;
    int origin_enable;
    int ps_print_enable;
    int orig_say;
    int view_hack_enable;
	int delayedframes;
	FILE *dump, *settings_file, *playback_file;
};
static struct tjbot_s tjbot;

typedef struct {
	char    *cmd;
	char	*res;
} vsay_cmd;

static vsay_cmd vc_cmds[] =
{
	{"PathCleared",			"Path cleared"},
	{"EnemyWeak",			"The enemy is weakened"},
	{ "AllClear",			"All clear" },
	{ "Incoming",			"Incoming" },
	{ "FireInTheHole",		"Fire in the hole" },
	{ "OnDefense",			"I'm on defense." },
	{ "OnOffense",			"I'm on offense." },
	{ "TakingFire",			"Taking fire" },
	{ "MinesCleared",		"Mines cleared." },
	{ "EnemyDisguised",		"Enemy in the skies" },
	{ "FollowMe",			"Follow me" },
	{ "LetsGo",				"Let's go" },
	{ "Move",				"Move" },
	{ "ClearPath",			"Clear the path" },
	{ "DefendObjective",	"Defend our objective" },
	{ "DisarmDynamite",		"Disarm the dynamite" },
	{ "ClearMines",			"Clear the mines" },
	{ "ReinforceDefense",	"Reinforce the defense" },
	{ "ReinforceOffense",	"Reinforce the offense" },
	{ "Affirmative",		"Yes" },
	{ "Negative",			"No" },
	{ "Thanks",				"Thanks" },
	{ "Welcome",			"No problem" },
	{ "Sorry",				"Sorry" },
	{ "Oops",				"Oops" },
	{ "Bye",				"Bye" },
	{ "GreatShot",			"Great shot" },
	{ "Cheer",				"Nice" },
	{ "GoodGame",			"Good game" },
	{ "HoldFire",			"Hold your fire" },
	{ "CommandAcknowledged","Alright" },
	{ "CommandDeclined",	"Can't do that" },
	{ "CommandCompleted",	"Command completed" },
	{ "ConstructionCommencing", "I'm constructing" },
	{ "DestroyPrimary",			"Destroy primary objective" },
	{ "DestroySecondary",		"Destroy secondary objective" },
	{ "DestroyConstruction",	"Destroy construction" },
	{ "RepairVehicle",			"Repair the vehicle" },
	{ "DestroyVehicle",			"Disable the vehicle" },
	{ "EscortVehicle",			"Escort the vehicle" },
	{ "IamSoldier",				"I'm a soldier" },
	{ "IamMedic",				"I'm a medic" },
	{ "IamEngineer",			"I'm an engineer" },
	{ "IamFieldOps",			"I'm a field ops" },
	{ "IamCovertOps",			"I'm a covert ops" },
	{ "NeedAmmo",				"I need ammo" },
	{ "NeedBackup",				"I need backup" },
	{ "NeedEngineer",			"We need an engineer" },
	{ "CoverMe",				"Cover me" },
	{ "WhereTo",				"Where to?" },
	{ "Medic",					"Need a Medic" },
	{ "Hi",						"Hi" },
	{ "NeedOps",				"We need Covert Ops" }
};

#define MAX_VA_STRING       32000

char* CG_TranslateString(const char *string) {
	static char staticbuf[2][MAX_VA_STRING];
	static int bufcount = 0;
	char *buf;

	// some code expects this to return a copy always, even
	// if none is needed for translation, so always supply
	// another buffer

	buf = staticbuf[bufcount++ % 2];

	tjbot.orig_syscall(CG_TRANSLATE_STRING, string, buf);

	return buf;
}

#define SETUP_MOUNTEDGUN_STATUS( ps )							\
	switch ( ps->persistant[PERS_HWEAPON_USE] ) {				 \
	case 1:													\
		ps->eFlags |= EF_MG42_ACTIVE;						\
		ps->eFlags &= ~EF_AAGUN_ACTIVE;						\
		ps->powerups[PW_OPS_DISGUISED] = 0;					\
		break;												\
	case 2:													\
		ps->eFlags |= EF_AAGUN_ACTIVE;						\
		ps->eFlags &= ~EF_MG42_ACTIVE;						\
		ps->powerups[PW_OPS_DISGUISED] = 0;					\
		break;												\
	default:												\
		ps->eFlags &= ~EF_MG42_ACTIVE;						\
		ps->eFlags &= ~EF_AAGUN_ACTIVE;						\
		break;												\
	}

void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap) {
	int i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR) { // || ps->pm_flags & PMF_LIMBO ) { // JPW NERVE limbo
		s->eType = ET_INVISIBLE;
	}
	else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH) {
		s->eType = ET_INVISIBLE;
	}
	else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy(ps->origin, s->pos.trBase);
	if (snap) {
		SnapVector(s->pos.trBase);
	}

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(ps->viewangles, s->apos.trBase);
	if (snap) {
		SnapVector(s->apos.trBase);
	}

	if (ps->movementDir > 128) {
		s->angles2[YAW] = (float)ps->movementDir - 256;
	}
	else {
		s->angles2[YAW] = ps->movementDir;
	}

	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;    // ET_PLAYER looks here instead of at number
									 // so corpses can also reference the proper config
									 // Ridah, let clients know if this person is using a mounted weapon
									 // so they don't show any client muzzle flashes

	if (ps->eFlags & EF_MOUNTEDTANK) {
		ps->eFlags &= ~EF_MG42_ACTIVE;
		ps->eFlags &= ~EF_AAGUN_ACTIVE;
	}
	else {
		SETUP_MOUNTEDGUN_STATUS(ps);
	}

	s->eFlags = ps->eFlags;

	if (ps->stats[STAT_HEALTH] <= 0) {
		s->eFlags |= EF_DEAD;
	}
	else {
		s->eFlags &= ~EF_DEAD;
	}

	// from MP
	if (ps->externalEvent) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	}
	else if (ps->entityEventSequence < ps->eventSequence) {
		int seq;

		if (ps->entityEventSequence < ps->eventSequence - MAX_EVENTS) {
			ps->entityEventSequence = ps->eventSequence - MAX_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_EVENTS - 1);
		s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}
	// end
	// Ridah, now using a circular list of events for all entities
	// add any new events that have been added to the playerState_t
	// (possibly overwriting entityState_t events)
	for (i = ps->oldEventSequence; i != ps->eventSequence; i++) {
		s->events[s->eventSequence & (MAX_EVENTS - 1)] = ps->events[i & (MAX_EVENTS - 1)];
		s->eventParms[s->eventSequence & (MAX_EVENTS - 1)] = ps->eventParms[i & (MAX_EVENTS - 1)];
		s->eventSequence++;
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for (i = 0; i < MAX_POWERUPS; i++) {
		if (ps->powerups[i]) {
			s->powerups |= 1 << i;
		}
	}

	s->nextWeapon = ps->nextWeapon; // Ridah
									//	s->loopSound = ps->loopSound;
	s->teamNum = ps->teamNum;
	s->aiState = ps->aiState;       // xkan, 1/10/2003
}

cgs_t			*cgs;
cg_t			cg;
trace_t			trace, iceTrace;

#endif /* __TJBOT_H__ */
