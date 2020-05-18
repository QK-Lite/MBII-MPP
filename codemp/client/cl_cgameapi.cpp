/*
===========================================================================
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// cl_cgameapi.cpp  -- client system interaction with client game
#include "qcommon/cm_public.h"
#include "qcommon/RoffSystem.h"
#include "qcommon/stringed_ingame.h"
#include "qcommon/timing.h"
#include "client.h"
#include "cl_uiapi.h"
#include "botlib/botlib.h"
#include "snd_ambient.h"
#include "FXExport.h"
#include "FxUtil.h"

#define USE_MPP
#ifdef USE_MPP
#include "multiplugin/mppHeader.h"
#define F_CG_MAIN_CALL  mppPluginCGMain
#define F_CG_SYS_CALL   mppPluginCGSystem
#else
#define F_CG_MAIN_CALL  CGMainCall
#define F_CG_SYS_CALL   CL_CgameSystemCalls
#endif

intptr_t _rvmf(float x) {
	byteAlias_t fi;
	fi.f = x;
	return fi.i;
}

#define RVMF(x) (_rvmf(x))

extern IHeapAllocator *G2VertSpaceClient;
extern botlib_export_t *botlib_export;

// cgame interface
static cgameExport_t *cge; // cgame export table
static vm_t *cgvm; // cgame vm, valid for legacy and new api

//
// cgame vmMain calls
//
intptr_t CGMainCall(int command, intptr_t arg0 = 0, intptr_t arg1 = 0, intptr_t arg2 = 0, intptr_t arg3 = 0, intptr_t arg4 = 0,
	intptr_t arg5 = 0, intptr_t arg6 = 0, intptr_t arg7 = 0, intptr_t arg8 = 0, intptr_t arg9 = 0, intptr_t arg10 = 0, intptr_t arg11 = 0) {
	switch (command)
	{
	case CG_INIT:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_INIT, arg0, arg1, arg2);
			return 0;
		}
		VMSwap v(cgvm);

		cge->Init(arg0, arg1, arg2);
		return 0;
	}
	case CG_SHUTDOWN:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_SHUTDOWN);
			return 0;
		}
		VMSwap v(cgvm);

		cge->Shutdown();
		return 0;
	}
	case CG_CONSOLE_COMMAND:
	{
		if (cgvm->isLegacy) {
			return (qboolean)VM_Call(cgvm, CG_CONSOLE_COMMAND);
		}
		VMSwap v(cgvm);

		return cge->ConsoleCommand();
	}
	case CG_DRAW_ACTIVE_FRAME:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_DRAW_ACTIVE_FRAME, arg0, arg1, arg2);
			return 0;
		}
		VMSwap v(cgvm);

		cge->DrawActiveFrame(arg0, arg1, (qboolean)arg2);
		return 0;
	}
	case CG_CROSSHAIR_PLAYER:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_CROSSHAIR_PLAYER);
		}
		VMSwap v(cgvm);

		return cge->CrosshairPlayer();
	}
	case CG_LAST_ATTACKER:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_LAST_ATTACKER);
		}
		VMSwap v(cgvm);

		return cge->LastAttacker();
	}
	case CG_KEY_EVENT:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_KEY_EVENT, arg0, arg1);
			return 0;
		}
		VMSwap v(cgvm);

		cge->KeyEvent(arg0, (qboolean)arg1);
		return 0;
	}
	case CG_MOUSE_EVENT:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_MOUSE_EVENT, arg0, arg1);
			return 0;
		}
		VMSwap v(cgvm);

		cge->MouseEvent(arg0, arg1);
		return 0;
	}
	case CG_EVENT_HANDLING:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_EVENT_HANDLING, arg0);
			return 0;
		}
		VMSwap v(cgvm);

		cge->EventHandling(arg0);
		return 0;
	}
	case CG_POINT_CONTENTS:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_POINT_CONTENTS);
		}
		VMSwap v(cgvm);

		return cge->PointContents();
	}
	case CG_GET_LERP_ORIGIN:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_GET_LERP_ORIGIN);
			return 0;
		}
		VMSwap v(cgvm);

		cge->GetLerpOrigin();
		return 0;
	}
	case CG_GET_LERP_DATA:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_GET_LERP_DATA);
			return 0;
		}
		VMSwap v(cgvm);

		cge->GetLerpData();
		return 0;
	}
	case CG_TRACE:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_TRACE);
			return 0;
		}
		VMSwap v(cgvm);

		cge->Trace();
		return 0;
	}
	case CG_G2TRACE:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_G2TRACE);
			return 0;
		}
		VMSwap v(cgvm);

		cge->G2Trace();
		return 0;
	}
	case CG_G2MARK:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_G2MARK);
			return 0;
		}
		VMSwap v(cgvm);

		cge->G2Mark();
		return 0;
	}
	case CG_RAG_CALLBACK:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_RAG_CALLBACK, arg0);
		}
		VMSwap v(cgvm);

		return cge->RagCallback(arg0);
	}
	case CG_INCOMING_CONSOLE_COMMAND:
	{
		if (cgvm->isLegacy) {
			return (qboolean)VM_Call(cgvm, CG_INCOMING_CONSOLE_COMMAND);
		}
		VMSwap v(cgvm);

		return cge->IncomingConsoleCommand();
	}
	case CG_GET_USEABLE_FORCE:
	{
		if (cgvm->isLegacy) {
			return (qboolean)VM_Call(cgvm, CG_GET_USEABLE_FORCE);
		}
		VMSwap v(cgvm);

		return cge->NoUseableForce();
	}
	case CG_GET_ORIGIN:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_GET_ORIGIN, arg0, reinterpret_cast< intptr_t >((float*)arg1));
			return 0;
		}
		VMSwap v(cgvm);

		cge->GetOrigin(arg0, (float *)arg1);
		return 0;
	}
	case CG_GET_ANGLES:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_GET_ANGLES, arg0, reinterpret_cast< intptr_t >((float*)arg1));
			return 0;
		}
		VMSwap v(cgvm);

		cge->GetAngles(arg0, (float *)arg1);
		return 0;
	}
	case CG_GET_ORIGIN_TRAJECTORY:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_GET_ORIGIN_TRAJECTORY, arg0);
		}
		VMSwap v(cgvm);

		return (intptr_t)(cge->GetOriginTrajectory(arg0));
	}
	case CG_GET_ANGLE_TRAJECTORY:
	{
		if (cgvm->isLegacy) {
			return VM_Call(cgvm, CG_GET_ANGLE_TRAJECTORY, arg0);
		}
		VMSwap v(cgvm);

		return (intptr_t)(cge->GetAngleTrajectory(arg0));
	}
	case CG_ROFF_NOTETRACK_CALLBACK:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_ROFF_NOTETRACK_CALLBACK, arg0, reinterpret_cast< intptr_t >((const char *)arg1));
			return 0;
		}
		VMSwap v(cgvm);

		cge->ROFF_NotetrackCallback(arg0, (const char *)arg1);
		return 0;
	}
	case CG_MAP_CHANGE:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_MAP_CHANGE);
			return 0;
		}
		VMSwap v(cgvm);

		cge->MapChange();
		return 0;
	}
	case CG_AUTOMAP_INPUT:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_AUTOMAP_INPUT);
			return 0;
		}
		VMSwap v(cgvm);

		cge->AutomapInput();
		return 0;
	}
	case CG_MISC_ENT:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_MISC_ENT);
			return 0;
		}
		VMSwap v(cgvm);

		cge->MiscEnt();
		return 0;
	}
	case CG_FX_CAMERASHAKE:
	{
		if (cgvm->isLegacy) {
			VM_Call(cgvm, CG_FX_CAMERASHAKE);
			return 0;
		}
		VMSwap v(cgvm);

		cge->CameraShake();
		return 0;
	}
	default:
		break;
	}

	return 0;
}

void CGVM_Init(int serverMessageNum, int serverCommandSequence, int clientNum) {
	F_CG_MAIN_CALL(CG_INIT, serverMessageNum, serverCommandSequence, clientNum);
}

void CGVM_Shutdown(void) {
	F_CG_MAIN_CALL(CG_SHUTDOWN);
}

qboolean CGVM_ConsoleCommand(void) {
	return (qboolean)F_CG_MAIN_CALL(CG_CONSOLE_COMMAND);
}

void CGVM_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback) {
	F_CG_MAIN_CALL(CG_DRAW_ACTIVE_FRAME, serverTime, stereoView, demoPlayback);
}

int CGVM_CrosshairPlayer(void) {
	return F_CG_MAIN_CALL(CG_CROSSHAIR_PLAYER);
}

int CGVM_LastAttacker(void) {
	return F_CG_MAIN_CALL(CG_LAST_ATTACKER);
}

void CGVM_KeyEvent(int key, qboolean down) {
	F_CG_MAIN_CALL(CG_KEY_EVENT, key, down);
}

void CGVM_MouseEvent(int x, int y) {
	F_CG_MAIN_CALL(CG_MOUSE_EVENT, x, y);
}

void CGVM_EventHandling(int type) {
	F_CG_MAIN_CALL(CG_EVENT_HANDLING, type);
}

int CGVM_PointContents(void) {
	return F_CG_MAIN_CALL(CG_POINT_CONTENTS);
}

void CGVM_GetLerpOrigin(void) {
	F_CG_MAIN_CALL(CG_GET_LERP_ORIGIN);
}

void CGVM_GetLerpData(void) {
	F_CG_MAIN_CALL(CG_GET_LERP_DATA);
}

void CGVM_Trace(void) {
	F_CG_MAIN_CALL(CG_TRACE);
}

void CGVM_G2Trace(void) {
	F_CG_MAIN_CALL(CG_G2TRACE);
}

void CGVM_G2Mark(void) {
	F_CG_MAIN_CALL(CG_G2MARK);
}

int CGVM_RagCallback(int callType) {
	return F_CG_MAIN_CALL(CG_RAG_CALLBACK, callType);
}

qboolean CGVM_IncomingConsoleCommand(void) {
	return (qboolean)F_CG_MAIN_CALL(CG_INCOMING_CONSOLE_COMMAND);
}

qboolean CGVM_NoUseableForce(void) {
	return (qboolean)F_CG_MAIN_CALL(CG_GET_USEABLE_FORCE);
}

void CGVM_GetOrigin(int entID, vec3_t out) {
	F_CG_MAIN_CALL(CG_GET_ORIGIN, entID, (intptr_t)(out));
}

void CGVM_GetAngles(int entID, vec3_t out) {
	F_CG_MAIN_CALL(CG_GET_ANGLES, entID, (intptr_t)(out));
}

trajectory_t *CGVM_GetOriginTrajectory(int entID) {
	return (trajectory_t*)F_CG_MAIN_CALL(CG_GET_ORIGIN_TRAJECTORY, entID);
}

trajectory_t *CGVM_GetAngleTrajectory(int entID) {
	return (trajectory_t*)F_CG_MAIN_CALL(CG_GET_ANGLE_TRAJECTORY, entID);
}

void CGVM_ROFF_NotetrackCallback(int entID, const char *notetrack) {
	F_CG_MAIN_CALL(CG_ROFF_NOTETRACK_CALLBACK, entID, (intptr_t)(notetrack));
}

void CGVM_MapChange(void) {
	F_CG_MAIN_CALL(CG_MAP_CHANGE);
}

void CGVM_AutomapInput(void) {
	F_CG_MAIN_CALL(CG_AUTOMAP_INPUT);
}

void CGVM_MiscEnt(void) {
	F_CG_MAIN_CALL(CG_MISC_ENT);
}

void CGVM_CameraShake(void) {
	F_CG_MAIN_CALL(CG_FX_CAMERASHAKE);
}


//
// cgame syscalls
//	only used by legacy mods!
//

extern int CL_GetValueForHidden( const char *s ); //cl_parse.cpp
extern qboolean cl_bUseFighterPitch; //cl_input.cpp
int CM_LoadSubBSP( const char *name, qboolean clientload ); //cm_load.cpp
void FX_FeedTrail( effectTrailArgStruct_t *a ); //FxPrimitives.cpp

// wrappers and such

static void CL_AddCgameCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

static void CL_CM_LoadMap( const char *mapname, qboolean subBSP ) {
	if ( subBSP )	CM_LoadSubBSP( va( "maps/%s.bsp", mapname+1 ), qfalse );
	else			CM_LoadMap( mapname, qtrue, NULL );
}

static void CL_GetGlconfig( glconfig_t *glconfig ) {
	*glconfig = cls.glconfig;
}

static void CL_GetGameState( gameState_t *gs ) {
	*gs = cl.gameState;
}

static void RegisterSharedMemory( char *memory ) {
	cl.mSharedMemory = memory;
}

static int CL_Milliseconds( void ) {
	return Sys_Milliseconds();
}

static void CL_AddReliableCommand2( const char *cmd ) {
	CL_AddReliableCommand( cmd, qfalse );
}

static int CL_CM_RegisterTerrain( const char *config ) {
	return 0;
}

extern int s_entityWavVol[MAX_GENTITIES];
static int CL_S_GetVoiceVolume( int entID ) {
	return s_entityWavVol[entID];
}

static void CL_S_Shutup( qboolean shutup ) {
	s_shutUp = shutup;
}

static int CL_GetCurrentCmdNumber( void ) {
	return cl.cmdNumber;
}

static void _CL_SetUserCmdValue( int stateValue, float sensitivityScale, float mPitchOverride, float mYawOverride, float mSensitivityOverride, int fpSel, int invenSel, qboolean fighterControls ) {
	cl_bUseFighterPitch = fighterControls;
	CL_SetUserCmdValue( stateValue, sensitivityScale, mPitchOverride, mYawOverride, mSensitivityOverride, fpSel, invenSel );
}

static void CL_OpenUIMenu( int menuID ) {
	UIVM_SetActiveMenu( (uiMenuCommand_t)menuID );
}

static void CGFX_AddLine( vec3_t start, vec3_t end, float size1, float size2, float sizeParm, float alpha1, float alpha2, float alphaParm, vec3_t sRGB, vec3_t eRGB, float rgbParm, int killTime, qhandle_t shader, int flags ) {
	FX_AddLine( start, end, size1, size2, sizeParm, alpha1, alpha2, alphaParm, sRGB, eRGB, rgbParm, killTime, shader, flags );
}

static void CGFX_AddPoly( addpolyArgStruct_t *p ) {
	FX_AddPoly( p->p, p->ev, p->numVerts, p->vel, p->accel, p->alpha1, p->alpha2, p->alphaParm, p->rgb1, p->rgb2, p->rgbParm, p->rotationDelta, p->bounce, p->motionDelay, p->killTime, p->shader, p->flags );
}

static void CGFX_AddBezier( addbezierArgStruct_t *b ) {
	FX_AddBezier( b->start, b->end, b->control1, b->control1Vel, b->control2, b->control2Vel, b->size1, b->size2, b->sizeParm, b->alpha1, b->alpha2, b->alphaParm, b->sRGB, b->eRGB, b->rgbParm, b->killTime, b->shader, b->flags );
}

static void CGFX_AddPrimitive( effectTrailArgStruct_t *e ) {
	FX_FeedTrail( e );
}

static void CGFX_AddSprite( addspriteArgStruct_t *s ) {
	vec3_t rgb = { 1.0f, 1.0f, 1.0f };
	FX_AddParticle( s->origin, s->vel, s->accel, s->scale, s->dscale, 0, s->sAlpha, s->eAlpha, 0, rgb, rgb, 0, s->rotation, 0, vec3_origin, vec3_origin, s->bounce, 0, 0, s->life, s->shader, s->flags );
}

static void CGFX_AddElectricity( addElectricityArgStruct_t *p ) {
	FX_AddElectricity( p->start, p->end, p->size1, p->size2, p->sizeParm, p->alpha1, p->alpha2, p->alphaParm, p->sRGB, p->eRGB, p->rgbParm, p->chaos, p->killTime, p->shader, p->flags );
}

static qboolean CL_ROFF_Clean( void ) {
	return theROFFSystem.Clean( qtrue );
}

static void CL_ROFF_UpdateEntities( void ) {
	theROFFSystem.UpdateEntities( qtrue );
}

static int CL_ROFF_Cache( char *file ) {
	return theROFFSystem.Cache( file, qtrue );
}

static qboolean CL_ROFF_Play( int entID, int roffID, qboolean doTranslation ) {
	return theROFFSystem.Play( entID, roffID, doTranslation, qtrue );
}

static qboolean CL_ROFF_Purge_Ent( int entID ) {
	return theROFFSystem.PurgeEnt( entID, qtrue );
}

static void CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

static void CL_SetClientForceAngle( int time, vec3_t angle ) {
	cl.cgameViewAngleForceTime = time;
	VectorCopy(angle, cl.cgameViewAngleForce);
}

static void CL_PrecisionTimerStart( void **p ) {
	timing_c *newTimer = new timing_c; //create the new timer
	*p = newTimer; //assign the pointer within the pointer to point at the mem addr of our new timer
	newTimer->Start(); //start the timer
}

static int CL_PrecisionTimerEnd( void *p ) {
	int r = 0;
	timing_c *timer = (timing_c *)p; //this is the pointer we assigned in start, so we can directly cast it back
	r = timer->End(); //get the result
	delete timer; //delete the timer since we're done with it
	return r; //return the result
}

static void CL_RMG_Init( int /* terrainID */, const char * /* terrainInfo */ ) { }

