
// dynControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dynControl.h"
#include "dynControlDlg.h"
#include "afxdialogex.h"
#include "atlstr.h"
#include "robot_control.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static char cmd[50];
static int cmdLen;
// CdynControlDlg dialog

void CdynControlDlg::log(const TCHAR *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    CString str;
    str.FormatV(fmt, args);

    int length = m_editLog.GetWindowTextLength();
    m_editLog.SetSel(length, length);        
    m_editLog.ReplaceSel(str);
}

CdynControlDlg::CdynControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdynControlDlg::IDD, pParent)
    , m_syncMode(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdynControlDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, ID_WIZNEXT, m_selectAll);
    DDX_Control(pDX, IDC_LIST1, m_predefined);
    DDX_Control(pDX, IDC_EDIT2, m_editLog);
    DDX_Check(pDX, IDC_CHECK1, m_syncMode);
    DDX_Control(pDX, IDC_BUTTON4, m_eject);
}

BEGIN_MESSAGE_MAP(CdynControlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY_RANGE(NM_RELEASEDCAPTURE, SLIDER_ID_OFFSET, SLIDER_ID_OFFSET + MAX_CTRL_ID, &CdynControlDlg::OnNMReleasedcaptureSlider)
    ON_CONTROL_RANGE(EN_CHANGE, VIEW_ID_OFFSET, VIEW_ID_OFFSET + MAX_CTRL_ID, &CdynControlDlg::OnEnChangeEdit)
    ON_CONTROL_RANGE(EN_KILLFOCUS, VIEW_ID_OFFSET, VIEW_ID_OFFSET + MAX_CTRL_ID, &CdynControlDlg::OnEnKillFocus)
    ON_CONTROL_RANGE(BN_CLICKED, SPEEDBTN_ID_OFFSET, SPEEDBTN_ID_OFFSET + MAX_CTRL_ID, &CdynControlDlg::OnBtnClickSetSpeed)
    ON_CONTROL_RANGE(BN_CLICKED, LOCKBTN_ID_OFFSET, LOCKBTN_ID_OFFSET + MAX_CTRL_ID, &CdynControlDlg::OnBtnClickLock)

    ON_BN_CLICKED(ID_WIZNEXT, &CdynControlDlg::OnBnClickedWiznext)
    ON_LBN_DBLCLK(IDC_LIST1, &CdynControlDlg::OnLbnDblclkList1)
    ON_BN_CLICKED(IDC_BUTTON2, &CdynControlDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON1, &CdynControlDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON3, &CdynControlDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_CHECK1, &CdynControlDlg::OnBnClickedCheck1)    
    ON_BN_CLICKED(IDC_BUTTON4, &CdynControlDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CdynControlDlg message handlers

void CdynControlDlg::OnCmd(UINT id) {

}

void CdynControlDlg::OnNMReleasedcaptureSlider(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
    int idx = id - SLIDER_ID_OFFSET;
    CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(id);    
    CEdit *edit = (CEdit*)GetDlgItem(idx + VIEW_ID_OFFSET);    
    CString str;
    str.Format(_T("%d"), slider->GetPos());
    edit->SetWindowText(str);

    UpdateData(true);
    if (m_syncMode)
        SendServo(idx);
}

TCHAR* charTotchar(char *str) {

    int len = strlen(str);
    TCHAR *strT = new TCHAR[len + 1];
    mbstowcs(strT, str, len);
    strT[len] = _T('\0');

    return strT;
}

void CdynControlDlg::SetServo(UINT idx, int deg)
{
    CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(SLIDER_ID_OFFSET + idx);
    CEdit *edit = (CEdit*)GetDlgItem(VIEW_ID_OFFSET + idx);

    slider->SetPos(deg);
    CString str;
    str.Format(_T("%d"), deg);
    edit->SetWindowText(str);

    slider->UpdateWindow();
    edit->UpdateWindow();
}

void CdynControlDlg::ResetAll()
{
    cJSON *config;

    for (int i = 0; i < cJSON_GetArraySize(m_servoJSON); i++) {
        config = cJSON_GetArrayItem(m_servoJSON, i);
        SetServo(cJSON_GetObjectItem(config, "id")->valueint,
                 cJSON_GetObjectItem(config, "init")->valueint);
    }
}

void CdynControlDlg::CreatePositionBundle(cJSON *json, int x, int y, int w, int h)
{   
    int idx = cJSON_GetObjectItem(json, "id")->valueint;
    ids.push_back(idx);

    CString str;
    char *name = cJSON_GetObjectItem(json, "name")->valuestring;

    int min = cJSON_GetObjectItem(json, "min")->valueint;
    int max = cJSON_GetObjectItem(json, "max")->valueint;

    str.Format(_T("0x%x: %s\n    [%d, %d]"), idx, charTotchar(name), min, max);

    CButton *checkBox = new CButton();
    checkBox->Create(str, 
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_TOP | BS_MULTILINE,
                     CRect(x, y, x + CHECK_WIDTH, y + h), 
                     this, 
                     CHECK_ID_OFFSET + idx);    
    checkBox->SetCheck(BST_CHECKED);

    CSliderCtrl *slider = new CSliderCtrl();
    int width = w - (CHECK_WIDTH + VIEW_WIDTH);
    int ptx = x + CHECK_WIDTH;    

    slider->Create(WS_CHILD | WS_VISIBLE | BS_TOP,
                   CRect(ptx, y, ptx + width, y + h),
                   this, 
                   SLIDER_ID_OFFSET + idx);                   
    slider->SetRange(min, max);
    slider->SetTicFreq(1);
    int initPos = cJSON_GetObjectItem(json, "init")->valueint;    
    slider->SetPos(initPos);
    
    CEdit *edit = new CEdit();    
    ptx += width;
    width = VIEW_WIDTH;

    edit->Create(WS_CHILD | WS_VISIBLE | BS_TOP, 
                 CRect(ptx, y, ptx + width, y + 30), 
                 this, 
                 VIEW_ID_OFFSET + idx);
    str.Format(_T("%d"), initPos);
    edit->SetWindowText(str);
}

void CdynControlDlg::CreateSpeedBundle(cJSON *json, int x, int y, int w, int h)
{
    int idx = cJSON_GetObjectItem(json, "id")->valueint;
    int speed = cJSON_GetObjectItem(json, "speed")->valueint;
    int ptx = x;
    int width = w - SPEEDBTN_WIDTH - LOCKBTN_WIDTH;
    CString str;

    CEdit *edit = new CEdit();
    edit->Create(WS_CHILD | WS_VISIBLE | BS_TOP,
        CRect(ptx, y, ptx + width, y + h),
        this,
        SPEED_ID_OFFSET + idx);
    str.Format(_T("%d"), speed);
    edit->SetWindowText(str);

    ptx += width;
    width = SPEEDBTN_WIDTH;
    CButton *btn = new CButton();
    btn->Create(_T("set"), WS_CHILD | WS_VISIBLE | BS_TOP,
        CRect(ptx, y, ptx + width, y + h),
        this,
        SPEEDBTN_ID_OFFSET + idx);

    ptx += width;
    width = LOCKBTN_WIDTH;
    btn = new CButton();
    btn->Create(_T("lock"), WS_CHILD | WS_VISIBLE | BS_TOP,
        CRect(ptx, y, ptx + width, y + h),
        this,
        LOCKBTN_ID_OFFSET + idx);
}

void CdynControlDlg::InitRobot()
{
    char *serial = cJSON_GetObjectItem(m_configJSON, "serial")->valuestring;
    int rate = cJSON_GetObjectItem(m_configJSON, "baudrate")->valueint;
    m_serial = serialPortOpen(serial, rate);

    CString str;
    
    str.Format(_T("%s:%d"), charTotchar(serial), rate);

    if (m_serial == INVALID_HANDLE_VALUE) {
        log(_T("can not init %s\n"), str);
    } else {
        log(_T("%s OK!\n"), str);
    }
}

BOOL CdynControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

    // Read config file
    FILE *fp = fopen("config.json", "r");
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = (char*)malloc(len + 1);
    fread(data, 1, len, fp);
    fclose(fp);
    data[len] = '\0';

    cJSON *root = cJSON_Parse(data);

    m_configJSON = cJSON_GetObjectItem(root, "config");
    m_servoJSON = cJSON_GetObjectItem(root, "servo");
    m_actionJSON = cJSON_GetObjectItem(root, "action");

    InitRobot();

    m_num = cJSON_GetArraySize(m_servoJSON);

    CRect degRect, speedRect;
    GetDlgItem(IDC_STATIC)->GetWindowRect(degRect);
    GetDlgItem(IDC_STATIC5)->GetWindowRect(speedRect);

    int step = degRect.Height() / m_num;

    for (int i = 0; i < m_num; i++) {
        cJSON *element = cJSON_GetArrayItem(m_servoJSON, i);
        CreatePositionBundle(element, degRect.left, degRect.top + step * i, degRect.Width() - 20, step);
        CreateSpeedBundle(element, speedRect.left, speedRect.top + step * i, speedRect.Width() - 10, 30);
    }
    m_selectAll.SetCheck(BST_CHECKED);    

    cJSON *action = m_actionJSON->child;
    while (action != NULL) {
        m_predefined.AddString(charTotchar(action->string));
        actions.push_back(action->string);
        action = action->next;
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CdynControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CdynControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CdynControlDlg::updateAllCheck(bool check)
{
    for (int i = 0; i < ids.size(); i++) {
        CButton* checkBox = (CButton*)GetDlgItem(CHECK_ID_OFFSET + ids[i]);
        checkBox->SetCheck(check ? BST_CHECKED : BST_UNCHECKED);
    }
}

void CdynControlDlg::OnBnClickedWiznext()
{
    // TODO: Add your control notification handler code here
    updateAllCheck(m_selectAll.GetCheck() == BST_CHECKED);
}


void CdynControlDlg::OnLbnDblclkList1()
{
    // TODO: Add your control notification handler code here
    int idx = m_predefined.GetCurSel();

    CString str;
    m_predefined.GetText(idx, str);
    str = _T("Run: ") + str;
    
    if (MessageBox(str, NULL, MB_OKCANCEL | MB_ICONWARNING) != IDOK)
        return;

    cJSON *action = cJSON_GetObjectItem(m_actionJSON, actions[idx]);
    
    int num = cJSON_GetArraySize(action);

    log(_T("\n-- start action %s with %d steps --\n"), str, num);
    for (int i = 0; i < num; i++) {
        cJSON *cmd = cJSON_GetArrayItem(action, i);
        SetServo(cJSON_GetObjectItem(cmd, "id")->valueint, 
                 cJSON_GetObjectItem(cmd, "deg")->valueint);
        SendServo(cJSON_GetObjectItem(cmd, "id")->valueint);
        Sleep(1000);
    }

    log(_T("-- end action --\n\n"), str, num);
}


void CdynControlDlg::OnBnClickedButton2()
{
    // TODO: Add your control notification handler code here
    ResetAll();
}

void CdynControlDlg::SendServo(UINT idx)
{
    CButton *check = (CButton*)GetDlgItem(CHECK_ID_OFFSET + idx);

    if (check->GetState() != BST_CHECKED)
        return;

    CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(SLIDER_ID_OFFSET + idx);    
    int deg = slider->GetPos();

    CEdit *view = (CEdit*)GetDlgItem(VIEW_ID_OFFSET + idx);
    CString str;
    view->GetWindowText(str);
    deg = _ttoi(str);

    log(_T("set servo 0x%02x, deg: %d\n"), idx, deg);
    jointAbsolutePosSet(cmd, &cmdLen, idx, &deg);
    SendData(cmd, cmdLen);
}

void CdynControlDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    UpdateData(true);

    CSliderCtrl *slider = NULL;    
    CButton *check = NULL;
    for (int i = 0; i < ids.size(); i++) {
        SendServo(ids[i]);
    }
}

void CdynControlDlg::OnEnChangeEdit(UINT id)
{
    int idx = id - VIEW_ID_OFFSET;
    CString str;
    CEdit *edit = (CEdit*) GetDlgItem(id);    
    edit->GetWindowText(str);
    int deg = _ttoi(str);
    CSliderCtrl *ctrl = (CSliderCtrl*)GetDlgItem(idx + SLIDER_ID_OFFSET);
    ctrl->SetPos(deg);
}

void CdynControlDlg::OnEnKillFocus(UINT id)
{
    int idx = id - VIEW_ID_OFFSET;

    UpdateData(true);
    if (m_syncMode)
        SendServo(idx);
}

void CdynControlDlg::OnBnClickedButton3()
{
    // TODO: Add your control notification handler code here
    m_editLog.SetWindowText(_T(""));
}


void CdynControlDlg::OnBnClickedCheck1()
{
    // TODO: Add your control notification handler code here
    UpdateData(true);
    if (m_syncMode) {
        MessageBox(_T("Immediately send value to robot"));
        log(_T("enter sync mode\n"));
    } else {
        MessageBox(_T("Press GO to send value to robot"));
        log(_T("quit sync mode\n"));
    }
}


void CdynControlDlg::OnBnClickedButton4()
{
    // TODO: Add your control notification handler code here    
    CString str;
    m_eject.GetWindowText(str);
    int deg;
    if (str == _T("Eject")) {
        log(_T("eject medicine tray\n"));
        deg = 90;
        jointAbsolutePosSet(cmd, &cmdLen, MEDECINE_TRAY_ID, &deg);
        SendData(cmd, cmdLen);
        m_eject.SetWindowText(_T("Close"));
    }
    else {
        log(_T("close medicine tray\n"));
        deg = -90;
        jointAbsolutePosSet(cmd, &cmdLen, MEDECINE_TRAY_ID, &deg);
        SendData(cmd, cmdLen);
        m_eject.SetWindowText(_T("Eject"));
    }
    
}

void CdynControlDlg::SendData(char *data, int len) {
    CString str;
    
    str = _T(" ");
    for (int i = 0; i < len; i++) {
        CString temp;
        temp.Format(_T("0x%02x "), (unsigned char)(data[i]));
        str += temp;
    }

    log(_T("send [ %s ] to robot\n"), str);

    sendData(m_serial, data, len);
}

void CdynControlDlg::OnBtnClickSetSpeed(UINT id)
{
    int idx = id - SPEEDBTN_ID_OFFSET;
    CEdit *edit = (CEdit*)GetDlgItem(SPEED_ID_OFFSET + idx);
    CString str;
    edit->GetWindowText(str);
    int speed = _ttoi(str);
    jointAbsoluteSpeedSet(cmd, &cmdLen, idx, &speed);    
    log(_T("set 0x%02x speed %d\n"), idx, speed);
    SendData(cmd, cmdLen);
}

void CdynControlDlg::OnBtnClickLock(UINT id)
{
    int idx = id - LOCKBTN_ID_OFFSET;
    CButton *btn = (CButton*)GetDlgItem(id);
    CString str;
    btn->GetWindowText(str);

    if (str == _T("unlock")) {
        btn->SetWindowText(_T("lock"));
        log(_T("unlock 0x%02x\n"), idx);
        jointPositionUnlock(cmd, &cmdLen, idx);
        SendData(cmd, cmdLen);

    } else {
        btn->SetWindowText(_T("unlock"));
        log(_T("lock 0x%02x\n"), idx);
        jointPositionLock(cmd, &cmdLen, idx);
        SendData(cmd, cmdLen);
    }
}