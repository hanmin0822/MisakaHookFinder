#include "MisakaHookFinder.h"

MisakaHookFinder* s_this = nullptr;

static bool isOpenClipboard = false;
static bool isCustomAttach = false;
static QString customHookcode = "";
static uint64_t currentFun = -1;

MisakaHookFinder::MisakaHookFinder(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	s_this = this;

	isHooking = false;
	
	GetProcessesList();

	fh = FindHooksHandle;

	pe = ProcessEventHandle;
	oct = OnCreateThreadHandle;
	ort = OnRemoveThreadHandle;
	opt = OutputTextHandle;

	connect(ui.AttachBtn, SIGNAL(clicked()), this, SLOT(AttachProcessBtn_Click()));
	connect(ui.CustomHookCodeBtn, SIGNAL(clicked()), this, SLOT(CustomHookCodeBtn_Click()));
	connect(ui.SearchForTextBtn, SIGNAL(clicked()), this, SLOT(SearchForTextBtn_Click()));
	connect(ui.SearchForHookBtn, SIGNAL(clicked()), this, SLOT(SearchForHookBtn_Click()));
	connect(ui.CopyHookCodeBtn, SIGNAL(clicked()), this, SLOT(CopyHookCodeBtn_Click()));
	connect(ui.ClipbordFlushBtn, SIGNAL(clicked()), this, SLOT(ClipbordFlushBtn_Click()));
	connect(ui.RemoveHookBtn, SIGNAL(clicked()), this, SLOT(RemoveHookBtn_Click()));
	connect(ui.HookFuncCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(HookFunCombox_currentIndexChanged(int)));
	
	connect(s_this, SIGNAL(onConsoleBoxContextChange(QString)), this, SLOT(ConsoleBox_Change(QString)));
	connect(s_this, SIGNAL(onGameTextBoxContextChange(QString)), this, SLOT(GameTextBox_Change(QString)));
	connect(s_this, SIGNAL(onHookFunComboxChange(QString,int)), this, SLOT(HookFunCombox_Change(QString,int)));
	connect(s_this, SIGNAL(onOpenResWin()), this, SLOT(Reswin_Open()));
	connect(s_this, SIGNAL(onClipboardChange(QString)), this, SLOT(Clipboard_Change(QString)));
	connect(s_this, SIGNAL(onRemoveHookFun(uint64_t)), this, SLOT(HookFun_Remove(uint64_t)));
}

/*************************
	退出事件
**************************/
void MisakaHookFinder::closeEvent(QCloseEvent* e) {
	
	if (QMessageBox::question(NULL, QStringLiteral("确认退出"), QStringLiteral("您确认要退出MisakaHookFinder吗？"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		if (isHooking == true) {
			QVariant var = ui.ProcessesCombox->currentData();
			DWORD pid = (DWORD)var.toInt();
			TextHost::DetachProcess(pid);
		}
	}
	else {
		e->ignore();
	}
}

/*********************************
	获取系统进程列表并显示到组合框中
**********************************/
void MisakaHookFinder::GetProcessesList() {
	ui.ProcessesCombox->clear();

	PROCESSENTRY32 pe32;
	// 在使用这个结构之前，先设置它的大小
	pe32.dwSize = sizeof(pe32);

	// 给系统内的所有进程拍一个快照
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		QMessageBox::information(NULL, QStringLiteral("错误"), QStringLiteral("无法搜索系统进程，请尝试使用管理员开启本软件!"));
		return;
	}

	// 遍历进程快照，轮流显示每个进程的信息
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		QString str1 = QString::fromWCharArray(pe32.szExeFile) + "-" + QString::number((int)pe32.th32ProcessID);
		ui.ProcessesCombox->addItem(str1, (int)pe32.th32ProcessID);

		bMore = ::Process32Next(hProcessSnap, &pe32);
	}

}

