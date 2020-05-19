/**************************************************
* MultiPlugin++ by Deathspike
*
* Client-side plugin project which allows any
* developer to use this framework as a basis for
* their own plugin.
**************************************************/

#include <multiplugin/mppHeader.h>
#include <qcommon/cm_public.h>

/**************************************************
* Don't export these.. direct screenshot access.
**************************************************/

static void ( *mppScreenshotPerform )( void );
static char *lpCommandString;

/**************************************************
* mppAimAtVector
*
* Aims your camera at the specified vector. This will
* only work when called in the 'post' context, after
* the current active frame.
**************************************************/

void mppAimAtVector( vec3_t targOrigin )
{
	vec3_t org, ang;

	VectorSubtract( targOrigin, refdef.vieworg, org );
	vectoangles( org, ang );
		
	cl.viewangles[PITCH] += AngleNormalize180( ang[PITCH] - refdef.viewangles[PITCH] );
	cl.viewangles[YAW] += AngleNormalize180( ang[YAW]   - refdef.viewangles[YAW]   );
}

/**************************************************
* mppIsPlayerAlly
*
* Determines whether or not the player client is an ally.
**************************************************/

qboolean mppIsPlayerAlly( int iIndex )
{
	if ( clientInfo[snap.ps.clientNum].team > TEAM_FREE && clientInfo[snap.ps.clientNum].team == clientInfo[iIndex].team )
	{
		return qtrue;
	}

	return qfalse;
}

/**************************************************
* mppIsPlayerEntity
*
* Determines wether or not the provided entity index
* is a player entity (including vehicles entities).
**************************************************/

centity_t *mppIsPlayerEntity( int iIndex )
{
	if ( iIndex >= 0 && iIndex < MAX_GENTITIES )
	{
		//entityState_t *es = &MultiPlugin.cg_entities[iIndex]->currentState;
		int i;

		if ( iIndex > MAX_CLIENTS )
		{
			for ( i = 0; i < MAX_CLIENTS; i++ )
			{
				if ( MultiPlugin.cg_entities[i]->currentState.m_iVehicleNum == iIndex )
				{
					return MultiPlugin.cg_entities[i];
				}
			}
		}
		else
		{
			return MultiPlugin.cg_entities[iIndex];
		}			
	}

	return NULL;
}

/**************************************************
* mppIsPlayerInDuel
*
* Determines wether or not the provided entity index
* is currently engaged in a private duel.
**************************************************/

qboolean mppIsPlayerInDuel( int iIndex )
{
	centity_t *cent = mppIsPlayerEntity( iIndex );

	if ( cent && cent->currentState.bolt1 )
	{
		return qtrue;
	}

	return qfalse;
}

/**************************************************
* mppLocateCommand
*
* Locates the provided command by string reference
* in the engine command table. The command
* structure allows any kind of manipulation to the
* command.
**************************************************/

extern cmd_t *Cmd_FindCommand(const char *cmd_name);
cmd_t *mppLocateCommand(char *name)
{
	return Cmd_FindCommand(name);
}

/**************************************************
* mppLocateCvar
*
* Locates the provided cvar by string reference in
* the engine controllable variable table. Be sure
* not to edit any value but the one used by the
* game.
**************************************************/

cvar_t *Cvar_FindVar(const char *var_name);
cvar_t *mppLocateCvar(char *name)
{
	return Cvar_FindVar(name);
}

/**************************************************
* mppRawTextCalculateDraw
*
* Calculates the position of the text and renders it.
**************************************************/

void mppRawTextCalculateDraw( char *lpString, float x, float y, float fScale, int iFont, int iLines, TextAlign align )
{
	if ( !MultiPlugin.inScreenshot )
	{
		vec4_t w = { 1, 1, 1, 1 };
		re->SetColor(w);
		int _x, _y;
		// Define x
		if (align == TopLeft || align == MiddleLeft || align == BottomLeft) {
			_x = x;
		}
		else if (align == TopCenter || align == MiddleCenter || align == BottomCenter) {
			_x = x - mppRawTextCalculateWidth(lpString, iFont, fScale) / 2;
		}
		else {
			_x = x - mppRawTextCalculateWidth(lpString, iFont, fScale);
		}

		// Define y
		if (align == TopLeft || align == TopCenter || align == TopRight) {
			_y = y;
		}
		else if (align == MiddleLeft || align == MiddleCenter || align == MiddleRight) {
			_y = y - (mppRawTextCalculateHeight(iFont, fScale) * iLines)/2;
		}
		else {
			_y = y - mppRawTextCalculateHeight(iFont, fScale) * iLines;
		}

		// Draw
		re->Font_DrawString(_x, _y, lpString, w, 0 | iFont, -1, fScale);
		re->SetColor(w);
	}
}

