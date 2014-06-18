//---------------------------------------------------------------------------
#ifndef GPRSWatchDogUH
#define GPRSWatchDogUH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <SvcMgr.hpp>
#include <vcl.h>
#include <ExtCtrls.hpp>
#include <Ras.h>
#include <winsock.h>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdIcmpClient.hpp>
#include <IdRawBase.hpp>
#include <IdRawClient.hpp>
//---------------------------------------------------------------------------
//��������� ��������� IPINFO
#ifndef __IPINFO_DEFINED__
#define __IPINFO_DEFINED__

typedef struct _IPINFO
{
 char Ttl;
 char Tos;
 char IPFlags;
 char OptSize;
 char Options;
} IPINFO;

#endif //_IPINFO

//��������� ��������� ICMPECHO
#ifndef __ICMPECHO_DEFINED__
#define __ICMPECHO_DEFINED__

typedef struct _ICMPECHO
{
  DWORD  Source;
  DWORD  Status;
  DWORD  RTTime;
  DWORD  DataSize;
  DWORD  Reserved;
  Variant *pData;
  IPINFO i_ipinfo;

} ICMPECHO;

#endif //_ICMPECHO

typedef IPINFO *LPIPINFO;
typedef ICMPECHO TIcmpEcho;

struct TWatchDogSettings
{
 AnsiString LogFile;    //���� � ��� ��� �����
 AnsiString DebugFile; //���� � ��� ����������� �����
 UINT LogMaxRow; //������������ ���-�� ����� � ��� �����
 UINT Connects; //������� ���������� �� �������
 UINT Calls; //��� -�� ������� �� �������
 AnsiString RAC; //��� ����������� � ����� "�������� ������"
 AnsiString SERV; //������ ������� ����� ���������
};

struct TWatchDogRasCon
{
 AnsiString RAC; //��� ����������� � ����� "�������� ������"
 RASCONN rasConn[100]; //���������� ��  �������� �����������
 HRASCONN hrasconn; //����� �����������
 RASCONNSTATUS rasConStatus; //����� ���������� �����������
 DWORD ConBufferSize; //������ �������
 DWORD Connections; //���������� ����������� ���������� ������� ��������
};

struct TWatchDogConnState
{
 UINT AMsg;
 RASCONNSTATE AState;
 DWORD Error;
};

typedef RASDIALPARAMS TRasDialParams;

class TWatchDog : public TService
{
__published:    // IDE-managed Components
        TIdIcmpClient *IdIcmpClient;
        void __fastcall ServiceStart(TService *Sender, bool &Started);
        void __fastcall ServiceContinue(TService *Sender, bool &Continued);
        void __fastcall ServicePause(TService *Sender, bool &Paused);
        void __fastcall ServiceStop(TService *Sender, bool &Stopped);
private:        // User declarations

public:         // User declarations
      	__fastcall TWatchDog(TComponent* Owner);
	TServiceController __fastcall GetServiceController(void);

	friend void __stdcall ServiceController(unsigned CtrlCode);
};
//---------------------------------------------------------------------------
extern PACKAGE TWatchDog *WatchDog;
//---------------------------------------------------------------------------
class TWatchDogThread: public TThread
{
 private:
  TMemoryStream *Stream;
  TWatchDogSettings *WatchDogSettings;
  TRasDialParams *RasDialParams;
  TWatchDogConnState *WatchDogConnState;
  TWatchDogRasCon *WatchDogRasCon;
  bool Connected;
  bool ConnectError;
 protected:
  void __fastcall Execute(); //����� ���������� ������
  bool __fastcall VerifyActiveConnection();
  bool __fastcall TestActiveConnection();
  bool __fastcall RestartConnection();
  void __fastcall TimerOverflow();
 public:
  TIdIcmpClient *IdIcmpClient;
  __fastcall TWatchDogThread(bool CreateSuspended); //�����������
  AnsiString __fastcall RasGetStatusString(RASCONNSTATE ConnState,DWORD Error);
  void __fastcall SetWDConnStateAMsg(UINT unMsg);
  void __fastcall SetWDConnStateAState(RASCONNSTATE rasconnstate);
  void __fastcall SetWDConnStateError(DWORD Error);
  void __fastcall AddToLogFile(AnsiString msg);
};
#endif