void MisakaHookFinder::AttachProcessBtn_Click() {

	QVariant var = ui.ProcessesCombox->currentData();
	DWORD pid = (DWORD)var.toInt();

	if (isHooking == false) {
		ui.ProcessesCombox->setEnabled(false);

		ui.ConsoleTextBox->appendPlainText(QStringLiteral("注入进程PID:") + var.toString());
		ui.AttachBtn->setText(QStringLiteral("结束注入"));

		TextHost::TextHostInit(pe, pe, oct, ort, opt);
		TextHost::InjectProcess(pid);

		isHooking = true;
		isCustomAttach = false;
	}
	else {
		TextHost::DetachProcess(pid);

		ui.ConsoleTextBox->appendPlainText(QStringLiteral("取消注入进程PID:") + var.toString());
		ui.AttachBtn->setText(QStringLiteral("注入进程"));

		ui.ProcessesCombox->setEnabled(true);
		isHooking = false;
		isCustomAttach = false;
	}
}


void MisakaHookFinder::CustomHookCodeBtn_Click() {
	if (isHooking == false) {
		QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("在使用指定特殊码直接注入游戏进程时，特殊码的输入格式有所变化，请参考说明或教程，确认知悉后再使用！"));
	}

	bool isOK;
	QString text = QInputDialog::getText(NULL, QStringLiteral("输入特殊码"),
		QStringLiteral("请输入自定义特殊码:"),
		QLineEdit::Normal,
		"",
		&isOK);

	if (isOK) {
		QVariant var = ui.ProcessesCombox->currentData();
		DWORD pid = (DWORD)var.toInt();

		if (isHooking == false) {
			//在没注入进程的情况下，先初始化，再注入，然后进行对比，不满足要求的直接移除
			TextHost::TextHostInit(pe, pe, oct, ort, opt);
			TextHost::InjectProcess(pid);
			ui.ConsoleTextBox->appendPlainText(QStringLiteral("注入进程PID:") + var.toString());
			ui.AttachBtn->setText(QStringLiteral("结束注入"));
			customHookcode = text;
			isCustomAttach = true;
			isHooking = true;
		}
		TextHost::InsertHook(pid, text.toStdWString().c_str());
	}
}

void MisakaHookFinder::SearchForTextBtn_Click() {
	QMessageBox::information(NULL, QStringLiteral("使用须知"), QStringLiteral("这是一个不稳定的功能，使用前请在游戏中存档防止游戏程序崩溃。\n使用前请查看这个功能的相关说明和教程，这可以在本项目的Github页面中找到。"));

	QDialog dialog(this);
	QFormLayout form(&dialog);
	form.addRow(new QLabel(QStringLiteral("请输入欲搜索的文本和字符代码页:")));
	
	QString value1 = QString(QStringLiteral("欲搜索文本: "));
	QLineEdit* linebox1 = new QLineEdit(&dialog);
	form.addRow(value1, linebox1);
	
	QString value2 = QString(QStringLiteral("代码页: "));
	QSpinBox* spinbox2 = new QSpinBox(&dialog);
	spinbox2->setMaximum(100000);
	spinbox2->setMinimum(0);
	spinbox2->setValue(932);
	form.addRow(value2, spinbox2);

	//确认取消扭
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	//按下确认扭
	if (dialog.exec() == QDialog::Accepted) {
		QVariant var = ui.ProcessesCombox->currentData();
		DWORD pid = (DWORD)var.toInt();

		TextHost::SearchForText(pid, linebox1->text().toStdWString().c_str(), spinbox2->value());
	}

}

void MisakaHookFinder::SearchForHookBtn_Click() {
	
	QMessageBox::information(NULL, QStringLiteral("使用须知"), QStringLiteral("这是一个不稳定的功能，使用前请在游戏中存档防止游戏程序崩溃。\n使用前请查看这个功能的相关说明和教程，这可以在本项目的Github页面中找到。"));

	SearchParam sp;
	sp.length = 0;//默认情况，其他用法见 https://github.com/Artikash/Textractor/blob/master/GUI/mainwindow.cpp 344行
	QVariant var = ui.ProcessesCombox->currentData();
	DWORD pid = (DWORD)var.toInt();

	TextHost::SearchForHooks(pid,&sp,fh);
	PrintToUI(0, QStringLiteral("开始搜索！在接下来的20s内请刷新几次游戏内文本以供内存查找。"));
}

