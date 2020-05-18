	
/**************************************************
* MultiPlugin++ by Deathspike
*
* Client-side plugin project which allows any
* developer to use this framework as a basis for
* their own plugin.
**************************************************/

//#include <cstddef>
#include <multiplugin/mppHeader.h>
#include <qcommon/cm_public.h>
#include <qcommon/RoffSystem.h>
#include <qcommon/stringed_ingame.h>
#include <qcommon/timing.h>
#include <client/cl_uiapi.h>
#include <botlib/botlib.h>
#include <client/snd_ambient.h>
#include <client/FXExport.h>
#include <client/FxUtil.h>



extern qboolean cl_bUseFighterPitch;
extern int CL_GetValueForHidden(const char *s);
extern void FX_FeedTrail(effectTrailArgStruct_t *a);
extern int s_entityWavVol[MAX_GENTITIES];

/**************************************************
* Imported functions from the software development kit.
**************************************************/

int PASSFLOAT( float x )
{
	float floatTemp;
	floatTemp = x;
	return *( int * ) &floatTemp;
}

/**************************************************
* Unavailable local api functions redefined here.
**************************************************/

static int CL_GetCurrentCmdNumber(void) {
	int ret = cl.cmdNumber;
	return ret;
}

static void CL_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime) {
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

static void CL_GetGameState(gameState_t *gs) {
	*gs = cl.gameState;
}

static void CL_GetGlconfig(glconfig_t *glconfig) {
	*glconfig = cls.glconfig;
}

static qboolean CL_SE_GetStringTextString(const char *text, char *buffer, int bufferLength) {
	const char *str;

	assert(text && buffer);

	str = SE_GetString(text);

	if (str[0]) {
		Q_strncpyz(buffer, str, bufferLength);
		return qtrue;
	}

	Com_sprintf(buffer, bufferLength, "??%s", str);
	return qfalse;
}

static int CL_Milliseconds() {
	return Sys_Milliseconds();
}
	
static void RegisterSharedMemory(char *memory) {
	cl.mSharedMemory = memory;
}

static void CGVM_Cmd_RemoveCommand(const char *cmd_name) {
	Cmd_VM_RemoveCommand(cmd_name, VM_CGAME);
}

static void CL_AddReliableCommand2(const char *cmd) {
	CL_AddReliableCommand(cmd, qfalse);
}

static void CL_SetClientForceAngle(int time, vec3_t angle) {
	cl.cgameViewAngleForceTime = time;
	VectorCopy(angle, cl.cgameViewAngleForce);
}

static void _CL_SetUserCmdValue(int stateValue, float sensitivityScale, float mPitchOverride, float mYawOverride, float mSensitivityOverride, int fpSel, int invenSel, qboolean fighterControls) {
	cl_bUseFighterPitch = fighterControls;
	CL_SetUserCmdValue(stateValue, sensitivityScale, mPitchOverride, mYawOverride, mSensitivityOverride, fpSel, invenSel);
}

void CL_Cvar_VM_Set(const char *var_name, const char *value, int vmslot) {
	Cvar_VM_Set(var_name, value, (vmSlots_t)vmslot);
}

static void CGFX_AddBezier(addbezierArgStruct_t *b) {
	FX_AddBezier(b->start, b->end, b->control1, b->control1Vel, b->control2, b->control2Vel, b->size1, b->size2, b->sizeParm, b->alpha1, b->alpha2, b->alphaParm, b->sRGB, b->eRGB, b->rgbParm, b->killTime, b->shader, b->flags);
}

static void CGFX_AddElectricity(addElectricityArgStruct_t *p) {
	FX_AddElectricity(p->start, p->end, p->size1, p->size2, p->sizeParm, p->alpha1, p->alpha2, p->alphaParm, p->sRGB, p->eRGB, p->rgbParm, p->chaos, p->killTime, p->shader, p->flags);
}

static void CGFX_AddLine(vec3_t start, vec3_t end, float size1, float size2, float sizeParm, float alpha1, float alpha2, float alphaParm, vec3_t sRGB, vec3_t eRGB, float rgbParm, int killTime, qhandle_t shader, int flags) {
	FX_AddLine(start, end, size1, size2, sizeParm, alpha1, alpha2, alphaParm, sRGB, eRGB, rgbParm, killTime, shader, flags);
}

static void CGFX_AddPoly(addpolyArgStruct_t *p) {
	FX_AddPoly(p->p, p->ev, p->numVerts, p->vel, p->accel, p->alpha1, p->alpha2, p->alphaParm, p->rgb1, p->rgb2, p->rgbParm, p->rotationDelta, p->bounce, p->motionDelay, p->killTime, p->shader, p->flags);
}

static void CGFX_AddSprite(addspriteArgStruct_t *s) {
	vec3_t rgb = { 1.0f, 1.0f, 1.0f };
	FX_AddParticle(s->origin, s->vel, s->accel, s->scale, s->dscale, 0, s->sAlpha, s->eAlpha, 0, rgb, rgb, 0, s->rotation, 0, vec3_origin, vec3_origin, s->bounce, 0, 0, s->life, s->shader, s->flags);
}

static qboolean CGFX_PlayBoltedEffectID(int id, vec3_t org, void *ghoul2, const int boltNum, const int entNum, const int modelNum, int iLooptime, qboolean isRelative) {
	if (!ghoul2) {
		return qfalse;
	}
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	int boltInfo = 0;
	if (re->G2API_AttachEnt(&boltInfo, g2, modelNum, boltNum, entNum, modelNum))
	{
		FX_PlayBoltedEffectID(id, org, boltInfo, &g2, iLooptime, isRelative);
		return qtrue;
	}
	return qfalse;
}

static void CL_Key_SetCatcher(int catcher) {
	// Don't allow the cgame module to close the console
	Key_SetCatcher(catcher | (Key_GetCatcher() & KEYCATCH_CONSOLE));
}

static void CL_PrecisionTimerStart(void **p) {
	timing_c *newTimer = new timing_c; //create the new timer
	*p = newTimer; //assign the pointer within the pointer to point at the mem addr of our new timer
	newTimer->Start(); //start the timer
}

static int CL_PrecisionTimerEnd(void *p) {
	int r = 0;
	timing_c *timer = (timing_c *)p; //this is the pointer we assigned in start, so we can directly cast it back
	r = timer->End(); //get the result
	delete timer; //delete the timer since we're done with it
	return r; //return the result
}

static void CL_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts) {
	re->AddPolyToScene(hShader, numVerts, verts, 1);
}
	