/**************************************************
* mppRawTextCalculateHeight
*
* Calculates the text height for a raw rendering call.
**************************************************/

int mppRawTextCalculateHeight( int iFont, float fScale ) 
{
	return re->Font_HeightPixels( iFont, fScale );
}

/**************************************************
* mppRawTextCalculateWidth
*
* Calculates the text width for a raw rendering call.
**************************************************/

int mppRawTextCalculateWidth( char *lpString, int iFont, float fScale ) 
{
	char *lpBuffer	= CopyString( lpString );
	int	  iLen		= re->Font_StrLenPixels( Q_CleanStr( lpBuffer ), iFont, fScale );
	Z_Free( lpBuffer );
	return iLen;
}

/**************************************************
* mppRenderTextAtEntity
*
* Renders the provided text at the entity position.
**************************************************/

int mppRenderTextAtEntity( int iIndex, char *lpString, qboolean bCheckWall, qboolean bPersistant, TextAlign align )
{
	int		i, iReturn = iRenders, iLines = 0;

	if ( iIndex < 0 || iIndex >= MAX_GENTITIES )
	{
		return -1;
	}

	for ( i = 0; i < iRenders; i++ )
	{
		if ( pRenders[i].iIndex == iIndex )
		{
			iLines++;
		}
	}
		
	while( pRenders[iRenders].bPersistant )
	{
		iRenders++;
	}

	pRenders[iRenders].lpText = CopyString( lpString );
	pRenders[iRenders].iIndex = iIndex;
	pRenders[iRenders].iLines = iLines;
	pRenders[iRenders].bCheckWall = bCheckWall;
	pRenders[iRenders].bPersistant = bPersistant;
	pRenders[iRenders].align = align;
	iRenders++;

	return iReturn;
}

/**************************************************
* mppRenderTextAtVector
*
* Renders the provided text at the provided origin.
**************************************************/

int mppRenderTextAtVector( vec3_t origin, char *lpString, qboolean bCheckWall, qboolean bPersistant, TextAlign align )
{
	int iReturn = iRenders;

	while( pRenders[iRenders].bPersistant )
	{
		iRenders++;
	}

	pRenders[iRenders].lpText = CopyString( lpString );
	pRenders[iRenders].iIndex = -1;
	pRenders[iRenders].bCheckWall = bCheckWall;
	pRenders[iRenders].bPersistant = bPersistant;
	pRenders[iRenders].align = align;
		
	VectorCopy( origin, pRenders[iRenders].vOrigin );
	iRenders++;

	return iReturn;
}

/**************************************************
* mppRenderTextClear
*
* Clears a persistant rendering buffer by removing the flag.
**************************************************/

void mppRenderTextClear( int iIndex )
{
	if ( iIndex >= 0 && iIndex < 1024 )
	{
		pRenders[iIndex].bPersistant = qfalse;
		pRenders[iIndex].align = MiddleCenter;
	}
}

/**************************************************
* mppRenderTextFinalize
*
* Finalizes the created buffers, rendering them to the scene.
**************************************************/

void mppRenderTextFinalize( void )
{
	float	fScale	= 0.4;
	int		iFont	= re->RegisterFont( "ergoec" );
	vec3_t	vLength, vOrigin;
	float	x, y;
	int		i;

	for ( i = 0; i < 1024; i++ )
	{
		if (( i < iRenders || pRenders[i].bPersistant ) )
		{
			if ( pRenders[i].iIndex == -1 )
			{
				VectorCopy( pRenders[i].vOrigin, vOrigin );
			}
			else
			{
				centity_t *cent = mppIsPlayerEntity( pRenders[i].iIndex );

				if ( cent == NULL )
				{
					continue;
				}

				VectorCopy( currentOrigin[cent->currentState.number], vOrigin );
			}

			if ( pRenders[i].lpText && ( !pRenders[i].bCheckWall || mppRenderTextVisible( vOrigin, qtrue )) && mppWorldToScreen( vOrigin, &x, &y ))
			{
				VectorSubtract( vOrigin, refdef.vieworg, vLength );
				fScale = 320 / VectorLength( vLength );
				fScale = ( fScale > 0.4 ) ? 0.4 : fScale;
				fScale = ( fScale < 0.3 ) ? 0.3 : fScale;
				mppRawTextCalculateDraw( pRenders[i].lpText, x, y, fScale, iFont, pRenders[i].iLines, pRenders[i].align );
			}
		}

		if ( !pRenders[i].bPersistant && pRenders[i].lpText )
		{
			Z_Free( pRenders[i].lpText );
			pRenders[i].lpText = NULL;
		}
	}

	iRenders = 0;
}