static qboolean CGFX_PlayBoltedEffectID( int id, vec3_t org, void *ghoul2, const int boltNum, const int entNum, const int modelNum, int iLooptime, qboolean isRelative ) {
	if ( !ghoul2 ) return qfalse;

	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	int boltInfo=0;
	if ( re->G2API_AttachEnt( &boltInfo, g2, modelNum, boltNum, entNum, modelNum ) )
	{
		FX_PlayBoltedEffectID(id, org, boltInfo, &g2, iLooptime, isRelative );
		return qtrue;
	}
	return qfalse;
}

static qboolean CL_SE_GetStringTextString( const char *text, char *buffer, int bufferLength ) {
	const char *str;

	assert( text && buffer );

	str = SE_GetString( text );

	if ( str[0] ) {
		Q_strncpyz( buffer, str, bufferLength );
		return qtrue;
	}

	Com_sprintf( buffer, bufferLength, "??%s", str );
	return qfalse;
}

static void CL_G2API_ListModelSurfaces( void *ghlInfo ) {
	re->G2API_ListSurfaces( (CGhoul2Info *)ghlInfo );
}

static void CL_G2API_ListModelBones( void *ghlInfo, int frame ) {
	re->G2API_ListBones( (CGhoul2Info *)ghlInfo, frame );
}

static void CL_G2API_SetGhoul2ModelIndexes( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList ) {
	if ( !ghoul2 ) return;
	re->G2API_SetGhoul2ModelIndexes( *((CGhoul2Info_v *)ghoul2), modelList, skinList );
}

static qboolean CL_G2API_HaveWeGhoul2Models( void *ghoul2) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_HaveWeGhoul2Models( *((CGhoul2Info_v *)ghoul2) );
}

