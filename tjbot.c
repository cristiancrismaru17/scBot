/*
 * TJBot
 * Copyright (c) 2016 Annihil
 * github.com/Annihil/TJBot
 */

#include "tjbot.h"

 // functions taken from Q3

qhandle_t trap_R_RegisterShader(const char *name) {
	//CG_DrawInformation(qtrue);
	tjbot.orig_syscall(CG_PUMPEVENTLOOP);
	return tjbot.orig_syscall(CG_R_REGISTERSHADER, name);
}

void    trap_CM_BoxTrace(trace_t *results, const vec3_t start, const vec3_t end,
	const vec3_t mins, const vec3_t maxs,
	clipHandle_t model, int brushmask) {
	tjbot.orig_syscall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
}

void    trap_R_SetColor(const float *rgba) {
	tjbot.orig_syscall(CG_R_SETCOLOR, rgba);
}

void    trap_R_DrawStretchPic(float x, float y, float w, float h,
	float s1, float t1, float s2, float t2, qhandle_t hShader) {
	tjbot.orig_syscall(CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader);
}
void CG_AdjustFrom640(float *x, float *y, float *w, float *h) {
	*x *= cgs->screenXScale;
	*y *= cgs->screenYScale;
	*w *= cgs->screenXScale;
	*h *= cgs->screenYScale;
}
void CG_DrawChar(int x, int y, int width, int height, int ch) {
	int row, col;
	float frow, fcol;
	float size;
	float ax, ay, aw, ah;

	ch &= 255;

	if (ch == ' ') {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	row = ch >> 4;
	col = ch & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic(ax, ay, aw, ah,
		fcol, frow,
		fcol + size, frow + size,
		//cgs->media.charsetShader);
		trap_R_RegisterShader("gfx/2d/hudchars"));
}

void CG_DrawStringExt(int x, int y, const char *string, const float *setColor,
	qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars) {
	vec4_t color;
	const char  *s;
	int xx;
	int cnt;

	if (maxChars <= 0) {
		maxChars = 32767; // do them all!

	}
	// draw the drop shadow
	if (shadow) {
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor(color);
		s = string;
		xx = x;
		cnt = 0;
		while (*s && cnt < maxChars) {
			if (Q_IsColorString(s)) {
				s += 2;
				continue;
			}
			CG_DrawChar(xx + 1, y + 1, charWidth, charHeight, *s);
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while (*s && cnt < maxChars) {
		if (Q_IsColorString(s)) {
			if (!forceColor) {
				if (*(s + 1) == COLOR_NULL) {
					memcpy(color, setColor, sizeof(color));
				}
				else {
					memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
					color[3] = setColor[3];
				}
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		CG_DrawChar(xx, y, charWidth, charHeight, *s);
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor(NULL);
}

static const char *CG_Argv(int arg) {
	static char buffer[MAX_STRING_CHARS];
	tjbot.orig_syscall(CG_ARGV, arg, buffer, sizeof(buffer));
	return buffer;
}

int doesFileExist(const char *filename) {
	return GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES;
}

static char *CG_Args(void) {
	static char cmd_args[MAX_STRING_CHARS];

	syscall(CG_ARGS, cmd_args, sizeof(cmd_args));

	return cmd_args;
}

//int i1, i2, i3, i4;
//float f1, f2;
//float view[3][3];
//unsigned int ui1, ui2, ui3, ui4;
char *outinfo;
int color = 41, sw = 0;

// hack functions
static void get_offsets() {

	//unsigned int cg_address = (unsigned int)tjbot.cgame_handle;
	//unsigned int predictedPS_address = PREDICTED_PS_OFFSET + CG_OFFSET + cg_address;
	unsigned int offsets[2];
	offsets[1] = CG_OFFSET_999;
	offsets[0] = CG_OFFSET_TWC;
	unsigned int predictedPS_address = offsets[tjbot.offsetno] + (unsigned int)tjbot.cgame_handle;
	//unsigned int predictedPS_address = ETJUMP_OFFSET + (unsigned int)tjbot.cgame_handle;
	tjbot.et_address = 0x400000;

	//printf("\x1B[33mtjbot: predicted playerstate @ 0x%x\x1B[0m\n", predictedPS_address - tjbot.cgame_address);
	tjbot.orig_x = (float *) (predictedPS_address + ORIGIN_X);
    tjbot.orig_y = (float *) (predictedPS_address + ORIGIN_Y);
    tjbot.orig_z = (float *) (predictedPS_address + ORIGIN_Z);
    tjbot.vel_x = (float *) (predictedPS_address + VELOCITY_X);
    tjbot.vel_y = (float *) (predictedPS_address + VELOCITY_Y);
    tjbot.vel_z = (float *) (predictedPS_address + VELOCITY_Z);
    tjbot.view_x = (float *) (predictedPS_address + VIEW_X);
    tjbot.view_y = (float *) (predictedPS_address + VIEW_Y);
	tjbot.velx = (float *)(tjbot.et_address + VELOCITYX);
	tjbot.vely = (float *)(tjbot.et_address + VELOCITYY);
	tjbot.velz = (float *)(tjbot.et_address + VELOCITYZ);
    tjbot.ground_ent_num = (int *) (predictedPS_address + GROUND_ENTITY_NUM);
	tjbot.forward = (unsigned int *)(tjbot.et_address + FORWARD_KEY);
	tjbot.back = (unsigned int *)(tjbot.et_address + BACK_KEY);
	tjbot.moveright = (unsigned int *)(tjbot.et_address + MOVERIGHT_KEY);
	tjbot.moveleft = (unsigned int *)(tjbot.et_address + MOVELEFT_KEY);
	tjbot.moveup = (unsigned int *)(tjbot.et_address + MOVEUP_KEY);
	tjbot.movedown = (unsigned int *)(tjbot.et_address + MOVEDOWN_KEY);
	tjbot.prone = (unsigned int *)(tjbot.et_address + PRONE_KEY);
	tjbot.sprint = (unsigned int *)(tjbot.et_address + SPRINT_KEY);
	tjbot.view_angle = (float *)((unsigned int)tjbot.cgame_handle + ANGLE_VIEW);
	tjbot.mouse_x = (float *)(tjbot.et_address + MOUSE_YAW);
	tjbot.console_address = (tjbot.et_address + CONSOLE);
	tjbot.clientNum = (int *)(tjbot.et_address + CLIENT_NUM);
	
	if (tjbot.offsetno == 0)
	{
		tjbot.cgaz_number = (unsigned int *)((unsigned int)tjbot.cgame_handle + CGAZ_TWC);
		tjbot.voiceCmd = (char *)(unsigned int)tjbot.cgame_handle + VOICECMD_TWC;
		tjbot.voiceLine = (char *)(unsigned int)tjbot.cgame_handle + VOICELINE_TWC;
		tjbot.chatID = (int *)((unsigned int)tjbot.cgame_handle + VOICECHATID_TWC);
		tjbot.plspeed = (float *)((unsigned int)tjbot.cgame_handle + VEL_SIZE_TWC);
		cgs = (cgs_t *)(unsigned int)((unsigned int)tjbot.cgame_handle + CGS_PTR);
	}
	else
	{
		tjbot.cgaz_number = (unsigned int *)((unsigned int)tjbot.cgame_handle + CGAZ_999);
		tjbot.voiceCmd = (char *)(unsigned int)tjbot.cgame_handle + VOICECMD_999;
		tjbot.voiceLine = (char *)(unsigned int)tjbot.cgame_handle + VOICELINE_999;
		tjbot.chatID = (int *)((unsigned int)tjbot.cgame_handle + VOICECHATID_999);
		tjbot.plspeed = (float *)((unsigned int)tjbot.cgame_handle + VEL_SIZE_999);
	}
}

void draw_cgaz()
{
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "etj_drawcgaz 0;");
	*tjbot.cgaz_number = (unsigned int)tjbot.draw_cgaz;
}

void set_styles(char *style)
{
	if (strchr(tjbot.styles, 'f') != NULL || strchr(tjbot.styles, 'f') != NULL) 
		tjbot.fb = 1; 
	else 
		tjbot.fb = 0;
	if (strchr(tjbot.styles, 'i') != NULL || strchr(tjbot.styles, 'I') != NULL) 
		tjbot.inv = 1; 
	else 
		tjbot.inv = 0;
	if (strchr(tjbot.styles, 'b') != NULL || strchr(tjbot.styles, 'B') != NULL) 
		tjbot.bw = 1; 
	else 
		tjbot.bw = 0;
}
/////////
/////////
int rang = .3; int lang = .3, isOnground = 0, isNoclip = 0, isInwater = 0, downtime = 0;
float yaw = 0.022f, velDelta;
float MOUSE_SAFE_PAD, saved_gnd = 0.0f, saved_air = 0.0f;
int rch = 0, lch = 0;
float saved_zpos[20], prev_vel_size[20], prevvelsize[20], prev_vel_x, prev_vel_y;
int notfirst = 0;
float vel_angle, accel_coef, per_angle, accel_angle, left_angle, right_angle;
//////////
static void jump_bot() {
	if ((*tjbot.back && !tjbot.bw) || \
		((!(*tjbot.moveright || *tjbot.moveleft) && *tjbot.forward) && !(tjbot.ronebeat || tjbot.lonebeat)) || \
		(!(*tjbot.moveright || *tjbot.moveleft) && *tjbot.back) || \
		(*tjbot.forward && !tjbot.fb) || \
		((*tjbot.moveright || *tjbot.moveleft) && !*tjbot.forward && !*tjbot.back && !tjbot.inv))
	{
		return;
	}
	else
	{
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
	}

	if (tjbot.vel_size < 380 && tjbot.smooth)
	{
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
		return;
	}

	char fps_c[256];
	tjbot.orig_syscall(6, "com_maxfps", fps_c, 256);
	float frame_rate = atof(fps_c);

	velDelta = tjbot.vel_size - prev_vel_size[18];

	if ((velDelta >= tjbot.max_gndaccel) && isOnground)
	{
		tjbot.max_gndaccel = velDelta;
		if (tjbot.groundaccel <= saved_gnd)
		{
			saved_gnd = tjbot.groundaccel;
				tjbot.groundaccel -= 1.f;
		}
		else if (tjbot.groundaccel > saved_gnd)
		{
			saved_gnd = tjbot.groundaccel;
				tjbot.groundaccel += 1.f;
		}
	}
	if ((velDelta >= tjbot.max_airaccel) && !isOnground)
	{
		tjbot.max_airaccel = velDelta;
		if (tjbot.airaccel <= saved_air)
		{
			saved_air = tjbot.airaccel;
				tjbot.airaccel -= 1.f;
		}
		else if (tjbot.airaccel > saved_air)
		{
			saved_air = tjbot.airaccel;
				tjbot.airaccel += 1.f;
		}
	}

	if ((velDelta < tjbot.max_gndaccel) && isOnground)
	{
		if (tjbot.groundaccel < saved_gnd)
		{
				tjbot.groundaccel += 1.f;
		}
		else if (tjbot.groundaccel > saved_gnd || velDelta < 0)
		{
				tjbot.groundaccel -= 1.f;
		}
	}
	else if ((velDelta < tjbot.max_airaccel) && !isOnground)
	{
		if (tjbot.airaccel < saved_air)
		{
				tjbot.airaccel += 1.f;
		}
		else if (tjbot.airaccel > saved_air || velDelta < 0)
		{
				tjbot.airaccel -= 1.f;
		}
	}

	//////// Velocity snapping magic formula //////////
	//	acos((g_speed - g_speed * pmove_msec / 1000) / speed).   //
	///////////////////////////////////////////////////

	float smoothness = tjbot.smooth;

	//breaks bot if hit wall and still falling//

	if (prev_vel_size[0] - tjbot.vel_size > 50 && saved_zpos[0] - *tjbot.orig_z > 0) //&& notfirst && tjbot.smooth)
		notfirst = 0;
	////////////////////////////////////////////

	if (tjbot.autosmooth)
	{
		if (tjbot.vel_size > 1000) smoothness = 8.0f;
		else smoothness = (float)((0.00007976190476190476 *tjbot.vel_size * tjbot.vel_size) + (-0.15511904761904763 * tjbot.vel_size) + 89.35714285714286);
	}
	else if (tjbot.vel_size < 650 && tjbot.safestart && tjbot.smooth || (*(int *)KEY_UP == qtrue && tjbot.smoothstyle))
	{
		smoothness = 75.0f;
	}
	if (tjbot.smooth > 0)
	{
		if (tjbot.smoothstyle == 1)
		{
			if (cmd.rightmove > 0 || (cmd.forwardmove > 0 && tjbot.ronebeat))
			{
				if (*tjbot.mouse_x < right_angle - rch + 2)
				{
					*tjbot.mouse_x += (right_angle - rch - *tjbot.mouse_x) / smoothness;
					if (!(tjbot.safestart && tjbot.vel_size < 550))
					{
						yaw = 0.022 - 0.022 * (right_angle - rch + 2 - *tjbot.mouse_x) / 4;// -((100 * rand()) % 5) / 1000;
						if (yaw < 0)
							yaw = 0.0f;
						tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("m_yaw %f;", yaw));
					}

				}
				else if (*tjbot.mouse_x > right_angle - rch + 2)
				{
					notfirst = 1;
					if (yaw < 0)
						yaw = 0.0f;
					tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
				}
				lch = rand() % 1;
			}
			else if (cmd.rightmove < 0 || (cmd.forwardmove > 0 && tjbot.lonebeat))
			{
				if (*tjbot.mouse_x > left_angle + lch - 2)
				{
					*tjbot.mouse_x -= (*tjbot.mouse_x - left_angle + lch) / smoothness;
					if (!(tjbot.safestart && tjbot.vel_size < 550))
					{
						yaw = 0.022 - 0.022 * (*tjbot.mouse_x - left_angle - lch + 2) / 4;// - ((100 * rand()) % 5) / 1000;
						if (yaw < 0)
							yaw = 0.0f;
						tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("m_yaw %f;", yaw));
					}
				}
				else if (*tjbot.mouse_x < left_angle + lch - 2)
				{
					notfirst = 1;
					tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
				}
				rch = rand() % 1;
			}
		}
		else if (tjbot.smoothstyle == 0)
		{
			if (notfirst || (!notfirst && (left_angle > *tjbot.mouse_x || right_angle < *tjbot.mouse_x)))
			{
				if (cmd.rightmove > 0 || (cmd.forwardmove > 0 && tjbot.ronebeat))
				{
					if (!notfirst && (right_angle - *tjbot.mouse_x) > 120)
					{
						notfirst = 0;
						return;
					}
					else if (*tjbot.mouse_x < right_angle)
					{
						*tjbot.mouse_x += (right_angle - *tjbot.mouse_x) / (smoothness + tjbot.aggressiveness * (right_angle - *tjbot.mouse_x - ((float)(rand() % 4 - rand() % 8)) / 10));
					}
					else if (*tjbot.mouse_x > right_angle)
					{
						//if(*tjbot.mouse_x - (*tjbot.mouse_x - right_angle) / (smoothness * 5)> right_angle)
						//	*tjbot.mouse_x -= (*tjbot.mouse_x - right_angle) / (smoothness * 5);
						notfirst = 1;
					}
				}
				else if (cmd.rightmove < 0 || (cmd.forwardmove > 0 && tjbot.lonebeat))
				{
					if (!notfirst && (*tjbot.mouse_x - left_angle) > 120)
					{
						notfirst = 0;
						return;
					}
					else if (*tjbot.mouse_x > left_angle)
					{
						*tjbot.mouse_x -= (*tjbot.mouse_x - left_angle) / (smoothness + tjbot.aggressiveness * (*tjbot.mouse_x - left_angle - ((float)(rand() % 4 - rand() % 8)) / 10));
					}
					else if (*tjbot.mouse_x < left_angle)
					{
						//if(*tjbot.mouse_x + (left_angle - *tjbot.mouse_x) / (smoothness * 5) < left_angle)
						//	*tjbot.mouse_x += (left_angle - *tjbot.mouse_x) / (smoothness * 5);
						notfirst = 1;
					}
				}
			}
		}
	}
	else if(!tjbot.smooth)
	{
		if (cmd.forwardmove >= 0)
		{
			if (cmd.rightmove > 0) // right halfbeat
			{
				*tjbot.mouse_x = right_angle;
			}
			else if (cmd.rightmove < 0) // left halfbeat
			{
				*tjbot.mouse_x = left_angle;
			}
		}
		else if (cmd.forwardmove < 0)
		{ // for viewhack
			if (cmd.rightmove > 0)
			{
				*tjbot.mouse_x = right_angle * (tjbot.view_hack_enable == 1 ? -1 : 1);
			}
			else if (cmd.rightmove < 0)
			{
				*tjbot.mouse_x = left_angle * (tjbot.view_hack_enable == 1 ? -1 : 1);
			}
		}
	}
}

void l_autobot()
{
	*tjbot.forward = *tjbot.back = *tjbot.moveleft = *tjbot.moveright = 0;
	if (*tjbot.mouse_x < left_angle + 20 && *tjbot.mouse_x >= left_angle)
	{
		*tjbot.forward = 1;
	}
	else if (*tjbot.mouse_x < left_angle - 25 && *tjbot.mouse_x >= left_angle - 45)
	{
		*tjbot.forward = *tjbot.moveleft = 1;
	}
	else if (*tjbot.mouse_x < left_angle - 75 && *tjbot.mouse_x >= left_angle - 90)
	{
		*tjbot.moveleft = 1;
	}
	else if (*tjbot.mouse_x < left_angle - 115 && *tjbot.mouse_x >= left_angle - 135)
	{
		*tjbot.moveleft = *tjbot.back = 1;
	}
}

void r_autobot()
{
	*tjbot.forward = *tjbot.back = *tjbot.moveleft = *tjbot.moveright = 0;
	if (*tjbot.mouse_x > right_angle - 20 && *tjbot.mouse_x <= right_angle)
	{
		*tjbot.forward = 1;
	}
	else if (*tjbot.mouse_x > right_angle + 25 && *tjbot.mouse_x <= right_angle + 45)
	{
		*tjbot.forward = *tjbot.moveright = 1;
	}
	else if (*tjbot.mouse_x > right_angle + 75 && *tjbot.mouse_x <= right_angle + 90)
	{
		*tjbot.moveright = 1;
	}
	else if (*tjbot.mouse_x > right_angle + 115 && *tjbot.mouse_x <= right_angle + 135)
	{
		*tjbot.moveright = *tjbot.back = 1;
	}
}

char spoken[1024];
char *color_msg(char *msg)
{
	int len = strlen(msg), ctr = 0, spaces = 0, found = 0;
	char catted[3] = "^ ";
	strcpy(spoken, "");

	while (ctr < len)
	{
		if (msg[ctr] == ' ')
		{
			strcat(spoken, " ");
		}
		else if(msg[ctr] == '@' && msg[ctr + 1] != '\0' && msg[ctr + 1] != ' ')
		{
			char clName[64]; strcpy(clName, "");
			char linK[64]; strcpy(linK, "");
			char aName[64]; strcpy(aName, "");
			char *p;
			if (p = strchr(msg + ctr + 1, ' '))
				strncpy(clName, msg + ctr + 1, strlen(msg + ctr + 1) - strlen(p));
			else if (p = strchr(msg + ctr + 1, '\0'))
				strcpy(clName, msg + ctr + 1);

			for (int con = 0; con < 40; con++)
			{
				strcpy(aName, (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + 0x40 + (unsigned int)(con * CLIENT_SIZE)));
				//tjbot.orig_syscall(CG_PRINT, va("Checking client %d on address %x\n", con, ((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + 0x40 + (unsigned int)(con * CLIENT_SIZE))));
				if (*aName != '\0')
				{
					if(strstr(strlwr(aName), strlwr(clName)))
					{
						//tjbot.orig_syscall(CG_PRINT, va("clName '%s' found in '%s'\n", clName, aName));
						strcpy(linK, (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(con * CLIENT_SIZE)));
						found++;
					}
				}
				if (found > 1)
				{
					//tjbot.orig_syscall(CG_PRINT, va("Found more than 1 matches\n"));
					break;
				}
			}
			//tjbot.orig_syscall(CG_PRINT, va("Printing %s\n", linK));
			if (found == 1)
			{
				strcat(spoken, "^7");
				strcat(spoken, linK);
				strcat(spoken, " ");
				found = 0;
			}
			ctr += strlen(clName) + 1;
		}
		else
		{
			switch (tjbot.colorchat) {
				case 1:
					if (ctr == 0)
					{
						catted[1] = tjbot.colourS[0];
						strcat(spoken, catted);
					}
					else if (ctr == len / 2)
					{
						catted[1] = tjbot.colourS[strlen(tjbot.colourS)-1];
						strcat(spoken, catted);
					}
					strncat(spoken, msg + ctr, 1);
					break;
				case 2:
					strcat(spoken, "^");
					strncat(spoken, msg + ctr, 1);
					strncat(spoken, msg + ctr, 1);
					break;
				case 3:
					catted[1] = (char)(48 + (rand() * rand() * rand()) % 74);
					while (catted[1] == ';' || catted[1] == '^' || catted[1] == '0' || catted[1] == 'P')
						catted[1] = (char)(48 + rand() % 74);
					strcat(spoken, catted);
					strncat(spoken, msg + ctr, 1);
					break;
				case 4:
					catted[1] = tjbot.colourS[(ctr - spaces) % (strlen(tjbot.colourS))];
					strcat(spoken, catted);
					strncat(spoken, msg + ctr, 1);
					break;
				default:
					strncat(spoken, msg + ctr, 1);
					break;
			}
		}
		ctr++;
	}

	return spoken;
}

void cat_msg(char *msg)
{
	if (tjbot.colorchat)
	{
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say \"%s\"", color_msg(msg)));
	}
	else if(!tjbot.colorchat)
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say \"%s\"", msg));
}

