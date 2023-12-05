#include "gui.h"
#include <sys/types.h>
#include "psxcommon.h"
#include "cdriso.h"
#include "cdrom.h"
#include "r3000a.h"
#include "gpu.h"
#include "sys\Mount.h"
#include "simpleini\SimpleIni.h"

std::string profile_filename;

void RenderXui();

DWORD * InGameThread() 
{
	while(1) 
	  {
	XINPUT_STATE InputState;
	DWORD XInputErr=XInputGetState( 0, &InputState );
	if(	XInputErr != ERROR_SUCCESS)
	continue;

	// Check if some button are pressed
	if(InputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK && InputState.Gamepad.wButtons & XINPUT_GAMEPAD_START) 
	{
	OutputDebugStringA("Pause !!");
	PausePcsx();
	xboxConfig.ShutdownRequested = true;
	}
    Sleep(50);
	  }
	return NULL;
}

#define MAX_FILENAME 256

HRESULT ShowKeyboard(std::wstring & resultText, std::wstring titleText, std::wstring descriptionText, std::wstring defaultText) {
  wchar_t result[MAX_FILENAME];
	
	XOVERLAPPED Overlapped;
	ZeroMemory( &Overlapped, sizeof( XOVERLAPPED ) );

	DWORD dwResult = XShowKeyboardUI(
	XUSER_INDEX_ANY,
	0,
	defaultText.c_str(),
	titleText.c_str(),
	descriptionText.c_str(),
	result,
	MAX_FILENAME,
	&Overlapped
	);

	while(!XHasOverlappedIoCompleted(&Overlapped)) {
		RenderXui();
	}

	resultText = result;

	return (dwResult == ERROR_SUCCESS)? S_OK : E_FAIL;
}

void SaveStatePcsx(int n) {
#if 1
	std::string game = xboxConfig.game;
	std::wstring wgame;
	std::wstring result;

	// Get basename of current game
	game.erase(0, game.rfind('\\')+1);

	get_wstring(game, wgame);

	//if (SUCCEEDED(ShowKeyboard(result, L"", L"", wgame.c_str()))) {
	ShowKeyboard(result, L"Enter the filename of the save states", L"If a file with the same name exists it will overwriten", wgame.c_str());
	{
    std::string save_path;
	std::string save_filename;
	save_path = xboxConfig.saveStateDir + "\\" + game;

	// Create save dir
	CreateDirectoryA(save_path.c_str(), NULL);

	// Save state
	get_string(result, save_filename);

	save_path += "\\" + save_filename;

	SaveState(save_path.c_str());
	}
#else
	std::string game = xboxConfig.game + ".sgz";
	SaveState(game.c_str());
#endif
}

void LoadStatePcsx(std::string save) {
	LoadState(save.c_str());

	// Resume emulation
	ResumePcsx();
}

void LoadStatePcsx(int n) {
	std::string game = xboxConfig.game + ".sgz";
	LoadState(game.c_str());

	// Resume emulation
	ResumePcsx();
}

void ChangeDisc(std::string game){

	CdromId[0] = '\0';
	CdromLabel[0] = '\0';

	SetIsoFile(game.c_str());

	if (ReloadCdromPlugin() < 0) {
      printf("failed to load cdr plugin");
	return;
	      }
    if (CDR_open() < 0) {
      printf("failed to open cdr plugin");
    return;
      }

    SetCdOpenCaseTime(time(NULL) + 2);
	LidInterrupt();

	// Resume emulation
	ResumePcsx();

}

void RenderXui() {

	g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
	0xff000000, 1.0f, 0L );

	if (xboxConfig.Running == true) {

	DrawPcsxSurface();
    }
    app.RunFrame();
	app.Render();
	XuiTimersRun();

	// Present the backbuffer contents to the display.
	VideoPresent();
}

CSimpleIniA ini;

bool slow_boot;

