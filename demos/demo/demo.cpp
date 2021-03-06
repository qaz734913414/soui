// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include <helper/MenuWndHook.h>
#include <helper/mybuffer.h>
#include "httpsvr/HTTPServer.h"
#include "MemFlash.h"
#include "../controls.extend/propgrid/SPropertyGrid.h"
#include "../controls.extend/SFlyWnd.h"
#include "../controls.extend/SFadeFrame.h"
#include "../controls.extend/sradiobox2.h"
#include "../controls.extend/SVscrollbar.h"
#include "../controls.extend/SChromeTabCtrl.h"
#include "../controls.extend/siectrl.h"
#include "../controls.extend/schatedit.h"
#include "../controls.extend/SScrollText.h"
#include "../controls.extend/SCalendar2.h"

#include "uianimation/UiAnimationWnd.h"
#include "appledock/SDesktopDock.h"
#include "SMatrixWindow.h"
#include "SmileyCreateHook.h"
#include "clock/sclock.h"

#if defined(_DEBUG) && !defined(_WIN64)
// #include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#define RES_TYPE 0      //从文件中加载资源，加载失败再从PE加载
// #define RES_TYPE 1   //从PE资源中加载UI资源
// #define RES_TYPE 2   //从zip包中加载资源

#include "../components/resprovider-zip/zipresprovider-param.h"

#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

#include "../controls.extend/smiley/ssmileyctrl.h"