static qboolean CL_G2API_GetBoltMatrix( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean CL_G2API_GetBoltMatrix_NoReconstruct( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixReconstruction( qfalse );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean CL_G2API_GetBoltMatrix_NoRecNoRot( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixSPMethod( qtrue );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static int CL_G2API_InitGhoul2Model( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	return re->G2API_InitGhoul2Model( (CGhoul2Info_v **)ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias );
}

static qboolean CL_G2API_SetSkin( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_SetSkin( g2, modelIndex, customSkin, renderSkin );
}

static void CL_G2API_CollisionDetect( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetect( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, rayStart, rayEnd, scale, G2VertSpaceClient, traceFlags, useLod, fRadius );
}

static void CL_G2API_CollisionDetectCache( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetectCache( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, rayStart, rayEnd, scale, G2VertSpaceClient, traceFlags, useLod, fRadius );
}

static void CL_G2API_CleanGhoul2Models( void **ghoul2Ptr ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	re->G2API_CleanGhoul2Models( (CGhoul2Info_v **)ghoul2Ptr );
}

static qboolean CL_G2API_SetBoneAngles( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags, const int up, const int right, const int forward, qhandle_t *modelList, int blendTime , int currentTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAngles( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, angles, flags, (const Eorientations)up, (const Eorientations)right, (const Eorientations)forward, modelList, blendTime , currentTime );
}

static qboolean CL_G2API_SetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame, const int flags, const float animSpeed, const int currentTime, const float setFrame, const int blendTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAnim( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, startFrame, endFrame, flags, animSpeed, currentTime, setFrame, blendTime );
}

static qboolean CL_G2API_GetBoneAnim( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetBoneAnim( g2, modelIndex, boneName, currentTime, currentFrame, startFrame, endFrame, flags, animSpeed, modelList );
}

static qboolean CL_G2API_GetBoneFrame( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *modelList, const int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	int iDontCare1 = 0, iDontCare2 = 0, iDontCare3 = 0;
	float fDontCare1 = 0;

	return re->G2API_GetBoneAnim(g2, modelIndex, boneName, currentTime, currentFrame, &iDontCare1, &iDontCare2, &iDontCare3, &fDontCare1, modelList);
}

static void CL_G2API_GetGLAName( void *ghoul2, int modelIndex, char *fillBuf ) {
	if ( !ghoul2 )
	{
		fillBuf[0] = '\0';
		return;
	}

	char *tmp = re->G2API_GetGLAName( *((CGhoul2Info_v *)ghoul2), modelIndex );
	if ( tmp )
		strcpy( fillBuf, tmp );
	else
		fillBuf[0] = '\0';
}

static int CL_G2API_CopyGhoul2Instance( void *g2From, void *g2To, int modelIndex ) {
	if ( !g2From || !g2To ) return 0;

	return re->G2API_CopyGhoul2Instance( *((CGhoul2Info_v *)g2From), *((CGhoul2Info_v *)g2To), modelIndex );
}

static void CL_G2API_CopySpecificGhoul2Model( void *g2From, int modelFrom, void *g2To, int modelTo ) {
	if ( !g2From || !g2To) return;
	re->G2API_CopySpecificG2Model( *((CGhoul2Info_v *)g2From), modelFrom, *((CGhoul2Info_v *)g2To), modelTo );
}

static void CL_G2API_DuplicateGhoul2Instance( void *g2From, void **g2To ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	if ( !g2From || !g2To ) return;
	re->G2API_DuplicateGhoul2Instance( *((CGhoul2Info_v *)g2From), (CGhoul2Info_v **)g2To );
}

static qboolean CL_G2API_HasGhoul2ModelOnIndex( void *ghlInfo, int modelIndex ) {
	return re->G2API_HasGhoul2ModelOnIndex( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean CL_G2API_RemoveGhoul2Model( void *ghlInfo, int modelIndex ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	return re->G2API_RemoveGhoul2Model( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean CL_G2API_SkinlessModel( void *ghlInfo, int modelIndex ) {
	if ( !ghlInfo ) return qfalse;

	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfo);
	return re->G2API_SkinlessModel( g2, modelIndex );
}

static int CL_G2API_GetNumGoreMarks( void *ghlInfo, int modelIndex ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return 0;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfo);
	return re->G2API_GetNumGoreMarks( g2, modelIndex );
#else
	return 0;
#endif
}

static void CL_G2API_AddSkinGore( void *ghlInfo, SSkinGoreData *gore ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return;
	re->G2API_AddSkinGore( *((CGhoul2Info_v *)ghlInfo), *(SSkinGoreData *)gore );
#endif
}

static void CL_G2API_ClearSkinGore( void *ghlInfo ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return;
	re->G2API_ClearSkinGore( *((CGhoul2Info_v *)ghlInfo) );
#endif
}

static int CL_G2API_Ghoul2Size( void *ghlInfo ) {
	if ( !ghlInfo ) return 0;
	return re->G2API_Ghoul2Size( *((CGhoul2Info_v *)ghlInfo) );
}

static int CL_G2API_AddBolt( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return 0;
	return re->G2API_AddBolt( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName );
}

static qboolean CL_G2API_AttachEnt( int *boltInfo, void *ghlInfoTo, int toBoltIndex, int entNum, int toModelNum ) {
	if ( !ghlInfoTo ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfoTo);
	return re->G2API_AttachEnt( boltInfo, g2, 0, toBoltIndex, entNum, toModelNum );
}

static void CL_G2API_SetBoltInfo( void *ghoul2, int modelIndex, int boltInfo ) {
	if ( !ghoul2 ) return;
	re->G2API_SetBoltInfo( *((CGhoul2Info_v *)ghoul2), modelIndex, boltInfo );
}

static qboolean CL_G2API_SetRootSurface( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetRootSurface( *((CGhoul2Info_v *)ghoul2), modelIndex, surfaceName );
}

static qboolean CL_G2API_SetSurfaceOnOff( void *ghoul2, const char *surfaceName, const int flags ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetSurfaceOnOff( *((CGhoul2Info_v *)ghoul2), surfaceName, flags );
}

static qboolean CL_G2API_SetNewOrigin( void *ghoul2, const int boltIndex ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetNewOrigin( *((CGhoul2Info_v *)ghoul2), boltIndex );
}

static qboolean CL_G2API_DoesBoneExist( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_DoesBoneExist( g2, modelIndex, boneName );
}

static int CL_G2API_GetSurfaceRenderStatus( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return -1;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetSurfaceRenderStatus( g2, modelIndex, surfaceName );
}

static int CL_G2API_GetTime( void ) {
	return re->G2API_GetTime( 0 );
}

static void CL_G2API_SetTime( int time, int clock ) {
	re->G2API_SetTime( time, clock );
}

static void CL_G2API_AbsurdSmoothing( void *ghoul2, qboolean status ) {
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	re->G2API_AbsurdSmoothing( g2, status );
}

static void CL_G2API_SetRagDoll( void *ghoul2, sharedRagDollParams_t *params ) {
	if ( !ghoul2 ) return;

	CRagDollParams rdParams;
	if ( !params ) {
		re->G2API_ResetRagDoll( *((CGhoul2Info_v *)ghoul2) );
		return;
	}

	VectorCopy( params->angles, rdParams.angles );
	VectorCopy( params->position, rdParams.position );
	VectorCopy( params->scale, rdParams.scale );
	VectorCopy( params->pelvisAnglesOffset, rdParams.pelvisAnglesOffset );
	VectorCopy( params->pelvisPositionOffset, rdParams.pelvisPositionOffset );

	rdParams.fImpactStrength = params->fImpactStrength;
	rdParams.fShotStrength = params->fShotStrength;
	rdParams.me = params->me;

	rdParams.startFrame = params->startFrame;
	rdParams.endFrame = params->endFrame;

	rdParams.collisionType = params->collisionType;
	rdParams.CallRagDollBegin = params->CallRagDollBegin;

	rdParams.RagPhase = (CRagDollParams::ERagPhase)params->RagPhase;
	rdParams.effectorsToTurnOff = (CRagDollParams::ERagEffector)params->effectorsToTurnOff;

	re->G2API_SetRagDoll( *((CGhoul2Info_v *)ghoul2), &rdParams );
}

static void CL_G2API_AnimateG2Models( void *ghoul2, int time, sharedRagDollUpdateParams_t *params ) {
	if ( !ghoul2 ) return;
	if ( !params ) return;

	CRagDollUpdateParams rduParams;
	VectorCopy( params->angles, rduParams.angles );
	VectorCopy( params->position, rduParams.position );
	VectorCopy( params->scale, rduParams.scale );
	VectorCopy( params->velocity, rduParams.velocity );

	rduParams.me = params->me;
	rduParams.settleFrame = params->settleFrame;

	re->G2API_AnimateG2ModelsRag( *((CGhoul2Info_v *)ghoul2), time, &rduParams );
}

static qboolean CL_G2API_RagPCJConstraint( void *ghoul2, const char *boneName, vec3_t min, vec3_t max ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJConstraint( *((CGhoul2Info_v *)ghoul2), boneName, min, max );
}

static qboolean CL_G2API_RagPCJGradientSpeed( void *ghoul2, const char *boneName, const float speed ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJGradientSpeed( *((CGhoul2Info_v *)ghoul2), boneName, speed );
}

static qboolean CL_G2API_RagEffectorGoal( void *ghoul2, const char *boneName, vec3_t pos ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorGoal( *((CGhoul2Info_v *)ghoul2), boneName, pos );
}

static qboolean CL_G2API_GetRagBonePos( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetRagBonePos( *((CGhoul2Info_v *)ghoul2), boneName, pos, entAngles, entPos, entScale );
}

static qboolean CL_G2API_RagEffectorKick( void *ghoul2, const char *boneName, vec3_t velocity ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorKick( *((CGhoul2Info_v *)ghoul2), boneName, velocity );
}

static qboolean CL_G2API_RagForceSolve( void *ghoul2, qboolean force ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagForceSolve( *((CGhoul2Info_v *)ghoul2), force );
}

static qboolean CL_G2API_SetBoneIKState( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneIKState( *((CGhoul2Info_v *)ghoul2), time, boneName, ikState, params );
}

static qboolean CL_G2API_IKMove( void *ghoul2, int time, sharedIKMoveParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_IKMove( *((CGhoul2Info_v *)ghoul2), time, params );
}

static qboolean CL_G2API_RemoveBone( void *ghoul2, const char *boneName, int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_RemoveBone( g2, modelIndex, boneName );
}

static void CL_G2API_AttachInstanceToEntNum( void *ghoul2, int entityNum, qboolean server ) {
	if ( !ghoul2 ) return;
	re->G2API_AttachInstanceToEntNum( *((CGhoul2Info_v *)ghoul2), entityNum, server );
}

static void CL_G2API_ClearAttachedInstance( int entityNum ) {
	re->G2API_ClearAttachedInstance( entityNum );
}

static void CL_G2API_CleanEntAttachments( void ) {
	re->G2API_CleanEntAttachments();
}

static qboolean CL_G2API_OverrideServer( void *serverInstance ) {
	if ( !serverInstance ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)serverInstance);
	return re->G2API_OverrideServerWithClientData( g2, 0 );
}

static void CL_G2API_GetSurfaceName( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf ) {
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	char *tmp = re->G2API_GetSurfaceName( g2, modelIndex, surfNumber );
	strcpy( fillBuf, tmp );
}

static void CL_Key_SetCatcher( int catcher ) {
	// Don't allow the cgame module to close the console
	Key_SetCatcher( catcher | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
}

static void CGVM_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_VM_Set( var_name, value, VM_CGAME );
}

static void CGVM_Cmd_RemoveCommand( const char *cmd_name ) {
	Cmd_VM_RemoveCommand( cmd_name, VM_CGAME );
}

// legacy syscall

intptr_t CL_CgameSystemCalls( intptr_t *args ) {
	switch ( args[0] ) {
		//rww - alright, DO NOT EVER add a GAME/CGAME/UI generic call without adding a trap to match, and
		//all of these traps must be shared and have cases in sv_game, cl_cgame, and cl_ui. They must also
		//all be in the same order, and start at 100.
	case TRAP_MEMSET:
		Com_Memset( VMA(1), args[2], args[3] );
		return 0;

	case TRAP_MEMCPY:
		Com_Memcpy( VMA(1), VMA(2), args[3] );
		return 0;

	case TRAP_STRNCPY:
		strncpy( (char *)VMA(1), (const char *)VMA(2), args[3] );
		return args[1];

	case TRAP_SIN:
		return FloatAsInt( sin( VMF(1) ) );

	case TRAP_COS:
		return FloatAsInt( cos( VMF(1) ) );

	case TRAP_ATAN2:
		return FloatAsInt( atan2( VMF(1), VMF(2) ) );

	case TRAP_SQRT:
		return FloatAsInt( sqrt( VMF(1) ) );

	case TRAP_MATRIXMULTIPLY:
		MatrixMultiply( (vec3_t *)VMA(1), (vec3_t *)VMA(2), (vec3_t *)VMA(3) );
		return 0;

	case TRAP_ANGLEVECTORS:
		AngleVectors( (const float *)VMA(1), (float *)VMA(2), (float *)VMA(3), (float *)VMA(4) );
		return 0;

	case TRAP_PERPENDICULARVECTOR:
		PerpendicularVector( (float *)VMA(1), (const float *)VMA(2) );
		return 0;

	case TRAP_FLOOR:
		return FloatAsInt( floor( VMF(1) ) );

	case TRAP_CEIL:
		return FloatAsInt( ceil( VMF(1) ) );

	case TRAP_TESTPRINTINT:
		return 0;

	case TRAP_TESTPRINTFLOAT:
		return 0;

	case TRAP_ACOS:
		return FloatAsInt( Q_acos( VMF(1) ) );

	case TRAP_ASIN:
		return FloatAsInt( Q_asin( VMF(1) ) );

	case CG_PRINT:
		Com_Printf( "%s", (const char*)VMA(1) );
		return 0;

	case CG_ERROR:
		Com_Error( ERR_DROP, "%s", (const char*)VMA(1) );
		return 0;

	case CG_MILLISECONDS:
		return CL_Milliseconds();

		//rww - precision timer funcs... -ALWAYS- call end after start with supplied ptr, or you'll get a nasty memory leak.
		//not that you should be using these outside of debug anyway.. because you shouldn't be. So don't.
	case CG_PRECISIONTIMER_START:
		CL_PrecisionTimerStart( (void **)VMA(1) );
		return 0;

	case CG_PRECISIONTIMER_END:
		return CL_PrecisionTimerEnd( (void *)args[1] );

	case CG_CVAR_REGISTER:
		Cvar_Register( (vmCvar_t *)VMA(1), (const char *)VMA(2), (const char *)VMA(3), args[4] );
		return 0;

	case CG_CVAR_UPDATE:
		Cvar_Update( (vmCvar_t *)VMA(1) );
		return 0;

	case CG_CVAR_SET:
		Cvar_VM_Set( (const char *)VMA(1), (const char *)VMA(2), VM_CGAME );
		return 0;

	case CG_CVAR_VARIABLESTRINGBUFFER:
		Cvar_VariableStringBuffer( (const char *)VMA(1), (char *)VMA(2), args[3] );
		return 0;

	case CG_CVAR_GETHIDDENVALUE:
		return CL_GetValueForHidden((const char *)VMA(1));

	case CG_ARGC:
		return Cmd_Argc();

	case CG_ARGV:
		Cmd_ArgvBuffer( args[1], (char *)VMA(2), args[3] );
		return 0;

	case CG_ARGS:
		Cmd_ArgsBuffer( (char *)VMA(1), args[2] );
		return 0;

	case CG_FS_FOPENFILE:
		return FS_FOpenFileByMode( (const char *)VMA(1), (int *)VMA(2), (fsMode_t)args[3] );

	case CG_FS_READ:
		FS_Read( VMA(1), args[2], args[3] );
		return 0;

	case CG_FS_WRITE:
		FS_Write( VMA(1), args[2], args[3] );
		return 0;

	case CG_FS_FCLOSEFILE:
		FS_FCloseFile( args[1] );
		return 0;

	case CG_FS_GETFILELIST:
		return FS_GetFileList( (const char *)VMA(1), (const char *)VMA(2), (char *)VMA(3), args[4] );

	case CG_SENDCONSOLECOMMAND:
		Cbuf_AddText( (const char *)VMA(1) );
		return 0;

	case CG_ADDCOMMAND:
		CL_AddCgameCommand( (const char *)VMA(1) );
		return 0;

	case CG_REMOVECOMMAND:
		Cmd_VM_RemoveCommand( (const char *)VMA(1), VM_CGAME );
		return 0;

	case CG_SENDCLIENTCOMMAND:
		CL_AddReliableCommand2( (const char *)VMA(1) );
		return 0;

	case CG_UPDATESCREEN:
		// this is used during lengthy level loading, so pump message loop
		//		Com_EventLoop();	// FIXME: if a server restarts here, BAD THINGS HAPPEN!
		// We can't call Com_EventLoop here, a restart will crash and this _does_ happen
		// if there is a map change while we are downloading at pk3.
		// ZOID
		SCR_UpdateScreen();
		return 0;

	case CG_CM_LOADMAP:
		CL_CM_LoadMap( (const char *)VMA(1), (qboolean)args[2] );
		return 0;

	case CG_CM_NUMINLINEMODELS:
		return CM_NumInlineModels();

	case CG_CM_INLINEMODEL:
		return CM_InlineModel( args[1] );

	case CG_CM_TEMPBOXMODEL:
		return CM_TempBoxModel( (const float *)VMA(1), (const float *)VMA(2), /*int capsule*/ qfalse );

	case CG_CM_TEMPCAPSULEMODEL:
		return CM_TempBoxModel( (const float *)VMA(1), (const float *)VMA(2), /*int capsule*/ qtrue );

	case CG_CM_POINTCONTENTS:
		return CM_PointContents( (const float *)VMA(1), args[2] );

	case CG_CM_TRANSFORMEDPOINTCONTENTS:
		return CM_TransformedPointContents( (const float *)VMA(1), args[2], (const float *)VMA(3), (const float *)VMA(4) );

	case CG_CM_BOXTRACE:
		CM_BoxTrace( (trace_t *)VMA(1), (const float *)VMA(2), (const float *)VMA(3), (const float *)VMA(4), (const float *)VMA(5), args[6], args[7], /*int capsule*/ qfalse );
		return 0;

	case CG_CM_CAPSULETRACE:
		CM_BoxTrace( (trace_t *)VMA(1), (const float *)VMA(2), (const float *)VMA(3), (const float *)VMA(4), (const float *)VMA(5), args[6], args[7], /*int capsule*/ qtrue );
		return 0;

	case CG_CM_TRANSFORMEDBOXTRACE:
		CM_TransformedBoxTrace( (trace_t *)VMA(1), (const float *)VMA(2), (const float *)VMA(3), (const float *)VMA(4), (const float *)VMA(5), args[6], args[7], (const float *)VMA(8), (const float *)VMA(9), /*int capsule*/ qfalse );
		return 0;

	case CG_CM_TRANSFORMEDCAPSULETRACE:
		CM_TransformedBoxTrace( (trace_t *)VMA(1), (const float *)VMA(2), (const float *)VMA(3), (const float *)VMA(4), (const float *)VMA(5), args[6], args[7], (const float *)VMA(8), (const float *)VMA(9), /*int capsule*/ qtrue );
		return 0;

	case CG_CM_MARKFRAGMENTS:
		return re->MarkFragments( args[1], (const vec3_t *)VMA(2), (const float *)VMA(3), args[4], (float *)VMA(5), args[6], (markFragment_t *)VMA(7) );

	case CG_S_GETVOICEVOLUME:
		return CL_S_GetVoiceVolume( args[1] );

	case CG_S_MUTESOUND:
		S_MuteSound( args[1], args[2] );
		return 0;

	case CG_S_STARTSOUND:
		S_StartSound( (float *)VMA(1), args[2], args[3], args[4] );
		return 0;

	case CG_S_STARTLOCALSOUND:
		S_StartLocalSound( args[1], args[2] );
		return 0;

	case CG_S_CLEARLOOPINGSOUNDS:
		S_ClearLoopingSounds();
		return 0;

	case CG_S_ADDLOOPINGSOUND:
		S_AddLoopingSound( args[1], (const float *)VMA(2), (const float *)VMA(3), args[4] );
		return 0;

	case CG_S_ADDREALLOOPINGSOUND:
		/*S_AddRealLoopingSound*/S_AddLoopingSound( args[1], (const float *)VMA(2), (const float *)VMA(3), args[4] );
		return 0;

	case CG_S_STOPLOOPINGSOUND:
		S_StopLoopingSound( args[1] );
		return 0;

	case CG_S_UPDATEENTITYPOSITION:
		S_UpdateEntityPosition( args[1], (const float *)VMA(2) );
		return 0;

	case CG_S_RESPATIALIZE:
		S_Respatialize( args[1], (const float *)VMA(2), (vec3_t *)VMA(3), args[4] );
		return 0;

	case CG_S_SHUTUP:
		CL_S_Shutup( (qboolean)args[1] );
		return 0;

	case CG_S_REGISTERSOUND:
		return S_RegisterSound( (const char *)VMA(1) );

	case CG_S_STARTBACKGROUNDTRACK:
		S_StartBackgroundTrack( (const char *)VMA(1), (const char *)VMA(2), args[3]?qtrue:qfalse );
		return 0;

	case CG_S_UPDATEAMBIENTSET:
		S_UpdateAmbientSet((const char *)VMA(1), (float *)VMA(2));
		return 0;

	case CG_AS_PARSESETS:
		AS_ParseSets(qfalse);
		return 0;

	case CG_AS_ADDPRECACHEENTRY:
		AS_AddPrecacheEntry((const char *)VMA(1));
		return 0;

	case CG_S_ADDLOCALSET:
		return S_AddLocalSet((const char *)VMA(1), (float *)VMA(2), (float *)VMA(3), args[4], args[5]);

	case CG_AS_GETBMODELSOUND:
		return AS_GetBModelSound((const char *)VMA(1), args[2]);

	case CG_R_LOADWORLDMAP:
		re->LoadWorld( (const char *)VMA(1) );
		return 0;

	case CG_R_REGISTERMODEL:
		return re->RegisterModel( (const char *)VMA(1) );

	case CG_R_REGISTERSKIN:
		return re->RegisterSkin( (const char *)VMA(1) );

	case CG_R_REGISTERSHADER:
		return re->RegisterShader( (const char *)VMA(1) );

	case CG_R_REGISTERSHADERNOMIP:
		return re->RegisterShaderNoMip( (const char *)VMA(1) );

	case CG_R_REGISTERFONT:
		return re->RegisterFont( (const char *)VMA(1) );

	case CG_R_FONT_STRLENPIXELS:
		return re->Font_StrLenPixels( (const char *)VMA(1), args[2], VMF(3) );

	case CG_R_FONT_STRLENCHARS:
		return re->Font_StrLenChars( (const char *)VMA(1) );

	case CG_R_FONT_STRHEIGHTPIXELS:
		return re->Font_HeightPixels( args[1], VMF(2) );

	case CG_R_FONT_DRAWSTRING:
		re->Font_DrawString( args[1], args[2], (const char *)VMA(3), (const float *) VMA(4), args[5], args[6], VMF(7) );
		return 0;

	case CG_LANGUAGE_ISASIAN:
		return re->Language_IsAsian();

	case CG_LANGUAGE_USESSPACES:
		return re->Language_UsesSpaces();

	case CG_ANYLANGUAGE_READCHARFROMSTRING:
		return re->AnyLanguage_ReadCharFromString( (const char *) VMA(1), (int *) VMA(2), (qboolean *) VMA(3) );

	case CG_R_CLEARSCENE:
		re->ClearScene();
		return 0;

	case CG_R_CLEARDECALS:
		re->ClearDecals();
		return 0;

	case CG_R_ADDREFENTITYTOSCENE:
		re->AddRefEntityToScene( (const refEntity_t *)VMA(1) );
		return 0;

	case CG_R_ADDPOLYTOSCENE:
		re->AddPolyToScene( args[1], args[2], (const polyVert_t *)VMA(3), 1 );
		return 0;

	case CG_R_ADDPOLYSTOSCENE:
		re->AddPolyToScene( args[1], args[2], (const polyVert_t *)VMA(3), args[4] );
		return 0;

	case CG_R_ADDDECALTOSCENE:
		re->AddDecalToScene( (qhandle_t)args[1], (const float*)VMA(2), (const float*)VMA(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), (qboolean)args[9], VMF(10), (qboolean)args[11] );
		return 0;

	case CG_R_LIGHTFORPOINT:
		return re->LightForPoint( (float *)VMA(1), (float *)VMA(2), (float *)VMA(3), (float *)VMA(4) );

	case CG_R_ADDLIGHTTOSCENE:
		re->AddLightToScene( (const float *)VMA(1), VMF(2), VMF(3), VMF(4), VMF(5) );
		return 0;

	case CG_R_ADDADDITIVELIGHTTOSCENE:
		re->AddAdditiveLightToScene( (const float *)VMA(1), VMF(2), VMF(3), VMF(4), VMF(5) );
		return 0;

	case CG_R_RENDERSCENE:
		re->RenderScene( (const refdef_t *)VMA(1) );
		return 0;

	case CG_R_SETCOLOR:
		re->SetColor( (const float *)VMA(1) );
		return 0;

	case CG_R_DRAWSTRETCHPIC:
		re->DrawStretchPic( VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), args[9] );
		return 0;

	case CG_R_MODELBOUNDS:
		re->ModelBounds( args[1], (float *)VMA(2), (float *)VMA(3) );
		return 0;

	case CG_R_LERPTAG:
		return re->LerpTag( (orientation_t *)VMA(1), args[2], args[3], args[4], VMF(5), (const char *)VMA(6) );

	case CG_R_DRAWROTATEPIC:
		re->DrawRotatePic( VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), VMF(9), args[10] );
		return 0;

	case CG_R_DRAWROTATEPIC2:
		re->DrawRotatePic2( VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), VMF(9), args[10] );
		return 0;

	case CG_R_SETRANGEFOG:
		re->SetRangedFog( VMF(1) );
		return 0;

	case CG_R_SETREFRACTIONPROP:
		re->SetRefractionProperties( VMF(1), VMF(2), (qboolean)args[3], (qboolean)args[4] );
		return 0;

	case CG_GETGLCONFIG:
		CL_GetGlconfig( (glconfig_t *)VMA(1) );
		return 0;

	case CG_GETGAMESTATE:
		CL_GetGameState( (gameState_t *)VMA(1) );
		return 0;

	case CG_GETCURRENTSNAPSHOTNUMBER:
		CL_GetCurrentSnapshotNumber( (int *)VMA(1), (int *)VMA(2) );
		return 0;

	case CG_GETSNAPSHOT:
		return CL_GetSnapshot( args[1], (snapshot_t *)VMA(2) );

	case CG_GETDEFAULTSTATE:
		return CL_GetDefaultState(args[1], (entityState_t *)VMA(2));

	case CG_GETSERVERCOMMAND:
		return CL_GetServerCommand( args[1] );

	case CG_GETCURRENTCMDNUMBER:
		return CL_GetCurrentCmdNumber();

	case CG_GETUSERCMD:
		return CL_GetUserCmd( args[1], (struct usercmd_s *)VMA(2) );

	case CG_SETUSERCMDVALUE:
		_CL_SetUserCmdValue( args[1], VMF(2), VMF(3), VMF(4), VMF(5), args[6], args[7], (qboolean)args[8] );
		return 0;

	case CG_SETCLIENTFORCEANGLE:
		CL_SetClientForceAngle(args[1], (float *)VMA(2));
		return 0;

	case CG_SETCLIENTTURNEXTENT:
		return 0;

	case CG_OPENUIMENU:
		CL_OpenUIMenu( args[1] );
		return 0;

	case CG_MEMORY_REMAINING:
		return Hunk_MemoryRemaining();

	case CG_KEY_ISDOWN:
		return Key_IsDown( args[1] );

	case CG_KEY_GETCATCHER:
		return Key_GetCatcher();

	case CG_KEY_SETCATCHER:
		CL_Key_SetCatcher( args[1] );
		return 0;

	case CG_KEY_GETKEY:
		return Key_GetKey( (const char *)VMA(1) );

	case CG_PC_ADD_GLOBAL_DEFINE:
		return botlib_export->PC_AddGlobalDefine( (char *)VMA(1) );

	case CG_PC_LOAD_SOURCE:
		return botlib_export->PC_LoadSourceHandle( (const char *)VMA(1) );

	case CG_PC_FREE_SOURCE:
		return botlib_export->PC_FreeSourceHandle( args[1] );

	case CG_PC_READ_TOKEN:
		return botlib_export->PC_ReadTokenHandle( args[1], (struct pc_token_s *)VMA(2) );

	case CG_PC_SOURCE_FILE_AND_LINE:
		return botlib_export->PC_SourceFileAndLine( args[1], (char *)VMA(2), (int *)VMA(3) );

	case CG_PC_LOAD_GLOBAL_DEFINES:
		return botlib_export->PC_LoadGlobalDefines ( (char *)VMA(1) );

	case CG_PC_REMOVE_ALL_GLOBAL_DEFINES:
		botlib_export->PC_RemoveAllGlobalDefines ( );
		return 0;

	case CG_S_STOPBACKGROUNDTRACK:
		S_StopBackgroundTrack();
		return 0;

	case CG_REAL_TIME:
		return Com_RealTime( (struct qtime_s *)VMA(1) );

	case CG_SNAPVECTOR:
		Sys_SnapVector( (float *)VMA(1) );
		return 0;

	case CG_CIN_PLAYCINEMATIC:
		return CIN_PlayCinematic((const char *)VMA(1), args[2], args[3], args[4], args[5], args[6]);

	case CG_CIN_STOPCINEMATIC:
		return CIN_StopCinematic(args[1]);

	case CG_CIN_RUNCINEMATIC:
		return CIN_RunCinematic(args[1]);

	case CG_CIN_DRAWCINEMATIC:
		CIN_DrawCinematic(args[1]);
		return 0;

	case CG_CIN_SETEXTENTS:
		CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
		return 0;

	case CG_R_REMAP_SHADER:
		re->RemapShader( (const char *)VMA(1), (const char *)VMA(2), (const char *)VMA(3) );
		return 0;

	case CG_R_GET_LIGHT_STYLE:
		re->GetLightStyle(args[1], (unsigned char *)VMA(2));
		return 0;

	case CG_R_SET_LIGHT_STYLE:
		re->SetLightStyle(args[1], args[2]);
		return 0;

	case CG_R_GET_BMODEL_VERTS:
		re->GetBModelVerts( args[1], (float (*)[3])VMA(2), (float *)VMA(3) );
		return 0;

	case CG_R_GETDISTANCECULL:
		{
			float *f = (float *)VMA(1);
			*f = re->GetDistanceCull();
		}
		return 0;

	case CG_R_GETREALRES:
		{
			int *w = (int *)VMA(1);
			int *h = (int *)VMA(2);
			re->GetRealRes( w, h );
		}
		return 0;

	case CG_R_AUTOMAPELEVADJ:
		re->AutomapElevationAdjustment(VMF(1));
		return 0;

	case CG_R_INITWIREFRAMEAUTO:
		return re->InitializeWireframeAutomap();

	case CG_GET_ENTITY_TOKEN:
		return re->GetEntityToken( (char *)VMA(1), args[2] );

	case CG_R_INPVS:
		return re->inPVS( (const float *)VMA(1), (const float *)VMA(2), (byte *)VMA(3) );

#ifndef DEBUG_DISABLEFXCALLS
	case CG_FX_ADDLINE:
		CGFX_AddLine( (float *)VMA(1), (float *)VMA(2), VMF(3), VMF(4), VMF(5),
			VMF(6), VMF(7), VMF(8),
			(float *)VMA(9), (float *)VMA(10), VMF(11),
			args[12], args[13], args[14]);
		return 0;
	case CG_FX_REGISTER_EFFECT:
		return FX_RegisterEffect((const char *)VMA(1));

	case CG_FX_PLAY_EFFECT:
		FX_PlayEffect((const char *)VMA(1), (float *)VMA(2), (float *)VMA(3), args[4], args[5] );
		return 0;

	case CG_FX_PLAY_ENTITY_EFFECT:
		assert(0);
		return 0;

	case CG_FX_PLAY_EFFECT_ID:
		FX_PlayEffectID(args[1], (float *)VMA(2), (float *)VMA(3), args[4], args[5] );
		return 0;

	case CG_FX_PLAY_PORTAL_EFFECT_ID:
		FX_PlayEffectID(args[1], (float *)VMA(2), (float *)VMA(3), args[4], args[5], qtrue );
		return 0;

	case CG_FX_PLAY_ENTITY_EFFECT_ID:
		FX_PlayEntityEffectID(args[1], (float *)VMA(2), (vec3_t *)VMA(3), args[4], args[5], args[6], args[7] );
		return 0;

	case CG_FX_PLAY_BOLTED_EFFECT_ID:
		return CGFX_PlayBoltedEffectID( args[1], (float *)VMA(2), (void *)args[3], args[4], args[5], args[6], args[7], (qboolean)args[8] );

	case CG_FX_ADD_SCHEDULED_EFFECTS:
		FX_AddScheduledEffects((qboolean)args[1]);
		return 0;

	case CG_FX_DRAW_2D_EFFECTS:
		FX_Draw2DEffects ( VMF(1), VMF(2) );
		return 0;

	case CG_FX_INIT_SYSTEM:
		return FX_InitSystem( (refdef_t*)VMA(1) );

	case CG_FX_SET_REFDEF:
		FX_SetRefDefFromCGame( (refdef_t*)VMA(1) );
		return 0;

	case CG_FX_FREE_SYSTEM:
		return FX_FreeSystem();

	case CG_FX_ADJUST_TIME:
		FX_AdjustTime(args[1]);
		return 0;

	case CG_FX_RESET:
		FX_Free ( false );
		return 0;

	case CG_FX_ADDPOLY:
		CGFX_AddPoly( (addpolyArgStruct_t *)VMA(1) );
		return 0;
	case CG_FX_ADDBEZIER:
		CGFX_AddBezier( (addbezierArgStruct_t *)VMA(1) );
		return 0;
	case CG_FX_ADDPRIMITIVE:
		CGFX_AddPrimitive( (effectTrailArgStruct_t *)VMA(1) );
		return 0;
	case CG_FX_ADDSPRITE:
		CGFX_AddSprite( (addspriteArgStruct_t *)VMA(1) );
		return 0;
	case CG_FX_ADDELECTRICITY:
		CGFX_AddElectricity( (addElectricityArgStruct_t *)VMA(1) );
		return 0;
#else
	case CG_FX_REGISTER_EFFECT:
	case CG_FX_PLAY_EFFECT:
	case CG_FX_PLAY_ENTITY_EFFECT:
	case CG_FX_PLAY_EFFECT_ID:
	case CG_FX_PLAY_PORTAL_EFFECT_ID:
	case CG_FX_PLAY_ENTITY_EFFECT_ID:
	case CG_FX_PLAY_BOLTED_EFFECT_ID:
	case CG_FX_ADD_SCHEDULED_EFFECTS:
	case CG_FX_INIT_SYSTEM:
	case CG_FX_FREE_SYSTEM:
	case CG_FX_ADJUST_TIME:
	case CG_FX_ADDPOLY:
	case CG_FX_ADDBEZIER:
	case CG_FX_ADDPRIMITIVE:
	case CG_FX_ADDSPRITE:
	case CG_FX_ADDELECTRICITY:
		return 0;
#endif

	case CG_ROFF_CLEAN:
		return CL_ROFF_Clean();

	case CG_ROFF_UPDATE_ENTITIES:
		CL_ROFF_UpdateEntities();
		return 0;

	case CG_ROFF_CACHE:
		return CL_ROFF_Cache( (char *)VMA(1) );

	case CG_ROFF_PLAY:
		return CL_ROFF_Play( args[1], args[2], (qboolean)args[3] );

	case CG_ROFF_PURGE_ENT:
		return CL_ROFF_Purge_Ent( args[1] );

	case CG_TRUEMALLOC:
		VM_Shifted_Alloc((void **)VMA(1), args[2]);
		return 0;

	case CG_TRUEFREE:
		VM_Shifted_Free((void **)VMA(1));
		return 0;

	case CG_G2_LISTSURFACES:
		CL_G2API_ListModelSurfaces( (void *)args[1] );
		return 0;

	case CG_G2_LISTBONES:
		CL_G2API_ListModelBones( (void *)args[1], args[2]);
		return 0;

	case CG_G2_HAVEWEGHOULMODELS:
		return CL_G2API_HaveWeGhoul2Models( (void *)args[1] );

	case CG_G2_SETMODELS:
		CL_G2API_SetGhoul2ModelIndexes( (void *)args[1],(qhandle_t *)VMA(2),(qhandle_t *)VMA(3));
		return 0;

	case CG_G2_GETBOLT:
		return CL_G2API_GetBoltMatrix((void *)args[1], args[2], args[3], (mdxaBone_t *)VMA(4), (const float *)VMA(5),(const float *)VMA(6), args[7], (qhandle_t *)VMA(8), (float *)VMA(9));

	case CG_G2_GETBOLT_NOREC:
		return CL_G2API_GetBoltMatrix_NoReconstruct((void *)args[1], args[2], args[3], (mdxaBone_t *)VMA(4), (const float *)VMA(5),(const float *)VMA(6), args[7], (qhandle_t *)VMA(8), (float *)VMA(9));

	case CG_G2_GETBOLT_NOREC_NOROT:
		return CL_G2API_GetBoltMatrix_NoRecNoRot((void *)args[1], args[2], args[3], (mdxaBone_t *)VMA(4), (const float *)VMA(5),(const float *)VMA(6), args[7], (qhandle_t *)VMA(8), (float *)VMA(9));

	case CG_G2_INITGHOUL2MODEL:
		return CL_G2API_InitGhoul2Model((void **)VMA(1), (const char *)VMA(2), args[3], (qhandle_t) args[4], (qhandle_t) args[5], args[6], args[7]);

	case CG_G2_SETSKIN:
		return CL_G2API_SetSkin( (void *)args[1], args[2], args[3], args[4] );

	case CG_G2_COLLISIONDETECT:
		CL_G2API_CollisionDetect ( (CollisionRecord_t*)VMA(1), (void *)args[2], (const float*)VMA(3), (const float*)VMA(4), args[5], args[6], (float*)VMA(7), (float*)VMA(8), (float*)VMA(9), args[10], args[11], VMF(12) );
		return 0;

	case CG_G2_COLLISIONDETECTCACHE:
		CL_G2API_CollisionDetectCache ( (CollisionRecord_t*)VMA(1), (void *)args[2], (const float*)VMA(3), (const float*)VMA(4), args[5], args[6], (float*)VMA(7), (float*)VMA(8), (float*)VMA(9), args[10], args[11], VMF(12) );
		return 0;

	case CG_G2_ANGLEOVERRIDE:
		return CL_G2API_SetBoneAngles( (void *)args[1], args[2], (const char *)VMA(3), (float *)VMA(4), args[5], args[6], args[7], args[8], (qhandle_t *)VMA(9), args[10], args[11] );

	case CG_G2_CLEANMODELS:
		CL_G2API_CleanGhoul2Models( (void **)VMA(1) );
		return 0;

	case CG_G2_PLAYANIM:
		return CL_G2API_SetBoneAnim( (void *)args[1], args[2], (const char *)VMA(3), args[4], args[5], args[6], VMF(7), args[8], VMF(9), args[10] );

	case CG_G2_GETBONEANIM:
		return CL_G2API_GetBoneAnim( (void *)args[1], (const char *)VMA(2), args[3], (float *)VMA(4), (int *)VMA(5), (int *)VMA(6), (int *)VMA(7), (float *)VMA(8), (int *)VMA(9), args[10] );

	case CG_G2_GETBONEFRAME:
		return CL_G2API_GetBoneFrame( (void *)args[1], (const char*)VMA(2), args[3], (float *)VMA(4), (int *)VMA(5), args[6] );

	case CG_G2_GETGLANAME:
		CL_G2API_GetGLAName( (void *)args[1], args[2], (char *)VMA(3) );
		return 0;

	case CG_G2_COPYGHOUL2INSTANCE:
		return CL_G2API_CopyGhoul2Instance( (void *)args[1], (void *)args[2], args[3] );

	case CG_G2_COPYSPECIFICGHOUL2MODEL:
		CL_G2API_CopySpecificGhoul2Model( (void *)args[1], args[2], (void *)args[3], args[4] );
		return 0;

	case CG_G2_DUPLICATEGHOUL2INSTANCE:
		CL_G2API_DuplicateGhoul2Instance( (void *)args[1], (void **)VMA(2) );
		return 0;

	case CG_G2_HASGHOUL2MODELONINDEX:
		return CL_G2API_HasGhoul2ModelOnIndex( (void *)VMA(1), args[2]);

	case CG_G2_REMOVEGHOUL2MODEL:
		return CL_G2API_RemoveGhoul2Model( (void *)VMA(1), args[2]);

	case CG_G2_SKINLESSMODEL:
		return CL_G2API_SkinlessModel( (void *)VMA(1), args[2] );

	case CG_G2_GETNUMGOREMARKS:
		return CL_G2API_GetNumGoreMarks( (void *)VMA(1), args[2] );

	case CG_G2_ADDSKINGORE:
		CL_G2API_AddSkinGore( (void *)args[1], (SSkinGoreData *)VMA(2));
		return 0;

	case CG_G2_CLEARSKINGORE:
		CL_G2API_ClearSkinGore ( (void *)args[1] );
		return 0;

	case CG_G2_SIZE:
		return CL_G2API_Ghoul2Size ( (void *)args[1] );

	case CG_G2_ADDBOLT:
		return CL_G2API_AddBolt( (void *)args[1], args[2], (const char *)VMA(3));

	case CG_G2_ATTACHENT:
		return CL_G2API_AttachEnt( (int*)VMA(1), (void *)args[2], args[3], args[4], args[5] );

	case CG_G2_SETBOLTON:
		CL_G2API_SetBoltInfo( (void *)args[1], args[2], args[3] );
		return 0;

	case CG_G2_SETROOTSURFACE:
		return CL_G2API_SetRootSurface( (void *)args[1], args[2], (const char *)VMA(3));

	case CG_G2_SETSURFACEONOFF:
		return CL_G2API_SetSurfaceOnOff( (void *)args[1], (const char *)VMA(2), args[3]);

	case CG_G2_SETNEWORIGIN:
		return CL_G2API_SetNewOrigin( (void *)args[1], args[2]);

	case CG_G2_DOESBONEEXIST:
		return CL_G2API_DoesBoneExist( (void *)args[1], args[2], (const char *)VMA(3));

	case CG_G2_GETSURFACERENDERSTATUS:
		return CL_G2API_GetSurfaceRenderStatus( (void *)args[1], args[2], (const char *)VMA(3));

	case CG_G2_GETTIME:
		return CL_G2API_GetTime();

	case CG_G2_SETTIME:
		CL_G2API_SetTime(args[1], args[2]);
		return 0;

	case CG_G2_ABSURDSMOOTHING:
		CL_G2API_AbsurdSmoothing( (void *)args[1], (qboolean)args[2]);
		return 0;


	case CG_G2_SETRAGDOLL:
		CL_G2API_SetRagDoll( (void *)args[1], (sharedRagDollParams_t *)VMA(2) );
		return 0;

	case CG_G2_ANIMATEG2MODELS:
		CL_G2API_AnimateG2Models( (void *)args[1], args[2], (sharedRagDollUpdateParams_t *)VMA(3) );
		return 0;

		//additional ragdoll options -rww
	case CG_G2_RAGPCJCONSTRAINT:
		return CL_G2API_RagPCJConstraint( (void *)args[1], (const char *)VMA(2), (float *)VMA(3), (float *)VMA(4));

	case CG_G2_RAGPCJGRADIENTSPEED:
		return CL_G2API_RagPCJGradientSpeed( (void *)args[1], (const char *)VMA(2), VMF(3));

	case CG_G2_RAGEFFECTORGOAL:
		return CL_G2API_RagEffectorGoal( (void *)args[1], (const char *)VMA(2), (float *)VMA(3));

	case CG_G2_GETRAGBONEPOS:
		return CL_G2API_GetRagBonePos( (void *)args[1], (const char *)VMA(2), (float *)VMA(3), (float *)VMA(4), (float *)VMA(5), (float *)VMA(6));

	case CG_G2_RAGEFFECTORKICK:
		return CL_G2API_RagEffectorKick( (void *)args[1], (const char *)VMA(2), (float *)VMA(3));

	case CG_G2_RAGFORCESOLVE:
		return CL_G2API_RagForceSolve( (void *)args[1], (qboolean)args[2]);

	case CG_G2_SETBONEIKSTATE:
		return CL_G2API_SetBoneIKState( (void *)args[1], args[2], (const char *)VMA(3), args[4], (sharedSetBoneIKStateParams_t *)VMA(5));

	case CG_G2_IKMOVE:
		return CL_G2API_IKMove( (void *)args[1], args[2], (sharedIKMoveParams_t *)VMA(3));

	case CG_G2_REMOVEBONE:
		return CL_G2API_RemoveBone( (void *)args[1], (const char *)VMA(2), args[3] );

	case CG_G2_ATTACHINSTANCETOENTNUM:
		CL_G2API_AttachInstanceToEntNum( (void *)args[1], args[2], (qboolean)args[3]);
		return 0;

	case CG_G2_CLEARATTACHEDINSTANCE:
		CL_G2API_ClearAttachedInstance(args[1]);
		return 0;

	case CG_G2_CLEANENTATTACHMENTS:
		CL_G2API_CleanEntAttachments();
		return 0;

	case CG_G2_OVERRIDESERVER:
		return CL_G2API_OverrideServer( (void *)args[1] );

	case CG_G2_GETSURFACENAME:
		CL_G2API_GetSurfaceName( (void *)args[1], args[2], args[3], (char *)VMA(4) );
		return 0;

	case CG_SP_GETSTRINGTEXTSTRING:
		return CL_SE_GetStringTextString( (const char *)VMA(1), (char *)VMA(2), args[3] );

	case CG_SET_SHARED_BUFFER:
		RegisterSharedMemory( (char *)VMA(1) );
		return 0;

	case CG_CM_REGISTER_TERRAIN:
		return 0;

	case CG_RMG_INIT:
		return 0;

	case CG_RE_INIT_RENDERER_TERRAIN:
		return 0;

	case CG_R_WEATHER_CONTENTS_OVERRIDE:
		//contentOverride = args[1];
		return 0;

	case CG_R_WORLDEFFECTCOMMAND:
		re->WorldEffectCommand((const char *)VMA(1));
		return 0;

	case CG_WE_ADDWEATHERZONE:
		re->AddWeatherZone( (float *)VMA(1), (float *)VMA(2) );
		return 0;

	default:
		assert(0); // bk010102
		Com_Error( ERR_DROP, "Bad cgame system trap: %ld", (long int) args[0] );
	}
	return 0;
}

