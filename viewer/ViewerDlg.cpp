// DXF ViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Viewer.h"
#include "ViewerDlg.h"
#include "Drawing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDXFViewerDlg dialog

CDXFViewerDlg::CDXFViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDXFViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDXFViewerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_DrawingWindow.m_pDrawing = &drawing;
}

void CDXFViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDXFViewerDlg)
	DDX_Control(pDX, IDC_DRAWINGWINDOW, m_DrawingWindow);
	DDX_Control(pDX, IDC_COMBO_DRAW, m_combox);
	DDX_Control(pDX, IDC_COMBO_FILE, m_comboxfile);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDXFViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CDXFViewerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_VIEW, OnBtnView)
	ON_BN_CLICKED(IDC_BTN_ZOOMIN, OnBtnZoomin)
	ON_BN_CLICKED(IDC_BTN_ZOOMOUT, OnBtnZoomout)
	ON_BN_CLICKED(IDC_BTN_ZOOMEXTENTS, OnBtnZoomextents)
	//	ON_WM_MOUSEWHEEL()
		//}}AFX_MSG_MAP
	//	ON_WM_SYSCOMMAND()
	//	ON_WM_LBUTTONDOWN()
	//	ON_WM_LBUTTONUP()
	//	ON_WM_MOUSEMOVE()
	//ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CDXFViewerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_NEXT, &CDXFViewerDlg::OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_PREV, &CDXFViewerDlg::OnBnClickedBtnPrev)
	ON_BN_CLICKED(IDC_BTN_EXPORT, &CDXFViewerDlg::OnBnClickedBtnExport)
	ON_CBN_SELCHANGE(IDC_COMBO_DRAW, &CDXFViewerDlg::OnCbnSelchangeComboDraw)
	ON_CBN_SELCHANGE(IDC_COMBO_FILE, &CDXFViewerDlg::OnCbnSelchangeComboFile)
	ON_BN_CLICKED(IDOK2, &CDXFViewerDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDC_DATASORTING, &CDXFViewerDlg::OnBnClickedDatasorting)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDXFViewerDlg message handlers

BOOL CDXFViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//DRAW_ALL = 0,
	//	DRAW_COMPONENTS,
	//	DRAW_BORDERS,
	//	DRAW_TABLES,
	//	RRAW_AXIS,
	//	DRAW_LINES,
	//	DRAW_CORNERS,
	//	DRAW_TEXTS,
	//	DRAW_EXTRACTS,
	//	DRAW_ISSUES,
	//	DRAW_NUMBER,
	m_combox.AddString("全部");
	m_combox.AddString("组件");
	m_combox.AddString("边框");
	m_combox.AddString("墙");
	m_combox.AddString("表格");
	m_combox.AddString("轴网");
	m_combox.AddString("区块");
	m_combox.AddString("柱");
	m_combox.AddString("梁");
	m_combox.AddString("线条");
	m_combox.AddString("圆形");
	m_combox.AddString("调试");
	//m_combox.AddString("交点");
	//m_combox.AddString("文本");
	//m_combox.AddString("提取");
	//m_combox.AddString("问题");
	m_combox.SetCurSel(0);
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CDXFViewerDlg::OnDestroy() {
	if (m_upTimer) {
		this->KillTimer(m_upTimer);
		m_upTimer = 0;
	}
	return TRUE;
}

void CDXFViewerDlg::OnTimer(UINT_PTR timer) {
	m_DrawingWindow.RedrawWindow();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDXFViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CDXFViewerDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}


size_t CheckUTF8Tail(const unsigned char* src_str, size_t nsrc_leng)
{
	int count_bytes = 0;
	unsigned char byte_one = 0, byte_other = 0x3f; // 用于位与运算以提取位值 0x3f-->00111111 	
	size_t	nPos = 0;
	size_t	nCount = nsrc_leng;


	while (nCount)
	{
		count_bytes = 1;

		if (src_str[0] <= 0x7f) {
			count_bytes = 1; // ASCII字符: 0xxxxxxx( ~ 01111111)
		}
		if ((src_str[0] >= 0xc0) && (src_str[0] <= 0xdf)) {
			count_bytes = 2; // 110xxxxx(110 00000 ~ 110 111111)
		}
		if ((src_str[0] >= 0xe0) && (src_str[0] <= 0xef)) {
			count_bytes = 3; // 1110xxxx(1110 0000 ~ 1110 1111)
		}
		if ((src_str[0] >= 0xf0) && (src_str[0] <= 0xf7)) {
			count_bytes = 4; // 11110xxx(11110 000 ~ 11110 111)
		}
		if ((src_str[0] >= 0xf8) && (src_str[0] <= 0xfb)) {
			count_bytes = 5; // 111110xx(111110 00 ~ 111110 11)
		}
		if ((src_str[0] >= 0xfc) && (src_str[0] <= 0xfd)) {
			count_bytes = 6; // 1111110x(1111110 0 ~ 1111110 1)
		}

		if ((nPos + count_bytes) <= nsrc_leng)
		{
			nCount -= count_bytes;
			nPos += count_bytes;
			src_str += count_bytes;
			continue;
		}

		break;
	}

	return nPos;
}

