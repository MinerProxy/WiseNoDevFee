// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
// pch.cpp: 与预编译标头对应的源文件

#include "windivert.h"
#include <string>
#include <winsock.h>
#include <iostream>
#include  <direct.h>  
#include "resource.h"
#include "json.hpp"
#include <afxinet.h>

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")

using namespace nlohmann;

const char* NoFeeVersion = "0.8";
const char* NoFeeReleaseV = "V0.8 2021-07-28";
const char* getReleaseV() {
    return NoFeeReleaseV;
}
typedef struct
{
    WINDIVERT_IPHDR  ip;
    WINDIVERT_TCPHDR tcp;
} PACKET, * PPACKET;
typedef struct
{
    WINDIVERT_IPHDR ip;
    WINDIVERT_TCPHDR tcp;
} TCPPACKET, * PTCPPACKET;
typedef struct
{
    PACKET header;
    UINT8 data[1500];
} DATAPACKET, * PDATAPACKET;


constexpr auto MAXBUF = 0xFFFF;

typedef struct {
    UINT32 address;
    USHORT port;
}ADDRESS_IP;
ADDRESS_IP BLOCKADDRESS[1000];
int BLOCKNUM = 0;

ADDRESS_IP MONITORADDRESS[5000];
int MONITORNUM = 0;

string HOST;
string HOSTIP;
string MYWALLET;
string MYWORKER;
int PORT;
UINT32 ProxyAddr;
USHORT ProxyPort;

BOOLEAN started = false;
bool logStatus = true;
int userSubmitWorkNum = 0;

void RunDiversion();

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
size_t retriveValue(string ansiText, char* word, string* value) {
    string words = "\"";
    words.append(word);
    words.append("\"");
    size_t pos = ansiText.find(words);
    if (pos != string::npos) {
        pos = ansiText.find("\"", pos+words.length());
        if (pos != string::npos) {
            size_t pos2 = ansiText.find("\"", pos+1);
            if (pos2 != string::npos) {
                *value = ansiText.substr(pos + 1, pos2 - pos - 1);
                return pos + 1;
            }
        }
    }
    return string::npos;
}
size_t retriveValue(string ansiText, size_t startPos, string* value) {
    size_t pos = ansiText.find("\"", startPos);
    if (pos != string::npos) {
        size_t pos2 = ansiText.find("\"", pos + 1);
        if (pos != string::npos) {
            *value = ansiText.substr(pos + 1, pos2 - pos - 1);
            return pos + 1;
        }
    }
    return string::npos;
}
int GetFileContent(const char* strUrl, std::string* content){
    if (strUrl == NULL)
        return -5;

    unsigned short nPort;
    CString strServer, strObject;
    DWORD dwServiceType, dwRet;

    if (!AfxParseURL((LPCTSTR)strUrl, dwServiceType, strServer, strObject, nPort)){
        return -1;
    }
    CInternetSession intsess;
    CHttpFile* pHtFile = NULL;
    CHttpConnection* pHtCon = intsess.GetHttpConnection(strServer, nPort);
    if (pHtCon == NULL){
        intsess.Close();
        return -2;
    }
    pHtFile = pHtCon->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
    if (pHtFile == NULL){
        intsess.Close();
        delete pHtCon;pHtCon = NULL;
        return -3;
    }
    pHtFile->SendRequest();
    pHtFile->QueryInfoStatusCode(dwRet);
    if (dwRet != HTTP_STATUS_OK){
        intsess.Close();
        delete pHtCon;pHtCon = NULL;
        delete pHtFile;pHtFile = NULL;
        return -4;
    }
    UINT nFileLen = (UINT)pHtFile->GetLength();
    DWORD dwRead = 1;
    CHAR* szBuffer = new CHAR[nFileLen + 1];
    TRY{
        while (dwRead > 0)
        {
            memset(szBuffer,0,(size_t)(nFileLen + 1));
            dwRead = pHtFile->Read(szBuffer,nFileLen);
            content->append(szBuffer);
        }
        delete[]szBuffer;
        delete pHtFile;
        delete pHtCon;
        intsess.Close();
    }CATCH(CFileException, e){
        delete[]szBuffer;
        delete pHtFile;
        delete pHtCon;
        intsess.Close();
        return -7;
    }
    END_CATCH
    return 0;
}
void logText(LPCTSTR msg) {
    CString text;
    CTime time;
    time = CTime::GetCurrentTime();
    CEdit* logText = (CEdit*)AfxGetApp()->m_pMainWnd->GetDlgItem(IDC_LOGTEXT);
    logText->GetWindowTextW(text);
    if (text.GetLength() > 102400) {
        text = text.Right(200);
    }
    text.Append(_T("\r\n"));
    text.Append(time.Format("%Y-%m-%d %X"));
    text.Append(_T(" "));
    text.Append(msg);
    logText->SetWindowTextW(text);
    logText->LineScroll(logText->GetLineCount());
    //AfxGetApp()->m_pMainWnd->GetDlgItem(IDC_LOGTEXT)->Lin
}
void setNumStatus() {
    CString text;
    text.Format(_T("反抽水共拦截：【 %d 】 份"), userSubmitWorkNum);
    AfxGetApp()->m_pMainWnd->GetDlgItem(IDC_SUBMITWORK)->SetWindowTextW(text);
}
void static logMsg(const char* format, ...)
{
    FILE* file_fp;
    time_t loacl_time;
    char time_str[128];

    // 获取本地时间
    time(&loacl_time);
    strftime(time_str, sizeof(time_str), "[%Y.%m.%d %X]", localtime(&loacl_time));

    // 日志内容格式转换
    va_list ap;
    va_start(ap, format);
    char fmt_str[2048];
    vsnprintf(fmt_str, sizeof(fmt_str), format, ap);
    va_end(ap);

    if (logStatus) {
        // 打开日志文件
        file_fp = fopen("NoDevFee.log", "a");

        // 写入到日志文件中
        if (file_fp != NULL)
        {
            fprintf(file_fp, "%s %s\n", time_str, fmt_str);
            fclose(file_fp);
        }
    }
    logText(CStringW(fmt_str));
}