// Stub function for old RMG system.
static void RE_InitRendererTerrain(const char * /*info*/) {}

// Plug the new API to the old one
#define forceLegacySystemCall(...) intptr_t args[] = { __VA_ARGS__ }; intptr_t ret = F_CG_SYS_CALL(args);

void CGI_Print(const char *msg, ...) {
	// TODO
}

void CGI_Error(int level, const char *error, ...) {
	// TODO
}

void CGI_SnapVector(float *v) {
	forceLegacySystemCall(CG_SNAPVECTOR, (intptr_t)(v));
}

int CGI_MemoryRemaining(void) {
	forceLegacySystemCall(CG_MEMORY_REMAINING);
	return ret;
}

void CGI_RegisterSharedMemory(char *memory) {
	forceLegacySystemCall(CG_SET_SHARED_BUFFER, (intptr_t)(memory));
}

void CGI_TrueMalloc(void **ptr, int size) {
	forceLegacySystemCall(CG_TRUEMALLOC, (intptr_t)(ptr), size);
}

void CGI_TrueFree(void **ptr) {
	forceLegacySystemCall(CG_TRUEFREE, (intptr_t)(ptr));
}


// timing
int CGI_Milliseconds(void) {
	forceLegacySystemCall(CG_MILLISECONDS);
	return ret;
}

