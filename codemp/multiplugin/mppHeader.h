/**************************************************
* MultiPlugin++ by Deathspike
*
* Modified by jkd3v to integrate with OpenJK
*
* Client-side plugin project which allows any
* developer to use this framework as a basis for
* their own plugin.
**************************************************/

// Improve path detection algorithme, it's rather silly.

// Detection:
//	- VM_Create				(Not In Synch)
//	- VM_Call				(Not In Synch)
//	- VM_DllLoad			(Not In Synch)
//	- NO-CD					(Not In Synch)
//	- ScreenShot			(Always)
//
// Modules:
//	- Immunize Mind			(Always, Removing Flags)
//	- Team Colorization		(Always, Rendering Flags)
//	- Texture Replacement	(Always, R_FindShader, Tracing Textures, Remap Data)

#include <stdio.h>
#include <client/client.h>
#include <cgame/cg_local.h>
#include <multiplugin/mppShared.h>
#include <SDL_loadso.h>


typedef struct {

	void				*hMod;				// The module handle which gets hidden almost instantly.

	int					( *pPreMainCall )( int, int, int, int, int, int, int, int, int, int, int, int, int );
	int					( *pPostMainCall )( int, int, int, int, int, int, int, int, int, int, int, int, int );
	int					( *pPreSystemCall )( int * );
	int					( *pPostSystemCall )( int * );

	void				*pNextPlugin;		// The handle of the next plugin, if any.

} MultiModule_t;

typedef struct
{

	qboolean	 bCheckWall;	// Set to true when a visibility check must be done!
	qboolean	 bPersistant;	// Set to true when persistant rendering is required (never cleared).
	char		*lpText;		// Allocated buffer of copied text.
	int			 iIndex;		// If set to -1, use the raw vector instead.
	int			 iLines;		// Increments the number of lines added for a specific entity.
	vec3_t		 vOrigin;		// The raw origin which is to be used when iIndex is invalid.
	TextAlign	 align;

} MultiRender_t;

/**************************************************
* This is the Shared Buffer across all the modules,
* and contains about any function you'd might wish.
**************************************************/

extern MultiPlugin_t  MultiPlugin;
extern MultiSystem_t  MultiSystem;
extern MultiModule_t *MultiModule;
extern MultiRender_t  pRenders[1024];
extern int			  iRenders;

/**************************************************
* These are data segments filled in by MultiPlugin++
* to avoid having to rely on unreliable data addresses,
* which modifications are able to alter.
**************************************************/

extern mpp_clientInfo_t	 clientInfo[MAX_CLIENTS];
extern vec3_t			 currentOrigin[MAX_GENTITIES];;
extern qboolean			 currentValid[MAX_GENTITIES];
extern refdef_t			 refdef;
extern snapshot_t		 snap;

/**************************************************
* mppGame
*
* This is the game interface, where only new game functions
* are stored. This includes methods to retrieve any command or
* cvar from the engine.
**************************************************/

void			 mppAimAtVector(vec3_t targOrigin);
qboolean		 mppIsPlayerAlly(int iIndex);
centity_t		*mppIsPlayerEntity(int iIndex);
qboolean		 mppIsPlayerInDuel(int iIndex);
cmd_t			*mppLocateCommand(char *name);
cvar_t			*mppLocateCvar(char *name);
void			 mppPerformScreenshot(qboolean bDoScreenshot);
void			 mppRawTextCalculateDraw(char *lpString, float x, float y, float fScale, int iFont, int iLines, TextAlign align);
int				 mppRawTextCalculateHeight(int iFont, float fScale);
int				 mppRawTextCalculateWidth(char *lpString, int iFont, float fScale);
int				 mppRenderTextAtEntity(int iIndex, char *lpString, qboolean bCheckWall, qboolean bPersistant, TextAlign align);
int				 mppRenderTextAtVector(vec3_t origin, char *lpString, qboolean bCheckWall, qboolean bPersistant, TextAlign align);
void			 mppRenderTextClear(int iIndex);
void			 mppRenderTextFinalize(void);
qboolean		 mppRenderTextVisible(vec3_t origin, qboolean checkWalls);
void			 mppScreenshotCall(void);
void			 mppScreenshotDetour();
qboolean		 mppWorldToScreen(vec3_t point, float *x, float *y);

/**************************************************
* mppImports
*
* Imports functions from the SDK, which are useful
* and should be available here. Please note that
* these are only core functions.
**************************************************/

char			*Cmd_ConcatArgs(int start);
void			 Com_StringEscape(char *in, char *out);

/**************************************************
* mppMain
*
* This is the main application entry point, attaches
* itself to the host process and detours some beloved
* functions. Provides the way to intercept, patch for
* a working NO-CD, and holds the metamod-like functions.
**************************************************/

void			 mppInit();
void			 mppDestroy();
void			 LoadPlugins();
void			 UnloadPlugins();

/**************************************************
* mppPlugin
*
* Manages the calls in and from the plugin, as well
* as including of our own required initialization
* calls. It also loads other DLL's, manages them
* and so forth.
**************************************************/

void			 mppPluginInit(MultiPlugin_t *pPlugin);
void			 mppPluginLoad(char *zModule);
int				 mppPluginCGMain(int cmd, int arg0 = 0, int arg1 = 0, int arg2 = 0, int arg3 = 0, int arg4 = 0, int arg5 = 0, int arg6 = 0, int arg7 = 0, int arg8 = 0, int arg9 = 0, int arg10 = 0, int arg11 = 0);
int				 mppPluginCGSystem(int *arg);
int				 mppPluginUIMain(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
int				 mppPluginUISystem(int *arg);

/**************************************************
* mppSystem
*
* Imports functions from the SDK, which are useful
* and should be available here. Please note that
* these are only system call functions. It incorporates
* a simple wrapper to make the actual calls.
**************************************************/

void			 mppSystemInit(MultiSystem_t *trap);