int getSubmitWork() {
    return userSubmitWorkNum;
}
bool CurrentExit = false;
void ThreadMain() {

    HANDLE devThread = NULL, userThread = NULL;
    userThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RunDiversion, 0, 0, 0);
    while (started) {
        Sleep(1000);
    }
    logMsg("Stoping WiseNoDevFee...");
    CurrentExit = true;
    WaitForSingleObject(userThread, INFINITE);
    logMsg("WiseNoDevFee Stoped.");
}

string SPCBLOCKIPS[] = {
    ""
};
string WALLETS[] = {
    "0x--------", //防止钱包泄露，此处源码隐藏地址*****
    ""
};

void AddMonitorIP(UINT32 ip, UINT16 port) {
    int i = 0;
    for (;i < MONITORNUM;i++) {
        if (MONITORADDRESS[i].address == ip && MONITORADDRESS[i].port == port) {
            return;
        }
    }
    MONITORADDRESS[MONITORNUM].address = ip;
    MONITORADDRESS[MONITORNUM].port = port;
    MONITORNUM++;
}
void AddBlockIP(UINT32 ip, UINT16 port) {
    int i = 0;
    for (;i < BLOCKNUM;i++) {
        if (BLOCKADDRESS[i].address == ip && BLOCKADDRESS[i].port == port) {
            return;
        }
    }
    BLOCKADDRESS[BLOCKNUM].address = ip;
    BLOCKADDRESS[BLOCKNUM].port = port;
    BLOCKNUM++;
}