int CGI_RealTime(qtime_t *qtime) {
	forceLegacySystemCall(CG_REAL_TIME, (intptr_t)(qtime));
	return ret;
}

void CGI_PrecisionTimerStart(void **timer) {
	forceLegacySystemCall(CG_PRECISIONTIMER_START, (intptr_t)(timer));
}

int CGI_PrecisionTimerEnd(void *timer) {
	forceLegacySystemCall(CG_PRECISIONTIMER_END, (intptr_t)(timer));
	return ret;
}


// cvar
void CGI_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags) {
	forceLegacySystemCall(CG_CVAR_REGISTER, (intptr_t)(vmCvar), (intptr_t)(varName), (intptr_t)(defaultValue), flags);
}

void CGI_Cvar_Set(const char *var_name, const char *value) {
	forceLegacySystemCall(CG_CVAR_SET, (intptr_t)(var_name), (intptr_t)(value));
}

void CGI_Cvar_Update(vmCvar_t *vmCvar) {
	forceLegacySystemCall(CG_CVAR_UPDATE, (intptr_t)(vmCvar));
}

void CGI_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize) {
	forceLegacySystemCall(CG_CVAR_VARIABLESTRINGBUFFER, (intptr_t)(var_name), (intptr_t)(buffer), bufsize);
}


// command
void CGI_AddCommand(const char *cmd_name) {
	forceLegacySystemCall(CG_ADDCOMMAND, (intptr_t)(cmd_name));
}

int CGI_Cmd_Argc(void) {
	forceLegacySystemCall(CG_ARGC);
	return ret;
}

void CGI_Cmd_Args(char *buffer, int bufferLength) {
	forceLegacySystemCall(CG_ARGS, (intptr_t)(buffer), bufferLength);
}

void CGI_Cmd_Argv(int arg, char *buffer, int bufferLength) {
	forceLegacySystemCall(CG_ARGV, arg, (intptr_t)(buffer), bufferLength);
}

void CGI_RemoveCommand(const char *cmd_name) {
	forceLegacySystemCall(CG_REMOVECOMMAND, (intptr_t)(cmd_name));
}

void CGI_SendClientCommand(const char *cmd) {
	forceLegacySystemCall(CG_SENDCLIENTCOMMAND, (intptr_t)(cmd));
}

void CGI_SendConsoleCommand(const char *text) {
	forceLegacySystemCall(CG_SENDCONSOLECOMMAND, (intptr_t)(text));
}


// filesystem
void CGI_FS_Close(fileHandle_t f) {
	forceLegacySystemCall(CG_FS_FCLOSEFILE, f);
}

int CGI_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize) {
	forceLegacySystemCall(CG_FS_GETFILELIST, (intptr_t)(path), (intptr_t)(extension), (intptr_t)(listbuf), bufsize);
	return ret;
}

int CGI_FS_Open(const char *qpath, fileHandle_t *f, fsMode_t mode) {
	forceLegacySystemCall(CG_FS_FOPENFILE, (intptr_t)(qpath), (intptr_t)(f), mode);
	return ret;
}

int CGI_FS_Read(void *buffer, int len, fileHandle_t f) {
	forceLegacySystemCall(CG_FS_READ, (intptr_t)(buffer), len, f);
	return ret;
}

int CGI_FS_Write(const void *buffer, int len, fileHandle_t f) {
	forceLegacySystemCall(CG_FS_WRITE, (intptr_t)(buffer), len, f);
	return ret;
}


// screen
void CGI_UpdateScreen(void) {
	forceLegacySystemCall(CG_UPDATESCREEN);
}


// clip model
clipHandle_t CGI_CM_InlineModel(int index) {
	forceLegacySystemCall(CG_CM_INLINEMODEL, index);
	return ret;
}

void CGI_CM_LoadMap(const char *mapname, qboolean subBSP) {
	forceLegacySystemCall(CG_CM_LOADMAP, (intptr_t)(mapname), subBSP);
}

int CGI_CM_NumInlineModels(void) {
	forceLegacySystemCall(CG_CM_NUMINLINEMODELS);
	return ret;
}

int CGI_CM_PointContents(const vec3_t p, clipHandle_t model) {
	forceLegacySystemCall(CG_CM_POINTCONTENTS, (intptr_t)(p), model);
	return ret;
}

int CGI_CM_RegisterTerrain(const char *config) {
	forceLegacySystemCall(CG_CM_REGISTER_TERRAIN, (intptr_t)(config));
	return ret;
}

clipHandle_t CGI_CM_TempModel(const vec3_t mins, const vec3_t maxs, int capsule) {
	if (!capsule) {
		forceLegacySystemCall(CG_CM_TEMPBOXMODEL, (intptr_t)(mins), (intptr_t)(maxs));
		return ret;
	}
	else {
		forceLegacySystemCall(CG_CM_TEMPCAPSULEMODEL, (intptr_t)(mins), (intptr_t)(maxs));
		return ret;
	}
}

void CGI_CM_Trace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, int capsule) {
	if (!capsule) {
		forceLegacySystemCall(CG_CM_BOXTRACE, (intptr_t)(results), (intptr_t)(start), (intptr_t)(end), (intptr_t)(mins), (intptr_t)(maxs), model, brushmask);
	}
	else {
		forceLegacySystemCall(CG_CM_CAPSULETRACE, (intptr_t)(results), (intptr_t)(start), (intptr_t)(end), (intptr_t)(mins), (intptr_t)(maxs), model, brushmask);
	}
}

int CGI_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles) {
	forceLegacySystemCall(CG_CM_TRANSFORMEDPOINTCONTENTS, (intptr_t)(p), model, (intptr_t)(origin), (intptr_t)(angles));
	return ret;
}

void CGI_CM_TransformedTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles, int capsule) {
	if (!capsule) {
		forceLegacySystemCall(CG_CM_TRANSFORMEDBOXTRACE, (intptr_t)(results), (intptr_t)(start), (intptr_t)(end), (intptr_t)(mins), (intptr_t)(maxs), model, brushmask, (intptr_t)(origin), (intptr_t)(angles));
	}
	else {
		forceLegacySystemCall(CG_CM_TRANSFORMEDCAPSULETRACE, (intptr_t)(results), (intptr_t)(start), (intptr_t)(end), (intptr_t)(mins), (intptr_t)(maxs), model, brushmask, (intptr_t)(origin), (intptr_t)(angles));
	}
}

void CGI_RMG_Init(int terrainID, const char *terrainInfo) {
	forceLegacySystemCall(CG_RMG_INIT, terrainID, (intptr_t)(terrainInfo));
}


// sound
int CGI_S_AddLocalSet(const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time) {
	forceLegacySystemCall(CG_S_ADDLOCALSET, (intptr_t)(name), (intptr_t)(listener_origin), (intptr_t)(origin), entID, time);
	return ret;
}

void CGI_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx) {
	forceLegacySystemCall(CG_S_ADDLOOPINGSOUND, entityNum, (intptr_t)(origin), (intptr_t)(velocity), sfx);
}

void CGI_S_ClearLoopingSounds(void) {
	forceLegacySystemCall(CG_S_CLEARLOOPINGSOUNDS);
}

int CGI_S_GetVoiceVolume(int entID) {
	forceLegacySystemCall(CG_S_GETVOICEVOLUME, entID);
	return ret;
}

void CGI_S_MuteSound(int entityNum, int entchannel) {
	forceLegacySystemCall(CG_S_MUTESOUND, entityNum, entchannel);
}

sfxHandle_t CGI_S_RegisterSound(const char *sample) {
	forceLegacySystemCall(CG_S_REGISTERSOUND, (intptr_t)(sample));
	return ret;
}

void CGI_S_Respatialize(int entityNum, const vec3_t origin, matrix3_t axis, int inwater) {
	forceLegacySystemCall(CG_S_MUTESOUND, entityNum, (intptr_t)(origin), (intptr_t)(axis), inwater);
}

