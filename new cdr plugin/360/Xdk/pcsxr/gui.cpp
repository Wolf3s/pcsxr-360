#include "gui.h"
#include "gui_class.h"
#include "misc.h"
#include "simpleini\SimpleIni.h"

CMyApp app;
XboxConfig xboxConfig;

FileBrowserProvider fileList;
FileBrowserProvider saveStateList;
FileBrowserProvider effectList;
FileBrowserProvider swapdiscList;
FileBrowserProvider gameguideList;

std::wstring CoverPath;
std::wstring GamePath;

//------------------------------------------------------GameGuide Stuff
std::string  game_guide;
std::string  game_guide_part2;

std::wstring txtfile;
std::wstring txtfile2;

bool fileOK = false;
bool fileOK2 = false;

float txt_speed = 130;
float txt_accel = 12;

std::wstring ReadFile(const std::string filename)
{
   std::wstring str;
   FILE *file = fopen(filename.c_str(),"rb");

   if (file)
   {
       fseek(file,0,SEEK_END);
       size_t file_size = ftell(file);
       fseek(file,0,SEEK_SET);
       wchar_t *buf = (wchar_t *)malloc(file_size);
       fread(buf,1,file_size,file);
	   str = buf;
	   free(buf);
	   fclose(file);
       fileOK = true;
   }
       return str;
}

std::wstring ReadFile2(const std::string filename2)
{
   std::wstring str2;
   FILE *file2 = fopen(filename2.c_str(),"rb");
   
   if (file2)
   {
       fseek(file2,0,SEEK_END);
       size_t file_size2 = ftell(file2);
       fseek(file2,0,SEEK_SET);
       wchar_t *buf2 = (wchar_t *)malloc(file_size2);
       fread(buf2,1,file_size2,file2);
	   str2 = buf2;
	   free(buf2);
	   fclose(file2);
       fileOK2 = true;
   }
       return str2;
}
//----------------------------------------------------------------

//----------------------------------------------------------------Cscene

class CMainMenuScene : public CXuiSceneImpl
{
protected:

	wchar_t fileBrowserInfoText[512];

	CXuiList      FileBrowser;
	CXuiList      FileBrowserV;
	CXuiNavButton OsdBtn;
	CXuiControl   FileBrowserInfo;

	CXuiCheckbox DynarecCbox;
	CXuiCheckbox GpuThCbox;
	CXuiCheckbox SpuIrqCbox;
	CXuiCheckbox FrameLimitCbox;
	CXuiCheckbox ParasiteEveFixCbox;
	CXuiCheckbox DarkForcesFixCbox;
	CXuiCheckbox CdrPluginCbox;
	CXuiCheckbox slowbootCbox;
	CXuiCheckbox VirtualMemoryCbox;
    
	CXuiControl      txtteste;
	CXuiNavButton    Backbtn;
	CXuiImageElement imgCover;
	CXuiNavButton    ChangeCoverMode;
	CXuiNavButton    BtnProfileSet;
	CXuiControl      btnSaveProfile;
	CXuiControl      bgcolorProfile;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_RENDER( OnRender )

		XUI_END_MSG_MAP()