ROBJ_IN_CPP

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
    
    //必须要调用OleInitialize来初始化运行环境
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));


    //LoadLibrary(L"E:\\soui.taobao\\richedit\\Debug\\riched20.dll");
    
    /* XML预编译前面加载效率比较
    pugi::xml_document doc;
    
    LARGE_INTEGER perf;
    QueryPerformanceFrequency(&perf);
    LARGE_INTEGER t1,t2;
    QueryPerformanceCounter(&t1);
    doc.load_file(L"e:\\play.xml",pugi::parse_default,pugi::encoding_utf8);
    QueryPerformanceCounter(&t2);
    doc.save_file(L"e:\\play1.xml");
    
    
    doc.save_bin(L"e:\\paly.xml.bin");
    pugi::xml_document doc2;
    LARGE_INTEGER t21,t22;
    QueryPerformanceCounter(&t21);
    doc2.load_bin_file(L"e:\\paly.xml.bin");
    QueryPerformanceCounter(&t22);
    doc2.save_file(L"e:\\play2.xml");
    
    LONGLONG SP1 = (t2.QuadPart-t1.QuadPart)*1000000/perf.QuadPart;        
    LONGLONG SP2 = (t22.QuadPart-t21.QuadPart)*1000000/perf.QuadPart;
    
    SStringW str;
    str.Format(L"!!!!!sp2=%d,sp1=%d\n",(int)SP2,(int)SP1);
    MessageBoxW(GetActiveWindow(),str,L"span",MB_OK);
    return 0;    
    */
    int nRet = 0; 

    //使用imgdecoder-png图片解码模块演示apng动画
    SComMgr *pComMgr = new SComMgr(_T("imgdecoder-png"));
    

    {

        int nType=MessageBox(GetActiveWindow(),_T("选择渲染类型：\n[yes]: Skia\n[no]:GDI\n[cancel]:Quit"),_T("select a render"),MB_ICONQUESTION|MB_YESNOCANCEL);
        if(nType == IDCANCEL)
        {
            nRet = -1;
            goto exit;
        }


        //定义一组类SOUI系统中使用的类COM组件
        //CAutoRefPtr是一个SOUI系统中使用的智能指针类
        CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory; //图片解码器，由imagedecoder-wid.dll模块提供
        CAutoRefPtr<IRenderFactory> pRenderFactory;         //UI渲染模块，由render-gdi.dll或者render-skia.dll提供
        CAutoRefPtr<ITranslatorMgr> trans;                  //多语言翻译模块，由translator.dll提供
        CAutoRefPtr<IScriptFactory> pScriptLua;              //lua脚本模块，由scriptmodule-lua.dll提供
        CAutoRefPtr<ILog4zManager>  pLogMgr;                //log4z对象
        
        BOOL bLoaded=FALSE;
        //从各组件中显式创建上述组件对象
        
        
        if(nType == IDYES)
            bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
        else
            bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),nType==IDYES?_T("render_skia"):_T("render_gdi"));
        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));
        bLoaded=pComMgr->CreateTranslator((IObjRef**)&trans);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("translator"));

        if(pComMgr->CreateLog4z((IObjRef**)&pLogMgr))
        if(pLogMgr){
            pLogMgr->createLogger("soui");//support output soui trace infomation to log
            pLogMgr->start();
        }
        
        //为渲染模块设置它需要引用的图片解码模块
        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        //定义一个唯一的SApplication对象，SApplication管理整个应用程序的资源
        SApplication *theApp=new SApplication(pRenderFactory,hInstance);
        
        theApp->SetLogManager(pLogMgr);
        LOGF("demo","test "<<100 << L"wchar");
        SLOGFMTE("log output using ansi format,str=%s, tick=%u","test",GetTickCount());
        SLOGFMTE(L"log output using unicode format,str=%s, tick=%u",L"中文",GetTickCount());
        
        //控件注册要放到AddResProvider前： 2016年3月8日
        
        //向SApplication系统中注册由外部扩展的控件及SkinObj类
        SWkeLoader wkeLoader;
        if(wkeLoader.Init(_T("wke.dll")))        
        {
            theApp->RegisterWndFactory(TplSWindowFactory<SWkeWebkit>());//注册WKE浏览器
        }
        theApp->RegisterWindowClass<SMatrixWindow>();//
        theApp->RegisterWindowClass<SFreeMoveWindow>();//
        theApp->RegisterWindowClass<SClock>();//
        theApp->RegisterWindowClass<SGifPlayer>();//theApp中增加方法：RegisterWindowClass,替换RegisterWndFactory(TplSWindowFactory<SGifPlayer>())
        theApp->RegisterSkinFactory(TplSkinFactory<SSkinGif>());//注册SkinGif
        theApp->RegisterSkinFactory(TplSkinFactory<SSkinAPNG>());//注册SSkinAPNG
        theApp->RegisterSkinFactory(TplSkinFactory<SSkinVScrollbar>());//注册纵向滚动条皮肤

        theApp->RegisterWndFactory(TplSWindowFactory<SIPAddressCtrl>());//注册IP控件
        theApp->RegisterWndFactory(TplSWindowFactory<SPropertyGrid>());//注册属性表控件
        theApp->RegisterWndFactory(TplSWindowFactory<SChromeTabCtrl>());//注册ChromeTabCtrl
        theApp->RegisterWndFactory(TplSWindowFactory<SIECtrl>());//注册IECtrl
        theApp->RegisterWndFactory(TplSWindowFactory<SChatEdit>());//注册ChatEdit
        theApp->RegisterWndFactory(TplSWindowFactory<SScrollText>());//注册SScrollText
        theApp->RegisterWndFactory(TplSWindowFactory<SDesktopDock>());//注册SDesktopDock
        theApp->RegisterWndFactory(TplSWindowFactory<SImageMaskWnd>());//注册SImageMaskWnd
        theApp->RegisterWndFactory(TplSWindowFactory<SRatingBar>());//注册SRatingBar
        if(SUCCEEDED(CUiAnimation::Init()))
        {
            theApp->RegisterWndFactory(TplSWindowFactory<SUiAnimationWnd>());//注册动画控件
        }
        theApp->RegisterWndFactory(TplSWindowFactory<SFlyWnd>());//注册飞行动画控件
        theApp->RegisterWndFactory(TplSWindowFactory<SFadeFrame>());//注册渐显隐动画控件
        theApp->RegisterWndFactory(TplSWindowFactory<SRadioBox2>());//注册RadioBox2
        theApp->RegisterWndFactory(TplSWindowFactory<SCalendar2>());//注册SCalendar2

        SSkinGif::Gdiplus_Startup();
        
        //如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id::namedXmlID是由uiresbuilder 增加-h .\res\resource.h idtable 这3个参数后生成的。
        theApp->InitXmlNamedID(namedXmlID,ARRAYSIZE(namedXmlID),TRUE);
        
        //将程序的运行路径修改到demo所在的目录
        SStringT strResDir = theApp->GetAppDir();
        strResDir += _T("\\..\\demos\\demo");
        SetCurrentDirectory(strResDir);
        //SOUI系统总是从appdir去查找资源
        theApp->SetAppDir(strResDir);

        //定义一人个资源提供对象,SOUI系统中实现了3种资源加载方式，分别是从文件加载，从EXE的资源加载及从ZIP压缩包加载
        CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)//从文件加载
        CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
        if(!pResProvider->Init((LPARAM)_T("uires"),0))
        {
            CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
            if(!pResProvider->Init((WPARAM)hInstance,0))
            {
                SASSERT(0);
                delete theApp;
                nRet = 1;
                goto exit;
            }
        }