void CG_catMessageSend_f(void) {
	char messageText[256];
	int messageType;

	// get values
	tjbot.orig_syscall(6, "cg_messageType", messageText, 256);
	messageType = atoi(messageText);
	tjbot.orig_syscall(6, "cg_messageText", messageText, 256);


	// don't send empty messages
	if (messageText[0] == '\0') {
		return;
	}

	// reset values
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cg_messageText \"\";"));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cg_messageType \"\";"));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cg_messagePlayer \"\";"));

	// team say
	if (messageType == 2) {
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say_team \"%s\"", messageText));

		// fireteam say
	}
	else if (messageType == 3) {
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say_buddy \"%s\"", messageText));

		// normal say
	}
	else {
		cat_msg(messageText);
	}

}

static void CG_catVoiceChat_f(void) {
	char chatCmd[64];

	if (tjbot.orig_syscall(CG_ARGC) != 2) {
		return;
	}

	tjbot.orig_syscall(CG_ARGV, 1, chatCmd, 64);

	char catChat[128];
	int i;
	for ( i = 0 ; i < 53 ; i++ ) 
	{
		if (strstr(vc_cmds[i].cmd, chatCmd))
			if(chatCmd[0] == (vc_cmds[i].cmd)[0])
			{
				strcpy(catChat, color_msg(vc_cmds[i].res));
				break;
			}
	}

	//if(tjbot.colorchat) tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cmd vsay %s %s\n", chatCmd, catChat));
	//else tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cmd vsay %s\n", chatCmd));

	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("cmd say %s\n", catChat));

	if (i == 51) tjbot.chatDelay[0] = 20 * 125;
	if (i == 25) tjbot.chatDelay[1] = 20 * 125;
}

