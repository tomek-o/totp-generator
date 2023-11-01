//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormMain.h"
#include "FormAbout.h"
#include "FormSettings.h"
#include "Settings.h"
#include "LogUnit.h"
#include "Log.h"
#include "Clipbrd.hpp"
#include <hmac/hmac.h>
#include <common/base32.h>
#include <time.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------

namespace
{

int64_t GetUnixTime(void)
{
	FILETIME ft;
	LARGE_INTEGER li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
	* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	int64_t ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000000LL;

	return ret;
}

}

__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner),
	outdatedTime(0)
{
#ifdef ACCEPT_WM_DROPFILES
	// inform OS that we accepting dropping files
	DragAcceptFiles(Handle, True);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
	AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
	appSettings.Read(asConfigFile);
	if (this->BorderStyle != bsSingle)
	{
		this->Width = appSettings.frmMain.iWidth;
		this->Height = appSettings.frmMain.iHeight;
	}
	this->Top = appSettings.frmMain.iPosY;
	this->Left = appSettings.frmMain.iPosX;
	if (appSettings.frmMain.bAlwaysOnTop)
		this->FormStyle = fsStayOnTop;
	else
		this->FormStyle = fsNormal;
	if (appSettings.frmMain.bWindowMaximized)
		this->WindowState = wsMaximized;
	if (appSettings.Logging.bLogToFile)
		CLog::Instance()->SetFile(ChangeFileExt(Application->ExeName, ".log").c_str());
	else
		CLog::Instance()->SetFile("");
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
	appSettings.frmMain.bWindowMaximized = (this->WindowState == wsMaximized);
	if (!appSettings.frmMain.bWindowMaximized)
	{
		// these values are meaningless is wnd is maximized
		appSettings.frmMain.iWidth = this->Width;
		appSettings.frmMain.iHeight = this->Height;
		appSettings.frmMain.iPosY = this->Top;
		appSettings.frmMain.iPosX = this->Left;
	}

	appSettings.Write(asConfigFile);

	CanClose = true;	
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::actShowAboutExecute(TObject *Sender)
{
	frmAbout->ShowModal();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowSettingsExecute(TObject *Sender)
{
	frmSettings->appSettings = &appSettings;
	frmSettings->ShowModal();
	if (appSettings.frmMain.bAlwaysOnTop)
		this->FormStyle = fsStayOnTop;
	else
		this->FormStyle = fsNormal;
	if (appSettings.Logging.bLogToFile)
		CLog::Instance()->SetFile(ChangeFileExt(Application->ExeName, ".log").c_str());
	else
		CLog::Instance()->SetFile("");
	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormShow(TObject *Sender)
{
    static bool once = false;
    if (!once)
    {
		once = true;
		frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);		
		CLog::Instance()->SetLevel(E_LOG_TRACE);
		CLog::Instance()->callbackLog = frmLog->OnLog;
	}
	LOG("Application started\n");
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormDestroy(TObject *Sender)
{
	CLog::Instance()->Destroy();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowLogExecute(TObject *Sender)
{
	if (!frmLog->Visible)
		frmLog->Show();
	frmLog->BringToFront();
}
//---------------------------------------------------------------------------

#ifdef ACCEPT_WM_DROPFILES
void __fastcall TfrmMain::WMDropFiles(TWMDropFiles &message)
{
    AnsiString FileName;
    FileName.SetLength(MAX_PATH);

	int Count = DragQueryFile((HDROP)message.Drop, 0xFFFFFFFF, NULL, MAX_PATH);

	if (Count > 1)
		Count = 1;

	for (int index = 0; index < Count; ++index)
	{
		FileName.SetLength(DragQueryFile((HDROP)message.Drop, index,FileName.c_str(), MAX_PATH));
//		appSettings.Editor.asDefaultDir = ExtractFileDir(FileName);
		AppOpenFile(FileName);
	}
	DragFinish((HDROP) message.Drop);
}
#endif

void __fastcall TfrmMain::btnGenerateClick(TObject *Sender)
{
	edOtp->Clear();

	uint8_t passBuf[1024];
	int ret;
	if (appSettings.Totp.password.Length())
		ret = base32_decode(passBuf, sizeof(passBuf), reinterpret_cast<const unsigned char*>(&appSettings.Totp.password[1]), appSettings.Totp.password.Length());
	else
	{
		ret = 0;
		StatusBar->SimpleText = "No password set - open settings!";
		return;
	}
	if (ret < 0)
	{
		MessageBox(this->Handle, "Failed to decode password as base32.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}

	uint8_t hmac_result[20] = {0};

	uint64_t x = 30; /* step in seconds */
	uint64_t t; /* number of steps */

	int64_t now = GetUnixTime();

	LOG("Generating, current time = %lld\n", now);

	t = now / x;
	outdatedTime = (t + 1) * x;

	size_t hmac_result_len = sizeof(hmac_result);

	uint8_t tbytes[8];
	memset(tbytes, 0, sizeof(tbytes));
	for (unsigned int i = 0 ; i < sizeof(t) ; i++) {
		tbytes[7 - i] = static_cast<uint8_t>(t & 0xFF);
		t = t >> 8;
	}
	hmac_sha1(passBuf, ret, tbytes, sizeof(tbytes), hmac_result, &hmac_result_len);

	/* dynamically truncate hash */
	uint64_t offset = hmac_result[19] & 0x0f;
	uint32_t bin_code =
		((hmac_result[offset] & 0x7f) << 24) |
		((hmac_result[offset + 1] & 0xff) << 16) |
		((hmac_result[offset + 2] & 0xff) << 8) |
		(hmac_result[offset + 3] & 0xff);

	/* truncate code to 6 digits */
	int TODO__TRUNCATION_SETTING;
	uint32_t totp = bin_code % 1000000;

	AnsiString text;
	text.sprintf("%06d", totp);

	edOtp->Text = text;
}
//---------------------------------------------------------------------------


void __fastcall TfrmMain::btnCopyToClipboardClick(TObject *Sender)
{
	Clipboard()->AsText = edOtp->Text;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::tmrRefreshOtpTimer(TObject *Sender)
{
	int64_t now = GetUnixTime();

	if (outdatedTime != 0)
	{
		int delta = outdatedTime - now;
		AnsiString text;
		if (delta >= 0)
		{
			text.sprintf("OTP valid for %d second(s)", delta);
		}
		else
		{
			text.sprintf("OTP outdated %d second(s) ago", -delta);
		}
		StatusBar->SimpleText = text;
	}

	{
		if (chbRefreshAutomatically->Checked == false)
			return;
		if (now >= outdatedTime)
		{
			btnGenerateClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------

