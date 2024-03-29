/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 5;        /* border pixel of windows */
static const unsigned int gappx     = 3;        /* gap pixel between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char col_red[]         = "#cc241d";
static const char col_fg[]          = "#ebdbb2";

static const char *colors[][3]      = {
	/*                fg         bg         border   */
	[SchemeNorm]  = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]   = { col_gray4, col_cyan,  col_cyan  },
	[SchemeAlert] = { col_fg,    col_red,   col_red },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       force    tags mask     iscentered     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,       0,            0,             1,           -1 },
	{ NULL,       NULL,       "ranger",   0,       0,            1,             1,           -1 },
	{ "Firefox",  "firefox",  NULL,       1,       0,            1,             1,           -1 },
	{ "Firefox",  "Dialog",   NULL,       1,       0,            1,             1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "|T|",      tile },    /* first entry is default */
	{ ">F<",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static char dmenuhist[PATH_MAX + 1] = "\"${XDG_CACHE_HOME:-$HOME/.cache}/dwm/dmenuhist\""; /* to be expanded in setup() */
static char dmenulogout[PATH_MAX + 1] = "${XDG_CONFIG_HOME:-$HOME/.config}/dwm/dmenu_logout"; /* to be expanded in setup() */
static const char *dmenucmd[] = { "dmenu_run", "-H", dmenuhist, "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *logout[] = { dmenulogout, "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static char dwmlock[PATH_MAX + 1] = "\"${XDG_CONFIG_HOME:-$HOME/.config}/dwm/dwmlock\""; /* to be expanded in setup() */
static const char *lockscreen[]   = { dwmlock, NULL };
static const char scratchpadname[] = "scratch";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "120x34", NULL };
static const char infoname[] = "htop-float";
static const char *infocmd[] = { "st", "-t", infoname, "-g", "120x34", "htop", NULL };
static char scrotfile[PATH_MAX + 1] = "${HOME}'/tmp/%Y-%m-%d-%H%M%S_$wx$h.png'"; /* to be expanded in setup() */
static const char *scrotcmd[] = { "scrot", "-fs", scrotfile, "-e", "xclip -selection clipboard -t image/png -i $f", NULL };

/* for audio control */
static const char *mute[] = { "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL };
static const char *changeaudiosink[] = { "sh", "-c", "pactl set-default-sink $(pacmd list-sinks | awk -F ': *' '/index/ { if (first == \"\") { first = $2 }; if (current != \"\") { target = $2; exit } } /* index/ { current = $2 } END { if (target == \"\") { target = first } print target }')", NULL };
static const char *volumedown[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL };
static const char *volumeup[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL };

/* for screen brightness keyboard keys */
static const char *brightnessdown[] = { "xbacklight", "-dec", "10", NULL };
static const char *brightnessup[] = { "xbacklight", "-inc", "10", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {1} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {0} },

	{ ALTKEY,                       XK_F2,        spawn,            {.v = dmenucmd } },
	{ ALTKEY,                       XK_F3,        spawn,            {.v = termcmd } },
	{ MODKEY,                       XK_grave,     togglescratch,    {.v = scratchpadcmd } },
	{ 0,                            XK_Print,     spawn,            {.v = scrotcmd } },
	{ MODKEY|ShiftMask,             XK_s,         spawn,            {.v = scrotcmd } },

	{ ALTKEY,                       XK_F4,        killclient,       {0} },
	{ ALTKEY,                       XK_Tab,       focusprev,        {0} },
	{ MODKEY|ShiftMask,             XK_Left,      tagmon,           {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,     tagmon,           {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Down,      focusstack,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Up,        focusstack,       {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_h,         tagmon,           {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_l,         tagmon,           {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_j,         focusstack,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,         focusstack,       {.i = -1 } },
	{ MODKEY,                       XK_Left,      focushorizontal,  {.i = -1 } },
	{ MODKEY,                       XK_Right,     focushorizontal,  {.i = +1 } },
	{ MODKEY,                       XK_Down,      focusvertical,    {.i = +1 } },
	{ MODKEY,                       XK_Up,        focusvertical,    {.i = -1 } },
	{ MODKEY,                       XK_h,         focushorizontal,  {.i = -1 } },
	{ MODKEY,                       XK_l,         focushorizontal,  {.i = +1 } },
	{ MODKEY,                       XK_j,         focusvertical,    {.i = +1 } },
	{ MODKEY,                       XK_k,         focusvertical,    {.i = -1 } },
	{ MODKEY|ControlMask,           XK_Left,      focusmon,         {.i = -1 } },
	{ MODKEY|ControlMask,           XK_Right,     focusmon,         {.i = +1 } },
	{ MODKEY|ControlMask,           XK_h,         focusmon,         {.i = -1 } },
	{ MODKEY|ControlMask,           XK_l,         focusmon,         {.i = +1 } },
	{ ALTKEY|ControlMask,           XK_BackSpace, spawn,            {.v = logout } },
	{ ALTKEY|ControlMask,           XK_l,         spawn,            {.v = lockscreen } },
	{ ALTKEY|ShiftMask,             XK_Left,      setmfact,         {.f = -0.05} },
	{ ALTKEY|ShiftMask,             XK_Right,     setmfact,         {.f = +0.05} },
	{ ALTKEY|ShiftMask,             XK_h,         setmfact,         {.f = -0.05} },
	{ ALTKEY|ShiftMask,             XK_l,         setmfact,         {.f = +0.05} },
	{ ALTKEY|ShiftMask,             XK_0,         setmfact,         {.f = 1.0 + mfact} },
	{ MODKEY|ShiftMask,             XK_f,         togglefullscreen, {0} },
	{ MODKEY,                       XK_z,         raiseclient,      {0} },

	{ MODKEY,                       XK_Delete,     spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_apostrophe, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_BackSpace,  killclient,     {0} },

	{ 0,                            XF86XK_AudioMute,         spawn,  {.v = mute } },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn,  {.v = volumedown } },
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn,  {.v = volumeup } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn,  {.v = brightnessdown } },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn,  {.v = brightnessup } },
};

/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[0]} },
	{ ClkLtSymbol,          0,              Button2,        setlayout,      {.v = &layouts[1]} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = mute } },
	{ ClkStatusText,        MODKEY,         Button2,        spawn,          {.v = changeaudiosink } },
	{ ClkStatusText,        0,              Button4,        spawn,          {.v = volumeup } },
	{ ClkStatusText,        0,              Button5,        spawn,          {.v = volumedown } },
	{ ClkStatusText,        0,              Button3,        spawn,          {.v = infocmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