// Automatically jumps if not in the air
int wait = 0, k_off = 0, iCond = 0;
static void autojump() {
	float orig_i[3], end[3];
	end[0] = orig_i[0] = *tjbot.orig_x;
	end[1] = orig_i[1] = *tjbot.orig_y;
	end[2] = orig_i[2] = *tjbot.orig_z;
	end[2] = orig_i[2] - 18;
	vec3_t mins = { -15, -15, 0 }, maxs = { 15, 15, 2 };

	trap_CM_BoxTrace(&trace, orig_i, end, mins, maxs, 0, MASK_PLAYERSOLID);
	/*if (trace.surfaceFlags == 3 && iCond == 0)
	{

		trap_CM_BoxTrace(&iceTrace, orig_i, end, 0, 0, 0, MASK_PLAYERSOLID);
		if(iceTrace.plane.dist)
		*(int *)KEY_UP = qfalse;
	}*/
	if (tjbot.smooth == 0 && tjbot.jumpbot_enable)
	{
		if(isOnground)
			*(int *)KEY_UP = qtrue;
		else
			*(int *)KEY_UP = qfalse;
	}
	else
	{
		if ((trace.plane.dist || isOnground || isNoclip || isInwater)
			|| (*(char *)((unsigned int)tjbot.cgame_handle + IS_PRONED) == 0x01))
		{
			*(int *)KEY_UP = qtrue;
			k_off = tjbot.jumpdelay + rand() % 7;
		}
		else
		{
			if (isOnground)
				k_off = tjbot.jumpdelay + rand() % 7;

			if (trace.plane.dist == 0 && k_off == 0)
			{
				*(int *)KEY_UP = qfalse;
			}
			else if (k_off)
				k_off--;
			else
				*(int *)KEY_UP = qfalse;
		}
	}
	iCond = 1;
	//CG_DrawStringExt(290, 350, va("%d", *tjbot.ground_ent_num), g_color_table[7], qtrue, qfalse, 10, 15, 80);
}

void crazy_bot() {
	*tjbot.sprint = qtrue;
	if (*tjbot.moveright)
	{
		*tjbot.moveright = qfalse;
		*tjbot.moveleft = qtrue;
	}
	else
	{
		*tjbot.moveright = qtrue;
		*tjbot.moveleft = qfalse;
	}
}

int onground_frames = 0;
static void ob()
{
	if (isOnground)
	{
		if (onground_frames > 0)
			*(int *)KEY_UP = qtrue;
		else
			onground_frames++;
	}
	else
	{
		*(int *)KEY_UP = qfalse;
		onground_frames = 0;
	}
}

// Help function
static void reset_keys() {
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "-speed;");
	*tjbot.moveleft = 0;
	*tjbot.moveright = 0;
	*tjbot.forward = 0;
	*tjbot.back = 0;
	*tjbot.moveup = 0;
	*tjbot.movedown = 0;
	*tjbot.prone = 0;
	*tjbot.sprint = 0;
}