		//----------------------------------------------------------------------------------
		// Performs initialization tasks - retrieves controls.
		//----------------------------------------------------------------------------------
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{
		GetChildById( L"FileBrowser",     &FileBrowser );
		GetChildById( L"FileBrowserV",    &FileBrowserV );		
		GetChildById( L"OsdBtn",          &OsdBtn );
		GetChildById( L"FileBrowserInfo", &FileBrowserInfo );

		GetChildById( L"DynarecCbox",        &DynarecCbox );
		GetChildById( L"GpuThCbox",          &GpuThCbox );
		GetChildById( L"SpuIrqCbox",         &SpuIrqCbox );
		GetChildById( L"FrameLimitCbox",     &FrameLimitCbox );
		GetChildById( L"ParasiteEveFixCbox", &ParasiteEveFixCbox );
        GetChildById( L"DarkForcesFixCbox",  &DarkForcesFixCbox );
		GetChildById( L"CdrPluginCbox",      &CdrPluginCbox );
		GetChildById( L"slowbootCbox",       &slowbootCbox );
		GetChildById( L"VirtualMemoryCbox",  &VirtualMemoryCbox );
		

		GetChildById( L"XuiLabel3",   &txtteste);
		GetChildById( L"Backbtn",     &Backbtn );
		GetChildById( L"PsxScreen",   &imgCover);
		GetChildById( L"BtnCoverMode",&ChangeCoverMode);
		GetChildById( L"BtnProfileSet",  &BtnProfileSet);
		GetChildById( L"btnSaveProfile", &btnSaveProfile);
        GetChildById( L"bgcolorProfile", &bgcolorProfile);

		txtteste.SetText(&CoverPath[0]);

		// Init values from xbox config
		DynarecCbox.SetCheck(xboxConfig.UseDynarec);
		GpuThCbox.SetCheck(xboxConfig.UseThreadedGpu);
		SpuIrqCbox.SetCheck(xboxConfig.UseSpuIrq);
		FrameLimitCbox.SetCheck(xboxConfig.UseFrameLimiter);
		ParasiteEveFixCbox.SetCheck(xboxConfig.UseParasiteEveFix);
		DarkForcesFixCbox.SetCheck(xboxConfig.DarkForcesFix);
		slowbootCbox.SetCheck(xboxConfig.slowboot);
		VirtualMemoryCbox.SetCheck(xboxConfig.usevm);

		return S_OK;
	}

	HRESULT OnRender(
		XUIMessageRender *pInputData,
		BOOL &bHandled
		) {

			imgCover.SetImagePath(&CoverPath[0]);
			
			swprintf(fileBrowserInfoText, L"%d/%d", FileBrowser.GetCurSel() + 1, FileBrowser.GetItemCount());
			FileBrowserInfo.SetText(fileBrowserInfoText);
			
  		    
			return S_OK;
	}


	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		if (hObjPressed == FileBrowser) {
			xboxConfig.noGameBrowse = TRUE;
			OsdBtn.Press();
		}
		if (hObjPressed == FileBrowserV) {
			xboxConfig.noGameBrowse = TRUE;
			OsdBtn.Press();
		}

		if (hObjPressed == DynarecCbox) {
			xboxConfig.UseDynarec = DynarecCbox.IsChecked();
		}

		if (hObjPressed == GpuThCbox) {
			xboxConfig.UseThreadedGpu = GpuThCbox.IsChecked();
		}

		if (hObjPressed == SpuIrqCbox) {
			xboxConfig.UseSpuIrq = SpuIrqCbox.IsChecked();
		}

		if (hObjPressed == FrameLimitCbox) {
			xboxConfig.UseFrameLimiter = FrameLimitCbox.IsChecked();
		}

		if (hObjPressed == ParasiteEveFixCbox) {
			xboxConfig.UseParasiteEveFix = ParasiteEveFixCbox.IsChecked();
		}

		if (hObjPressed == DarkForcesFixCbox) {
			xboxConfig.DarkForcesFix = DarkForcesFixCbox.IsChecked();
		
		}

		if (hObjPressed == slowbootCbox) {
			xboxConfig.slowboot = slowbootCbox.IsChecked();
		
		}

		if (hObjPressed == VirtualMemoryCbox) {
			xboxConfig.usevm = VirtualMemoryCbox.IsChecked();
	
		}

		if (hObjPressed == CdrPluginCbox) {
//			XLaunchNewImage ("D:\\default.xex", NULL);
			XLaunchNewImage ("D:\\default_oldcdr.xex", NULL);
		}

        if (hObjPressed == Backbtn) {
//         NavigateBack();// not used in this build
		   char*path=0;
		   LoadSettings(path);

           std::wstring wpath = GamePath;

          const WCHAR * button_text = L"OK";
		   ShowMessageBoxEx(NULL,NULL,L"Game Profile Settings Loaded",wpath.c_str(), 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL);

		   bHandled = TRUE;
		}	