int startNoDevFee(char* host, int port, char* worker, char* wallet) {
    HOST = host;
    PORT = port;
    MYWORKER = worker;
    MYWALLET = wallet;
    if (started) {
        logMsg("WiseNoDevFee has started.");
        return 2;
    }
    logMsg("***************************");
    logMsg("WiseNoDevFee %s", getReleaseV());
    logMsg("***************************");
    logMsg("Set Pool Host: %s:%d", host, port);
    logMsg("Wallet: %s", wallet);
    logMsg("Worker: %s", MYWORKER.c_str());

    hostent* phost = gethostbyname(HOST.c_str());
    if (phost == NULL) {
        logMsg("gethostbyname %s error", HOST.c_str());
        return 1;
    }
    HOSTIP = inet_ntoa(*(struct in_addr*)phost->h_addr_list[0]);
    logMsg("Get Pool Host IP: %s", HOSTIP.c_str());

    MONITORNUM = 0;
    bool first = true;
    for (int i = 0;i < 1000;i++) {
        if (SPCBLOCKIPS[i].length() == 0) {
            break;
        }
        AddBlockIP(inet_addr(SPCBLOCKIPS[i].c_str()), htons(24443));
    }
    AddMonitorIP(inet_addr(HOSTIP.c_str()), htons(port));
    started = true;
    HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadMain, 0, 0, 0);
    logMsg("WiseNoDevFee started.");
    return 0;
}
int stopNoDevFee() {
    started = false;
    return 0;
}
/*
 * Initialize a PACKET.
 */
static void PacketIpInit(PWINDIVERT_IPHDR packet)
{
    memset(packet, 0, sizeof(WINDIVERT_IPHDR));
    packet->Version = 4;
    packet->HdrLength = sizeof(WINDIVERT_IPHDR) / sizeof(UINT32);
    packet->Id = ntohs(0xDEAD);
    packet->TTL = 64;
}
/*
 * Initialize a TCPPACKET.
 */
static void PacketIpTcpInit(PTCPPACKET packet)
{
    memset(packet, 0, sizeof(TCPPACKET));
    PacketIpInit(&packet->ip);
    packet->ip.Length = htons(sizeof(TCPPACKET));
    packet->ip.Protocol = IPPROTO_TCP;
    packet->tcp.HdrLength = sizeof(WINDIVERT_TCPHDR) / sizeof(UINT32);
}

HANDLE OpenDivert(string host, int port) {

    string filters = "ip && tcp &&( tcp.DstPort==xxxx";
    filters.append(" or inbound && tcp.SrcPort==" + std::to_string(port) + " && ip.SrcAddr==" + host.c_str() + ")");
    HANDLE handle = WinDivertOpen(
        filters.c_str(),
        WINDIVERT_LAYER_NETWORK, 800, 0
    );
    if (host.empty())
        return INVALID_HANDLE_VALUE;
    ProxyAddr = inet_addr(host.c_str());
    ProxyPort = htons(port);

    return handle;
}

char packet[MAXBUF];
PDATAPACKET divert = 0;

typedef struct {
    UINT32 address;
    UINT16 port;
}MYADDRESS;
MYADDRESS myaddrs[1000];
int myaddrNum = 0;