// Tries to move to a given origin
static void origin_move() {
	*tjbot.mouse_x += 0.0f - *tjbot.view_x;
    reset_keys();
	if ((abs((int)(tjbot.origin_x - *tjbot.orig_x)) < 5) && (abs((int)(tjbot.origin_y - *tjbot.orig_y)) < 5))
	{
		tjbot.origin_enable = 0;
		reset_keys();
	}
	else
	{
		if ((abs((int)(tjbot.origin_x - *tjbot.orig_x)) < 100) && (abs((int)(tjbot.origin_y - *tjbot.orig_y)) < 100))
			tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "+speed;");
		if (tjbot.origin_x + 1 > *tjbot.orig_x)
		{
			*tjbot.forward = 1;
			if (tjbot.origin_y + 1 > *tjbot.orig_y)
				*tjbot.moveleft = 1;
			else if (tjbot.origin_y - 1 < *tjbot.orig_y)
				*tjbot.moveright = 1;
		}
		else if (tjbot.origin_x - 1 < *tjbot.orig_x)
		{
			*tjbot.back = 1;
			if (tjbot.origin_y + 1 > *tjbot.orig_y)
				*tjbot.moveleft = 1;
			else if (tjbot.origin_y - 1 < *tjbot.orig_y)
				*tjbot.moveright = 1;
		}
	}
}

// Starts dumping (opens specified file for dumping)
static void start_dump() {
    if (tjbot.dump != NULL) {
        tjbot.orig_syscall(CG_PRINT, "Already dumping.\n");
        return;
    }
    if (tjbot.orig_syscall(CG_ARGC) != 2) {
        tjbot.orig_syscall(CG_PRINT, START_DUMP_COMMAND " <filename>\n");
        return;
    }
    if ((tjbot.dump = fopen(CG_Argv(1), "w")) == NULL) {
        tjbot.orig_syscall(CG_PRINT, "Error opening specified file.\n");
        return;
    }
}

char *va(char *format, ...) {
    static char buf[MAX_STRING_CHARS];
    va_list va;
    va_start(va, format);
    vsprintf(buf, format, va);
    va_end(va);
    return buf;
}

int safesay[] = { 94, 57, 77, 97, 100, 101, 32, 98, 121, 32, 94, 48, 83, 116, 114, 97, 121, 94, 119, 99, 97, 116 };

int isPremium(char *str)
{
	if (strlen(str) == 4)
		if (str[0] == '!')
			if (str[1] == 'p')
				if (str[2] == 'r')
					if (str[3] == 'o')
						return 1;
	return 1;
}

char *uncrypt(int *say, int l)
{
	char ret[33];
	int k;
	for (k = 0; k < l; k++)
	{
		ret[k] = say[k];
	}
	return ret;
}
static void graf1() {
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                ^o _                        \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                ^o \\`*^k-.                    \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                 ^k)  ^o _^k`-.                 \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                ^k.  ^o :^I|^o`. ^k.                \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                ^8: ^9_   ^o \'^k  \\               \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s;", "\"                ^o; ^w*^k` _.   `*-^o._          \""));
}

static void graf2() {
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                ^)`^w-.-\'          ^o`-.       \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^8;       `       `.     \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^k:.       .        \\    \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^8. \\  .   :   .-\'   .   \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^o\'  ^8`+.^o;  ;  \'      :   \""));
}

static void graf3() {
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^o:  \'  |    ;       ;^8-. \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                  ^k; \'   : :`^8-^k:     _.`^w* ^8;\""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"                ^k.*\' /  .*\' ; .*`- +\'  ^8`^o*\' \""));
	tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("say %s; ", "\"               ^o`*-*   `*-*  `*-*\'        \""));
}

static void spray() {
	tjbot.orig_syscall(CG_PRINT, ("\n%s\n", infomsg1));
	tjbot.orig_syscall(CG_PRINT, ("%s\n", infomsg2));
}

// replaced functions
int syscall(int cmd, ...) {
    int args[11];
    va_list va;
    va_start(va, cmd);
    for (int i = 0; i < sizeof(args) / sizeof(args[0]); i++)
        args[i] = va_arg(va, int);
    va_end(va);

    if (cmd == CG_R_RENDERSCENE && tjbot.view_hack_enable) {
        // 180 degs rotation matrix
        float mat[3][3] = {
                {-1.0f, 0.0f,  0.0f},
                {0.0f,  -1.0f, 0.0f},
                {0.0f,  0.0f,  1.0f}
        }; 

		refdef_t *rd = (refdef_t *)args[0];
        float res[3][3];
		MatrixMultiply(rd->viewaxis, mat, res);
        memcpy(rd->viewaxis, res, sizeof(rd->viewaxis));
        return tjbot.orig_syscall(cmd, rd);
    }
    return tjbot.orig_syscall(cmd, args[0], args[1], args[2], args[3], args[4],
                              args[5], args[6], args[7], args[8], args[9], args[10]);
}

void cg_dllEntry(int(*syscallptr)(int arg, ...)) {
    tjbot.orig_syscall = syscallptr;
    tjbot.orig_cg_dllEntry(syscall);
}

void update_cvars(char arg_s[3][64], int arg_d[6], float arg_f[2]) 
{
	strcpy(tjbot.password, arg_s[0]);
	strcpy(tjbot.styles, arg_s[1]);
	strcpy(tjbot.colourS, arg_s[2]);
	set_styles(tjbot.styles);
	tjbot.colorchat = arg_d[0];
	tjbot.safestart = arg_d[1];
	tjbot.autosmooth = arg_d[2];
	if (isPremium(tjbot.password))
		tjbot.draw_cgaz = arg_d[3];
	else
		tjbot.draw_cgaz = 0;
	tjbot.jumpdelay = arg_d[4];
	tjbot.smoothstyle = arg_d[5];
	tjbot.smooth = arg_f[0];
	tjbot.aggressiveness = arg_f[1];
}


void write_config()
{
	FILE *settings_file;
	settings_file = fopen("settings.cfg", "w");
	int row_ctr = 0;
	fprintf(settings_file, "%s %s\n", CMD_LIST[row_ctr++], tjbot.password);
	fprintf(settings_file, "%s %s\n", CMD_LIST[row_ctr++], tjbot.styles);
	fprintf(settings_file, "%s %s\n", CMD_LIST[row_ctr++], tjbot.colourS);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.colorchat);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.safestart);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.autosmooth);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.draw_cgaz);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.jumpdelay);
	fprintf(settings_file, "%s %d\n", CMD_LIST[row_ctr++], tjbot.smoothstyle);
	fprintf(settings_file, "%s %.1f\n", CMD_LIST[row_ctr++], tjbot.smooth);
	fprintf(settings_file, "%s %.1f\n", CMD_LIST[row_ctr++], tjbot.aggressiveness);
	fclose(settings_file);
}

void tjbot_reset()
{
	strcpy(tjbot.password, "");
	strcpy(tjbot.styles, "fi"); set_styles("fi");
	strcpy(tjbot.colourS, "22");
	tjbot.colorchat = 0;
	tjbot.safestart = 0;
	tjbot.smooth = 40.0f;
	tjbot.autosmooth = 0;
	tjbot.draw_cgaz = 0;
	tjbot.jumpdelay = 10;
	tjbot.smoothstyle = 0;
	tjbot.aggressiveness = 20.0f;
	write_config();
}

char saved_time[11];
char HUDstring[80];
char txt[12];
int pbline = 0, onlyOnce = 1;