		if (hObjPressed == ChangeCoverMode)
		{
		 
            //txtteste.SetText(&CoverPath[0]);
			xboxConfig.CoverModeHorizontal = !xboxConfig.CoverModeHorizontal;
			FileBrowser.SetShow(xboxConfig.CoverModeHorizontal);
			FileBrowser.SetEnable(xboxConfig.CoverModeHorizontal);
			imgCover.SetShow(!xboxConfig.CoverModeHorizontal);
			FileBrowserV.SetShow(!xboxConfig.CoverModeHorizontal);
			FileBrowserV.SetEnable(!xboxConfig.CoverModeHorizontal);
			if(xboxConfig.CoverModeHorizontal){
				//FileBrowser.SetCurSel(0);
				FileBrowser.SetFocus();
			}
			else{
				//FileBrowserV.SetCurSel(FileBrowser.GetCurSel());
				//FileBrowserV.SetCurSel(0);
				FileBrowserV.SetFocus();
			}
			bHandled = TRUE;
		}

		if(hObjPressed == BtnProfileSet)
		{   
			
			bgcolorProfile.SetShow(TRUE);
			bgcolorProfile.SetEnable(TRUE);
			DynarecCbox.SetShow(TRUE);
			GpuThCbox.SetShow(TRUE);
			SpuIrqCbox.SetShow(TRUE);
			FrameLimitCbox.SetShow(TRUE);
			ParasiteEveFixCbox.SetShow(TRUE);
			DarkForcesFixCbox.SetShow(TRUE);
			slowbootCbox.SetShow(TRUE);
			VirtualMemoryCbox.SetShow(TRUE);
			DynarecCbox.SetFocus();
			btnSaveProfile.SetShow(TRUE);

			ApplySettings_default("game:\\default.ini");
			LoadSettings_default ("game:\\default.ini");

            bHandled = TRUE;			

        }

		if (hObjPressed == btnSaveProfile)
		{
			char * path = 0;

			bgcolorProfile.SetShow(FALSE);
			bgcolorProfile.SetEnable(FALSE);
			DynarecCbox.SetShow(FALSE);
			GpuThCbox.SetShow(FALSE);
			SpuIrqCbox.SetShow(TRUE);
			FrameLimitCbox.SetShow(FALSE);
			ParasiteEveFixCbox.SetShow(FALSE);
			DarkForcesFixCbox.SetShow(FALSE);
			slowbootCbox.SetShow(FALSE);
			VirtualMemoryCbox.SetShow(FALSE);
			btnSaveProfile.SetShow(FALSE);
			SpuIrqCbox.SetShow(FALSE);

			
			if(xboxConfig.CoverModeHorizontal)
			{
				FileBrowser.SetFocus();

			    ApplySettings_default("game:\\default.ini");
				Sleep(1);
                ApplySettings(path);
				LoadSettings(path);
			}
			else
			{
				FileBrowserV.SetFocus();

			    ApplySettings_default("game:\\default.ini");
				Sleep(1);
			    ApplySettings(path);
				LoadSettings(path);
			}
        }       

		return S_OK;
	}

public:
	XUI_IMPLEMENT_CLASS( CMainMenuScene, L"MainMenu", XUI_CLASS_SCENE )
};

class COsdMenuScene: public CXuiSceneImpl
{
protected:

	std::string game;
	std::wstring wgame;

	CXuiControl BackBtn;
	CXuiControl ResetBtn;
	CXuiControl SelectBtn;
	CXuiControl btnBack;
	CXuiImageElement xuiGameImg;
	CXuiControl SaveStateBtn;

	CXuiList RegionList;

	COsdMenuScene() {
	}

	~COsdMenuScene() {
	}

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_RENDER( OnRenderOSD )
		XUI_ON_XM_NOTIFY_KILL_FOCUS( OnNotifyKillFocus )
	XUI_END_MSG_MAP()

	//----------------------------------------------------------------------------------
	// Performs initialization tasks - retrieves controls.
	//----------------------------------------------------------------------------------
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{
		GetChildById( L"BackBtn", &BackBtn );
		GetChildById( L"ResetBtn", &ResetBtn );
		GetChildById( L"SelectBtn", &SelectBtn );
		GetChildById( L"btnBack",&btnBack);

		GetChildById( L"SaveStateBtn", &SaveStateBtn );
		GetChildById( L"RegionList", &RegionList);
		GetChildById( L"xuiGameImg", &xuiGameImg);
		xboxConfig.noGameBrowse = TRUE;
		effectList.UpdateDirList( L"game:\\hlsl");

		RegionList.SetCurSel(xboxConfig.region);

		xuiGameImg.SetImagePath(&CoverPath[0]);
		

		return S_OK;
	}