void CGI_S_Shutup(qboolean shutup) {
	forceLegacySystemCall(CG_S_SHUTUP, shutup);
}

void CGI_S_StartBackgroundTrack(const char *intro, const char *loop, qboolean bReturnWithoutStarting) {
	forceLegacySystemCall(CG_S_STARTBACKGROUNDTRACK, (intptr_t)(intro), (intptr_t)(loop), bReturnWithoutStarting);
}

void CGI_S_StartLocalSound(sfxHandle_t sfx, int channelNum) {
	forceLegacySystemCall(CG_S_STARTLOCALSOUND, sfx, channelNum);
}

void CGI_S_StartSound(const vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx) {
	forceLegacySystemCall(CG_S_STARTSOUND, (intptr_t)(origin), entnum, entchannel, sfx);
}

void CGI_S_StopBackgroundTrack(void) {
	forceLegacySystemCall(CG_S_STOPBACKGROUNDTRACK);
}

void CGI_S_StopLoopingSound(int entityNum) {
	forceLegacySystemCall(CG_S_STOPLOOPINGSOUND, entityNum);
}

void CGI_S_UpdateEntityPosition(int entityNum, const vec3_t origin) {
	forceLegacySystemCall(CG_S_UPDATEENTITYPOSITION, entityNum, (intptr_t)(origin));
}

void CGI_S_UpdateAmbientSet(const char *name, vec3_t origin) {
	forceLegacySystemCall(CG_S_UPDATEAMBIENTSET, (intptr_t)(name), (intptr_t)(origin));
}


// ambient sound
void CGI_AS_AddPrecacheEntry(const char *name) {
	forceLegacySystemCall(CG_AS_ADDPRECACHEENTRY, (intptr_t)(name));
}

sfxHandle_t CGI_AS_GetBModelSound(const char *name, int stage) {
	forceLegacySystemCall(CG_AS_GETBMODELSOUND, (intptr_t)(name), stage);
	return ret;
}

void CGI_AS_ParseSets(void) {
	forceLegacySystemCall(CG_AS_PARSESETS);
}

// renderer
void CGI_R_AddAdditiveLightToScene(const vec3_t org, float intensity, float r, float g, float b) {
	forceLegacySystemCall(CG_R_ADDADDITIVELIGHTTOSCENE, (intptr_t)(org), RVMF(intensity), RVMF(r), RVMF(g), RVMF(b));
}

void CGI_R_AddDecalToScene(qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary) {
	forceLegacySystemCall(CG_R_ADDDECALTOSCENE, shader, (intptr_t)(origin), (intptr_t)(dir), RVMF(orientation), RVMF(r), RVMF(g), RVMF(b), RVMF(a), alphaFade, RVMF(radius), temporary);
}

void CGI_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b) {
	forceLegacySystemCall(CG_R_ADDLIGHTTOSCENE, (intptr_t)(org), RVMF(intensity), RVMF(r), RVMF(g), RVMF(b));
}

void CGI_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts, int num) {
	forceLegacySystemCall(CG_R_ADDPOLYSTOSCENE, hShader, numVerts, (intptr_t)(verts), num);
}

void CGI_R_AddRefEntityToScene(const refEntity_t *re) {
	forceLegacySystemCall(CG_R_ADDREFENTITYTOSCENE, (intptr_t)(re));
}

unsigned int CGI_R_AnyLanguage_ReadCharFromString(const char *psText, int *piAdvanceCount, qboolean *pbIsTrailingPunctuation) {
	forceLegacySystemCall(CG_ANYLANGUAGE_READCHARFROMSTRING, (intptr_t)(psText), (intptr_t)(piAdvanceCount), (intptr_t)(pbIsTrailingPunctuation));
	return ret;
}

void CGI_R_AutomapElevationAdjustment(float newHeight) {
	forceLegacySystemCall(CG_R_AUTOMAPELEVADJ, RVMF(newHeight));
}

void CGI_R_ClearDecals(void) {
	forceLegacySystemCall(CG_R_CLEARDECALS);
}

void CGI_R_ClearScene(void) {
	forceLegacySystemCall(CG_R_CLEARSCENE);
}

void CGI_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader) {
	forceLegacySystemCall(CG_R_DRAWSTRETCHPIC, RVMF(x), RVMF(y), RVMF(w), RVMF(h), RVMF(s1), RVMF(t1), RVMF(s2), RVMF(t2), hShader);
}

void CGI_R_DrawRotatePic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, float a1, qhandle_t hShader) {
	forceLegacySystemCall(CG_R_DRAWROTATEPIC, RVMF(x), RVMF(y), RVMF(w), RVMF(h), RVMF(s1), RVMF(t1), RVMF(s2), RVMF(t2), RVMF(a1), hShader);
}

void CGI_R_DrawRotatePic2(float x, float y, float w, float h, float s1, float t1, float s2, float t2, float a1, qhandle_t hShader) {
	forceLegacySystemCall(CG_R_DRAWROTATEPIC2, RVMF(x), RVMF(y), RVMF(w), RVMF(h), RVMF(s1), RVMF(t1), RVMF(s2), RVMF(t2), RVMF(a1), hShader);
}

void CGI_R_Font_DrawString(int ox, int oy, const char *text, const float *rgba, const int setIndex, int iCharLimit, const float scale) {
	forceLegacySystemCall(CG_R_FONT_DRAWSTRING, ox, oy, (intptr_t)(text), (intptr_t)(rgba), setIndex, iCharLimit, RVMF(scale));
}

int CGI_R_Font_HeightPixels(const int iFontIndex, const float scale) {
	forceLegacySystemCall(CG_R_FONT_STRHEIGHTPIXELS, iFontIndex, RVMF(scale));
	return ret;
}

int CGI_R_Font_StrLenChars(const char *text) {
	forceLegacySystemCall(CG_R_FONT_STRLENCHARS, (intptr_t)(text));
	return ret;
}

int CGI_R_Font_StrLenPixels(const char *text, const int iFontIndex, const float scale) {
	forceLegacySystemCall(CG_R_FONT_STRLENPIXELS, (intptr_t)(text), iFontIndex, RVMF(scale));
	return ret;
}

void CGI_R_GetBModelVerts(int bmodelIndex, vec3_t *vec, vec3_t normal) {
	forceLegacySystemCall(CG_R_GET_BMODEL_VERTS, bmodelIndex, (intptr_t)(vec), (intptr_t)(normal));
}

float CGI_R_GetDistanceCull(void) {
	float f;
	forceLegacySystemCall(CG_R_GETDISTANCECULL, (intptr_t)&f);
	return f;
}

qboolean CGI_R_GetEntityToken(char *buffer, int size) {
	forceLegacySystemCall(CG_GET_ENTITY_TOKEN, (intptr_t)(buffer), size);
	return (qboolean)ret;
}

void CGI_R_GetLightStyle(int style, color4ub_t color) {
	forceLegacySystemCall(CG_R_GET_LIGHT_STYLE, style, (intptr_t)(color));
}

void CGI_R_GetRealRes(int *w, int *h) {
	forceLegacySystemCall(CG_R_GETREALRES, (intptr_t)(w), (intptr_t)(h));
}

qboolean CGI_R_InitializeWireframeAutomap(void) {
	forceLegacySystemCall(CG_R_INITWIREFRAMEAUTO);
	return (qboolean)ret;
}

qboolean CGI_R_InPVS(const vec3_t p1, const vec3_t p2, byte *mask) {
	forceLegacySystemCall(CG_R_INPVS, (intptr_t)(p1), (intptr_t)(p2), (intptr_t)(mask));
	return (qboolean)ret;
}

qboolean CGI_R_Language_IsAsian(void) {
	forceLegacySystemCall(CG_LANGUAGE_ISASIAN);
	return (qboolean)ret;
}

qboolean CGI_R_Language_UsesSpaces(void) {
	forceLegacySystemCall(CG_LANGUAGE_USESSPACES);
	return (qboolean)ret;
}

int CGI_R_LerpTag(orientation_t *tag, qhandle_t model, int startFrame, int endFrame, float frac, const char *tagName) {
	forceLegacySystemCall(CG_R_LERPTAG, (intptr_t)(tag), model, startFrame, endFrame, RVMF(frac), (intptr_t)(tagName));
	return ret;
}

int CGI_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir) {
	forceLegacySystemCall(CG_R_LIGHTFORPOINT, (intptr_t)(point), (intptr_t)(ambientLight), (intptr_t)(directedLight), (intptr_t)(lightDir));
	return ret;
}

void CGI_R_LoadWorld(const char *name) {
	forceLegacySystemCall(CG_R_LOADWORLDMAP, (intptr_t)(name));
}

int CGI_R_MarkFragments(int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer) {
	forceLegacySystemCall(CG_CM_MARKFRAGMENTS, numPoints, (intptr_t)(points), (intptr_t)(projection), maxPoints, (intptr_t)(pointBuffer), maxFragments, (intptr_t)(fragmentBuffer));
	return ret;
}

void CGI_R_ModelBounds(qhandle_t model, vec3_t mins, vec3_t maxs) {
	forceLegacySystemCall(CG_R_MODELBOUNDS, model, (intptr_t)(mins), (intptr_t)(maxs));
}

qhandle_t CGI_R_RegisterFont(const char *fontName) {
	forceLegacySystemCall(CG_R_REGISTERFONT, (intptr_t)(fontName));
	return ret;
}

qhandle_t CGI_R_RegisterModel(const char *name) {
	forceLegacySystemCall(CG_R_REGISTERMODEL, (intptr_t)(name));
	return ret;
}

qhandle_t CGI_R_RegisterShader(const char *name) {
	forceLegacySystemCall(CG_R_REGISTERSHADER, (intptr_t)(name));
	return ret;
}

qhandle_t CGI_R_RegisterShaderNoMip(const char *name) {
	forceLegacySystemCall(CG_R_REGISTERSHADERNOMIP, (intptr_t)(name));
	return ret;
}

qhandle_t CGI_R_RegisterSkin(const char *name) {
	forceLegacySystemCall(CG_R_REGISTERSKIN, (intptr_t)(name));
	return ret;
}

void CGI_R_RemapShader(const char *oldShader, const char *newShader, const char *offsetTime) {
	forceLegacySystemCall(CG_R_REMAP_SHADER, (intptr_t)(oldShader), (intptr_t)(newShader), (intptr_t)(offsetTime));
}

void CGI_R_RenderScene(const refdef_t *fd) {
	forceLegacySystemCall(CG_R_RENDERSCENE, (intptr_t)(fd));
}

void CGI_R_SetColor(const float *rgba) {
	forceLegacySystemCall(CG_R_SETCOLOR, (intptr_t)(rgba));
}

void CGI_R_SetLightStyle(int style, int color) {
	forceLegacySystemCall(CG_R_SET_LIGHT_STYLE, style, color);
}

void CGI_R_SetRangedFog(float range) {
	forceLegacySystemCall(CG_R_SETRANGEFOG, RVMF(range));
}

void CGI_R_SetRefractionProperties(float distortionAlpha, float distortionStretch, qboolean distortionPrePost, qboolean distortionNegate) {
	forceLegacySystemCall(CG_R_SETREFRACTIONPROP, RVMF(distortionAlpha), RVMF(distortionStretch), distortionPrePost, distortionNegate);
}

void CGI_R_WorldEffectCommand(const char *cmd) {
	forceLegacySystemCall(CG_R_WORLDEFFECTCOMMAND, (intptr_t)(cmd));
}

void CGI_RE_InitRendererTerrain(const char *info) {
	forceLegacySystemCall(CG_RE_INIT_RENDERER_TERRAIN, (intptr_t)(info));
}

void CGI_WE_AddWeatherZone(vec3_t mins, vec3_t maxs) {
	forceLegacySystemCall(CG_WE_ADDWEATHERZONE, (intptr_t)(mins), (intptr_t)(maxs));
}


// client
void CGI_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime) {
	forceLegacySystemCall(CG_GETCURRENTSNAPSHOTNUMBER, (intptr_t)(snapshotNumber), (intptr_t)(serverTime));
}

int CGI_GetCurrentCmdNumber(void) {
	forceLegacySystemCall(CG_GETCURRENTCMDNUMBER);
	return ret;
}

qboolean CGI_GetDefaultState(int index, entityState_t *state) {
	forceLegacySystemCall(CG_GETDEFAULTSTATE, index, (intptr_t)(state));
	return (qboolean)ret;
}

void CGI_GetGameState(gameState_t *gs) {
	forceLegacySystemCall(CG_GETGAMESTATE, (intptr_t)(gs));
}

void CGI_GetGlconfig(glconfig_t *glconfig) {
	forceLegacySystemCall(CG_GETGLCONFIG, (intptr_t)(glconfig));
}

qboolean CGI_GetServerCommand(int serverCommandNumber) {
	forceLegacySystemCall(CG_GETSERVERCOMMAND, serverCommandNumber);
	return (qboolean)ret;
}

qboolean CGI_GetSnapshot(int snapshotNumber, snapshot_t *snapshot) {
	forceLegacySystemCall(CG_GETSNAPSHOT, snapshotNumber, (intptr_t)(snapshot));
	return (qboolean)ret;
}

qboolean CGI_GetUserCmd(int cmdNumber, usercmd_t *ucmd) {
	forceLegacySystemCall(CG_GETUSERCMD, cmdNumber, (intptr_t)(ucmd));
	return (qboolean)ret;
}

void CGI_OpenUIMenu(int menu) {
	forceLegacySystemCall(CG_OPENUIMENU, menu);
}

void CGI_SetClientForceAngle(int time, vec3_t angle) {
	forceLegacySystemCall(CG_SETCLIENTFORCEANGLE, time, (intptr_t)(angle));
}

void CGI_SetUserCmdValue(int stateValue, float sensitivityScale, float mPitchOverride, float mYawOverride, float mSensitivityOverride, int fpSel, int invenSel, qboolean fighterControls) {
	forceLegacySystemCall(CG_SETUSERCMDVALUE, stateValue, RVMF(sensitivityScale), RVMF(mPitchOverride), RVMF(mYawOverride), RVMF(mSensitivityOverride), fpSel, invenSel, fighterControls);
}


// keys
int CGI_Key_GetCatcher(void) {
	forceLegacySystemCall(CG_KEY_GETCATCHER);
	return ret;
}

int CGI_Key_GetKey(const char *binding) {
	forceLegacySystemCall(CG_KEY_GETKEY, (intptr_t)(binding));
	return ret;
}

qboolean CGI_Key_IsDown(int keynum) {
	forceLegacySystemCall(CG_KEY_ISDOWN, keynum);
	return (qboolean)ret;
}

void CGI_Key_SetCatcher(int catcher) {
	forceLegacySystemCall(CG_KEY_SETCATCHER, catcher);
}


// preprocessor (botlib_export->PC_***)
int CGI_PC_AddGlobalDefine(char *string) {
	forceLegacySystemCall(CG_PC_ADD_GLOBAL_DEFINE, (intptr_t)(string));
	return ret;
}

int CGI_PC_FreeSource(int handle) {
	forceLegacySystemCall(CG_PC_FREE_SOURCE, handle);
	return ret;
}