/**************************************************
* mppRenderTextVisible
*
* Determines wether or not the provided origin is visible for drawing.
**************************************************/

qboolean mppRenderTextVisible( vec3_t origin, qboolean checkWalls )
{
	if ( checkWalls )
	{
		trace_t results;
		CM_BoxTrace( &results, refdef.vieworg, origin, NULL, NULL, 0, CONTENTS_SOLID, qfalse );

		if ( results.fraction < 1.0f )
		{
			return qfalse;
		}
	}

	return mppWorldToScreen( origin, NULL, NULL );
}

/**************************************************
* mppScreenshotCall
*
* Detoured function, which sets a boolean when we're
* going to screenshot. This gives us a frame time to 
* disable visuals to avoid flagging on the screenshot.
**************************************************/
extern int			cmd_argc;
extern char		*cmd_argv[MAX_STRING_TOKENS];
void mppScreenshotCall( void )
{
	/**************************************************
	* We're ready to take the screenshot. We've waited
	* a few frames and may now take the shot without the
	* renderered visuals!
	**************************************************/

	if ( MultiPlugin.inScreenshot >= 5 )
	{
		char *lpCommandOriginal = Cmd_Argv( 1 );
		int iCommandOriginal = Cmd_Argc();

		if ( lpCommandString && lpCommandString[0] )
		{
			cmd_argc = 2;
			cmd_argv[1] = lpCommandString;
		}
		else
		{
			cmd_argc = 0;
		}

		mppScreenshotPerform();
		MultiPlugin.inScreenshot = 0;

		cmd_argc = iCommandOriginal;
		cmd_argv[1] = lpCommandOriginal;
		return;
	}

	/**************************************************
	* We're not currently in a screenshot or in a visual
	* disable, thus check if we're making an ordinary
	* screenshot or one named as mpp (screenshot the hack).
	**************************************************/

	else if ( !MultiPlugin.inScreenshot && Cmd_Argc() >= 2 )
	{
		if ( Q_stricmp( Cmd_Argv( 1 ), "mpp" ) == 0 )
		{
			cmd_argc = 0;
			mppScreenshotPerform();
		}
		else
		{
			lpCommandString = CopyString( Cmd_Argv( 1 ));
		}
	}
		
	/**************************************************
	* Increment the screenshot counter. This will ensure
	* the screenshot is going to be taken. We will
	* increment with a few since it can be -1 to avoid visuals.
	**************************************************/

	MultiPlugin.inScreenshot += 2;
}

/**************************************************
* mppScreenshotDetour
*
* Detours the screenshot command in the command table.
**************************************************/

void mppScreenshotDetour()
{
	cmd_t *cmd = mppLocateCommand( "screenshot" );
	if ( cmd->function != mppScreenshotCall ) mppScreenshotPerform = cmd->function;
	cmd->function = &mppScreenshotCall;
}

/**************************************************
* mppWorldToScreen
*
* Converts the world model to screen view.
**************************************************/

qboolean mppWorldToScreen( vec3_t point, float *x, float *y )
{
	vec3_t  trans;
	float   xc, yc;
	float   px, py;
	float   z;

	px = tan( refdef.fov_x * M_PI / 360.0 );
	py = tan( refdef.fov_y * M_PI / 360.0 );

	VectorSubtract( point, refdef.vieworg, trans );

	xc = 640.0f / 2.0f;
	yc = 480.0f / 2.0f;

	z = DotProduct( trans, refdef.viewaxis[0] );

	if( z <= 0.001f )
	{
		return qfalse;
	}

	if( x )
	{
		*x = xc - DotProduct( trans, refdef.viewaxis[1] ) * xc / ( z * px );
	}

	if( y )
	{
		*y = yc - DotProduct( trans, refdef.viewaxis[2] ) * yc / ( z * py );
	}

	return qtrue;
}
