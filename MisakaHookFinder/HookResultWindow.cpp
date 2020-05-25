#include "HookResultWindow.h"

HookResultWindow::HookResultWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	

	QFile file("result.txt");
	if (!file.open(QFile::ReadOnly|QFile::Text)) {
		QMessageBox::warning(this, QStringLiteral("文件读取错误"), QStringLiteral("读取结果出错:%1").arg(file.errorString()));
	}
	else 
	{
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		QStringList sitem = stream.readAll().split("<=====>\n");

		QStandardItemModel* res = new QStandardItemModel(this);

		res->setColumnCount(2);
		res->setHeaderData(0, Qt::Horizontal, QStringLiteral("特殊码"));
		res->setHeaderData(1, Qt::Horizontal, QStringLiteral("文本"));

		int length = sitem.size();

		for (int i = 0;i < length;i++) {
			QStringList it = sitem[i].split(" => ");

			if (it.size() == 2) {
				QList<QStandardItem*>* row = new QList<QStandardItem*>;
				*row << new QStandardItem(it[0]) << new QStandardItem(it[1]);
				res->appendRow(*row);
			}
		}

		//ui.resTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
		ui.resTableView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
		ui.resTableView->setSelectionMode(QAbstractItemView::SingleSelection);//设置选中单个
		ui.resTableView->resizeColumnsToContents();//适配宽度
		ui.resTableView->setModel(res);
	}

	connect(ui.FindNextBtn, SIGNAL(clicked()), this, SLOT(FindNextBtn_Click()));
	connect(ui.AddCustomBtn, SIGNAL(clicked()), this, SLOT(AddCustomBtn_Click()));
}

HookResultWindow::~HookResultWindow()
{
}

void HookResultWindow::FindNextBtn_Click() {
	int row = ui.resTableView->currentIndex().row();
	QAbstractItemModel* model = ui.resTableView->model();
	QString findStr = ui.SearchBox->text();
	int rowsCount = model->rowCount();

	if (findStr == "") {
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入要查找的关键字！"));
		return;
	}

	if (row == -1) {
		//没有选中项的时候，默认第一条开始
		row == 0;
	}
	else {
		row++;//要从下一行开始
	}

	for (int i = row;i < rowsCount;i++) {
		QModelIndex index = model->index(i, 1);//选中行第二列的内容
		QVariant data = model->data(index);

		if (data.toString().contains(findStr, Qt::CaseInsensitive) == true) {
			ui.resTableView->setCurrentIndex(index);
			break;
		}
	}

	if (row >= rowsCount - 1) {
		QMessageBox::warning(this, QStringLiteral("无结果"), QStringLiteral("查找下一行无结果，自动返回首行！"));
		QModelIndex index = model->index(0, 1);
		ui.resTableView->setCurrentIndex(index);
	}

}

void HookResultWindow::AddCustomBtn_Click() {
	int row = ui.resTableView->currentIndex().row();
	if (row == -1) {
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请选中一个要注入特殊码的行！"));
		return;
	}

	QAbstractItemModel* model = ui.resTableView->model();
	QModelIndex index = model->index(row, 0);//选中行第一列的内容
	QVariant data = model->data(index);
	
	TextHost::InsertHook(processID,data.toString().toStdWString().c_str());
	QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("已添加自定义Hook特殊码，请在主页面确认！"));
}