static qboolean CL_ROFF_Clean(void) {
	return theROFFSystem.Clean(qtrue);
}

static void CL_ROFF_UpdateEntities(void) {
	theROFFSystem.UpdateEntities(qtrue);
}

static int CL_ROFF_Cache(char *file) {
	return theROFFSystem.Cache(file, qtrue);
}

static qboolean CL_ROFF_Play(int entID, int roffID, qboolean doTranslation) {
	return theROFFSystem.Play(entID, roffID, doTranslation, qtrue);
}

static qboolean CL_ROFF_Purge_Ent(int entID) {
	return theROFFSystem.PurgeEnt(entID, qtrue);
}

static int CL_S_GetVoiceVolume(int entID) {
	return s_entityWavVol[entID];
}

static void CL_S_Shutup(qboolean shutup) {
	s_shutUp = shutup;
}

/**************************************************
* mppSystemInit
*
* Load all the call-able functions into the
* MultiSystem_t structure, send as a pointer. This
* way we can separate any plugin calls.
**************************************************/

void mppSystemInit(MultiSystem_t *trap)
{
	trap->AddCommand 					= Cmd_AddCommand;
	trap->AddWeatherZone 				= re->AddWeatherZone;
	trap->Argc 							= Cmd_Argc;
	trap->Args 							= Cmd_ArgsBuffer;
	trap->Argv 							= Cmd_ArgvBuffer;
	trap->GetCurrentCmdNumber 			= CL_GetCurrentCmdNumber;
	trap->GetCurrentSnapshotNumber		= CL_GetCurrentSnapshotNumber;
	trap->GetDefaultState 				= CL_GetDefaultState;
	trap->GetEntityToken 				= re->GetEntityToken;
	trap->GetGameState 					= CL_GetGameState;
	trap->GetGlconfig 					= CL_GetGlconfig;
	trap->GetServerCommand 				= CL_GetServerCommand;
	trap->GetSnapshot 					= CL_GetSnapshot;
	trap->GetStringTextString 			= CL_SE_GetStringTextString;
	trap->GetUserCmd 					= CL_GetUserCmd;
	trap->MemoryRemaining 				= Hunk_MemoryRemaining;
	trap->Milliseconds 					= CL_Milliseconds;
	trap->OpenUIMenu 					= (void(*)(int))UIVM_SetActiveMenu;
	trap->RealTime 						= Com_RealTime;
	trap->RegisterSharedMemory 			= RegisterSharedMemory;
	trap->RemoveCommand 				= CGVM_Cmd_RemoveCommand;
	trap->SendConsoleCommand 			= Cbuf_AddText;
	trap->SendClientCommand 			= CL_AddReliableCommand2;
	trap->SetClientForceAngle 			= CL_SetClientForceAngle;
	trap->SetUserCmdValue 				= _CL_SetUserCmdValue;
	trap->SnapVector					= Sys_SnapVector;
	trap->TrueMalloc 					= VM_Shifted_Alloc;
	trap->TrueFree 						= VM_Shifted_Free;
	trap->UpdateScreen 					= SCR_UpdateScreen;

	trap->AS.AddPrecacheEntry 			= AS_AddPrecacheEntry;
	trap->AS.GetBModelSound 			= AS_GetBModelSound;
	//trap->AS.ParseSets 				= AS_ParseSets;

	trap->CIN.DrawCinematic 			= CIN_DrawCinematic;
	trap->CIN.PlayCinematic 			= CIN_PlayCinematic;
	trap->CIN.RunCinematic 				= CIN_RunCinematic;
	trap->CIN.SetExtents 				= CIN_SetExtents;
	trap->CIN.StopCinematic 			= CIN_StopCinematic;

	trap->CM.Trace 						= CM_BoxTrace;
	trap->CM.InlineModel 				= CM_InlineModel;
	trap->CM.LoadMap 					= CM_LoadMap;
	trap->CM.NumInlineModels	 		= CM_NumInlineModels;
	trap->CM.MarkFragments 				= re->MarkFragments;
	trap->CM.PointContents 				= CM_PointContents;
	trap->CM.TempModel 					= CM_TempBoxModel;
	trap->CM.TransformedTrace 			= CM_TransformedBoxTrace;
	trap->CM.TransformedPointContents 	= CM_TransformedPointContents;

	trap->Cvar.GetHiddenVarValue 		= CL_GetValueForHidden;
	trap->Cvar.Register 				= Cvar_Register;
	trap->Cvar.Set 						= CL_Cvar_VM_Set;
	trap->Cvar.Update 					= Cvar_Update;
	trap->Cvar.VariableStringBuffer 	= Cvar_VariableStringBuffer;

	trap->FS.FCloseFile 				= FS_FCloseFile;
	trap->FS.FOpenFile 					= FS_FOpenFileByMode;
	trap->FS.GetFileList 				= FS_GetFileList;
	trap->FS.Read 						= FS_Read;
	trap->FS.Write 						= FS_Write;

	trap->FX.AddBezier 					= CGFX_AddBezier;
	trap->FX.AddElectricity				= CGFX_AddElectricity;
	trap->FX.AddLine 					= CGFX_AddLine;
	trap->FX.AddPoly 					= CGFX_AddPoly;
	trap->FX.AddPrimitive 				= FX_FeedTrail;
	trap->FX.AddScheduledEffects 		= FX_AddScheduledEffects;
	trap->FX.AddSprite 					= CGFX_AddSprite;
	trap->FX.AdjustTime 				= FX_AdjustTime;
	trap->FX.Draw2DEffects 				= FX_Draw2DEffects;
	trap->FX.FreeSystem 				= FX_FreeSystem;
	trap->FX.InitSystem 				= FX_InitSystem;
	trap->FX.RegisterEffect 			= FX_RegisterEffect;
	trap->FX.PlayBoltedEffectID 		= CGFX_PlayBoltedEffectID;
	trap->FX.PlayEffect 				= FX_PlayEffect;
	trap->FX.PlayEntityEffectID 		= FX_PlayEntityEffectID;
	trap->FX.PlayEffectID 				= FX_PlayEffectID;
	trap->FX.SetRefDef 					= FX_SetRefDef;

	/*trap->G2.HaveWeGhoul2Models 		= trap_G2_HaveWeGhoul2Models;
	trap->G2.ListModelSurfaces 			= trap_G2_ListModelSurfaces;
	trap->G2.ListModelBones 			= trap_G2_ListModelBones;
	trap->G2.SetGhoul2ModelIndexes		= trap_G2_SetGhoul2ModelIndexes;

	trap->G2API.AbsurdSmoothing 		= trap_G2API_AbsurdSmoothing;
	trap->G2API.AddBolt 				= trap_G2API_AddBolt;
	trap->G2API.AddSkinGore 			= trap_G2API_AddSkinGore;
	trap->G2API.AnimateG2Models 		= trap_G2API_AnimateG2Models;
	trap->G2API.AttachEnt 				= trap_G2API_AttachEnt;
	trap->G2API.AttachInstanceToEntNum 	= trap_G2API_AttachInstanceToEntNum;
	trap->G2API.ClearAttachedInstance 	= trap_G2API_ClearAttachedInstance;
	trap->G2API.CleanEntAttachments 	= trap_G2API_CleanEntAttachments;
	trap->G2API.CleanGhoul2Models 		= trap_G2API_CleanGhoul2Models;
	trap->G2API.ClearSkinGore 			= trap_G2API_ClearSkinGore;
	trap->G2API.CollisionDetect 		= trap_G2API_CollisionDetect;
	trap->G2API.CollisionDetectCache 	= trap_G2API_CollisionDetectCache;
	trap->G2API.CopyGhoul2Instance 		= trap_G2API_CopyGhoul2Instance;
	trap->G2API.CopySpecificGhoul2Model = trap_G2API_CopySpecificGhoul2Model;
	trap->G2API.DoesBoneExist			= trap_G2API_DoesBoneExist;
	trap->G2API.DuplicateGhoul2Instance = trap_G2API_DuplicateGhoul2Instance;
	trap->G2API.GetBoltMatrix 			= trap_G2API_GetBoltMatrix;
	trap->G2API.GetBoltMatrix_NoReconstruct = trap_G2API_GetBoltMatrix_NoReconstruct;
	trap->G2API.GetBoltMatrix_NoRecNoRot = trap_G2API_GetBoltMatrix_NoRecNoRot;
	trap->G2API.GetBoneAnim 			= trap_G2API_GetBoneAnim;
	trap->G2API.GetBoneFrame 			= trap_G2API_GetBoneFrame;
	trap->G2API.GetGLAName 				= trap_G2API_GetGLAName;
	trap->G2API.GetRagBonePos 			= trap_G2API_GetRagBonePos;
	trap->G2API.GetNumGoreMarks		 	= trap_G2API_GetNumGoreMarks;
	trap->G2API.GetTime 				= trap_G2API_GetTime;
	trap->G2API.GetSurfaceName 			= trap_G2API_GetSurfaceName;
	trap->G2API.GetSurfaceRenderStatus 	= trap_G2API_GetSurfaceRenderStatus;
	trap->G2API.Ghoul2Size 				= trap_G2API_Ghoul2Size;
	trap->G2API.HasGhoul2ModelOnIndex 	= trap_G2API_HasGhoul2ModelOnIndex;
	trap->G2API.IKMove 					= trap_G2API_IKMove;
	trap->G2API.InitGhoul2Model 		= trap_G2API_InitGhoul2Model;
	trap->G2API.OverrideServer 			= trap_G2API_OverrideServer;
	trap->G2API.RagEffectorGoal 		= trap_G2API_RagEffectorGoal;
	trap->G2API.RagEffectorKick 		= trap_G2API_RagEffectorKick;
	trap->G2API.RagForceSolve 			= trap_G2API_RagForceSolve;
	trap->G2API.RagPCJConstraint 		= trap_G2API_RagPCJConstraint;
	trap->G2API.RagPCJGradientSpeed 	= trap_G2API_RagPCJGradientSpeed;
	trap->G2API.RemoveBone 				= trap_G2API_RemoveBone;
	trap->G2API.RemoveGhoul2Model 		= trap_G2API_RemoveGhoul2Model;
	trap->G2API.SetBoltInfo 			= trap_G2API_SetBoltInfo;
	trap->G2API.SetBoneAnim 			= trap_G2API_SetBoneAnim;
	trap->G2API.SetBoneAngles 			= trap_G2API_SetBoneAngles;
	trap->G2API.SetBoneIKState 			= trap_G2API_SetBoneIKState;
	trap->G2API.SetRootSurface 			= trap_G2API_SetRootSurface;
	trap->G2API.SetNewOrigin 			= trap_G2API_SetNewOrigin;
	trap->G2API.SetRagDoll 				= trap_G2API_SetRagDoll;
	trap->G2API.SetSkin 				= trap_G2API_SetSkin;
	trap->G2API.SetSurfaceOnOff 		= trap_G2API_SetSurfaceOnOff;
	trap->G2API.SetTime 				= trap_G2API_SetTime;
	trap->G2API.SkinlessModel 			= trap_G2API_SkinlessModel;*/

	trap->Key.GetCatcher 				= Key_GetCatcher;
	trap->Key.GetKey 					= Key_GetKey;
	trap->Key.IsDown 					= Key_IsDown;
	trap->Key.SetCatcher 				= CL_Key_SetCatcher;

	trap->Language.IsAsian 				= re->Language_IsAsian;
	trap->Language.ReadCharFromString 	= re->AnyLanguage_ReadCharFromString;
	trap->Language.UsesSpaces 			= re->Language_UsesSpaces;

	/*trap->PC.AddGlobalDefine 			= trap_PC_AddGlobalDefine;
	trap->PC.FreeSource 				= trap_PC_FreeSource;
	trap->PC.LoadGlobalDefines 			= trap_PC_LoadGlobalDefines;
	trap->PC.LoadSource 				= trap_PC_LoadSource;
	trap->PC.ReadToken 					= trap_PC_ReadToken;
	trap->PC.RemoveAllGlobalDefines 	= trap_PC_RemoveAllGlobalDefines;
	trap->PC.SourceFileAndLine 			= trap_PC_SourceFileAndLine;*/

	trap->PrecisionTimer.End 			= CL_PrecisionTimerEnd;
	trap->PrecisionTimer.Start 			= CL_PrecisionTimerStart;

	trap->R.AutomapElevAdj 				= re->AutomapElevationAdjustment;
	trap->R.AddAdditiveLightToScene 	= re->AddAdditiveLightToScene;
	trap->R.AddDecalToScene 			= re->AddDecalToScene;
	trap->R.AddLightToScene 			= re->AddLightToScene;
	trap->R.AddPolyToScene 				= CL_R_AddPolyToScene;
	trap->R.AddPolysToScene 			= re->AddPolyToScene;
	trap->R.AddRefEntityToScene 		= re->AddRefEntityToScene;
	trap->R.ClearDecals 				= re->ClearDecals;
	trap->R.ClearScene 					= re->ClearScene;
	trap->R.DrawRotatePic 				= re->DrawRotatePic;
	trap->R.DrawRotatePic2 				= re->DrawRotatePic2;
	trap->R.DrawStretchPic 				= re->DrawStretchPic;
	trap->R.Font.DrawString 			= re->Font_DrawString;
	trap->R.Font.HeightPixels 			= re->Font_HeightPixels;
	trap->R.Font.StrLenChars 			= re->Font_StrLenChars;
	trap->R.Font.StrLenPixels 			= re->Font_StrLenPixels;
	trap->R.GetBModelVerts 				= re->GetBModelVerts;
	trap->R.GetDistanceCull 			= re->GetDistanceCull;
	trap->R.GetLightStyle 				= re->GetLightStyle;
	trap->R.GetRealRes 					= re->GetRealRes;
	trap->R.InPVS 						= re->inPVS;
	trap->R.LerpTag 					= re->LerpTag;
	trap->R.LightForPoint 				= re->LightForPoint;
	trap->R.ModelBounds 				= re->ModelBounds;
	trap->R.RenderScene 				= re->RenderScene;
	trap->R.RemapShader 				= re->RemapShader;
	trap->R.RegisterFont 				= re->RegisterFont;
	trap->R.RegisterModel 				= re->RegisterModel;
	trap->R.RegisterShader 				= re->RegisterShader;
	trap->R.RegisterShaderNoMip 		= re->RegisterShaderNoMip;
	trap->R.RegisterSkin 				= re->RegisterSkin;
	trap->R.SetColor 					= re->SetColor;
	trap->R.SetLightStyle 				= re->SetLightStyle;
	trap->R.WorldEffectCommand 			= re->WorldEffectCommand;

	trap->ROFF.Cache 					= CL_ROFF_Cache;
	trap->ROFF.Clean 					= CL_ROFF_Clean;
	trap->ROFF.Play 					= CL_ROFF_Play;
	trap->ROFF.PurgeEnt 				= CL_ROFF_Purge_Ent;
	trap->ROFF.UpdateEntities 			= CL_ROFF_UpdateEntities;

	trap->S.AddLocalSet 				= S_AddLocalSet;
	trap->S.AddLoopingSound 			= S_AddLoopingSound;
	trap->S.ClearLoopingSounds 			= S_ClearLoopingSounds;
	trap->S.GetVoiceVolume 				= CL_S_GetVoiceVolume;
	trap->S.MuteSound 					= S_MuteSound;
	trap->S.RegisterSound 				= S_RegisterSound;
	trap->S.Respatialize 				= S_Respatialize;
	trap->S.ShutUp 						= CL_S_Shutup;
	trap->S.StartBackgroundTrack 		= S_StartBackgroundTrack;
	trap->S.StartLocalSound 			= S_StartLocalSound;
	trap->S.StartSound 					= S_StartSound;
	trap->S.StopBackgroundTrack 		= S_StopBackgroundTrack;
	trap->S.StopLoopingSound 			= S_StopLoopingSound;
	trap->S.UpdateAmbientSet 			= S_UpdateAmbientSet;
	trap->S.UpdateEntityPosition 		= S_UpdateEntityPosition;
}