#elif (RES_TYPE==1)//从EXE资源加载
        CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)hInstance,0);
#elif (RES_TYPE==2)//从ZIP包加载
        bLoaded=pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("resprovider_zip"));

        ZIPRES_PARAM param;
        param.ZipFile(pRenderFactory, _T("uires.zip"),"souizip");
        bLoaded = pResProvider->Init((WPARAM)&param,0);
        SASSERT(bLoaded);
#endif
        //将创建的IResProvider交给SApplication对象
        theApp->AddResProvider(pResProvider);

        //创建一个http服务器，用来从资源中加载flash
        CMemFlash   memFlash;

        CHTTPServer flashSvr(&memFlash);
        flashSvr.Start(CMemFlash::HomeDir(),"",82,0);

        if(trans)
        {//加载语言翻译包
            theApp->SetTranslator(trans);
            pugi::xml_document xmlLang;
            if(theApp->LoadXmlDocment(xmlLang,_T("lang_cn"),_T("translator")))
            {
                CAutoRefPtr<ITranslator> langCN;
                trans->CreateTranslator(&langCN);
                langCN->Load(&xmlLang.child(L"language"),1);//1=LD_XML
                trans->InstallTranslator(langCN);
            }
        }
#if defined(DLL_CORE) && !defined(_WIN64)
        //加载LUA脚本模块，注意，脚本模块只有在SOUI内核是以DLL方式编译时才能使用。
        bLoaded=pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLua);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("scirpt_lua"));
        theApp->SetScriptFactory(pScriptLua);
#endif//DLL_CORE

        
        //加载系统资源
        HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
        if(hSysResource)
        {
            CAutoRefPtr<IResProvider> sysSesProvider;
            CreateResProvider(RES_PE,(IObjRef**)&sysSesProvider);
            sysSesProvider->Init((WPARAM)hSysResource,0);
            theApp->LoadSystemNamedResource(sysSesProvider);
        }
        //采用hook绘制菜单的边框
        CMenuWndHook::InstallHook(hInstance,L"_skin.sys.menu.border");
        
        //加载全局资源描述XML
        //theApp->Init(_T("xml_init")); 不再需要这句，在AddResProvider时自动执行初始化
        
        //演示R.color.xxx,R.string.xxx在代码中的使用。
        COLORREF crRed = GETCOLOR(R.color.red);
        SStringW strTitle = GETSTRING(R.string.title);
        
		SNotifyCenter *pNotifyCenter = new SNotifyCenter;
        {
            //创建并显示使用SOUI布局应用程序窗口,为了保存窗口对象的析构先于其它对象，把它们缩进一层。
            CMainDlg dlgMain;  
            dlgMain.Create(GetActiveWindow(),0,0,800,650);

            dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow();
            dlgMain.ShowWindow(SW_SHOWNORMAL);

            SmileyCreateHook  smileyHook; //不知道MainDlg里哪块和mhook冲突了，在win10中，如果hook放到dlgmain.create前会导致hook失败。
            nRet=theApp->Run(dlgMain.m_hWnd);
        }
		delete pNotifyCenter;

        theApp->UnregisterWindowClass<SGifPlayer>();
        //应用程序退出
        delete theApp; 
        
        if(pLogMgr)
        {
            pLogMgr->stop();
        }
        
        flashSvr.Shutdown();

        //卸载菜单边框绘制hook
        CMenuWndHook::UnInstallHook();
        CUiAnimation::Free();
                
        SSkinGif::Gdiplus_Shutdown();
    }
exit:
    delete pComMgr;
    

    OleUninitialize();

    return nRet;
}