void ApplySettings(const char* path) {

	Config.Cpu        = xboxConfig.UseDynarec ? CPU_DYNAREC : CPU_INTERPRETER;
    UseSpu_Irq        = xboxConfig.UseSpuIrq;
	UseFrameLimit     = xboxConfig.UseFrameLimiter;
	Config.RCntFix    = xboxConfig.UseParasiteEveFix;
	Use_DarkForcesFix = xboxConfig.DarkForcesFix;
	slow_boot         = xboxConfig.slowboot;
	use_vm            = !xboxConfig.usevm;

 	ini.SetLongValue("pcsx", "UseDynarec",        xboxConfig.UseDynarec);
	ini.SetLongValue("pcsx", "UseThreadedGpu",    xboxConfig.UseThreadedGpu);
	ini.SetLongValue("pcsx", "UseSpuIrq",         xboxConfig.UseSpuIrq);
	ini.SetLongValue("pcsx", "UseFrameLimiter",   xboxConfig.UseFrameLimiter);
	ini.SetLongValue("pcsx", "UseParasiteEveFix", xboxConfig.UseParasiteEveFix);
	ini.SetLongValue("pcsx", "DarkForcesFix",     xboxConfig.DarkForcesFix);
	ini.SetLongValue("pcsx", "slowboot",          xboxConfig.slowboot);
	ini.SetLongValue("pcsx", "usevm",             xboxConfig.usevm);
//  ini.SetLongValue("pcsx", "region",            xboxConfig.region);
	

	ini.SetValue("pcsx", "saveStateDir", xboxConfig.saveStateDir.c_str());
	ini.SetValue("pcsx", "Bios", Config.Bios);	
	ini.SetValue("pcsx", "BiosDir", Config.BiosDir);
	ini.SetValue("pcsx", "Mcd1", Config.Mcd1);
	ini.SetValue("pcsx", "Mcd2", Config.Mcd2);
	ini.SetValue("pcsx", "IsoFolder",xboxConfig.IsoFolder);
	ini.SetValue("pcsx", "Game2Boot",xboxConfig.Game2Boot);
	
	//debug propurses
	ini.SetValue("debug","IDCover",xboxConfig.GameIDCover);
	ini.SetValue("debug","GameID",xboxConfig.CurrentgameID);
	ini.SetValue("debug","GameID",xboxConfig.CurrentgameIDFullPath);
	 
	
	std::wstring wgame;
    wgame = GamePath;
    get_string(wgame, profile_filename);
    //Get basename of current game
	profile_filename.erase(0, profile_filename.rfind('\\')+1);
    profile_filename = "game:\\gameprofile\\"  + profile_filename;
    profile_filename = profile_filename + ".ini";

	ini.SaveFile(profile_filename.c_str());

}

void LoadSettings(const char* path) {

	// defalut
	strcpy(Config.Bios, "SCPH1001.BIN");
	strcpy(Config.BiosDir, "game:\\BIOS");
	strcpy(Config.Mcd1,"game:\\memcards\\Memcard1.mcd");
	strcpy(Config.Mcd2,"game:\\memcards\\Memcard2.mcd");

	xboxConfig.UseDynarec        = 0;
	xboxConfig.UseThreadedGpu    = 0;
	xboxConfig.UseSpuIrq         = 0;
	xboxConfig.UseFrameLimiter   = 0;
	xboxConfig.UseParasiteEveFix = 0;
	xboxConfig.DarkForcesFix     = 0;
	xboxConfig.slowboot          = 0;
	xboxConfig.usevm             = 0;
//	xboxConfig.region            = 0;// Auto
	xboxConfig.saveStateDir = "game:\\states";
                                     
	std::wstring wgame;
    wgame = GamePath;
	get_string(wgame, profile_filename);
    //Get basename of current game
	profile_filename.erase(0, profile_filename.rfind('\\')+1);
    profile_filename = "game:\\gameprofile\\" + profile_filename;
    profile_filename = profile_filename + ".ini";
    //Merge cfg file
	ini.LoadFile(profile_filename.c_str());

	xboxConfig.UseDynarec        = ini.GetLongValue("pcsx", "UseDynarec",        0);
	xboxConfig.UseThreadedGpu    = ini.GetLongValue("pcsx", "UseThreadedGpu",    0);
	xboxConfig.UseSpuIrq         = ini.GetLongValue("pcsx", "UseSpuIrq",         0);
	xboxConfig.UseFrameLimiter   = ini.GetLongValue("pcsx", "UseFrameLimiter",   0);
	xboxConfig.UseParasiteEveFix = ini.GetLongValue("pcsx", "UseParasiteEveFix", 0);
	xboxConfig.DarkForcesFix     = ini.GetLongValue("pcsx", "DarkForcesFix",     0);
	xboxConfig.slowboot          = ini.GetLongValue("pcsx", "slowboot",          0);
	xboxConfig.usevm             = ini.GetLongValue("pcsx", "usevm",             0);
//	xboxConfig.region            = ini.GetLongValue("pcsx", "Region",            0);
	xboxConfig.saveStateDir      = strdup(ini.GetValue("pcsx", "saveStateDir", "game:\\states"));
	
	strcpy(Config.Bios,          strdup(ini.GetValue("pcsx", "Bios", "SCPH1001.BIN")));
	strcpy(Config.BiosDir,       strdup(ini.GetValue("pcsx", "BiosDir", "game:\\BIOS")));
	strcpy(Config.Mcd1,          strdup(ini.GetValue("pcsx", "Mcd1", "game:\\memcards\\Memcard1.mcd")));
	strcpy(Config.Mcd2,          strdup(ini.GetValue("pcsx", "Mcd2", "game:\\memcards\\Memcard2.mcd")));
	strcpy(xboxConfig.IsoFolder, strdup(ini.GetValue("pcsx", "IsoFolder", "game:")));
	strcpy(xboxConfig.Game2Boot, strdup(ini.GetValue("pcsx", "Game2Boot", "game:")));

}