void MisakaHookFinder::CopyHookCodeBtn_Click() {
	QClipboard* clipboard = QApplication::clipboard();   //获取系统剪贴板指针
	clipboard->setText(ui.HookFuncCombox->currentText());                          //设置剪贴板内容

	QMessageBox::information(NULL, QStringLiteral("提示"), QStringLiteral("复制特殊码到剪贴板成功！"));
	
}

void MisakaHookFinder::ClipbordFlushBtn_Click() {
	if (isOpenClipboard == true) {
		isOpenClipboard = false;
		ui.ClipbordFlushBtn->setText(QStringLiteral("开启剪贴板更新"));
		PrintToUI(0, QStringLiteral("已关闭剪贴板更新"));
	}
	else {
		isOpenClipboard = true;
		ui.ClipbordFlushBtn->setText(QStringLiteral("关闭剪贴板更新"));
		PrintToUI(0, QStringLiteral("已开启剪贴板更新，现在该方法所有输出的文本将实时刷新到剪贴板！"));
	}
}

void MisakaHookFinder::RemoveHookBtn_Click() {
	QVariant var = ui.ProcessesCombox->currentData();
	DWORD pid = (DWORD)var.toInt();
	TextHost::RemoveHook(pid, GetAddressByHookComboxContent(ui.HookFuncCombox->currentText()));

	//移除组合框项
	ui.HookFuncCombox->removeItem(ui.HookFuncCombox->currentIndex());
	ui.HookFuncCombox->setCurrentIndex(0);
	PrintToUI(0, QStringLiteral("已移除这个钩子！"));
}



void MisakaHookFinder::FindHooksHandle() {
	PrintToUI(0, QStringLiteral("搜索已结束！正在加载结果..."));
	
	OpenResultWin();
}

/**************************
	拼接信息字符串
**************************/
QString MisakaHookFinder::TextThreadString(int64_t thread_id, DWORD processId, uint64_t addr, uint64_t context, uint64_t subcontext, LPCWSTR name, LPCWSTR hookcode)
{
	return QString("%1:%2:%3:%4:%5:%6:%7").arg(
		QString::number(thread_id, 16),
		QString::number(processId, 16),
		QString::number(addr, 16),
		QString::number(context, 16),
		QString::number(subcontext, 16)
	).toUpper().arg(name).arg(hookcode);
}

/**************************
	根据Hook方法组合框中的内容得到地址，用于移除钩子
***************************/
uint64_t MisakaHookFinder::GetAddressByHookComboxContent(QString str) {
	QStringList sitem = str.split(":");
	return sitem[2].toULongLong((bool*)nullptr, 16);
}


void MisakaHookFinder::HookFunCombox_currentIndexChanged(int index) {
	QStringList sitem = ui.HookFuncCombox->currentText().split(":");
	currentFun = sitem[0].toULongLong((bool*)nullptr,16);
	ui.TextOutPutBox->clear();
}

void MisakaHookFinder::ProcessEventHandle(DWORD processId) {
	PrintToUI(0,QStringLiteral("进程事件PID:") + QString::fromStdString(std::to_string(processId)));
}

void MisakaHookFinder::OnCreateThreadHandle(int64_t thread_id, DWORD processId, uint64_t addr, uint64_t context, uint64_t subcontext, LPCWSTR name, LPCWSTR hookcode) {
	PrintToUI(0,QStringLiteral("添加Hook线程ID") + QString::fromStdString(std::to_string(thread_id)));

	if (isCustomAttach == true) {
		//在使用特定特殊码注入的情况下，遇到特殊码不一致的直接删
		QString currentHookcode = QString::fromStdWString(hookcode);
		QStringList sitem = currentHookcode.split(":");
		if (customHookcode.compare(sitem[0], Qt::CaseInsensitive) != 0) {
			PrintToUI(0, QStringLiteral("智能删除Hook:") + QString::fromStdWString(hookcode));
			RemoveHookFun(addr);
		}
		else {
			QString str = TextThreadString(thread_id, processId, addr, context, subcontext, name, hookcode);
			AddHookFunc(str, thread_id);
		}
	}
	else {
		QString str = TextThreadString(thread_id, processId, addr, context, subcontext, name, hookcode);
		AddHookFunc(str, thread_id);
	}

	
}