int CGI_PC_LoadGlobalDefines(const char *filename) {
	forceLegacySystemCall(CG_PC_LOAD_GLOBAL_DEFINES, (intptr_t)(filename));
	return ret;
}

int CGI_PC_LoadSource(const char *filename) {
	forceLegacySystemCall(CG_PC_LOAD_SOURCE, (intptr_t)(filename));
	return ret;
}

int CGI_PC_ReadToken(int handle, pc_token_t *pc_token) {
	forceLegacySystemCall(CG_PC_READ_TOKEN, handle, (intptr_t)(pc_token));
	return ret;
}

void CGI_PC_RemoveAllGlobalDefines(void) {
	forceLegacySystemCall(CG_PC_REMOVE_ALL_GLOBAL_DEFINES);
}

int CGI_PC_SourceFileAndLine(int handle, char *filename, int *line) {
	forceLegacySystemCall(CG_PC_SOURCE_FILE_AND_LINE, handle, (intptr_t)(filename), (intptr_t)(line));
	return ret;
}


// cinematics
void CGI_CIN_DrawCinematic(int handle) {
	forceLegacySystemCall(CG_CIN_DRAWCINEMATIC, handle);
}

int CGI_CIN_PlayCinematic(const char *arg, int x, int y, int w, int h, int systemBits) {
	forceLegacySystemCall(CG_CIN_PLAYCINEMATIC, (intptr_t)(arg), x, y, w, h, systemBits);
	return ret;
}

e_status CGI_CIN_RunCinematic(int handle) {
	forceLegacySystemCall(CG_CIN_RUNCINEMATIC, handle);
	return (e_status)ret;
}