int cg_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8,
              int arg9, int arg10, int arg11) {
	switch (command) {
	case CG_INIT:
	{
		tjbot.orig_syscall(CG_REMOVECOMMAND, "messageSend");
		tjbot.orig_syscall(CG_REMOVECOMMAND, "VoiceChat");
		tjbot.orig_syscall(CG_REMOVECOMMAND, "VoiceTeamChat");
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_BOT_L_ON);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_BOT_L_OFF);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_BOT_R_ON);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_BOT_R_OFF);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTO_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, BOT_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, BOT_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, LONEBEAT_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, LONEBEAT_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, OB_ON);
		tjbot.orig_syscall(CG_ADDCOMMAND, OB_OFF);
		tjbot.orig_syscall(CG_ADDCOMMAND, CRAZY_ON);
		tjbot.orig_syscall(CG_ADDCOMMAND, CRAZY_OFF);
		tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, PS_PRINT_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, PS_PRINT_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, RONEBEAT_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, RONEBEAT_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, VIEW_HACK_ON_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, VIEW_HACK_OFF_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, ANGLE_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, CHAT_BOT);
		tjbot.orig_syscall(CG_ADDCOMMAND, CHAT_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, CHAT_COLOR);
		tjbot.orig_syscall(CG_ADDCOMMAND, CGAZ_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, CJ_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, INFO_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, JUMP_DELAY_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, MS_PRINT_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, MS_RESET_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_SET_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, ORIGIN_GET_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, PASSWORD_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, RESET_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, REC_START);
		tjbot.orig_syscall(CG_ADDCOMMAND, REC_STOP);
		tjbot.orig_syscall(CG_ADDCOMMAND, PB_START);
		tjbot.orig_syscall(CG_ADDCOMMAND, PB_STOP);
		tjbot.orig_syscall(CG_ADDCOMMAND, REC_FILE);
		tjbot.orig_syscall(CG_ADDCOMMAND, SAY_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, SAFE_START_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, SMOOTH_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, SMOOTHNESS_STYLE);
		tjbot.orig_syscall(CG_ADDCOMMAND, AUTOSMOOTH_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, START_DUMP_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, STOP_DUMP_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, STYLES_COMMAND);
		tjbot.orig_syscall(CG_ADDCOMMAND, SMOOTHNESS_AGGRESSIVE);
		tjbot.orig_syscall(CG_ADDCOMMAND, "messageSend");
		tjbot.orig_syscall(CG_ADDCOMMAND, "cg_messageText");
		tjbot.orig_syscall(CG_ADDCOMMAND, "VoiceChat");
		tjbot.orig_syscall(CG_ADDCOMMAND, "VoiceTeamChat");

		tjbot.ob_on = tjbot.crazy = tjbot.chatBot = tjbot.offsetno = tjbot.catspray = tjbot.chatDelay[0] = tjbot.chatDelay[1] = tjbot.chatDelay[2] = tjbot.chatDelay [3] = tjbot.cj_info = 0;
		tjbot.isAvailable = 1;
		strcpy(tjbot.colourS, "22");
		strcpy(tjbot.rec_file, "test");

		int error = 0;
		if (!doesFileExist("settings.cfg"))
		{

			tjbot.settings_file = fopen("settings.cfg", "w");
			tjbot_reset();
			fclose(tjbot.settings_file);
		}
		else if (doesFileExist("settings.cfg"))
		{
			tjbot.settings_file = fopen("settings.cfg", "r");

			char buff[100], cmd_arg_str[3][64];
			int line_ctr = 0, cmd_arg_d[6];
			float cmd_arg_f[2];

			while (fgets(buff, sizeof(buff), tjbot.settings_file) != NULL)
			{

				if (strstr(buff, CMD_LIST[line_ctr]))
				{
					char cmd_arg[31];
					strcpy(cmd_arg, (buff + strlen(CMD_LIST[line_ctr]) + 1));
					char *p = strchr(cmd_arg, '\n');
					if (line_ctr < 3)
					{

						int arglen = strlen(cmd_arg);
						strncpy(cmd_arg_str[line_ctr], cmd_arg, arglen - 1);
						cmd_arg_str[line_ctr][arglen - 1] = 0;
					}
					else if (line_ctr < 9)
					{
						cmd_arg_d[line_ctr - 3] = (int)strtol(cmd_arg, &p, 10);
					}
					else if (line_ctr < 11)
					{
						cmd_arg_f[line_ctr - 9] = (float)strtof(cmd_arg, &p);
					}
				}
				else if (*buff != '\n')
				{
					error = 1;
				}

				if (line_ctr > 9)
				{
					error = 1;
					break;
				}
				line_ctr++;
			}

			if (line_ctr < 9)
				error = 1;

			if (error)
			{
				tjbot_reset();
			}

			update_cvars(cmd_arg_str, cmd_arg_d, cmd_arg_f);

			fclose(tjbot.settings_file);
		}

		get_offsets();

		Beep(20, 2000);

		break;
	}
	case CG_SHUTDOWN:
	{
		// Warning: file is closed for vid_restart too
		if (tjbot.jumpbot_enable)
			tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
		if (tjbot.dump) {
			write_config();
			fclose(tjbot.dump);
			tjbot.dump = NULL;
		}
		if (tjbot.settings_file) {
			fclose(tjbot.settings_file);
			tjbot.settings_file = NULL;
		}
	}
        case CG_CONSOLE_COMMAND: 
		{
			const char *cmd = CG_Argv(0);
			if (!strcmp(cmd, BOT_ON_COMMAND)) {
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
				tjbot.groundaccel = tjbot.airaccel = 0.1f;
				tjbot.max_gndaccel = tjbot.max_airaccel = 1.1f;
				saved_air = saved_gnd = 0.1f;
				tjbot.jumpbot_enable = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, BOT_OFF_COMMAND)) {
				notfirst = 0;
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "m_yaw 0.022;");
				tjbot.jumpbot_enable = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_BOT_L_ON))
			{
				if (tjbot.r_autobot == 0)
					tjbot.l_autobot = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_BOT_L_OFF))
			{
				if (tjbot.l_autobot)
					tjbot.l_autobot = 0;
				*tjbot.forward = *tjbot.back = *tjbot.moveleft = *tjbot.moveright = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_BOT_R_ON))
			{
				if (tjbot.l_autobot == 0)
					tjbot.r_autobot = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_BOT_R_OFF))
			{
				if (tjbot.r_autobot)
					tjbot.r_autobot = 0;
				*tjbot.forward = *tjbot.back = *tjbot.moveleft = *tjbot.moveright = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_ON_COMMAND)) {
				tjbot.autojump_enable = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, AUTO_OFF_COMMAND)) {
				*(int *)KEY_UP = qfalse;
				k_off = wait = iCond = tjbot.autojump_enable = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, OB_ON))
			{
				tjbot.ob_on = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, OB_OFF))
			{
				*(int *)KEY_UP = 0;
				tjbot.ob_on = 0;
				onground_frames = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, CRAZY_ON))
			{
				tjbot.crazy = 1;
				tjbot.jumpbot_enable = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, CRAZY_OFF))
			{
				tjbot.crazy = 0;
				tjbot.jumpbot_enable = 0;
				reset_keys();
				return qtrue;
			}
			else if (!strcmp(cmd, REC_START) && !tjbot.playback) {
				if (strlen(tjbot.rec_file))
				{
					if (tjbot.playback_file == NULL)
					{
						tjbot.orig_syscall(CG_PRINT, va("Started recording!\n"));
						tjbot.playback_file = fopen(tjbot.rec_file, "w+");
						tjbot.rec = 1;
					}
					else
					{
						tjbot.orig_syscall(CG_PRINT, va("file %s already opened\n", tjbot.rec_file));
						tjbot.orig_syscall(CG_PRINT, va("Started recording!\n"));
						fclose(tjbot.playback_file);
						tjbot.playback_file = fopen(tjbot.rec_file, "w+");
						tjbot.rec = 1;
					}
				}
				return qtrue;
			}
			else if (!strcmp(cmd, REC_STOP)) {
				if (tjbot.playback_file && tjbot.rec)
				{
					fprintf(tjbot.playback_file, "EOF\n");
					tjbot.orig_syscall(CG_PRINT, va("Closing file %s\n", tjbot.rec_file));
					fclose(tjbot.playback_file);
					tjbot.rec = 0;
				}
				return qtrue;
			}
			else if (!strcmp(cmd, PB_START)) {
				if (strlen(tjbot.rec_file) && !tjbot.rec && !tjbot.playback)
				{
					if (doesFileExist(tjbot.rec_file))
					{
						tjbot.playback_file = fopen(tjbot.rec_file, "r");
						tjbot.playback = 1;
						pbline = 0;
						tjbot.orig_syscall(CG_PRINT, va("Started playback!\n"));
					}
					else
					{
						tjbot.orig_syscall(CG_PRINT, va("File %s does not exist!\n", tjbot.rec_file));
					}
				}
				return qtrue;
			}
			else if (!strcmp(cmd, PB_STOP)) {
				if (tjbot.playback && tjbot.playback_file)
				{
					tjbot.orig_syscall(CG_PRINT, va("Closing file %s\n", tjbot.rec_file));
					fclose(tjbot.playback_file);
					reset_keys();
				}
				tjbot.playback = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, ORIGIN_ON_COMMAND)) {
				tjbot.origin_enable = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, ORIGIN_OFF_COMMAND)) {
				reset_keys();
				tjbot.origin_enable = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, VIEW_HACK_ON_COMMAND)) {
				tjbot.view_hack_enable = 1;
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "bind a \"+moveright\"; bind d \"+moveleft\"; bind w \"+back\"; bind s \"+forward\"; ");
				return qtrue;
			}
			else if (!strcmp(cmd, VIEW_HACK_OFF_COMMAND)) {
				tjbot.view_hack_enable = 0;
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "bind a \"+moveleft\"; bind d \"+moveright\"; bind w \"+forward\"; bind s \"+back\"; ");
				return qtrue;
			}
			else if (!strcmp(cmd, PS_PRINT_ON_COMMAND)) {
				tjbot.ps_print_enable = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, PS_PRINT_OFF_COMMAND)) {
				tjbot.ps_print_enable = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, RONEBEAT_ON_COMMAND)) {
				tjbot.ronebeat = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, RONEBEAT_OFF_COMMAND)) {
				tjbot.ronebeat = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, LONEBEAT_ON_COMMAND)) {
				tjbot.lonebeat = 1;
				return qtrue;
			}
			else if (!strcmp(cmd, LONEBEAT_OFF_COMMAND)) {
				tjbot.lonebeat = 0;
				return qtrue;
			}
			else if (!strcmp(cmd, "VoiceChat"))
			{
				CG_catVoiceChat_f();
				return qtrue;
			}
			else if (!strcmp(cmd, "VoiceTeamChat"))
			{
				CG_catVoiceChat_f();
				return qtrue;
			}
			else if (!strcmp(cmd, "messageSend"))
			{
				CG_catMessageSend_f();
				return qtrue;
			}
			else if (!strcmp(cmd, "cg_messageText"))
			{
				strcpy(tjbot.sayBuffer, CG_Argv(1));
				return qtrue;
			}
			else if (!strcmp(cmd, ORIGIN_SET_COMMAND))
			{
				if (strlen(CG_Argv(1)) && strlen(CG_Argv(2)))
				{
					tjbot.origin_x = (float)atof(CG_Argv(1));
					tjbot.origin_y = (float)atof(CG_Argv(2));
				}
				else
				{
					tjbot.origin_x = *tjbot.orig_x;
					tjbot.origin_y = *tjbot.orig_y;
				}
				return qtrue;
			}
			else if (!strcmp(cmd, RESET_COMMAND))
			{
				tjbot_reset();
				write_config();
				return qtrue;
			}
			else if (!strcmp(cmd, SAY_COMMAND))
			{
				CG_catMessageSend_f();
				return qtrue;
			}
			else if (!strcmp(cmd, ORIGIN_GET_COMMAND))
			{
				tjbot.orig_syscall(CG_PRINT, va("origin: %f %f %f\n", *tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z));
				return qtrue;
			}
			else if (!strcmp(cmd, CHAT_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.colorchat = (int)strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_colorchat: %i\n", tjbot.colorchat));
					tjbot.orig_syscall(CG_PRINT, va("^wCustom coloured chat\n"));
					tjbot.orig_syscall(CG_PRINT, va("^wPress \\ instead of ENTER to send msg!(only via console)\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, SAFE_START_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.safestart = (int)strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_safestart: %i\n", tjbot.safestart));
					tjbot.orig_syscall(CG_PRINT, va("^wHigh smoothness for tjbot at start\n"));
					tjbot.orig_syscall(CG_PRINT, va("^wRequires sc_smoothness > 0\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, JUMP_DELAY_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.jumpdelay = (int)(strtof(CG_Argv(1), NULL));
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_jumpdelay: %i   [default = 10]\n", tjbot.jumpdelay));
					tjbot.orig_syscall(CG_PRINT, va("^wAmount of frames jump key is pressed\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, SMOOTHNESS_STYLE))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.smoothstyle = (int)(strtof(CG_Argv(1), NULL));
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_smoothness_style: %i   [default = 0]\n", tjbot.smoothstyle));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, SMOOTH_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.smooth = strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_smoothness: %f   [default = 20]\n", tjbot.smooth));
					tjbot.orig_syscall(CG_PRINT, va("^wAmount of smoothness for tjbot movement\n"));
					tjbot.orig_syscall(CG_PRINT, va("^w0 = bot movement ; greater values = smoother movement\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, CJ_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.cj_info = strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_cj_info: %.0f   [default = 1]\n", tjbot.cj_info));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, CHAT_BOT))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.chatBot = (int)strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_chat_bot: %d   [default = 1]\n", tjbot.chatBot));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, AUTOSMOOTH_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.autosmooth = (int)strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_smoothness_auto: %i\n", tjbot.autosmooth));
					tjbot.orig_syscall(CG_PRINT, va("^wSets smoothness automatically based on speed\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, CGAZ_COMMAND))
			{
				if (!isPremium(tjbot.password))
				{
					tjbot.orig_syscall(CG_PRINT, va("Premium feature only!\n"));
					return qtrue;
				}
				if (strlen(CG_Argv(1)))
				{
					tjbot.draw_cgaz = (int)strtof(CG_Argv(1), NULL);
					draw_cgaz();
				}
				else if (!strlen(CG_Argv(1)))
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_drawCgaz: %i\n", tjbot.draw_cgaz));
					tjbot.orig_syscall(CG_PRINT, va("^wHides status of cgaz on the server\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, STYLES_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					strcpy(tjbot.styles, CG_Argv(1));
					set_styles(tjbot.styles);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^9sc_styles = %s  ^w  default : fi\n", tjbot.styles));
					tjbot.orig_syscall(CG_PRINT, va("^9Usage : f = fullbeat ; i = invert ; b = backwards\n"));
					tjbot.orig_syscall(CG_PRINT, va("^9Example : /sc_styles fib ^w <- enables fullbeat, invert, backwards\n\n"));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, CHAT_COLOR))
			{
				if (strlen(CG_Argv(1)))
				{
					strcpy(tjbot.colourS, CG_Argv(1));
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^9sc_coloured_chat_colors = %s\n", tjbot.colourS));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, REC_FILE))
			{
				if (strlen(CG_Argv(1)))
				{
					strcpy(tjbot.rec_file, CG_Argv(1));
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("^wsc_recfile: %s\n", tjbot.rec_file));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, ANGLE_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					float a = strtof(CG_Argv(1), NULL);
					*tjbot.mouse_x += a - *tjbot.view_x;
				}
				else
					tjbot.orig_syscall(CG_PRINT, va("^9Rotates view angle by [amount] degrees\n"));
				return qtrue;
			}
			else if (!strcmp(cmd, SMOOTHNESS_AGGRESSIVE))
			{
				if (strlen(CG_Argv(1)))
				{
					tjbot.aggressiveness = strtof(CG_Argv(1), NULL);
				}
				else
				{
					tjbot.orig_syscall(CG_PRINT, va("tjbot.aggressiveness = %f [default = 20]\n", tjbot.aggressiveness));
				}
				return qtrue;
			}
			else if (!strcmp(cmd, PASSWORD_COMMAND))
			{
				if (strlen(CG_Argv(1)))
				{
					strcpy(tjbot.password, CG_Argv(1));
				}
				else
					tjbot.orig_syscall(CG_PRINT, va("^9sc_password = %s\n", tjbot.password));
				return qtrue;
			}
			else if (!strcmp(cmd, START_DUMP_COMMAND))
			{
				start_dump();
				return qtrue;
			}
			else if (!strcmp(cmd, STOP_DUMP_COMMAND))
			{
				if (tjbot.dump) {
					fclose(tjbot.dump);
					tjbot.dump = NULL;
				}
				return qtrue;
			}
			else if (!strcmp(cmd, MS_PRINT_COMMAND))
			{
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND,
					va("orig_say ^7(^eTJ^7Bot) my max speed is ^e%.2f ^7UPS\n", tjbot.max_speed));
				return qtrue;
			}
			else if (!strcmp(cmd, MS_RESET_COMMAND))
			{
				tjbot.max_speed = 0.0f;
				return qtrue;
			}
			else if (!strcmp(cmd, INFO_COMMAND))
			{
				tjbot.catspray = 1;
				return qtrue;
			}
			break;
		}
        default:
            break;
    }

    int res = tjbot.orig_cg_vmMain(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

	switch (command) {
	case CG_DRAW_ACTIVE_FRAME:
	{
	}

	isOnground = *tjbot.ground_ent_num != ENTITYNUM_NONE ? 1 : 0;
	isNoclip = *(int *)((unsigned int)tjbot.cgame_handle + 0x1DEAF5C) == 1 ? 1 : 0;
	isInwater = *(int *)(tjbot.et_address + 0x13885CC) == 1 ? 1 : 0;

	vel_angle = rad2deg(atan2(*tjbot.vel_y, *tjbot.vel_x));

	accel_coef = isOnground ? (4.1f) : (4.0f);
	per_angle = AngleNormalize180(rad2deg(acos((MIN_SPEED - MIN_SPEED / 125.0f * accel_coef) / tjbot.vel_size * 1.1f)));
	accel_angle = rad2deg(atan2(-cmd.rightmove, cmd.forwardmove));


	left_angle = (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle + per_angle));
	right_angle = (*tjbot.mouse_x - AngleDelta(*tjbot.view_x + accel_angle, vel_angle - per_angle));


	cmd_num = tjbot.orig_syscall(CG_GETCURRENTCMDNUMBER);
	tjbot.orig_syscall(CG_GETUSERCMD, cmd_num, &cmd);


	if (tjbot.rec)
	{
		/*int buff, left, right, back, forth, up, down, sprint, prone;

		tjbot.s_key = (unsigned char *)((unsigned int)tjbot.cgame_handle + SPEC_KEY);
		tjbot.s_prone = (unsigned char *)((unsigned int)tjbot.cgame_handle + SPEC_PRONE);
		tjbot.s_sprint = (unsigned char *)((unsigned int)tjbot.cgame_handle + SPEC_SPRINT);

		if (*tjbot.s_sprint == 0xA0) sprint = 1; else sprint = 0;
		if (*tjbot.s_prone == 0x80) prone = 1; else prone = 0;
		if ((unsigned int)*tjbot.s_key % 0x10 == 0x1) forth = 1; else forth = 0;
		if ((unsigned int)*tjbot.s_key % 0x10 == 0x2) back = 1; else back = 0;
		if ((unsigned int)*tjbot.s_key % 0x10 == 0x4) left = 1; else left = 0;
		if ((unsigned int)*tjbot.s_key % 0x10 == 0x8) right = 1; else right = 0;
		if ((unsigned int)*tjbot.s_key % 0x10 == 0x5) { forth = 1; left = 1; }
		else if ((unsigned int)*tjbot.s_key % 0x10 == 0x9) { forth = 1; right = 1; }
		else if ((unsigned int)*tjbot.s_key % 0x10 == 0x6) { back = 1; left = 1; }
		else if ((unsigned int)*tjbot.s_key % 0x10 == 0xA) { back = 1; right = 1; }
		if ((unsigned int)*tjbot.s_key / 0xF == 0x1) up = 1; else up = 0;
		if ((unsigned int)*tjbot.s_key / 0xF == 0x2) down = 1; else down = 0;
		fprintf(tjbot.playback_file, "%d%d%d%d%d%d%d%d%.1f\n", left, right, forth, back, up, down, sprint, prone, *tjbot.view_x);*/
		
		fprintf(tjbot.playback_file, "%d%d%d%d%d%d%d%d%.1f\n", (cmd.rightmove < 0 ? 1 : 0), (cmd.rightmove > 0 ? 1 : 0), (cmd.forwardmove > 0 ? 1 : 0), (cmd.forwardmove < 0 ? 1 : 0), (cmd.upmove > 0 ? 1 : 0), (cmd.upmove < 0 ? 1 : 0), (cmd.buttons == 160 ? 1 : 0), (cmd.wbuttons == 128 ? 1 : 0), *tjbot.view_x);
		
	}

	if (tjbot.playback)
	{
		char buff[100], strvalue[16];
		int left, right, back, forth, up, down, sprint, prone;
		float ang; 
		fgets(buff, sizeof(buff), tjbot.settings_file);
		if (strstr(buff, "EOF"))
		{
			tjbot.orig_syscall(CG_PRINT, va("Closing file %s\n", tjbot.rec_file));
			fclose(tjbot.playback_file);
			reset_keys();
			tjbot.playback = 0;
		}
		else if (strlen(buff))
		{
			char *p = strchr(buff, '\n');
			strncpy(strvalue, buff, 1); left = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 1, 1); right = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 2, 1); forth = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 3, 1); back = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 4, 1); up = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 5, 1); down = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 6, 1); sprint = (int)strtol(strvalue, NULL, 10);
			strncpy(strvalue, buff + 7, 1); prone = (int)strtol(strvalue, NULL, 10);
			strcpy(strvalue, buff + 8); ang = (float)strtof(strvalue, NULL);

			if (left)  *tjbot.moveleft = qtrue;
			else  *tjbot.moveleft = qfalse;

			if (right) *tjbot.moveright = qtrue;
			else *tjbot.moveright = qfalse;

			if (forth) *tjbot.forward = qtrue;
			else *tjbot.forward = qfalse;

			if (back) *tjbot.back = qtrue;
			else *tjbot.back = qfalse;

			if (up) *tjbot.moveup = qtrue;
			else *tjbot.moveup = qfalse;

			if (down) *tjbot.movedown = qtrue;
			else *tjbot.movedown = qfalse;

			if (sprint) *tjbot.sprint = qtrue;
			else *tjbot.sprint = qfalse;

			if (prone) *tjbot.prone = qtrue;
			else *tjbot.prone = qfalse;

			*tjbot.mouse_x += ang - *tjbot.view_x;
		}
	}

	tjbot.velsize = (float)sqrt(*tjbot.velx * *tjbot.velx + *tjbot.vely * *tjbot.vely);
	tjbot.vel_size = (float)sqrt(*tjbot.vel_x * *tjbot.vel_x + *tjbot.vel_y * *tjbot.vel_y);

	// max speed
	if (tjbot.vel_size > tjbot.max_speed)
		tjbot.max_speed = tjbot.vel_size;
	strncpy(tjbot.curr_uinfo, (char *)((unsigned int)tjbot.cgame_handle + (tjbot.offsetno == 0 ? ARG1_UINFO_TWC : ARG1_UINFO_999)), 4);
	if (isPremium(tjbot.password) && tjbot.draw_cgaz)
	{
		if (tjbot.log)
			tjbot.orig_syscall(CG_PRINT, va("pass = %s\nbot_uinfo = %s", tjbot.password, tjbot.curr_uinfo));
		if (!strcmp(tjbot.curr_uinfo, "1533") || !strcmp(tjbot.curr_uinfo, "1501"))
		{
			*tjbot.cgaz_number = 0;
			if (sw)
			{
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "etj_drawcgaz -1;");
				sw = 0;
			}
			else if (!sw)
			{
				tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "etj_drawcgaz 0;");
				sw = 1;
			}
		}
		else
			*tjbot.cgaz_number = tjbot.draw_cgaz;
	}

	if (tjbot.jumpbot_enable) jump_bot();
	if (tjbot.l_autobot && tjbot.frame_count % 2) l_autobot();
	if (tjbot.r_autobot && tjbot.frame_count % 2) r_autobot();
	if (tjbot.autojump_enable) autojump(); else tjbot.delayedframes = 0;
	if (tjbot.ob_on) ob();
	if (tjbot.origin_enable) origin_move();
	if (tjbot.dump)
		fprintf(tjbot.dump, "view: %f %f, origin: %f %f %f, velocity: %f %f %f\n", *tjbot.view_x, *tjbot.view_y,
			*tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z, *tjbot.vel_x, *tjbot.vel_y, *tjbot.vel_z);
	if (tjbot.ps_print_enable)
		tjbot.orig_syscall(CG_PRINT, va("view: %f %f, origin : %f %f %f, velocity : %f %f %f\n", *tjbot.view_x,
			*tjbot.view_y,
			*tjbot.orig_x, *tjbot.orig_y, *tjbot.orig_z, *tjbot.vel_x, *tjbot.vel_y,
			*tjbot.vel_z));

	/////// settings file update
	if (tjbot.frame_count >= 1250)
	{
		write_config();
		tjbot.frame_count = 0;
	}
	tjbot.frame_count++;

	prev_vel_size[19] = tjbot.vel_size; prevvelsize[19] = *tjbot.plspeed;
	saved_zpos[19] = *tjbot.orig_z;
	for (int i = 0; i < 19; i++)
	{
		prev_vel_size[i] = prev_vel_size[i + 1];
		prevvelsize[i] = prevvelsize[i + 1];
		saved_zpos[i] = saved_zpos[i + 1];
	}

	if (tjbot.catspray)
	{
		if (tjbot.catspray == 200)
			graf1();
		if (tjbot.catspray == 351)
			graf2();
		if (tjbot.catspray == 502)
			graf3();
		tjbot.catspray++;
		if (tjbot.catspray > 702)
			tjbot.catspray = 0;
	}

	/*if (!isPremium(tjbot.password))
	{
		if (tjbot.info_count)
		{
			if (tjbot.info_count > 75000)
			{
				//spray();
				//tjbot.orig_syscall(CG_PRINT, va(uncrypt(safesay, sizeof(safesay) / sizeof(safesay[0]))));
				//tjbot.orig_syscall(CG_PRINT, "\n");
				tjbot.info_count = 0;
				//tjbot.catspray = 1;
			}

		}
		tjbot.info_count++;

		char *chat = (char *)((tjbot.offsetno == 0 ? (unsigned int)tjbot.cgame_handle + SERVERCHAT : tjbot.et_address + CHAT999));
		if (strstr(chat, "^5^6^7^1m^8r^3r^o^Mr^go^ww^Mw^1?") && tjbot.isAvailable)
		{
			tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, "say ^1M^8e^3e^oow^Mw^gw^ww\n");
			tjbot.isAvailable = 0;
		}

	}*/

	if (tjbot.cj_info)
	{
		if (strstr(strlwr((char *)((unsigned int)tjbot.cgame_handle + SERVERCHAT_TWC)), "tail") && strncmp(txt, (char *)((unsigned int)tjbot.cgame_handle + SERVERCHAT_TWC), 11)
			&& strncmp((char *)((unsigned int)tjbot.cgame_handle + SERVERCHAT_TWC), (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(*tjbot.clientNum * CLIENT_SIZE)), 12))
		{
			Beep(580, 300);
		}
		strncpy(txt, (char *)((unsigned int)tjbot.cgame_handle + SERVERCHAT_TWC), 11);
		int msec;

		if (cmd.upmove > 0)
		{
			if (downtime < 125)
				downtime++;
		}
		else
		{
			if (downtime < 125 && downtime)
				msec = downtime;
			else if (downtime == 125)
				msec = 125;
			downtime = 0;
		}

		if (downtime == 2 && prevvelsize[19] > 370 && prevvelsize[19] < 455)
		{
			if((int)tjbot.cj_info != 1) tjbot.pre = prevvelsize[19];
			else tjbot.pre = tjbot.vel_size;
			strcpy(HUDstring, va("%.3f", tjbot.pre));
		}
		else if (tjbot.vel_size < 440 && !downtime && isOnground)
		{
			if ((int)tjbot.cj_info != 1) strcpy(HUDstring, va("%.3f", *tjbot.plspeed));
			else strcpy(HUDstring, va("%.3f", tjbot.vel_size));
		}
		if (tjbot.vel_size - prev_vel_size[17] < 0)
		{
			colz[0] = colz[3] = 1.0f;
			colz[1] = colz[2] = 0.0f;
		}
		else if(tjbot.vel_size - prev_vel_size[17] > 0)
		{
			colz[1] = colz[3] = 1.0f;
			colz[0] = colz[2] = 0.0f;
		}
		else
		{
			colz[0] = colz[1] = colz[2] = colz[3] = 1.0f;
		}
		CG_DrawStringExt(290, 400, HUDstring, colz, qtrue, qfalse, 10, 15, 80);
	}

	if (tjbot.chatBot)
	{
		if (strlen(tjbot.voicetime) && strcmp(saved_time, tjbot.voicetime) && *tjbot.chatID != *tjbot.clientNum)
		{
			if (!strcmp(tjbot.voiceCmd, vc_cmds[51].cmd) && tjbot.chatDelay[0] == 0)
			{
				tjbot.chatDelay[2] = 5;
				strcpy(saved_time, tjbot.voicetime);
			}
			else if(!strcmp(tjbot.voiceCmd, vc_cmds[25].cmd) && tjbot.chatDelay[1] == 0)
			{
				tjbot.chatDelay[3] = 5;
				strcpy(saved_time, tjbot.voicetime);
			}
		}
		if (tjbot.chatDelay[0] > 0)
		{
			strcpy(saved_time, tjbot.voicetime);
			tjbot.chatDelay[0]--;
		}
		else if(tjbot.chatDelay[1] > 0)
		{
			strcpy(saved_time, tjbot.voicetime);
			tjbot.chatDelay[1]--;
		}
	}

	if (tjbot.chatDelay[3] > 0)
	{
		tjbot.chatDelay[3]--;
	}
	else if (tjbot.chatDelay[3] == 0)
	{
		if (strlen((char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(*tjbot.chatID * CLIENT_SIZE))))
		{
			tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("Vsay bye -Bye, ^w%s!;", (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(*tjbot.chatID * CLIENT_SIZE))));
			tjbot.chatDelay[1] = 3750;
		}
		tjbot.chatDelay[3]--;
	}
	if (tjbot.chatDelay[2] > 0)
	{
		tjbot.chatDelay[2]--;
	}
	else if (tjbot.chatDelay[2] == 0)
	{
		if (strlen((char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(*tjbot.chatID * CLIENT_SIZE))))
		{
			tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("Vsay hi -Hello, ^w%s!;", (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(*tjbot.chatID * CLIENT_SIZE))));
			tjbot.chatDelay[0] = 3750;
		}
		tjbot.chatDelay[2]--;
	}
	else strncpy(tjbot.voicetime, tjbot.voiceLine, 10);

	if (onlyOnce && tjbot.frame_count % 30 == 0)
	{
		for (int i = 0; i < 40; i++)
		{
			tjbot.clients[i] = *(int *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + CLIENT_TEAM + (unsigned int)(i * CLIENT_SIZE));
			//tjbot.orig_syscall(CG_PRINT, va("ID %d %s %s\n", i, tjbot.clients[i] ? "CONNECTED" : "NOT CONNECTED", (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(i * CLIENT_SIZE))));
		}
		//tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("VoiceChat %s;", "Hi"));
		tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, ";exec tj;");
		onlyOnce = 0;
	}
	/*
	if (tjbot.frame_count % 124 == 0)
	{
		for (int i = 0; i < 40; i++)
		{
			if (*(int *)((unsigned int)tjbot.cgame_handle + CLIENT_0_ID - 0x4 + (unsigned int)(i * CLIENT_SIZE)))
			{
				if (tjbot.clients[i] == 0)
				{
					tjbot.clients[i] = *(int *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + CLIENT_TEAM + (unsigned int)(i * CLIENT_SIZE));
					if (tjbot.clients[i])
					{
						char clName[64], welcome[64], end[128];
						strcpy(welcome, color_msg("Welcome"));
						strcpy(end, color_msg(", good to see you around!"));
						strcpy(clName, (char *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + (unsigned int)(i * CLIENT_SIZE)));
						tjbot.orig_syscall(CG_SENDCONSOLECOMMAND, va("m %d \"%s ^7%s %s!\";", i, welcome, clName, end));
						break;
					}
				}
				else
				{
					tjbot.clients[i] = *(int *)((unsigned int)tjbot.cgame_handle + CLIENT_0_NAME + CLIENT_TEAM + (unsigned int)(i * CLIENT_SIZE));
				}
			}
		}
	}*/

	if (tjbot.vel_size - abs(*tjbot.vel_x) < tjbot.vel_size - abs(*tjbot.vel_y))
	{
		//CG_DrawStringExt(450, 190, "switch", g_color_table[7], qfalse, qfalse, 8, 10, 10);
		if (*tjbot.vel_x == prev_vel_x && tjbot.crazy && tjbot.frame_count % 125 == 0) crazy_bot();
	}
	else if (tjbot.vel_size - abs(*tjbot.vel_y) < tjbot.vel_size - abs(*tjbot.vel_x))
	{
		//CG_DrawStringExt(450, 190, "switch", g_color_table[7], qfalse, qfalse, 8, 10, 10);
		if (*tjbot.vel_y == prev_vel_y && tjbot.crazy && tjbot.frame_count % 125 == 0) crazy_bot();
	}

	float diff_x = *tjbot.vel_x - prev_vel_x;
	float diff_y = *tjbot.vel_y - prev_vel_y;
	CG_DrawStringExt(290, 320, va("X : %.1f", diff_x), g_color_table[diff_x == 0 ? 1 : 2] , qfalse, qfalse, 8, 10, 10);
	CG_DrawStringExt(290, 330, va("Y : %.1f", diff_y), g_color_table[diff_y == 0 ? 1 : 2], qfalse, qfalse, 8, 10, 10);

	//CG_DrawStringExt(290, 390, va("diff : %.1f", *tjbot.view_x - per_angle), g_color_table[7], qfalse, qfalse, 8, 10, 10);

	prev_vel_x = *tjbot.vel_x;
	prev_vel_y = *tjbot.vel_y;

	if (!*tjbot.orig_x && !*tjbot.orig_y && !*tjbot.orig_z)
	{
		tjbot.offsetno = !tjbot.offsetno;
		get_offsets();
	}

	break;
    default:
        break;
    }
	return res;
}