	HRESULT OnNotifyKillFocus(
         HXUIOBJ hObjSource,
         XUINotifyFocus *pNotifyFocusData,
         BOOL &bHandled
	) {
		if (hObjSource == RegionList) {
			xboxConfig.region = RegionList.GetCurSel();
		}


		return S_OK;
	}


	HRESULT OnRenderOSD( XUIMessageRender *pInputData, BOOL &bHandled ) {
			//Getting and setting up the cover
			//xuiGameImg.SetImagePath(&CoverPath[0]);
			return S_OK;
	}


	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled ) {

		if (hObjPressed == BackBtn) {
			xboxConfig.noGameBrowse = TRUE;
			ResumePcsx();//test
			bHandled = TRUE;
		}

		if (hObjPressed == ResetBtn) {
			xboxConfig.noGameBrowse = TRUE;
			ResetPcsx();
		}

		if (hObjPressed == SelectBtn) {
			xboxConfig.game.clear();
			xboxConfig.noGameBrowse = FALSE;
			ShutdownPcsx();
			xboxConfig.Running = false;
			NavigateBack();
			bHandled = TRUE;
		}

		// show
		if (hObjPressed == SaveStateBtn) {			
			xboxConfig.noGameBrowse = FALSE;
			SaveStatePcsx(-1);

			bHandled = TRUE;
		}

		if(hObjPressed == btnBack){
			xboxConfig.noGameBrowse = TRUE;
			ResumePcsx();
			bHandled = TRUE;
		}


		return S_OK;
	}
public:

	XUI_IMPLEMENT_CLASS( COsdMenuScene, L"InGameMenu", XUI_CLASS_SCENE )
};

class CLoadStates: public CXuiSceneImpl 
{
protected:

	CXuiList LoadStateBrowser;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
	XUI_END_MSG_MAP()

	//----------------------------------------------------------------------------------
	// Performs initialization tasks - retrieves controls.
	//----------------------------------------------------------------------------------
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{

		GetChildById( L"LoadStateBrowser", &LoadStateBrowser );

		 // path
		 std::string path = xboxConfig.game;
		 path.erase(0, path.rfind('\\')+1);

		 std::string saveStateDir = xboxConfig.saveStateDir + "\\" + path;
		 std::wstring wg;
		 get_wstring(saveStateDir, wg);
		 saveStateList.UpdateDirList(wg);

		return S_OK;
	}
public:

	XUI_IMPLEMENT_CLASS( CLoadStates, L"LoadStates", XUI_CLASS_SCENE )
};

class CSelectFilter: public CXuiSceneImpl 
{
protected:

	CXuiList EffectList;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
	XUI_END_MSG_MAP()

	//----------------------------------------------------------------------------------
	// Performs initialization tasks - retrieves controls.
	//----------------------------------------------------------------------------------
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{

		GetChildById( L"EffectList", &EffectList );

		effectList.UpdateDirList(L"game:\\hlsl");

		return S_OK;
	}
public:

	XUI_IMPLEMENT_CLASS( CSelectFilter, L"SelectFilter", XUI_CLASS_SCENE )
};

class CDiscSwap: public CXuiSceneImpl 
{
protected:
	CXuiList DiscSwapBrowser;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
	XUI_END_MSG_MAP()

	//----------------------------------------------------------------------------------
	// Performs initialization tasks - retrieves controls.
	//----------------------------------------------------------------------------------
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{

		GetChildById( L"DiscSwapBrowser", &DiscSwapBrowser );
        swapdiscList.UpdateDirList(L"game:\\games");

		return S_OK;
	}
public:

	XUI_IMPLEMENT_CLASS( CDiscSwap, L"DiscSwap", XUI_CLASS_SCENE )
};

class CGameGuide: public CXuiSceneImpl 
{
protected:

    // Control and Element wrapper objects.
    CXuiControl TextBack_button;
    CXuiControl TextNext_button;
    CXuiTextElement m_RendererText;
    CXuiEdit m_Edit;
	CXuiEdit m_Edit_page2;

    // Message map.
    XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
    XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
        fileOK =  false;
		fileOK2 = false;

		// Retrieve controls for later use.
        GetChildById( L"TextBack",     &TextBack_button );
	    GetChildById( L"TextNext",     &TextNext_button );
        GetChildById( L"RendererText", &m_RendererText );
        GetChildById( L"TextEdit",     &m_Edit );
		GetChildById( L"TextEdit2",    &m_Edit_page2 );

        game_guide = xboxConfig.game;
        // Get basename of current game
	    game_guide.erase(0, game_guide.rfind('\\')+1);
		game_guide = "game:\\gameguides\\" + game_guide;
		game_guide_part2 = game_guide + ".part2.txt";
	    game_guide = game_guide + ".txt";

        txtfile  = ReadFile(game_guide);
		txtfile2 = ReadFile2(game_guide_part2);

if(fileOK)
{
      m_Edit.SetShow(TRUE);
      m_Edit.SetFocus();
      // Init text field and edit control text
	  m_Edit.SetText(txtfile.c_str());
	  m_Edit.SetVSmoothScroll(true,100,txt_accel,txt_speed);
      m_RendererText.SetText( L"About The Game - Page 1/2" );
}
else
{
      m_Edit.SetShow(TRUE);
	  m_RendererText.SetText( L"No GameGuide Found - Page 1/2" );
      m_Edit.SetText        ( L"Remember to Rename your GameGuide Part 1 - Ex: game.bin.txt" );//teste
}

        return S_OK;
    }

    //----------------------------------------------------------------------------------
    // Handler for the button press message.
    //----------------------------------------------------------------------------------
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{

     if( hObjPressed == TextNext_button )
     {
if(fileOK2)
{
      m_Edit.SetShow(FALSE);
	  m_Edit_page2.SetShow(TRUE);
	  m_Edit_page2.SetFocus();
	  m_Edit_page2.SetText(txtfile2.c_str());
      m_Edit.SetVSmoothScroll(true,100,txt_accel,txt_speed);
	  m_RendererText.SetText( L"About The Game - Page 2/2" );

	  bHandled = TRUE;
}
else
{
	  m_Edit.SetShow(FALSE);
	  m_Edit_page2.SetShow(TRUE);
	  m_RendererText.SetText( L"No GameGuide Found - Page 2/2" );
      m_Edit_page2.SetText  ( L"Remember to Rename your GameGuide Part 2 - Ex: game.bin.part2.txt" );//teste

     bHandled = TRUE;
}
     }

	if( hObjPressed == TextBack_button )
     {
if(fileOK)
{
      m_Edit_page2.SetShow(FALSE);
      m_Edit.SetShow(TRUE);
	  m_Edit.SetFocus();
	  m_Edit.SetText(txtfile.c_str());
	  m_Edit.SetVSmoothScroll(true,100,txt_accel,txt_speed);
	  m_RendererText.SetText( L"About The Game - Page 1/2" );

	  bHandled = TRUE;
}
else
{
      m_Edit_page2.SetShow(FALSE);
	  m_Edit.SetShow(TRUE);
	  m_RendererText.SetText( L"No GameGuide Found - Page 1/2" );
      m_Edit.SetText        ( L"Remember to Rename your GameGuide Part 1  - Ex: game.bin.txt" );//teste

      bHandled = TRUE;
}
    }
      return S_OK;
}

public:

	XUI_IMPLEMENT_CLASS( CGameGuide, L"GameGuide", XUI_CLASS_SCENE )
};

//----------------------------------------------------------------

//----------------------------------------------------------------Clist

class CXuiFileBrowser : public CXuiListImpl
{
protected:

	FileBrowserProvider * fileBrowser;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_GET_SOURCE_TEXT( OnGetSourceText )
		XUI_ON_XM_GET_ITEMCOUNT_ALL( OnGetItemCountAll )
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_SET_CURSEL (OnSetCurSel)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
	XUI_END_MSG_MAP()

