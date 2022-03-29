// DXF Viewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Viewer.h"
#include "ViewerDlg.h"
#include "intern/drw_dbg.h"
#include "log.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/spdlog.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDXFViewerApp

BEGIN_MESSAGE_MAP(CDXFViewerApp, CWinApp)
	//{{AFX_MSG_MAP(CDXFViewerApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDXFViewerApp construction

CDXFViewerApp::CDXFViewerApp() {
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDXFViewerApp object

CDXFViewerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDXFViewerApp initialization

void initLogger() {
	using namespace spdlog;

	const std::string formate1 = "[%d/%m/%Y %H:%M:%S][%=n][%=l] %v";

	spdlog::init_thread_pool(10000, 1);

	//sinks
	std::vector<sink_ptr> sinkVec;
	std::vector<std::shared_ptr<logger>> loggers;

	//  
	auto allSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(".log/all.log",true);
	allSink->set_pattern(formate1);
	allSink->set_level(level::trace);
	auto all = std::make_shared<spdlog::logger>("all", allSink);
	loggers.push_back(all);


	//loggers
	auto dxfrwResult = spdlog::basic_logger_mt<spdlog::async_factory>("dxfrwResult", ".log/dxfrwResult.log");
	dxfrwResult->set_level(level::info);
	dxfrwResult->set_pattern(formate1);



	//loggers register 
	for (const auto& it : loggers) {
		spdlog::register_logger(it);
	}

	/*
	trace = SPDLOG_LEVEL_TRACE,
	debug = SPDLOG_LEVEL_DEBUG,
	info = SPDLOG_LEVEL_INFO,
	warn = SPDLOG_LEVEL_WARN,
	err = SPDLOG_LEVEL_ERROR,
	critical = SPDLOG_LEVEL_CRITICAL,
	off = SPDLOG_LEVEL_OFF,
	*/

	/*get("all")->trace("msg from trace");
	get("all")->debug("msg from debug");
	get("all")->info("msg from info");
	get("all")->warn("msg from warn");
	get("all")->error("msg from error");
	get("all")->critical("msg from crit");*/

	DRW_dbg::getInstance()->setLevel(DRW_dbg::NONE);
}

BOOL CDXFViewerApp::InitInstance() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(368877);

	initLogger();

	AfxEnableControlContainer();

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CDXFViewerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
	} else if (nResponse == IDCANCEL) {
	}
	DRW_dbg::getInstance()->deleteDRW_dbg();
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}