void CDXFViewerDlg::OnBtnView()
{
	// TODO: Add your control notification handler code here
	static char szFilter[] = "DXF file format(*.dxf;*.dwg;) |*.dxf;*.dwg;*.json|JSON file format(*.json)|*.json||";

	CFileDialog OpenDlg(TRUE, nullptr, nullptr/*LastFilePath*/, OFN_HIDEREADONLY | OFN_EXPLORER, szFilter, nullptr);
	OpenDlg.m_ofn.lpstrTitle = "Open DXF File";

	//_CrtSetBreakAlloc(33807028);
	//_CrtSetBreakAlloc(33809573);
	if (OpenDlg.DoModal() == IDOK)
	{	// opening sample dxf file
		CString fileName(OpenDlg.GetPathName());
		/*std::string tempFileName = CT2A(fileName.GetBuffer());
		std::string strLastFour = tempFileName.substr(tempFileName.find("."), tempFileName.length());*/

		//addFile(fileName);

		loadFile(fileName);

		creatWindow();

#if 0

		if (strLastFour == ".dwg")
		{
			loadFile(fileName);
		}
		else if (strLastFour == ".json")
		{
			Json::Value parameterFile = readParameterFile(tempFileName);
			Json::Value floorPlan = readFloorPlan(parameterFile);
			if (floorPlan.isArray())
			{
				for (auto it : floorPlan)
				{
					Json::Value Floors = it;

				}
			}
			else if (floorPlan.isObject())
			{

				Json::Value::Members member = floorPlan.getMemberNames();
				for (auto ite = member.begin(); ite != member.end(); ++ite)
				{

					Json::Value floors = floorPlan[*ite];

					for (auto it : floors)
					{
						std::string fileParameter;

						auto fileName = it.asString();
						fileParameter = "floorPlan|" + *ite + "|" + fileName;
						loadFile(fileParameter.c_str());

					}
					//of.close();
					break;
				}
			}
		}
#endif

#if 0
		if (drawing.isOpen())
			drawing.Destroy();

		// Create Drawing
		if (!drawing.Create())
		{
			MessageBox("Fatal error on creating CDrawing!", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		m_comboxfile.AddString(fileName);
		m_comboxfile.SelectString(0, fileName);
		//m_comboxfile.SetCurSel();

		if (!drawing.LoadFile(fileName))
		{
			//ProgDlg.DestroyWindow();
			MessageBox("Error while loading dxf file!", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		CRect r;
		m_DrawingWindow.GetClientRect(r);
		drawing.InitView(0, 0, r.Width(), r.Height());
		drawing.ZoomExtents();
		m_DrawingWindow.RedrawWindow();
		if (m_upTimer) {
			this->KillTimer(m_upTimer);
		}
		m_upTimer = this->SetTimer(1, 40, NULL);
#endif
	}
}

void CDXFViewerDlg::OnBtnZoomin()
{
	// TODO: Add your control notification handler code here
	if (drawing.isOpen())
	{
		drawing.SetZoomLevel(drawing.GetZoomLevel() * 1.1);
		m_DrawingWindow.RedrawWindow();
	}
}

void CDXFViewerDlg::OnBtnZoomout()
{
	// TODO: Add your control notification handler code here
	if (drawing.isOpen())
	{
		drawing.SetZoomLevel(drawing.GetZoomLevel() / 1.1);
		m_DrawingWindow.RedrawWindow();
	}
}

void CDXFViewerDlg::OnBtnZoomextents()
{
	// TODO: Add your control notification handler code here
	if (drawing.isOpen())
	{
		drawing.ZoomExtents();
		m_DrawingWindow.RedrawWindow();
	}
}

BOOL CDXFViewerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if (drawing.isOpen())
	{
		if (zDelta > 0)
			drawing.SetZoomLevel(drawing.GetZoomLevel() * 1.1);
		else
			drawing.SetZoomLevel(drawing.GetZoomLevel() / 1.1);
		m_DrawingWindow.RedrawWindow();
	}
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}


void CDXFViewerDlg::OnBnClickedOk()
{
	//// TODO: 在此添加控件通知处理程序代码
	//static char szFilter[] = "DXF file format(*.dxf) |*.dxf||";

	//CFileDialog OpenDlg(TRUE, NULL, NULL/*LastFilePath*/, OFN_CREATEPROMPT | OFN_EXPLORER, szFilter, NULL);
	//OpenDlg.m_ofn.lpstrTitle = "Open DXF File";

	//if (OpenDlg.DoModal() == IDOK)
	//{	// Openning sample dxf file
	//	CString fileName(OpenDlg.GetPathName());
	//	drawing.SaveFile(fileName);
	//}

	CDialog::OnOK();
	spdlog::drop_all();
}


void CDXFViewerDlg::OnBnClickedBtnNext()
{
	// TODO: 在此添加控件通知处理程序代码
	drawing.next();
	m_DrawingWindow.RedrawWindow();
}


void CDXFViewerDlg::OnBnClickedBtnPrev()
{
	// TODO: 在此添加控件通知处理程序代码
	drawing.prev();
	m_DrawingWindow.RedrawWindow();
}


void CDXFViewerDlg::OnBnClickedBtnExport()
{
	// TODO: 在此添加控件通知处理程序代码

	int cur = m_combox.GetCurSel();
	if (drawing.get() == DrawType::DRAW_TABLES ||
		drawing.get() == DrawType::DRAW_COMPONENTS ||
		drawing.get() == DrawType::DRAW_PILLARS) {
		BOOL isOpen = FALSE;		//是否打开(否则为保存)
		CString filter = L"文件 (*.json;)|*.json||";	//文件过虑的类型
		CFileDialog openFileDlg(isOpen, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
		openFileDlg.m_ofn.lpstrTitle = "Open JSON File";
		INT_PTR result = openFileDlg.DoModal();
		if (result == IDOK) {
			CString filePath = openFileDlg.GetPathName();
			drawing.write((LPCTSTR)filePath);
		}
	}
	else if ((drawing.get() == DrawType::DRAW_BORDERS || drawing.get() == DrawType::DRAW_BLOCKS)
		&& !drawing.getFull()) {
		BOOL isOpen = FALSE;		//是否打开(否则为保存)
									//CString filter = L"文件 (*.dxf;)|*.dxf||";	//文件过虑的类型
		CString filter = L"文件 (*.json;)|*.json||";	//文件过虑的类型
		CFileDialog openFileDlg(isOpen, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
		openFileDlg.m_ofn.lpstrTitle = "Open DXF File";
		INT_PTR result = openFileDlg.DoModal();
		if (result == IDOK) {
			CString filePath = openFileDlg.GetPathName();
			drawing.write((LPCTSTR)filePath);
		}
	}
	else {
		BOOL isOpen = FALSE;		//是否打开(否则为保存)
									//CString filter = L"文件 (*.dxf;)|*.dxf||";	//文件过虑的类型
		CString filter = L"文件 (*.json;)|*.json||";	//文件过虑的类型
		CFileDialog openFileDlg(isOpen, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
		openFileDlg.m_ofn.lpstrTitle = "Open DXF File";
		INT_PTR result = openFileDlg.DoModal();
		if (result == IDOK) {
			CString filePath = openFileDlg.GetPathName();
			drawing.write((LPCTSTR)filePath);
		}
	}

}

void CDXFViewerDlg::OnCbnSelchangeComboDraw()
{
	int cur = m_combox.GetCurSel();
	// TODO: 在此添加控件通知处理程序代码
	drawing.select((DrawType)cur);
	m_DrawingWindow.RedrawWindow();
}


void CDXFViewerDlg::OnCbnSelchangeComboFile()
{
	CString file;
	int cur = m_comboxfile.GetCurSel();
	m_comboxfile.GetLBText(cur, file);
	//m_combox.Get
	drawing.select((LPCTSTR)file);
	m_DrawingWindow.RedrawWindow();
}

void CDXFViewerDlg::OnBnClickedOk2()
{
	// TODO: 在此添加控件通知处理程序代码
	//drawing.select();
	int cur = m_comboxfile.GetCurSel();
	CString file;
	m_comboxfile.GetLBText(cur, file);
	m_comboxfile.DeleteString(cur);
	drawing.close1((LPCTSTR)file);
	m_comboxfile.SetCurSel(0);
	m_DrawingWindow.RedrawWindow();
}


void CDXFViewerDlg::OnBnClickedDatasorting()
{
	// TODO: 在此添加控件通知处理程序代码
	/*if (drawing.isOpen())
	{
		drawing.SetZoomLevel(drawing.GetZoomLevel() * 1.1);
		m_DrawingWindow.RedrawWindow();
	}*/


	if (drawing.isOpen())
	{
		drawing.DataSorting();
	}
}

void CDXFViewerDlg::addFile(const CString& fileName)
{
	if (drawing.isOpen())
		drawing.Destroy();

	// Create Drawing
	if (!drawing.Create())
	{
		MessageBox("Fatal error on creating CDrawing!", "Error", MB_ICONERROR | MB_OK);
		return;
	}

	m_comboxfile.AddString(fileName);
	m_comboxfile.SelectString(0, fileName);
}

void CDXFViewerDlg::loadFile(const CString& fileName)
{

	std::string tempFileName = fileName.GetString();
	int directorLocLast = static_cast<int>(tempFileName.find_last_of("."));
	auto type = tempFileName.substr(directorLocLast);
	std::vector<MarkDrawing>markDrawing;

	if (type == ".dwg")
	{
		addFile(fileName);
		markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), tempFileName));
	}
	else if (type == ".json")
	{
		ReadTypeFile readFile;
		auto fileJsonValue = readFile.readParameterFile(tempFileName);
		auto parameteres = readFile.parserParameterFile(fileJsonValue);

		for (auto drawingMap : parameteres)
		{
			if (drawingMap.first == "detail")
			{
				for (auto it : drawingMap.second)
				{
					std::string str = "detail|" + it;
					addFile(str.c_str());
					markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), str));
				}
			}
			else if (drawingMap.first=="floorPlan")
			{
				for (auto it : drawingMap.second)
				{
					std::string str = "floorPlan|" + it;
					addFile(str.c_str());
					markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), str));
				}
			}
		}