void LoadSettings_default(const char* path){

/*
	xboxConfig.UseDynarec        = 1;
	xboxConfig.UseThreadedGpu    = 1;
	xboxConfig.UseSpuIrq         = 0;
	xboxConfig.UseFrameLimiter   = 1;
	xboxConfig.UseParasiteEveFix = 0;
	xboxConfig.DarkForcesFix     = 0;
	xboxConfig.slowboot          = 0;
//	xboxConfig.region            = 0;// Auto
*/
	xboxConfig.UseFrameLimiter   = 1;
    //Merge cfg file
	ini.LoadFile(path);
/*
	xboxConfig.UseDynarec        = ini.GetLongValue("pcsx",  "UseDynarec",        1);
	xboxConfig.UseThreadedGpu    = ini.GetLongValue("pcsx",  "UseThreadedGpu",    1);
	xboxConfig.UseSpuIrq         = ini.GetLongValue("pcsx",  "UseSpuIrq",         0);
	xboxConfig.UseFrameLimiter   = ini.GetLongValue("pcsx",  "UseFrameLimiter",   1);
	xboxConfig.UseParasiteEveFix = ini.GetLongValue("pcsx",  "UseParasiteEveFix", 0);
	xboxConfig.DarkForcesFix     = ini.GetLongValue("pcsx",  "DarkForcesFix",     0);
	xboxConfig.slowboot          = ini.GetLongValue("pcsx",  "slowboot",          0);
//	xboxConfig.region            = ini.GetLongValue("pcsx",  "Region",            0);
*/	
}

void ApplySettings_default(const char* path){
/*
	ini.SetLongValue("pcsx", "UseDynarec",       xboxConfig.UseDynarec);
	ini.SetLongValue("pcsx", "UseThreadedGpu",   xboxConfig.UseThreadedGpu);
	ini.SetLongValue("pcsx", "UseSpuIrq",        xboxConfig.UseSpuIrq);
	ini.SetLongValue("pcsx", "UseFrameLimiter",  xboxConfig.UseFrameLimiter);
	ini.SetLongValue("pcsx", "UseParasiteEveFix",xboxConfig.UseParasiteEveFix);
	ini.SetLongValue("pcsx", "DarkForcesFix",    xboxConfig.DarkForcesFix);
	ini.SetLongValue("pcsx", "slowboot",         xboxConfig.slowboot);
//	ini.SetLongValue("pcsx", "Region",           xboxConfig.region);
*/
	ini.SetLongValue("pcsx", "UseFrameLimiter",  xboxConfig.UseFrameLimiter);

	ini.SaveFile(path);
}