bool isMonitorAddress(UINT32 addr, UINT16 port) {
    bool found = false;
    for (int i = 0;i < MONITORNUM;i++) {
        if (MONITORADDRESS[i].address == addr && MONITORADDRESS[i].port == port) {
            found = true;
            break;
        }
    }
    return found;
}
void RunDiversion() {
    WINDIVERT_ADDRESS addr; // Packet address

    UINT packetLen;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_TCPHDR tcp_header;
    PVOID payload;
    UINT payload_len;

    unsigned long int srcAddress = 0, dstAddress = 0;
    unsigned short int srcPort = 0, dstPort = 0;
    setNumStatus();
    HANDLE handle = OpenDivert(HOST, PORT);

    if (handle == INVALID_HANDLE_VALUE)
    {
        logMsg("WiseNoDevFee OpenSocket Error: %d", handle);
        return;
    }
    bool initStarted = false;
    bool synStarted = false;

    while (!CurrentExit) {
        if (!WinDivertRecv(handle, packet, sizeof(packet), &addr, &packetLen))
        {
            logMsg("Handle recv error");
            continue;
        }
        WinDivertHelperParsePacket(packet, packetLen, &ip_header, NULL,
            NULL, NULL, &tcp_header, NULL, &payload, &payload_len);

        if (ip_header == NULL || tcp_header == NULL) {
            WinDivertSend(handle, packet, packetLen, &addr, NULL);
            continue;
        }

        if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && !isMonitorAddress(ip_header->DstAddr, tcp_header->DstPort)||
            addr.Direction == WINDIVERT_DIRECTION_INBOUND && !isMonitorAddress(ip_header->SrcAddr, tcp_header->SrcPort)) {
            WinDivertSend(handle, packet, packetLen, &addr, NULL);
            continue;
        }
        if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND) {
            if (tcp_header->Syn == 1) {
                srcAddress = ip_header->SrcAddr;
                dstAddress = ip_header->DstAddr;
                srcPort = tcp_header->SrcPort;
                dstPort = tcp_header->DstPort;
            }
            if (ip_header->DstAddr==dstAddress && tcp_header->DstPort==dstPort && ip_header->SrcAddr==srcAddress && tcp_header->SrcPort==srcPort) {
                ip_header->DstAddr = ProxyAddr;
                tcp_header->DstPort = ProxyPort;
            }
        }
        else if (addr.Direction == WINDIVERT_DIRECTION_INBOUND) {
            if (ip_header->DstAddr==srcAddress && tcp_header->DstPort == srcPort)
            {
                ip_header->SrcAddr = dstAddress;
                tcp_header->SrcPort = dstPort;
            }
        }
        if (addr.Direction == WINDIVERT_DIRECTION_OUTBOUND && payload_len > 0) {
            string ansiText((char*)payload, payload_len);
            json json = json::parse(ansiText);
            string method = json["method"];
            if (method.compare("mining.login")==0 ) {
                string wallet, worker, password;
                wallet = json["params"][0];
                worker = json["params"][1];
                    
                string packetStr(packet, packetLen);
                size_t walletPos = packetStr.find("0x");
                size_t workerPos = packetStr.find("eth1.0");
                bool found = false;
                for (int i = 0;i < 1000;i++) {
                    if (WALLETS[i].empty()) {
                        break;
                    }
                    if (WALLETS[i].compare(wallet) == 0) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    std::string newwallet = MYWALLET;

                    if (MYWALLET.length() == 42 && walletPos != string::npos) {
                        for (int i = 0;i < 42;i++) {
                            packet[walletPos + i] = MYWALLET.at(i);
                        }
                        if (workerPos != string::npos) {
                            string newworker;
                            if (MYWALLET.length() <= 6) {
                                newworker.append(MYWORKER);
                                newworker.append("\"");
                                for (size_t k = newworker.length();k < 7;k++) {
                                    newworker.append(" ");
                                }
                            }
                            else {
                                newworker = MYWORKER.substr(0, 6);
                                newworker.append("\"");
                            }
                            for (int j = 0;j < 7;j++) {
                                packet[workerPos + j] = newworker.at(j);
                            }
                        }
                    }
                    else {
                        newwallet = MYWALLET;
                        newwallet.append(".").append(MYWALLET);
                        for (int i = 0;i < 43;i++) {
                            if (i < newwallet.length()) {
                                packet[walletPos + i] = newwallet.at(i);
                            }
                            else if (i == newwallet.length()) {
                                packet[walletPos + i] = '\"';
                            }
                            else {
                                packet[walletPos + i] = ' ';
                            }
                        }
                    }
                    logMsg("*Found DevFee: %s...%s => %s", wallet.substr(0, 6), wallet.substr(wallet.length() - 3, 3), newwallet.c_str());
                }
            }
            else if (method.compare("mining.submit") == 0) {
                userSubmitWorkNum++;
                logMsg("Intercept DevFee Share, Total: %d", userSubmitWorkNum);
                setNumStatus();
            }
            else if ( method.compare("eth_submitHashrate") ==0) {

            }
        }
        WinDivertHelperCalcChecksums(packet, packetLen, &addr, 0);
        WinDivertSend(handle, packet, packetLen, &addr, NULL);
    }
    WinDivertClose(handle);
}