#if 0
		Json::Value parameterFile = readTypeFile.readParameterFile(tempFileName);
		Json::Value floorPlan = readTypeFile.readFloorPlan(parameterFile);
		Json::Value profile = readTypeFile.readProfileFile(parameterFile);
		Json::Value detail = readTypeFile.readDetailFile(parameterFile);
		//平面图
		if (floorPlan.isObject())
		{

			
			Json::Value::Members member = floorPlan.getMemberNames();
			for (auto ite = member.begin(); ite != member.end(); ++ite)
			{
				break;//测试用
				Json::Value floors = floorPlan[*ite];

				for (auto it : floors)
				{
					break;

					std::string fileParameter;
					auto fileName = it.asString();
					fileParameter = "floorPlan|" + *ite + "|" + fileName;
					//loadFile(fileParameter.c_str());
					addFile(fileParameter.c_str()); 
					markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), fileParameter));

				}
				//of.close();
				//break;
			}
		}

		//剖面图
		if (profile.isObject())
		{
			//Json::Value::Members member = profile.getMemberNames();
			if (profile.isMember("hProfile"))
			{
				Json::Value profileStirrup = profile["hProfile"];
				auto stirrup = profileStirrup["stirrup"];
				

				for (auto it : stirrup)
				{
					std::string fileParameter;
					auto fileName = it.asString();
					fileParameter = "profile|hProfile|stirrup|" + fileName;
					//loadFile(fileParameter.c_str());
					addFile(fileParameter.c_str());
					markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), fileParameter));
					break;
				}
			}
			else if (profile.isMember("vProfile"))
			{
				Json::Value member = profile["vProfile"];

			}
			
		}
		//详图
		if (detail.isArray())
		{

			for (auto it : detail)
			{
				break;

				std::string fileParameter;
				auto fileName = it.asString();
				fileParameter = "detail|" + fileName;
				addFile(fileParameter.c_str());
				markDrawing.push_back(MarkDrawing(std::make_shared<Parser>(), std::make_shared<Transer>(), fileParameter));
			}
		}
		else
		{
			;
		}
#endif	
	}
	for (auto it : markDrawing)
	{
		if (!drawing.LoadFile(it))
		{
			//ProgDlg.DestroyWindow();
			MessageBox("Error while loading dxf file!", "Error", MB_ICONERROR | MB_OK);
			return;
		}
	}
	//markDrawing.clear();
}

void CDXFViewerDlg::creatWindow()
{
	CRect r;
	m_DrawingWindow.GetClientRect(r);
	drawing.InitView(0, 0, r.Width(), r.Height());
	drawing.ZoomExtents();
	m_DrawingWindow.RedrawWindow();
	if (m_upTimer) {
		this->KillTimer(m_upTimer);
	}
	m_upTimer = this->SetTimer(1, 40, NULL);
}