void CGI_CIN_SetExtents(int handle, int x, int y, int w, int h) {
	forceLegacySystemCall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

e_status CGI_CIN_StopCinematic(int handle) {
	forceLegacySystemCall(CG_CIN_STOPCINEMATIC, handle);
	return (e_status)ret;
}


// FX
void CGI_FX_AddLine(vec3_t start, vec3_t end, float size1, float size2, float sizeParm, float alpha1, float alpha2, float alphaParm, vec3_t sRGB, vec3_t eRGB, float rgbParm, int killTime, qhandle_t shader, int flags) {
	forceLegacySystemCall(CG_FX_ADDLINE, (intptr_t)(start), (intptr_t)(end), RVMF(size1), RVMF(size2), RVMF(sizeParm), RVMF(alpha1), RVMF(alpha2), RVMF(alphaParm), (intptr_t)(sRGB), (intptr_t)(eRGB), RVMF(rgbParm), killTime, shader, flags);
}

int CGI_FX_RegisterEffect(const char *file) {
	forceLegacySystemCall(CG_FX_REGISTER_EFFECT, (intptr_t)(file));
	return ret;
}

void CGI_FX_PlayEffect(const char *file, vec3_t org, vec3_t fwd, int vol, int rad) {
	forceLegacySystemCall(CG_FX_PLAY_EFFECT, (intptr_t)(file), (intptr_t)(org), (intptr_t)(fwd), vol, rad);
}

void CGI_FX_PlayEffectID(int id, vec3_t org, vec3_t fwd, int vol, int rad, qboolean isPortal) {
	forceLegacySystemCall(CG_FX_PLAY_EFFECT_ID, id, (intptr_t)(org), (intptr_t)(fwd), vol, rad, isPortal);
}

void CGI_FX_PlayEntityEffectID(int id, vec3_t org, matrix3_t axis, const int boltInfo, const int entNum, int vol, int rad) {
	forceLegacySystemCall(CG_FX_PLAY_ENTITY_EFFECT_ID, id, (intptr_t)(org), (intptr_t)(axis), boltInfo, entNum, vol, rad);
}

qboolean CGI_FX_PlayBoltedEffectID(int id, vec3_t org, void *pGhoul2, const int boltNum, const int entNum, const int modelNum, int iLooptime, qboolean isRelative) {
	forceLegacySystemCall(CG_FX_PLAY_BOLTED_EFFECT_ID, id, (intptr_t)(org), (intptr_t)(pGhoul2), boltNum, entNum, modelNum, iLooptime, isRelative);
	return (qboolean)ret;
}

void CGI_FX_AddScheduledEffects(qboolean portal) {
	forceLegacySystemCall(CG_FX_ADD_SCHEDULED_EFFECTS, portal);
}

int CGI_FX_InitSystem(refdef_t *refdef) {
	forceLegacySystemCall(CG_FX_INIT_SYSTEM, (intptr_t)(refdef));
	return ret;
}

void CGI_FX_SetRefDef(refdef_t *refdef) {
	forceLegacySystemCall(CG_FX_SET_REFDEF, (intptr_t)(refdef));
}

qboolean CGI_FX_FreeSystem(void) {
	forceLegacySystemCall(CG_FX_FREE_SYSTEM);
	return (qboolean)ret;
}

void CGI_FX_AdjustTime(int time) {
	forceLegacySystemCall(CG_FX_ADJUST_TIME, time);
}

void CGI_FX_Draw2DEffects(float screenXScale, float screenYScale) {
	forceLegacySystemCall(CG_FX_DRAW_2D_EFFECTS, RVMF(screenXScale), RVMF(screenYScale));
}

void CGI_FX_AddPoly(addpolyArgStruct_t *p) {
	forceLegacySystemCall(CG_FX_ADDPOLY, (intptr_t)(p));
}

void CGI_FX_AddBezier(addbezierArgStruct_t *p) {
	forceLegacySystemCall(CG_FX_ADDBEZIER, (intptr_t)(p));
}

void CGI_FX_AddPrimitive(effectTrailArgStruct_t *p) {
	forceLegacySystemCall(CG_FX_ADDPRIMITIVE, (intptr_t)(p));
}

void CGI_FX_AddSprite(addspriteArgStruct_t *p) {
	forceLegacySystemCall(CG_FX_ADDSPRITE, (intptr_t)(p));
}

void CGI_FX_AddElectricity(addElectricityArgStruct_t *p) {
	forceLegacySystemCall(CG_FX_ADDELECTRICITY, (intptr_t)(p));
}


// stringed
qboolean CGI_SE_GetStringTextString(const char *text, char *buffer, int bufferLength) {
	forceLegacySystemCall(CG_SP_GETSTRINGTEXTSTRING, (intptr_t)(text), (intptr_t)(buffer), bufferLength);
	return (qboolean)ret;
}


// roff
qboolean CGI_ROFF_Clean(void) {
	forceLegacySystemCall(CG_ROFF_CLEAN);
	return (qboolean)ret;
}

void CGI_ROFF_UpdateEntities(void) {
	forceLegacySystemCall(CG_ROFF_UPDATE_ENTITIES);
}

int CGI_ROFF_Cache(char *file) {
	forceLegacySystemCall(CG_ROFF_CACHE, (intptr_t)(file));
	return ret;
}

qboolean CGI_ROFF_Play(int entID, int roffID, qboolean doTranslation) {
	forceLegacySystemCall(CG_ROFF_PLAY, entID, roffID, doTranslation);
	return (qboolean)ret;
}

qboolean CGI_ROFF_Purge_Ent(int entID) {
	forceLegacySystemCall(CG_ROFF_PURGE_ENT, entID);
	return (qboolean)ret;
}


// ghoul2
/*void CGI_G2_ListModelSurfaces(void *ghlInfo) {
}
void CGI_G2_ListModelBones(void *ghlInfo, int frame) {
}
void CGI_G2_SetGhoul2ModelIndexes(void *ghoul2, qhandle_t *modelList, qhandle_t *skinList) {
}
qboolean CGI_G2_HaveWeGhoul2Models(void *ghoul2) {
}
qboolean CGI_G2API_GetBoltMatrix(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale) {
}
qboolean CGI_G2API_GetBoltMatrix_NoReconstruct(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale) {
}
qboolean CGI_G2API_GetBoltMatrix_NoRecNoRot(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale) {
}
int CGI_G2API_InitGhoul2Model(void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias) {
}
qboolean CGI_G2API_SetSkin(void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin) {
}
void CGI_G2API_CollisionDetect(CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius) {
}
void CGI_G2API_CollisionDetectCache(CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius) {
}
void CGI_G2API_CleanGhoul2Models(void **ghoul2Ptr) {
}
qboolean CGI_G2API_SetBoneAngles(void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags, const int up, const int right, const int forward, qhandle_t *modelList, int blendTime, int currentTime) {
}
qboolean CGI_G2API_SetBoneAnim(void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame, const int flags, const float animSpeed, const int currentTime, const float setFrame, const int blendTime) {
}
qboolean CGI_G2API_GetBoneAnim(void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex) {
}
qboolean CGI_G2API_GetBoneFrame(void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *modelList, const int modelIndex) {
}
void CGI_G2API_GetGLAName(void *ghoul2, int modelIndex, char *fillBuf) {
}
int CGI_G2API_CopyGhoul2Instance(void *g2From, void *g2To, int modelIndex) {
}
void CGI_G2API_CopySpecificGhoul2Model(void *g2From, int modelFrom, void *g2To, int modelTo) {
}
void CGI_G2API_DuplicateGhoul2Instance(void *g2From, void **g2To) {
}
qboolean CGI_G2API_HasGhoul2ModelOnIndex(void *ghlInfo, int modelIndex) {
}
qboolean CGI_G2API_RemoveGhoul2Model(void *ghlInfo, int modelIndex) {
}
qboolean CGI_G2API_SkinlessModel(void *ghlInfo, int modelIndex) {
}
int CGI_G2API_GetNumGoreMarks(void *ghlInfo, int modelIndex) {
}
void CGI_G2API_AddSkinGore(void *ghlInfo, SSkinGoreData *gore) {
}
void CGI_G2API_ClearSkinGore(void *ghlInfo) {
}
int CGI_G2API_Ghoul2Size(void *ghlInfo) {
}
int CGI_G2API_AddBolt(void *ghoul2, int modelIndex, const char *boneName) {
}
qboolean CGI_G2API_AttachEnt(int *boltInfo, void *ghlInfoTo, int toBoltIndex, int entNum, int toModelNum) {
}
void CGI_G2API_SetBoltInfo(void *ghoul2, int modelIndex, int boltInfo) {
}
qboolean CGI_G2API_SetRootSurface(void *ghoul2, const int modelIndex, const char *surfaceName) {
}
qboolean CGI_G2API_SetSurfaceOnOff(void *ghoul2, const char *surfaceName, const int flags) {
}
qboolean CGI_G2API_SetNewOrigin(void *ghoul2, const int boltIndex) {
}
qboolean CGI_G2API_DoesBoneExist(void *ghoul2, int modelIndex, const char *boneName) {
}
int CGI_G2API_GetSurfaceRenderStatus(void *ghoul2, const int modelIndex, const char *surfaceName) {
}
int CGI_G2API_GetTime(void) {
}
void CGI_G2API_SetTime(int time, int clock) {
}
void CGI_G2API_AbsurdSmoothing(void *ghoul2, qboolean status) {
}
void CGI_G2API_SetRagDoll(void *ghoul2, sharedRagDollParams_t *params) {
}
void CGI_G2API_AnimateG2Models(void *ghoul2, int time, sharedRagDollUpdateParams_t *params) {
}
qboolean CGI_G2API_RagPCJConstraint(void *ghoul2, const char *boneName, vec3_t min, vec3_t max) {
}
qboolean CGI_G2API_RagPCJGradientSpeed(void *ghoul2, const char *boneName, const float speed) {
}
qboolean CGI_G2API_RagEffectorGoal(void *ghoul2, const char *boneName, vec3_t pos) {
}
qboolean CGI_G2API_GetRagBonePos(void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale) {
}
qboolean CGI_G2API_RagEffectorKick(void *ghoul2, const char *boneName, vec3_t velocity) {
}
qboolean CGI_G2API_RagForceSolve(void *ghoul2, qboolean force) {
}
qboolean CGI_G2API_SetBoneIKState(void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params) {
}
qboolean CGI_G2API_IKMove(void *ghoul2, int time, sharedIKMoveParams_t *params) {
}
qboolean CGI_G2API_RemoveBone(void *ghoul2, const char *boneName, int modelIndex) {
}
void CGI_G2API_AttachInstanceToEntNum(void *ghoul2, int entityNum, qboolean server) {
}
void CGI_G2API_ClearAttachedInstance(int entityNum) {
}
void CGI_G2API_CleanEntAttachments(void) {
}
qboolean CGI_G2API_OverrideServer(void *serverInstance) {
}
void CGI_G2API_GetSurfaceName(void *ghoul2, int surfNumber, int modelIndex, char *fillBuf) {
}*/

void CL_BindCGame( void ) {
	static cgameImport_t cgi;
	cgameExport_t		*ret;
	GetCGameAPI_t		GetCGameAPI;
	char				dllName[MAX_OSPATH] = "cgame" ARCH_STRING DLL_EXT;

	memset( &cgi, 0, sizeof( cgi ) );

	cgvm = VM_Create( VM_CGAME );
	if ( cgvm && !cgvm->isLegacy ) {
		cgi.Print								= Com_Printf;
		cgi.Error								= Com_Error;
		cgi.SnapVector							= CGI_SnapVector;
		cgi.MemoryRemaining						= CGI_MemoryRemaining;
		cgi.RegisterSharedMemory				= CGI_RegisterSharedMemory;
		cgi.TrueMalloc							= CGI_TrueMalloc;
		cgi.TrueFree							= CGI_TrueFree;
		cgi.Milliseconds						= CGI_Milliseconds;
		cgi.RealTime							= CGI_RealTime;
		cgi.PrecisionTimerStart					= CGI_PrecisionTimerStart;
		cgi.PrecisionTimerEnd					= CGI_PrecisionTimerEnd;
		cgi.Cvar_Register						= CGI_Cvar_Register;
		cgi.Cvar_Set							= CGI_Cvar_Set;
		cgi.Cvar_Update							= CGI_Cvar_Update;
		cgi.Cvar_VariableStringBuffer			= CGI_Cvar_VariableStringBuffer;
		cgi.AddCommand							= CGI_AddCommand;
		cgi.Cmd_Argc							= CGI_Cmd_Argc;
		cgi.Cmd_Args							= CGI_Cmd_Args;
		cgi.Cmd_Argv							= CGI_Cmd_Argv;
		cgi.RemoveCommand						= CGI_RemoveCommand;
		cgi.SendClientCommand					= CGI_SendClientCommand;
		cgi.SendConsoleCommand					= CGI_SendConsoleCommand;
		cgi.FS_Close							= CGI_FS_Close;
		cgi.FS_GetFileList						= CGI_FS_GetFileList;
		cgi.FS_Open								= CGI_FS_Open;
		cgi.FS_Read								= CGI_FS_Read;
		cgi.FS_Write							= CGI_FS_Write;
		cgi.UpdateScreen						= CGI_UpdateScreen;
		cgi.CM_InlineModel						= CGI_CM_InlineModel;
		cgi.CM_LoadMap							= CGI_CM_LoadMap;
		cgi.CM_NumInlineModels					= CGI_CM_NumInlineModels;
		cgi.CM_PointContents					= CGI_CM_PointContents;
		cgi.CM_RegisterTerrain					= CGI_CM_RegisterTerrain;
		cgi.CM_TempModel						= CGI_CM_TempModel;
		cgi.CM_Trace							= CGI_CM_Trace;
		cgi.CM_TransformedPointContents			= CGI_CM_TransformedPointContents;
		cgi.CM_TransformedTrace					= CGI_CM_TransformedTrace;
		cgi.RMG_Init							= CGI_RMG_Init;
		cgi.S_AddLocalSet						= CGI_S_AddLocalSet;
		cgi.S_AddLoopingSound					= CGI_S_AddLoopingSound;
		cgi.S_ClearLoopingSounds				= CGI_S_ClearLoopingSounds;
		cgi.S_GetVoiceVolume					= CGI_S_GetVoiceVolume;
		cgi.S_MuteSound							= CGI_S_MuteSound;
		cgi.S_RegisterSound						= CGI_S_RegisterSound;
		cgi.S_Respatialize						= CGI_S_Respatialize;
		cgi.S_Shutup							= CGI_S_Shutup;
		cgi.S_StartBackgroundTrack				= CGI_S_StartBackgroundTrack;
		cgi.S_StartLocalSound					= CGI_S_StartLocalSound;
		cgi.S_StartSound						= CGI_S_StartSound;
		cgi.S_StopBackgroundTrack				= CGI_S_StopBackgroundTrack;
		cgi.S_StopLoopingSound					= CGI_S_StopLoopingSound;
		cgi.S_UpdateEntityPosition				= CGI_S_UpdateEntityPosition;
		cgi.S_UpdateAmbientSet					= CGI_S_UpdateAmbientSet;
		cgi.AS_AddPrecacheEntry					= CGI_AS_AddPrecacheEntry;
		cgi.AS_GetBModelSound					= CGI_AS_GetBModelSound;
		cgi.AS_ParseSets						= CGI_AS_ParseSets;
		cgi.R_AddAdditiveLightToScene			= CGI_R_AddAdditiveLightToScene;
		cgi.R_AddDecalToScene					= CGI_R_AddDecalToScene;
		cgi.R_AddLightToScene					= CGI_R_AddLightToScene;
		cgi.R_AddPolysToScene					= CGI_R_AddPolysToScene;
		cgi.R_AddRefEntityToScene				= CGI_R_AddRefEntityToScene;
		cgi.R_AnyLanguage_ReadCharFromString	= CGI_R_AnyLanguage_ReadCharFromString;
		cgi.R_AutomapElevationAdjustment		= CGI_R_AutomapElevationAdjustment;
		cgi.R_ClearDecals						= CGI_R_ClearDecals;
		cgi.R_ClearScene						= CGI_R_ClearScene;
		cgi.R_DrawStretchPic					= CGI_R_DrawStretchPic;
		cgi.R_DrawRotatePic						= CGI_R_DrawRotatePic;
		cgi.R_DrawRotatePic2					= CGI_R_DrawRotatePic2;
		cgi.R_Font_DrawString					= CGI_R_Font_DrawString;
		cgi.R_Font_HeightPixels					= CGI_R_Font_HeightPixels;
		cgi.R_Font_StrLenChars					= CGI_R_Font_StrLenChars;
		cgi.R_Font_StrLenPixels					= CGI_R_Font_StrLenPixels;
		cgi.R_GetBModelVerts					= CGI_R_GetBModelVerts;
		cgi.R_GetDistanceCull					= CGI_R_GetDistanceCull;
		cgi.R_GetEntityToken					= CGI_R_GetEntityToken;
		cgi.R_GetLightStyle						= CGI_R_GetLightStyle;
		cgi.R_GetRealRes						= CGI_R_GetRealRes;
		cgi.R_InitializeWireframeAutomap		= CGI_R_InitializeWireframeAutomap;
		cgi.R_InPVS								= CGI_R_InPVS;
		cgi.R_Language_IsAsian					= CGI_R_Language_IsAsian;
		cgi.R_Language_UsesSpaces				= CGI_R_Language_UsesSpaces;
		cgi.R_LerpTag							= CGI_R_LerpTag;
		cgi.R_LightForPoint						= CGI_R_LightForPoint;
		cgi.R_LoadWorld							= CGI_R_LoadWorld;
		cgi.R_MarkFragments						= CGI_R_MarkFragments;
		cgi.R_ModelBounds						= CGI_R_ModelBounds;
		cgi.R_RegisterFont						= CGI_R_RegisterFont;
		cgi.R_RegisterModel						= CGI_R_RegisterModel;
		cgi.R_RegisterShader					= CGI_R_RegisterShader;
		cgi.R_RegisterShaderNoMip				= CGI_R_RegisterShaderNoMip;
		cgi.R_RegisterSkin						= CGI_R_RegisterSkin;
		cgi.R_RemapShader						= CGI_R_RemapShader;
		cgi.R_RenderScene						= CGI_R_RenderScene;
		cgi.R_SetColor							= CGI_R_SetColor;
		cgi.R_SetLightStyle						= CGI_R_SetLightStyle;
		cgi.R_SetRangedFog						= CGI_R_SetRangedFog;
		cgi.R_SetRefractionProperties			= CGI_R_SetRefractionProperties;
		cgi.R_WorldEffectCommand				= CGI_R_WorldEffectCommand;
		cgi.RE_InitRendererTerrain				= CGI_RE_InitRendererTerrain;
		cgi.WE_AddWeatherZone					= CGI_WE_AddWeatherZone;
		cgi.GetCurrentSnapshotNumber			= CGI_GetCurrentSnapshotNumber;
		cgi.GetCurrentCmdNumber					= CGI_GetCurrentCmdNumber;
		cgi.GetDefaultState						= CGI_GetDefaultState;
		cgi.GetGameState						= CGI_GetGameState;
		cgi.GetGlconfig							= CGI_GetGlconfig;
		cgi.GetServerCommand					= CGI_GetServerCommand;
		cgi.GetSnapshot							= CGI_GetSnapshot;
		cgi.GetUserCmd							= CGI_GetUserCmd;
		cgi.OpenUIMenu							= CGI_OpenUIMenu;
		cgi.SetClientForceAngle					= CGI_SetClientForceAngle;
		cgi.SetUserCmdValue						= CGI_SetUserCmdValue;
		cgi.Key_GetCatcher						= CGI_Key_GetCatcher;
		cgi.Key_GetKey							= CGI_Key_GetKey;
		cgi.Key_IsDown							= CGI_Key_IsDown;
		cgi.Key_SetCatcher						= CGI_Key_SetCatcher;
		cgi.PC_AddGlobalDefine					= CGI_PC_AddGlobalDefine;
		cgi.PC_FreeSource						= CGI_PC_FreeSource;
		cgi.PC_LoadGlobalDefines				= CGI_PC_LoadGlobalDefines;
		cgi.PC_LoadSource						= CGI_PC_LoadSource;
		cgi.PC_ReadToken						= CGI_PC_ReadToken;
		cgi.PC_RemoveAllGlobalDefines			= CGI_PC_RemoveAllGlobalDefines;
		cgi.PC_SourceFileAndLine				= CGI_PC_SourceFileAndLine;
		cgi.CIN_DrawCinematic					= CGI_CIN_DrawCinematic;
		cgi.CIN_PlayCinematic					= CGI_CIN_PlayCinematic;
		cgi.CIN_RunCinematic					= CGI_CIN_RunCinematic;
		cgi.CIN_SetExtents						= CGI_CIN_SetExtents;
		cgi.CIN_StopCinematic					= CGI_CIN_StopCinematic;
		cgi.FX_AddLine							= CGI_FX_AddLine;
		cgi.FX_RegisterEffect					= CGI_FX_RegisterEffect;
		cgi.FX_PlayEffect						= CGI_FX_PlayEffect;
		cgi.FX_PlayEffectID						= CGI_FX_PlayEffectID;
		cgi.FX_PlayEntityEffectID				= CGI_FX_PlayEntityEffectID;
		cgi.FX_PlayBoltedEffectID				= CGI_FX_PlayBoltedEffectID;
		cgi.FX_AddScheduledEffects				= CGI_FX_AddScheduledEffects;
		cgi.FX_InitSystem						= CGI_FX_InitSystem;
		cgi.FX_SetRefDef						= CGI_FX_SetRefDef;
		cgi.FX_FreeSystem						= CGI_FX_FreeSystem;
		cgi.FX_AdjustTime						= CGI_FX_AdjustTime;
		cgi.FX_Draw2DEffects					= CGI_FX_Draw2DEffects;
		cgi.FX_AddPoly							= CGI_FX_AddPoly;
		cgi.FX_AddBezier						= CGI_FX_AddBezier;
		cgi.FX_AddPrimitive						= CGI_FX_AddPrimitive;
		cgi.FX_AddSprite						= CGI_FX_AddSprite;
		cgi.FX_AddElectricity					= CGI_FX_AddElectricity;
		cgi.SE_GetStringTextString				= CGI_SE_GetStringTextString;
		cgi.ROFF_Clean							= CGI_ROFF_Clean;
		cgi.ROFF_UpdateEntities					= CGI_ROFF_UpdateEntities;
		cgi.ROFF_Cache							= CGI_ROFF_Cache;
		cgi.ROFF_Play							= CGI_ROFF_Play;
		cgi.ROFF_Purge_Ent						= CGI_ROFF_Purge_Ent;
		cgi.G2_ListModelSurfaces				= CL_G2API_ListModelSurfaces;
		cgi.G2_ListModelBones					= CL_G2API_ListModelBones;
		cgi.G2_SetGhoul2ModelIndexes			= CL_G2API_SetGhoul2ModelIndexes;
		cgi.G2_HaveWeGhoul2Models				= CL_G2API_HaveWeGhoul2Models;
		cgi.G2API_GetBoltMatrix					= CL_G2API_GetBoltMatrix;
		cgi.G2API_GetBoltMatrix_NoReconstruct	= CL_G2API_GetBoltMatrix_NoReconstruct;
		cgi.G2API_GetBoltMatrix_NoRecNoRot		= CL_G2API_GetBoltMatrix_NoRecNoRot;
		cgi.G2API_InitGhoul2Model				= CL_G2API_InitGhoul2Model;
		cgi.G2API_SetSkin						= CL_G2API_SetSkin;
		cgi.G2API_CollisionDetect				= CL_G2API_CollisionDetect;
		cgi.G2API_CollisionDetectCache			= CL_G2API_CollisionDetectCache;
		cgi.G2API_CleanGhoul2Models				= CL_G2API_CleanGhoul2Models;
		cgi.G2API_SetBoneAngles					= CL_G2API_SetBoneAngles;
		cgi.G2API_SetBoneAnim					= CL_G2API_SetBoneAnim;
		cgi.G2API_GetBoneAnim					= CL_G2API_GetBoneAnim;
		cgi.G2API_GetBoneFrame					= CL_G2API_GetBoneFrame;
		cgi.G2API_GetGLAName					= CL_G2API_GetGLAName;
		cgi.G2API_CopyGhoul2Instance			= CL_G2API_CopyGhoul2Instance;
		cgi.G2API_CopySpecificGhoul2Model		= CL_G2API_CopySpecificGhoul2Model;
		cgi.G2API_DuplicateGhoul2Instance		= CL_G2API_DuplicateGhoul2Instance;
		cgi.G2API_HasGhoul2ModelOnIndex			= CL_G2API_HasGhoul2ModelOnIndex;
		cgi.G2API_RemoveGhoul2Model				= CL_G2API_RemoveGhoul2Model;
		cgi.G2API_SkinlessModel					= CL_G2API_SkinlessModel;
		cgi.G2API_GetNumGoreMarks				= CL_G2API_GetNumGoreMarks;
		cgi.G2API_AddSkinGore					= CL_G2API_AddSkinGore;
		cgi.G2API_ClearSkinGore					= CL_G2API_ClearSkinGore;
		cgi.G2API_Ghoul2Size					= CL_G2API_Ghoul2Size;
		cgi.G2API_AddBolt						= CL_G2API_AddBolt;
		cgi.G2API_AttachEnt						= CL_G2API_AttachEnt;
		cgi.G2API_SetBoltInfo					= CL_G2API_SetBoltInfo;
		cgi.G2API_SetRootSurface				= CL_G2API_SetRootSurface;
		cgi.G2API_SetSurfaceOnOff				= CL_G2API_SetSurfaceOnOff;
		cgi.G2API_SetNewOrigin					= CL_G2API_SetNewOrigin;
		cgi.G2API_DoesBoneExist					= CL_G2API_DoesBoneExist;
		cgi.G2API_GetSurfaceRenderStatus		= CL_G2API_GetSurfaceRenderStatus;
		cgi.G2API_GetTime						= CL_G2API_GetTime;
		cgi.G2API_SetTime						= CL_G2API_SetTime;
		cgi.G2API_AbsurdSmoothing				= CL_G2API_AbsurdSmoothing;
		cgi.G2API_SetRagDoll					= CL_G2API_SetRagDoll;
		cgi.G2API_AnimateG2Models				= CL_G2API_AnimateG2Models;
		cgi.G2API_RagPCJConstraint				= CL_G2API_RagPCJConstraint;
		cgi.G2API_RagPCJGradientSpeed			= CL_G2API_RagPCJGradientSpeed;
		cgi.G2API_RagEffectorGoal				= CL_G2API_RagEffectorGoal;
		cgi.G2API_GetRagBonePos					= CL_G2API_GetRagBonePos;
		cgi.G2API_RagEffectorKick				= CL_G2API_RagEffectorKick;
		cgi.G2API_RagForceSolve					= CL_G2API_RagForceSolve;
		cgi.G2API_SetBoneIKState				= CL_G2API_SetBoneIKState;
		cgi.G2API_IKMove						= CL_G2API_IKMove;
		cgi.G2API_RemoveBone					= CL_G2API_RemoveBone;
		cgi.G2API_AttachInstanceToEntNum		= CL_G2API_AttachInstanceToEntNum;
		cgi.G2API_ClearAttachedInstance			= CL_G2API_ClearAttachedInstance;
		cgi.G2API_CleanEntAttachments			= CL_G2API_CleanEntAttachments;
		cgi.G2API_OverrideServer				= CL_G2API_OverrideServer;
		cgi.G2API_GetSurfaceName				= CL_G2API_GetSurfaceName;

		cgi.ext.R_Font_StrLenPixels				= re->ext.Font_StrLenPixels;

		GetCGameAPI = (GetCGameAPI_t)cgvm->GetModuleAPI;
		ret = GetCGameAPI( CGAME_API_VERSION, &cgi );
		if ( !ret ) {
			//free VM?
			cls.cgameStarted = qfalse;
			Com_Error( ERR_FATAL, "GetGameAPI failed on %s", dllName );
		}
		cge = ret;

		return;
	}

	// fall back to legacy syscall/vm_call api
	cgvm = VM_CreateLegacy( VM_CGAME, F_CG_SYS_CALL );
	if ( !cgvm ) {
		cls.cgameStarted = qfalse;
		Com_Error( ERR_DROP, "VM_CreateLegacy on cgame failed" );
	}
}

void CL_UnbindCGame( void ) {
	CGVM_Shutdown();
	VM_Free( cgvm );
	cgvm = NULL;
}
