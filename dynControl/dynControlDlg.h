
// dynControlDlg.h : header file
//

#pragma once

#include "cJSON.h"
#include "afxwin.h"
#include <vector>
using namespace std;

#define MAX_CTRL_ID 64

#define SLIDER_ID_OFFSET 1020
#define VIEW_ID_OFFSET (SLIDER_ID_OFFSET + MAX_CTRL_ID)
#define CHECK_ID_OFFSET (VIEW_ID_OFFSET + MAX_CTRL_ID)
#define SPEED_ID_OFFSET (CHECK_ID_OFFSET + MAX_CTRL_ID)
#define SPEEDBTN_ID_OFFSET (SPEED_ID_OFFSET + MAX_CTRL_ID)
#define LOCKBTN_ID_OFFSET (SPEEDBTN_ID_OFFSET + MAX_CTRL_ID)

#define VIEW_WIDTH 50

#define CHECK_WIDTH 150
#define VIEW_WIDTH   30
#define DEG_WIDTH 30
#define SPEED_WIDTH  50
#define SPEEDBTN_WIDTH 40
#define LOCKBTN_WIDTH 50

#define MILLISEC_WIDTH 30
#define MEDECINE_TRAY_ID 0x50
// CdynControlDlg dialog
class CdynControlDlg : public CDialogEx
{
// Construction
public:
	CdynControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DYNCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    int m_num;
    cJSON *m_configJSON;
    cJSON *m_servoJSON;
    cJSON *m_actionJSON;

    vector<int> ids;
    vector<char*> actions;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnCmd(UINT nID);
    afx_msg void OnNMReleasedcaptureSlider(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

    void CreatePositionBundle(cJSON *json, int x, int y, int w, int h);
    void CreateSpeedBundle(cJSON *json, int x, int y, int w, int h);

    void updateAllCheck(bool check);
    void ResetAll();
    void SendAll();
    void SetServo(UINT idx, int deg);
    void SendServo(UINT idx);
    void SendData(char *data, int len);

    void InitRobot();
public:
    CButton m_selectAll;
    CListBox m_predefined;
    CEdit m_editLog;
    BOOL m_syncMode;
    HANDLE m_serial;

    afx_msg void OnBnClickedWiznext();
    afx_msg void OnEnChangeEdit(UINT id);
    afx_msg void OnEnKillFocus(UINT id);
    
    afx_msg void OnLbnDblclkList1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton1();    
    
    afx_msg void OnBnClickedButton3();
    
    afx_msg void OnBnClickedCheck1();
    void log(const TCHAR *fmt, ...);
    afx_msg void OnBnClickedButton4();    
    afx_msg void OnBtnClickSetSpeed(UINT id);
    afx_msg void OnBtnClickLock(UINT id);
    CButton m_eject;
};
