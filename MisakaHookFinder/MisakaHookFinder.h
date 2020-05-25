#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MisakaHookFinder.h"
#include <windows.h>
#include <TlHelp32.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include "texthost.h"
#include "HookResultWindow.h"
#include <QClipboard>

class MisakaHookFinder : public QMainWindow
{
    Q_OBJECT

public:
    MisakaHookFinder(QWidget *parent = Q_NULLPTR);

private:
    Ui::MisakaHookFinderClass ui;
    
    FindHooks* fh;
    ProcessEvent* pe;
    OnCreateThread* oct;
    OnRemoveThread* ort;
    OutputText* opt;

    

    static void ProcessEventHandle(DWORD processId);
    static void OnCreateThreadHandle(int64_t thread_id, DWORD processId, uint64_t addr, uint64_t context, uint64_t subcontext, LPCWSTR name, LPCWSTR hookcode);
    static void OnRemoveThreadHandle(int64_t thread_id);
    static void OutputTextHandle(int64_t thread_id, LPCWSTR output);
    static void FindHooksHandle();

    //打印到UI中的编辑框的方法
    static void PrintToUI(int editboxID, QString str);

    //添加一个HOOK方法到下拉框
    static void AddHookFunc(QString str,int data);

    //打开搜索结果窗口
    static void OpenResultWin();

    //更新剪贴板内容
    static void FlushClipboard(QString str);

    void emitConsoleBoxSignal(QString str);
    void emitGameTextBoxSignal(QString str);
    void emitHookFunComboxSignal(QString str, int data);
    void emitResultWinSignal();
    void emitClipboardSignal(QString str);

private:
    void GetProcessesList();
    static QString TextThreadString(int64_t thread_id, DWORD processId, uint64_t addr, uint64_t context, uint64_t subcontext, LPCWSTR name, LPCWSTR hookcode);
    uint64_t GetAddressByHookComboxContent(QString str);
    bool isHooking;//是否正在注入的状态
    
signals:
    void onConsoleBoxContextChange(QString str);
    void onGameTextBoxContextChange(QString str);
    void onHookFunComboxChange(QString str,int data);
    void onClipboardChange(QString str);
    void onOpenResWin();

public slots:
    void AttachProcessBtn_Click();
    void CustomHookCodeBtn_Click();
    void SearchForTextBtn_Click();
    void SearchForHookBtn_Click();
    void CopyHookCodeBtn_Click();
    void ClipbordFlushBtn_Click();
    void RemoveHookBtn_Click();
    void HookFunCombox_currentIndexChanged(int);

    void ConsoleBox_Change(QString str);
    void GameTextBox_Change(QString str);
    void HookFunCombox_Change(QString str,int data);
    void Clipboard_Change(QString str);
    void Reswin_Open();

private:
    void closeEvent(QCloseEvent* event);
};
