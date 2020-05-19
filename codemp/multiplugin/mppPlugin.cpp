	
/**************************************************
* MultiPlugin++ by Deathspike
*
* Client-side plugin project which allows any
* developer to use this framework as a basis for
* their own plugin.
**************************************************/

#include <multiplugin/mppHeader.h>

intptr_t CL_CgameSystemCalls(intptr_t *args);
intptr_t CGMainCall(int command, intptr_t arg0, intptr_t arg1, intptr_t arg2, intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6, intptr_t arg7, intptr_t arg8, intptr_t arg9, intptr_t arg10, intptr_t arg11);

#define MPP_GOT_CURRENTORIGIN	(1<<0)
#define MPP_GOT_CURRENTVALID	(1<<1)
#define MPP_GOT_CLIENTINFO		(1<<2)
#define MPP_GOT_REFDEF			(1<<3)
#define MPP_GOT_SNAP			(1<<4)
#define MPP_GOT_CG				(1<<5)
#define MPP_GOT_CGS				(1<<6)
#define MPP_GOT_ENTITIES		(1<<7)
#define MPP_INITIALIZED			(MPP_GOT_CURRENTORIGIN | MPP_GOT_CURRENTVALID | MPP_GOT_CLIENTINFO | MPP_GOT_REFDEF | MPP_GOT_SNAP | MPP_GOT_CG | MPP_GOT_CGS | MPP_GOT_ENTITIES)
static int mppState = 0;

#ifdef MPP_USE_PLUGINS
/**************************************************
* mppPluginInit
*
* Initializes the plugin structure for any new plugin,
* this is done so we can seperate the given memory
* for each plugin. One plugin cannot screw up another,
* in this case.
**************************************************/

void mppPluginInit( MultiPlugin_t *pPlugin )
{
//	int	i								= 0;
	mppState							= 0;
	pPlugin->noBreakCode				= 1001;

	/**************************************************
	* Game Function Pointers
	*
	* These function pointers show custom functions able
	* to perform a useful task easily and swiftly. For example,
	* find a cvar or command in the engine.
	**************************************************/

	pPlugin->mppAimAtVector				= mppAimAtVector;
	pPlugin->mppIsPlayerAlly			= mppIsPlayerAlly;
	pPlugin->mppIsPlayerEntity			= mppIsPlayerEntity;
	pPlugin->mppIsPlayerInDuel			= mppIsPlayerInDuel;
	pPlugin->mppLocateCommand			= mppLocateCommand;
	pPlugin->mppLocateCvar				= mppLocateCvar;
	pPlugin->mppRawTextCalculateDraw	= mppRawTextCalculateDraw;
	pPlugin->mppRawTextCalculateHeight	= mppRawTextCalculateHeight;
	pPlugin->mppRawTextCalculateWidth	= mppRawTextCalculateWidth;
	pPlugin->mppRenderTextAtEntity		= mppRenderTextAtEntity;
	pPlugin->mppRenderTextAtVector		= mppRenderTextAtVector;
	pPlugin->mppRenderTextClear			= mppRenderTextClear;
	pPlugin->mppRenderTextFinalize		= mppRenderTextFinalize;
	pPlugin->mppRenderTextVisible		= mppRenderTextVisible;
	pPlugin->mppWorldToScreen			= mppWorldToScreen;

	/**************************************************
	* Game Pointers
	*
	* These are data segments filled in by MultiPlugin++ 
	* to avoid having to rely on unreliable data addresses,
	* which modifications are able to alter.
	**************************************************/

	pPlugin->currentOrigin				= currentOrigin;
	pPlugin->currentValid				= currentValid;
	pPlugin->clientInfo					= clientInfo;
	pPlugin->refdef						= &refdef;
	pPlugin->snap						= &snap;

	pPlugin->cg							= NULL;
	pPlugin->cgs						= NULL;
	for (int j = 0; j < MAX_GENTITIES; j++)
		pPlugin->cg_entities[j]			= NULL;

	/**************************************************
	* SDK Function Pointers
	*
	* Imports functions from the SDK, which are useful
	* and should be available here. Please note that
	* these are only core functions.
	**************************************************/

	pPlugin->AngleNormalize360			= AngleNormalize360;
	pPlugin->AngleNormalize180			= AngleNormalize180;
	pPlugin->AngleDelta					= AngleDelta;
	pPlugin->AnglesToAxis				= AnglesToAxis;
	pPlugin->AngleVectors				= AngleVectors;
	pPlugin->Cmd_Argc					= Cmd_Argc;
	pPlugin->Cmd_Argv					= Cmd_Argv;
	pPlugin->Cmd_ConcatArgs				= Cmd_ConcatArgs;
	pPlugin->Com_Error					= Com_Error;
	pPlugin->Com_Printf					= Com_Printf;
	pPlugin->Com_Sprintf				= Com_sprintf;
	pPlugin->Com_StringEscape			= Com_StringEscape;
	pPlugin->CopyString					= CopyString;
	pPlugin->FreeString					= Z_Free;
	pPlugin->Info_RemoveKey				= Info_RemoveKey;
	pPlugin->Info_SetValueForKey		= Info_SetValueForKey;
	pPlugin->Info_ValueForKey			= Info_ValueForKey;
	pPlugin->Q_CleanStr					= Q_CleanStr;
	pPlugin->Q_stricmpn					= Q_stricmpn;
	pPlugin->Q_stricmp					= Q_stricmp;
	pPlugin->Q_strncpyz					= Q_strncpyz;
	pPlugin->va							= va;
	pPlugin->vectoangles				= vectoangles;
	pPlugin->VectorNormalize			= VectorNormalize;
}
#endif