void ApplyShader(std::string game){

    ini.SetValue("pcsxshader", "Shader",xboxConfig.Shaders);

    game = xboxConfig.game;
    // Get basename of current game
	game.erase(0, game.rfind('\\')+1);
    game = "game:\\gameshader\\" + game;
    game = game + ".ini";

	ini.SaveFile(game.c_str());
}

void LoadShader(std::string game){

	game = xboxConfig.game;
    // Get basename of current game
	game.erase(0, game.rfind('\\')+1);
    game = "game:\\gameshader\\" + game;
    game = game + ".ini";
    //Merge cfg file
	ini.LoadFile(game.c_str());

    strcpy(xboxConfig.Shaders, strdup(ini.GetValue("pcsxshader", "Shader", "game:\\hlsl\\stock.cg")));
}

void InitPcsx() {
	// Initialize pcsx with default settings
	XMemSet(&Config, 0, sizeof(PcsxConfig));

    // Create dir ...
	CreateDirectory("game:\\states\\",               NULL);
	CreateDirectory("game:\\memcards\\",             NULL);
	CreateDirectory("game:\\gameprofile\\",          NULL);
	CreateDirectory("game:\\covers\\",               NULL);
	CreateDirectory("game:\\gameguides\\",           NULL);
	CreateDirectory("game:\\gameshader\\",           NULL);
	CreateDirectory(xboxConfig.saveStateDir.c_str(), NULL);
	char* path = 0;
	LoadSettings(path);

	Config.PsxOut                  = 1;
	Config.HLE                     = 0;
    Config.Xa                      = 0;  //XA enabled
	Config.Cdda                    = 0;
	Config.PsxAuto                 = 0;
	xboxConfig.noGameBrowse        = 0;
	xboxConfig.CoverModeHorizontal = 0;
	
}

void DoPcsx(char* game) {
  int ret;
  std::string sgame;
  char *path= 0;

    Config.PsxAuto = TRUE;

    sgame = game;
    LoadShader(sgame);
	ApplySettings(path);

    POKOPOM_Init();
    SetIsoFile(game);
	gpuDmaThreadInit();

	if (SysInit() == -1) {
		// Display an error ?
		printf("SysInit() Error!\n");
		return;
	}

	gpuThreadEnable(xboxConfig.UseThreadedGpu);

	ret = CDR_open();
	if (ret < 0) { SysMessage (_("Error Opening CDR Plugin")); return; }
    ret = GPU_open(NULL);
	if (ret < 0) { SysMessage (_("Error Opening GPU Plugin (%d)"), ret); return; }
    ret = SPU_open(NULL);
	SPU_registerCallback(SPUirq);
	if (ret < 0) { SysMessage (_("Error Opening SPU Plugin (%d)"), ret); return; }
	ret = PAD1_open(NULL);
	if (ret < 0) { SysMessage (_("Error Opening PAD1 Plugin (%d)"), ret); return; }
	PAD1_registerVibration(GPU_visualVibration);
	ret = PAD2_open(NULL);
	if (ret < 0) { SysMessage (_("Error Opening PAD2 Plugin (%d)"), ret); return; }	
	PAD2_registerVibration(GPU_visualVibration); 

	GPU_clearDynarec(clearDynarec);

	if(slow_boot)
	Config.SlowBoot = 1;
    else
    Config.SlowBoot = 0;

	SysPrintf("CheckCdrom\r\n");
	int res = CheckCdrom();
	if(res)
		SysPrintf("CheckCdrom: %08x\r\n",res);
    SysReset();
    res=LoadCdrom();
	if(res)
		SysPrintf("LoadCdrom: %08x\r\n",res);

	LoadShaderFromFile(xboxConfig.Shaders);
	CoverSystemAndGameProfile(getgameID());

	SysPrintf("Execute\r\n");
	psxCpu->Execute();
}

void CoverSystemAndGameProfile(const char* gameID) {
	
	
	std::wstring wgame;
    wgame = GamePath;
	get_string(wgame, profile_filename);
    //Get basename of current game
	profile_filename.erase(0, profile_filename.rfind('\\')+1);
    profile_filename = "game:\\gameprofile\\" + profile_filename;
    profile_filename = profile_filename + ".ini";
    wsprintf(xboxConfig.CurrentgameID,profile_filename.c_str());

}