		//----------------------------------------------------------------------------------
		// Performs initialization tasks - retrieves controls.
		//----------------------------------------------------------------------------------
		HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
	{
		fileBrowser->Init();
		fileBrowser->UpdateDirList();

		return S_OK;
	}

	//----------------------------------------------------------------------------------
	// Returns the number of items in the list.
	//----------------------------------------------------------------------------------
	HRESULT OnGetItemCountAll( XUIMessageGetItemCount* pGetItemCountData, BOOL& bHandled )
	{
		pGetItemCountData->cItems = fileBrowser->Size();
		bHandled = TRUE;
		return S_OK;
	}

	//----------------------------------------------------------------------------------
	// Returns the text for the items in the list.
	//----------------------------------------------------------------------------------
	HRESULT OnGetSourceText( XUIMessageGetSourceText* pGetSourceTextData, BOOL& bHandled )
	{
		if( pGetSourceTextData->bItemData && pGetSourceTextData->iItem >= 0 )
		{
			pGetSourceTextData->szText = fileBrowser->At(pGetSourceTextData->iItem);
			bHandled = TRUE;
		}
		return S_OK;
	}

	//Go get game cover
	HRESULT OnGetSourceDataImage( XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
	{
		if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {

			pGetSourceImageData->szPath = fileBrowser->GameCover(pGetSourceImageData->iItem);
			
			bHandled = TRUE;
		}
		return S_OK;
	}


    HRESULT OnSetCurSel(XUIMessageSetCurSel *pSetCurSelData, BOOL &bHandled){

		std::string rompathcover ="";//,y="",z="";
		std::string rompath ="";

		get_string(fileBrowser->GameCover(pSetCurSelData->iItem),rompathcover);//Full rom path ->Filename; display only romname use ->AT ;; Gamecover returns game cover path from the rom list :D hehe
        get_string(fileBrowser->Filename(pSetCurSelData->iItem),rompath);

		CoverPath = std::wstring(rompathcover.begin(),rompathcover.end());
        GamePath =  std::wstring(rompath.begin(),rompath.end());

        bHandled = FALSE;
		return S_OK;

	}

	virtual HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		    // Change the current dir
		if (fileBrowser->IsDir(GetCurSel())) {
			// Delete old item count
			DeleteItems(0, fileBrowser->Size());

			// Update filelist
			fileBrowser->UpdateDirList(fileBrowser->Filename(GetCurSel()));

			// Insert item count
			InsertItems(0, fileBrowser->Size());

			// Move to top
			SetCurSel(0);			

			// Don't Notify parent
			bHandled = TRUE;

		} else {		
			get_string(fileBrowser->Filename(GetCurSel()), xboxConfig.game);

			// scene must done some work
			bHandled = FALSE;
		}
		return S_OK;
	}

public:

    // Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiFileBrowser, L"CXuiFileBrowser", XUI_CLASS_SCENE )
};

//------------------------------------------------------------------

//------------------------------------------------------------------Cxuifilebrowser

class CFileBrowserList : public CXuiFileBrowser
{
protected:

	CFileBrowserList() {
		fileBrowser = &fileList;
	}


	virtual HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		// Change the current dir
		if (fileBrowser->IsDir(GetCurSel())) {
			// Delete old item count
			DeleteItems(0, fileBrowser->Size());

			// Update filelist
			fileBrowser->UpdateDirList(fileBrowser->Filename(GetCurSel()));

			// Insert item count
			InsertItems(0, fileBrowser->Size());

			// Move to top
			SetCurSel(0);			

			// Don't Notify parent
			bHandled = TRUE;

		} else {		
			get_string(fileBrowser->Filename(GetCurSel()), xboxConfig.game);

			// scene must done some work
			bHandled = FALSE;
		}
		return S_OK;
	}

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CFileBrowserList, L"FileBrowserList", XUI_CLASS_LIST )
};

class CLoadStateBrowser : public CXuiFileBrowser
{
protected:

	CLoadStateBrowser() {
		fileBrowser = &saveStateList;
	}