void MisakaHookFinder::OnRemoveThreadHandle(int64_t thread_id) {
	PrintToUI(0,QStringLiteral("移除Hook线程ID") + QString::fromStdString(std::to_string(thread_id)));
}

void MisakaHookFinder::OutputTextHandle(int64_t thread_id, LPCWSTR output){
	if (thread_id == 0) {
		//控制台输出线程，依然打印到控制台
		PrintToUI(0, QString::fromStdWString(output));
	}

	if (thread_id == currentFun) {
		//当前输出的线程是组合框选中的输出线程
		PrintToUI(1, QString::fromStdWString(output));

		if (isOpenClipboard == true) {
			FlushClipboard(QString::fromStdWString(output));
		}
	}
}

void MisakaHookFinder::PrintToUI(int editboxID, QString str) {
	switch (editboxID)
	{
	case 0:
		s_this->emitConsoleBoxSignal(str);
		break;
	case 1:
		s_this->emitGameTextBoxSignal(str);
		break;
	default:
		break;
	}
}

void MisakaHookFinder::AddHookFunc(QString str, int data) {
	s_this->emitHookFunComboxSignal(str,data);
}

void MisakaHookFinder::emitHookFunComboxSignal(QString str,int data) {
	emit this->onHookFunComboxChange(str,data);
}

void MisakaHookFinder::emitConsoleBoxSignal(QString str)
{
	emit this->onConsoleBoxContextChange(str);
}

void MisakaHookFinder::emitGameTextBoxSignal(QString str)
{
	emit this->onGameTextBoxContextChange(str);
}

void MisakaHookFinder::ConsoleBox_Change(QString str) {
	ui.ConsoleTextBox->appendPlainText(str);
}

void MisakaHookFinder::GameTextBox_Change(QString str) {
	ui.TextOutPutBox->appendPlainText(str);
	ui.TextOutPutBox->appendPlainText("==================");
}

void MisakaHookFinder::HookFunCombox_Change(QString str, int data) {
	ui.HookFuncCombox->addItem(str,data);
}

void MisakaHookFinder::OpenResultWin() {
	s_this->emitResultWinSignal();
}

void MisakaHookFinder::emitResultWinSignal() {
	emit this->onOpenResWin();
}

void MisakaHookFinder::Reswin_Open() {
	QVariant var = ui.ProcessesCombox->currentData();
	DWORD pid = (DWORD)var.toInt();
	HookResultWindow* hrw = new HookResultWindow();
	hrw->processID = pid;
	hrw->show();
}

void  MisakaHookFinder::FlushClipboard(QString str) {
	s_this->emitClipboardSignal(str);
}


void MisakaHookFinder::emitClipboardSignal(QString str) {
	emit this->onClipboardChange(str);
}

void MisakaHookFinder::Clipboard_Change(QString str) {
	QClipboard* clipboard = QApplication::clipboard();   //获取系统剪贴板指针
	clipboard->setText(str);
}

void MisakaHookFinder::RemoveHookFun(uint64_t thread) {
	s_this->emitRemoveHookFunSignal(thread);
}

void MisakaHookFinder::emitRemoveHookFunSignal(uint64_t thread) {
	emit this->onRemoveHookFun(thread);
}

void MisakaHookFinder::HookFun_Remove(uint64_t thread) {
	QVariant var = ui.ProcessesCombox->currentData();
	DWORD pid = (DWORD)var.toInt();
	TextHost::RemoveHook(pid,thread);
}