// hooked functions

#define SIZE 6
BYTE oldBytesLLA[SIZE] = { 0 };
BYTE JMPLLA[SIZE] = { 0 };
HMODULE WINAPI tjbot_LoadLibraryA(LPCTSTR lpFileName) {
    memcpy(LoadLibraryA, oldBytesLLA, SIZE);

    HINSTANCE hInst = LoadLibraryA(lpFileName); 
	if (lpFileName && hInst && strstr(lpFileName, "cgame_mp_x86.dll")) {
		strcpy(tjbot.cgame_filename, lpFileName);
		tjbot.cgame_handle = hInst;
	}

    memcpy(LoadLibraryA, JMPLLA, SIZE);
    return hInst;
}

BYTE oldBytesGPA[SIZE] = { 0 };
BYTE JMPGPA[SIZE] = { 0 };
FARPROC WINAPI tjbot_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    memcpy(GetProcAddress, oldBytesGPA, SIZE);

    FARPROC fRet = GetProcAddress(hModule, lpProcName);
    if (hModule == tjbot.cgame_handle) {
        if (!strcmp(lpProcName, "dllEntry")) {
            tjbot.orig_cg_dllEntry = (void(*)(int(*)(int, ...))) fRet;
            fRet = (FARPROC)cg_dllEntry;
        }
        else if (!strcmp(lpProcName, "vmMain")) {
            tjbot.orig_cg_vmMain = (int(*)(int, int, int, int, int, int, int, int, int, int, int, int, int)) fRet;
            fRet = (FARPROC)cg_vmMain;
        }
    }

    memcpy(GetProcAddress, JMPGPA, SIZE);
    return fRet;
}

void hijack(LPVOID newFunction, LPVOID origFunction, BYTE oldBytes[SIZE], BYTE JMP[SIZE]) {
    BYTE tempJMP[SIZE] = { ASM_JMP, 0x90, 0x90, 0x90, 0x90, ASM_RETN };
    memcpy(JMP, tempJMP, SIZE);
    DWORD JMPSize = ((DWORD)newFunction - (DWORD)origFunction - 5);
    DWORD dwProtect;
    VirtualProtect((LPVOID)origFunction, SIZE, PAGE_EXECUTE_READWRITE, &dwProtect);
    memcpy(oldBytes, origFunction, SIZE);
    memcpy(&JMP[1], &JMPSize, 4);
    memcpy(origFunction, JMP, SIZE);
}

BOOL WINAPI DllMain(HMODULE hDll, DWORD dwReason, PVOID pvReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        hijack(tjbot_LoadLibraryA, LoadLibraryA, oldBytesLLA, JMPLLA);
        hijack(tjbot_GetProcAddress, GetProcAddress, oldBytesGPA, JMPGPA);
    }
    return 1;
}