/**************************************************
* mppPluginCGMain
* 
* This is where all the calls from the engine into
* the module go, so we are free to manipulate it
* in any way we see fit. We, however, choose to forward
* it to our plugins.
**************************************************/

int mppPluginCGMain( int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 )
{
	int				 retCall		= 0;
	int				 retFlag		= 0;
	MultiPlugin.mainRetValue = MultiPlugin.noBreakCode;

	/**************************************************
	* Run internal functionality in pre-execution mode.
	**************************************************/

	switch( cmd )
	{
		case CG_INIT:
		{
#ifdef MPP_USE_PLUGINS
			LoadPlugins();
#endif
			mppScreenshotDetour();
			break;
		}

		case CG_CONSOLE_COMMAND:
		{
			char *cmd = Cmd_Argv( 0 );

			if ( Q_stricmp( cmd, "mpp_toggleVisual" ) == 0 )
			{
				if ( MultiPlugin.inScreenshot )
				{
					MultiPlugin.inScreenshot = 0;
				}
				else
				{
					MultiPlugin.inScreenshot = -1;
				}

				return qtrue;
			}

			break;
		}

		case CG_DRAW_ACTIVE_FRAME:
		{
			if ( MultiPlugin.inScreenshot && MultiPlugin.inScreenshot != -1 )
			{
				mppScreenshotCall();
			}

			break;
		}
	}

	/**************************************************
	* Run module functionality in pre-execution mode.
	**************************************************/

	if (mppState == MPP_INITIALIZED) {
		for (MultiModule_t *pMultiModule = MultiModule; pMultiModule; pMultiModule = (MultiModule_t*)pMultiModule->pNextPlugin) {
			if (pMultiModule->pPreMainCall) {
				retCall = pMultiModule->pPreMainCall(cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
				MultiPlugin.mainRetValue = (retCall != MultiPlugin.noBreakCode) ? retCall : MultiPlugin.mainRetValue;
			}
		}
	}

	/**************************************************
	* Run game module functionality which is basically
	* the core of the actual game. We're simply wrapping
	* around it.
	**************************************************/

	if (MultiPlugin.mainRetValue == MultiPlugin.noBreakCode )
	{
		retFlag = CGMainCall( cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11 );
	}

	/**************************************************
	* Run internal functionality in post-execution mode.
	**************************************************/

	switch( cmd )
	{
		case CG_DRAW_ACTIVE_FRAME:
		{
			mppRenderTextFinalize();
			break;
		}
	}

	/**************************************************
	* Run module functionality in post-execution mode.
	**************************************************/
	if (mppState == MPP_INITIALIZED) {
		for (MultiModule_t *pMultiModule = MultiModule; pMultiModule; pMultiModule = (MultiModule_t*)pMultiModule->pNextPlugin) {
			if (pMultiModule->pPostMainCall) {
				retCall = pMultiModule->pPostMainCall(cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
				MultiPlugin.mainRetValue = (retCall != MultiPlugin.noBreakCode) ? retCall : MultiPlugin.mainRetValue;
			}
		}
	}

	if (cmd == CG_SHUTDOWN) {
		mppState = 0;
#ifdef MPP_USE_PLUGINS
		UnloadPlugins();
#endif
	}

	return (MultiPlugin.mainRetValue != MultiPlugin.noBreakCode ) ? MultiPlugin.mainRetValue : retFlag;
}

/**************************************************
* mppPluginCGSystem
*
* This is where a call is being made from the module
* to the engine, and we can alter its response in
* any way we see fit. Even prevent it, whatever.
**************************************************/

int mppPluginCGSystem( int *arg )
{
	int				 retCall		= 0;
	int				 retFlag		= 0;
	MultiPlugin.sysRetValue = MultiPlugin.noBreakCode;

	/**************************************************
	* Run internal functionality in pre-execution mode.
	**************************************************/

	switch( arg[0] )
	{
		case CG_GETCURRENTSNAPSHOTNUMBER:
		{
			MultiPlugin.cg = ( cg_t * ) (( int ) arg[2] - 32 );
			mppState |= MPP_GOT_CG;
			break;
		}

		case CG_GETGAMESTATE:
		{
			MultiPlugin.cgs = ( cgs_t * ) arg[1];
			mppState |= MPP_GOT_CGS;
			break;
		}

		case CG_GETDEFAULTSTATE:
		{
			MultiPlugin.cg_entities[arg[1]] = ( centity_t * ) arg[2];
			mppState |= MPP_GOT_ENTITIES;
			break;
		}

		case CG_R_RENDERSCENE:
		{
			refdef_t *fd = ( refdef_t * ) arg[1];
				
			if ( fd->rdflags & RDF_DRAWSKYBOX )
			{
				memcpy( &refdef, fd, sizeof( refdef_t ));
			}

			mppState |= MPP_GOT_REFDEF;

			break;
		}

		case CG_S_UPDATEENTITYPOSITION:
		{
			if ( arg[1] >= 0 && arg[1] < MAX_GENTITIES )
			{
				MultiPlugin.gameEntity = ( int ) arg[1];
				VectorCopy( *( vec3_t * ) arg[2], currentOrigin[MultiPlugin.gameEntity] );
				mppState |= MPP_GOT_CURRENTORIGIN;
			}

			break;
		}
	}

	/**************************************************
	* Run module functionality in pre-execution mode.
	**************************************************/

	if (mppState == MPP_INITIALIZED) {
		for (MultiModule_t *pMultiModule = MultiModule; pMultiModule; pMultiModule = (MultiModule_t*)pMultiModule->pNextPlugin) {
			if (pMultiModule->pPreSystemCall) {
				retCall = pMultiModule->pPreSystemCall(arg);
				MultiPlugin.sysRetValue = (retCall != MultiPlugin.noBreakCode) ? retCall : MultiPlugin.sysRetValue;
			}
		}
	}

	/**************************************************
	* Run game module functionality which is basically
	* the core of the actual game. We're simply wrapping
	* around it.
	**************************************************/

	if (MultiPlugin.sysRetValue == MultiPlugin.noBreakCode )
	{
		MultiPlugin.sysRetValue = CL_CgameSystemCalls( arg );
	}

	/**************************************************
	* Run internal functionality in post-execution mode.
	**************************************************/

	switch( arg[0] )
	{
		case CG_GETGAMESTATE:
		{
			int i, iNum = atoi( Cmd_Argv( 1 ));
			gameState_t *gameState = ( gameState_t * ) arg[1];

			if ( iNum < CS_PLAYERS && iNum >= ( CS_PLAYERS + MAX_CLIENTS ))
			{
				break;
			}

			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				const char *configstring = ( const char * )(gameState->stringData + gameState->stringOffsets[CS_PLAYERS + i]);
				mpp_clientInfo_t *ci = &clientInfo[i];
				memset(ci, 0, sizeof(mpp_clientInfo_t));
				
				if ( !configstring[0] || Info_ValueForKey( configstring, "n" ) == NULL )
				{
					continue;
				}
				
				Q_strncpyz( ci->name, Info_ValueForKey( configstring, "n" ), sizeof( ci->name ));
				Q_strncpyz( ci->modelName, Info_ValueForKey( configstring, "model" ), sizeof( ci->modelName ));
				Q_strncpyz( ci->forcePowers, Info_ValueForKey( configstring, "forcepowers" ), sizeof( ci->forcePowers ));
				Q_strncpyz( ci->saberName, Info_ValueForKey( configstring, "st" ), sizeof( ci->saberName ));
				Q_strncpyz( ci->saber2Name, Info_ValueForKey( configstring, "st2" ), sizeof( ci->saber2Name ));

				ci->infoValid			= qtrue;
				ci->icolor1				= atoi( Info_ValueForKey( configstring, "c1" ));
				ci->icolor2				= atoi( Info_ValueForKey( configstring, "c2" ));
				ci->botSkill			= atoi( Info_ValueForKey( configstring, "skill" ));
				ci->handicap			= atoi( Info_ValueForKey( configstring, "hc" ));
				ci->wins				= atoi( Info_ValueForKey( configstring, "w" ));
				ci->losses				= atoi( Info_ValueForKey( configstring, "l" ));
				ci->teamTask			= atoi( Info_ValueForKey( configstring, "tt" ));
				ci->teamLeader			= ( qboolean ) atoi( Info_ValueForKey( configstring, "tl" ));
				ci->siegeDesiredTeam	= atoi( Info_ValueForKey( configstring, "sdt" ));
				ci->duelTeam			= atoi( Info_ValueForKey( configstring, "dt" ));

				if ( strlen( Info_ValueForKey( configstring, "MBClass" )))
				{
					if ( atoi( Info_ValueForKey( configstring, "t" )) == TEAM_RED || atoi( Info_ValueForKey( configstring, "t" )) == TEAM_BLUE )
					{
						ci->team = (team_t)atoi( Info_ValueForKey( configstring, "t" ));
					}

					ci->siegeDesiredTeam = atoi( Info_ValueForKey( configstring, "MBClass" ));
				}
				else
				{
					ci->team = (team_t)atoi( Info_ValueForKey( configstring, "t" ));
				}
			}

			mppState |= MPP_GOT_CLIENTINFO;
			break;
		}

		case CG_GETSNAPSHOT:
		{
			if ( snap.serverCommandSequence < arg[1] )
			{
				int i;
				memset( &currentValid, 0, sizeof( qboolean ) * MAX_CLIENTS );
				memcpy( &snap, ( snapshot_t * ) arg[2], sizeof (snapshot_t));

				for ( i = 0; i < MAX_GENTITIES; i++ )
				{
					currentValid[i] = qfalse;
				}

				for ( i = 0; i < snap.numEntities; i++ )
				{
					currentValid[snap.entities[i].number] = qtrue;
				}

				mppState |= MPP_GOT_CURRENTVALID | MPP_GOT_SNAP;
			}

			break;
		}
	}

	/**************************************************
	* Run module functionality in post-execution mode.
	**************************************************/

	if (mppState == MPP_INITIALIZED) {
		for (MultiModule_t *pMultiModule = MultiModule; pMultiModule; pMultiModule = (MultiModule_t*)pMultiModule->pNextPlugin) {
			if (pMultiModule->pPostSystemCall) {
				retCall = pMultiModule->pPostSystemCall(arg);
				MultiPlugin.sysRetValue = (retCall != MultiPlugin.noBreakCode) ? retCall : MultiPlugin.sysRetValue;
			}
		}
	}

	return (MultiPlugin.sysRetValue != MultiPlugin.noBreakCode ) ? MultiPlugin.sysRetValue : retFlag;
}

#ifdef MPP_USE_PLUGINS
/**************************************************
* mppPluginLoad
* 
* This loads all the modules into memory, writes
* down the path and name, retrieves a handle, allocates
* the possible functions and hides the module.
**************************************************/

void mppPluginLoad(char *zModule)
{
	/**************************************************
	* Store the previous loaded module and allocate the
	* space for a new entry. Clear this newly allocated
	* space for proper writing.
	**************************************************/

	MultiModule_t *newModule = (MultiModule_t *)malloc(sizeof(MultiModule_t));
	memset(newModule, 0, sizeof(MultiModule_t));
	newModule->hMod = SDL_LoadObject(zModule);

	/**************************************************
	* See if we can find an entry point to give the
	* MultiPlugin structure and system call pointers.
	**************************************************/
	if (newModule->hMod) {
		void(*entryCall)(MultiPlugin_t *) = (void(*)(MultiPlugin_t *)) SDL_LoadFunction(newModule->hMod, "mpp");
		if (entryCall != NULL)
		{
			// Link the system pointer to this system struct.
			//MultiPlugin.System = (void *)&MultiSystem;

			// Perform the entry call. When we're done, everything is free.
			entryCall(&MultiPlugin);

			/**************************************************
			* Determine the function pointers and finally
			* hide this module from the PEB. We're done now!
			**************************************************/

			newModule->pPreMainCall = (int(*)(int, int, int, int, int, int, int, int, int, int, int, int, int)) SDL_LoadFunction(newModule->hMod, "mppPreMain");
			newModule->pPostMainCall = (int(*)(int, int, int, int, int, int, int, int, int, int, int, int, int)) SDL_LoadFunction(newModule->hMod, "mppPostMain");

			newModule->pPreSystemCall = (int(*)(int *)) SDL_LoadFunction(newModule->hMod, "mppPreSystem");
			newModule->pPostSystemCall = (int(*)(int *)) SDL_LoadFunction(newModule->hMod, "mppPostSystem");

			// ADD IT
			if (MultiModule == NULL) MultiModule = newModule;
			else {
				MultiModule_t *pMultiModule;
				for (pMultiModule = MultiModule; pMultiModule->pNextPlugin; pMultiModule = (MultiModule_t*)pMultiModule->pNextPlugin);
				pMultiModule->pNextPlugin = newModule;
			}
		}
	}
	else {
		free(newModule);
	}
}
#endif