static void InitXui() {
	// Initialize the xui application
	HRESULT hr = app.InitShared(g_pd3dDevice, &g_d3dpp, XuiD3DXTextureLoader, NULL);

    // Register a default typeface
    hr = app.RegisterDefaultTypeface( L"Arial Unicode MS", L"file://game:/media/xui/xarialuni.ttf" );

    // Load the skin file used for the scene. 
    app.LoadSkin( L"file://game:/media/xui/simple_scene_skin.xur" );

    // Load the scene.
	app.LoadFirstScene( L"file://game:/media/xui/", L"scene.xur", NULL );
//	app.LoadFirstScene( L"file://game:/media/xui/", L"scene2.xur", NULL );

	// Start the in game thread
	HANDLE hInGameThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InGameThread, NULL, CREATE_SUSPENDED, NULL);

	XSetThreadProcessor(hInGameThread, 5);
	ResumeThread(hInGameThread);


    // Set matrice for xui - allow to resize move correctly in small tvs
	D3DXMATRIX mat;
	D3DXMATRIX tr;

	XuiRenderGetViewTransform(app.GetDC(), &mat);
	D3DXMatrixTranslation(&tr, 0, (((float)g_d3dpp.BackBufferHeight-720.f)/2.f), 0);
	D3DXMatrixMultiply(&mat, &mat, &tr);
	XuiRenderSetViewTransform(app.GetDC(), &mat);

}

void Add(char* d, char* mountpoint){
	Map(mountpoint,d);
}

HRESULT MountDevices(){

	Add("\\Device\\Cdrom0","cdrom:");

	Add("\\Device\\Flash","flash:");

	//hdd
	//Add("\\Device\\Harddisk0\\SystemPartition\\","hddx:");
	Add("\\Device\\Harddisk0\\Partition0","hdd0:");
	Add("\\Device\\Harddisk0\\Partition1","hdd1:");
	Add("\\Device\\Harddisk0\\Partition2","hdd2:");
	Add("\\Device\\Harddisk0\\Partition3","hdd3:");

	//mu
	Add("\\Device\\Mu0","mu0:");
	Add("\\Device\\Mu1","mu1:");
	Add("\\Device\\Mass0PartitionFile\\Storage","usbmu0:");

	//usb
	Add("\\Device\\Mass0", "usb0:");
	Add("\\Device\\Mass1", "usb1:");
	Add("\\Device\\Mass2", "usb2:");

	Add("\\Device\\BuiltInMuSfc","sfc:");

	return S_OK;
}

VOID __cdecl main(){

	MountDevices();

	InitD3D();

	InitPcsx();

	InitXui();

	while(1) {
		// Render
		RenderXui();

		//new code to execute roms from Aurora Dash wip
		/*if((args.length() > 0) && (xboxConfig.Running == false)){
			xboxConfig.Running = true;
			//get_string((std::string)args,xboxConfig.game);
			wsprintf(xboxConfig.Game2Boot,args.c_str());
			ApplySettings("game:\\pcsx.ini");
			cdrIsoInit();
			DoPcsx((char*)args.c_str());

		}else {*/
		
			// a game can be loaded !
			if (xboxConfig.game.empty() == false && xboxConfig.Running == false) {
			// run it !!!			
				xboxConfig.Running = true;
				wsprintf(xboxConfig.Game2Boot,xboxConfig.game.c_str());

				RemoveSound();
                cdrIsoInit();
				DoPcsx((char*)xboxConfig.game.c_str());
			}else
			{
				if(xboxConfig.runBios)
				{
					xboxConfig.Running = true;
					wsprintf(xboxConfig.Game2Boot,xboxConfig.game.c_str());

				    RemoveSound();
					cdrIsoInit();
					DoPcsx((char*)xboxConfig.game.c_str());
					xboxConfig.runBios = false;
				}
		    }
	    }

    // Free resources, unregister custom classes, and exit.
    app.Uninit();
}