    virtual HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		// Change the current dir
		if (fileBrowser->IsDir(GetCurSel())) {
			// Delete old item count
			DeleteItems(0, fileBrowser->Size());

			// Update filelist
			fileBrowser->UpdateDirList(fileBrowser->Filename(GetCurSel()));

			// Insert item count
			InsertItems(0, fileBrowser->Size());

			// Move to top
			SetCurSel(0);			

			// Don't Notify parent
			bHandled = TRUE;

		} else {	
			std::string path;
			get_string(fileBrowser->Filename(GetCurSel()), path);

			LoadStatePcsx(path);
			bHandled = FALSE;
		}
		return S_OK;
	}

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CLoadStateBrowser, L"LoadStateBrowser", XUI_CLASS_LIST )
};

class CDiscSwapBrowser : public CXuiFileBrowser
{
protected:

	CDiscSwapBrowser() {
		fileBrowser = &swapdiscList;

    }

    virtual HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		// Change the current dir
		if (fileBrowser->IsDir(GetCurSel())) {
			// Delete old item count
			DeleteItems(0, fileBrowser->Size());

			// Update filelist
			fileBrowser->UpdateDirList(fileBrowser->Filename(GetCurSel()));

			// Insert item count
			InsertItems(0, fileBrowser->Size());

			// Move to top
			SetCurSel(0);			

			// Don't Notify parent
			bHandled = TRUE;

		} else {	
			std::string path;
            get_string(fileBrowser->Filename(GetCurSel()), path);
			
            ChangeDisc(path);
			bHandled = FALSE;
		}
		return S_OK;
	}

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CDiscSwapBrowser, L"DiscSwapBrowser", XUI_CLASS_LIST )
};

class CEffectBrowser : public CXuiFileBrowser
{
protected:

	CEffectBrowser() {
		fileBrowser = &effectList;
	}

    virtual HRESULT OnNotifyPress( HXUIOBJ hObjPressed, 
		BOOL& bHandled )
	{

		// Change the current dir
		if (fileBrowser->IsDir(GetCurSel())) {
			// Delete old item count
			DeleteItems(0, fileBrowser->Size());

			// Update filelist
			fileBrowser->UpdateDirList(fileBrowser->Filename(GetCurSel()));

			// Insert item count
			InsertItems(0, fileBrowser->Size());

			// Move to top
			SetCurSel(0);			

			// Don't Notify parent
			bHandled = TRUE;

		} else {	
			std::string path;
			get_string(fileBrowser->Filename(GetCurSel()), path);
			strcpy(xboxConfig.Shaders,path.c_str());
			CoverSystemAndGameProfile(xboxConfig.CurrentgameID);

			LoadShaderFromFile(path.c_str());
			ApplyShader(path.c_str());

			bHandled = FALSE;
		}
		return S_OK;
	}

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CEffectBrowser, L"EffectBrowser", XUI_CLASS_LIST )
};

//--------------------------------------------------------------------------------------

HRESULT CMyApp::RegisterXuiClasses()
{
	HRESULT hr = CMainMenuScene::Register();
	if( FAILED( hr ) )
		return hr;

	hr = COsdMenuScene::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CLoadStates::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CSelectFilter::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CDiscSwap::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CGameGuide::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CXuiFileBrowser::Register();
	if( FAILED( hr ) )
		return hr;

	hr = CFileBrowserList::Register();
	if( FAILED( hr ) )
		return hr;

    hr = CLoadStateBrowser::Register();
	if( FAILED( hr ) )
		return hr;

    hr = CDiscSwapBrowser::Register();
	if( FAILED( hr ) )
		return hr;

    hr = CEffectBrowser::Register();
	if( FAILED( hr ) )
		return hr;

    return S_OK;
}

HRESULT CMyApp::UnregisterXuiClasses()
{
	CMainMenuScene::Unregister();
	COsdMenuScene::Unregister();
	CLoadStates::Unregister();
	CSelectFilter::Unregister();
	CDiscSwap::Unregister();
	CGameGuide::Unregister();
	CXuiFileBrowser::Unregister();
	CFileBrowserList::Unregister();
	CLoadStateBrowser::Unregister();
	CDiscSwapBrowser::Unregister();
	CEffectBrowser::Unregister();
	
	return S_OK;
}

