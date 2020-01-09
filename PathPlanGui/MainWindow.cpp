#include "stdafx.h"
#include "MainWindow.h"

#pragma execution_character_set("utf-8")

int choice_emitter = 0;
int choice_radar = 0;
int choice_esmstrategy = 0;
int choice_ecmstrategy = 0;
int choice_section = 0;
int choice_ecm = 0;
int choice_ownplatform = 0;
int choice_point = 1;
sce::Scenario scenario;
QDomDocument dom;
QFile file;

//-----------------------------------------��־�����ֿ�ʼ

//��ʼ����̬����
logger * logger::_instance = 0;

//������ʵ����־����
logger * logger::instance()
{
	if (!logger::_instance)
		logger::_instance = new logger;
	return logger::_instance;
}


void logger::loggerMaster(QtMsgType type, const QString & msg)
{
	//����־��Ϣ�м���ʱ����
	//QString newLog = QDateTime::currentDateTime().toString(QLatin1String("MM-dd hh:mm:ss:zzz")) + QLatin1Char(' ') + msg;
	//���ʹ�������־��Ϣ
	emit G_sndMsg(type, msg);
}

//��־������
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static QMutex mutex;
	mutex.lock();

	QString text;
	switch (type)
	{
	case QtDebugMsg:
		text = QString("Debug:");
		break;

	case QtWarningMsg:
		text = QString("Warning:");
		break;

	case QtCriticalMsg:
		text = QString("Critical:");
		break;

	case QtFatalMsg:
		text = QString("Fatal:");
	}

	QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
	QString current_date = QString("(%1)").arg(current_date_time);
	QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

	QFile file("log.txt");
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << message << "\r\n";
	file.flush();
	file.close();

	logger::instance()->loggerMaster(type, message);
	//onStatusInfo(type, msg);
	//emit outputMsg(type, msg);
	mutex.unlock();
}
//------------------------------------------��־�����ֽ���

PathPlanGui::PathPlanGui(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.SceTab->setCurrentIndex(0);
	ui.SceTab->setTabPosition(QTabWidget::North);//����ѡ��ķ�λ��Ĭ�����Ϸ�(��������)
	ui.SceTab->setTabShape(QTabWidget::Triangular);//����ѡ�����״ Rounded
	ui.SceTab->usesScrollButtons();//ѡ�����
	//setCentralWidget(ui->splitter);
	//VertexModel = new QStandardItemModel(this); //����ģ��
	//theSelection = new QItemSelectionModel(VertexModel);//ѡ��ģ��
	//connect(theSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(on_currentChanged(QModelIndex, QModelIndex)));
	//ui.tableView_Vertex->setModel(VertexModel); //��������ģ��
	//ui.tableView_Vertex->setSelectionModel(theSelection); //����ѡ��ģ��
	//ui.tableView_Vertex->setSelectionMode(QAbstractItemView::ExtendedSelection);
	//ui.tableView_Vertex->setSelectionBehavior(QAbstractItemView::SelectItems);

	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(on_actOpen_triggered()));
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(setOpenFileFlag()));
	connect(this, SIGNAL(sign_show_xml_data()), this, SLOT(show_xml_data()));
	connect(ui.actionRefresh, SIGNAL(triggered()), this, SLOT(show_xml_data()));

	//connect(ui.actionInsert, SIGNAL(triggered()), this, SLOT(on_actInsert_triggered()));
	//connect(ui.actionDelete, SIGNAL(triggered()), this, SLOT(on_actDelete_triggered()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(on_actSave_triggered()));
	connect(ui.pushButton_back, SIGNAL(clicked()), this, SLOT(backTab()));
	connect(ui.pushButton_next, SIGNAL(clicked()), this, SLOT(nextTab()));
	connect(logger::instance(),SIGNAL(G_sndMsg(QtMsgType, QString)), this, SLOT(onStatusInfo(QtMsgType, QString)));

	//����״̬�������
	LabCurFile->setAlignment(Qt::AlignLeft);
	LabCurFile->setMinimumSize(LabCurFile->sizeHint());
	//LabCurFile->setFrameShape(QFrame::WinPanel); //���ñ�ǩ��״
	//LabCurFile->setFrameShadow(QFrame::Sunken); //���ñ�ǩ��Ӱ
	ui.statusBar->addWidget(LabCurFile);

	LabCellPos->setMinimumSize(LabCellPos->sizeHint());
	ui.statusBar->addWidget(LabCellPos);

	LabCellText->setMinimumSize(LabCellText->sizeHint());
	ui.statusBar->addWidget(LabCellText);
}

//void PathPlanGui::on_currentChanged(const QModelIndex &current, const QModelIndex &previous)
//{ //ѡ��Ԫ��仯ʱ����Ӧ
//	if (current.isValid())
//	{
//		LabCellPos->setText(QString::asprintf("��ǰ��Ԫ�񣺣�d �У���d ��", current.row(), current.column()));
//		QStandardItem* aItem = VertexModel->itemFromIndex(current);
//		this->LabCellText->setText("��Ԫ�����ݣ�" + aItem->text());
//		QFont font = aItem->font();
//		//ui.actFontBold->setChecked(font.bold());
//	}
//}

void PathPlanGui::on_actOpen_triggered()
{
	//���ļ�
	//QString str;
	QString curPath = QCoreApplication::applicationDirPath(); //��ȡӦ�ó����·��
	qDebug() << curPath;
	//���ô��ļ��Ի����һ���ļ�
	QString aFileName = QFileDialog::getOpenFileName(this, tr("Select file"), curPath, tr("xml(*.xml);;�����ļ�(*.*)"));
	file.setFileName(aFileName);//���ļ���ʽ����

	if (aFileName.isEmpty())
		return; //���δѡ���ļ����˳�
	QStringList fFileContent;//�ļ������ַ����б�
	//QFile file(aFileName);  //���ļ���ʽ����
	if (!file.open(QIODevice::ReadWrite))
		return;

	this->LabCurFile->setText("��ǰ�ļ���" + aFileName);//״̬����ʾ

	if(!dom.setContent(&file))
	{
		QMessageBox::about(this, tr("��ʾ"), tr("�ļ���ʧ�ܣ�"));
		file.close();
		return;
		
		//QTextStream aStream(&file); //���ı�����ȡ�ļ�
		//ui.plainTextEdit->clear();//���
		//while (!aStream.atEnd())
		//{
		//	QString str = aStream.readLine();//��ȡ�ļ���һ��
		//	ui.plainTextEdit->appendPlainText(str); //��ӵ��ı�����ʾ
		//	fFileContent.append(str); //��ӵ� StringList
		//}
		//
		//ui.actAppend->setEnabled(true); //����Actions��enable����
		//ui.actInsert->setEnabled(true);
		//ui.actDelete->setEnabled(true);
		//ui.actSave->setEnabled(true);
		//initModelFromScenario();//��Scenario�����ݳ�ʼ������ģ��
	}
	file.close();//�ر��ļ�

	//�ٴδ�xml�ļ���Ҫ���֮ǰʵ������scenario�����ͷ�Vector������ռ�õ��ڴ�
	if (open_flag != 0)
	{
		std::vector<std::shared_ptr<sce::Platform>> platforms_temp;
		std::vector<std::shared_ptr<sce::OwnPlatform>> ownplatforms_temp;
		std::vector<std::shared_ptr<sce::Esm>> esms_temp;
		std::vector<std::shared_ptr<sce::Ecm>> ecms_temp;
		std::vector<std::shared_ptr<sce::Vertex>> vertexs_temp;
		std::vector<std::shared_ptr<sce::Weapon>> weapons_temp;
		std::vector<std::shared_ptr<sce::Site>> sites_temp;
		std::vector<std::shared_ptr<sce::Emitter>> emitters_temp;
		std::vector<std::shared_ptr<sce::Route>> routes_temp;
		std::vector<std::shared_ptr<sce::EsmStrategy>> esmstrategys_temp;
		std::vector<std::shared_ptr<sce::EcmStrategy>> ecmstrategys_temp;
		std::vector<sce::PlatformSiteRelation> PSRs_temp;
		std::vector<sce::PlatformEmitterRelation> PERs_temp;
		std::vector<sce::PlatformWeaponRelation> PWRs_temp;
		std::vector<sce::OwnPlatformEsmRelation> OPEsmRs_temp;
		std::vector<sce::EsmEsmStrategyRelation> EsmESRs_temp;
		std::vector<sce::OwnPlatformEcmRelation> OPEcmRs_temp;
		std::vector<sce::EcmEcmStrategyRelation> EcmESRs_temp;
		std::vector<sce::OwnPlatformRouteRelation> OPRRs_temp;

		scenario.getAllPlatform().swap(platforms_temp);
		scenario.getAllOwnPlatform().swap(ownplatforms_temp);
		scenario.getAllEsm().swap(esms_temp);
		scenario.getAllEcm().swap(ecms_temp);
		scenario.getAllVertex().swap(vertexs_temp);
		scenario.getAllWeapon().swap(weapons_temp);
		scenario.getAllSite().swap(sites_temp);
		scenario.getAllEmitter().swap(emitters_temp);
		scenario.getAllRoute().swap(routes_temp);
		scenario.getAllEsmStrategy().swap(esmstrategys_temp);
		scenario.getAllEcmStrategy().swap(ecmstrategys_temp);
		scenario.getAllPlatformSiteRelation().swap(PSRs_temp);
		scenario.getAllPlatformEmitterRelation().swap(PERs_temp);
		scenario.getAllPlatformWeaponRelation().swap(PWRs_temp);
		scenario.getAllOwnPlatformEsmRelation().swap(OPEsmRs_temp);
		scenario.getAllEsmEsmStrategyRelation().swap(EsmESRs_temp);
		scenario.getAllOwnPlatformEcmRelation().swap(OPEcmRs_temp);
		scenario.getAllEcmEcmStrategyRelation().swap(EcmESRs_temp);
		scenario.getAllOwnPlatformRouteRelation().swap(OPRRs_temp);
	}

	QMessageBox::about(this, tr("Tip"), tr("File opened successfully"));
	QDomElement docelem = dom.documentElement();
	listDom(docelem);

	emit sign_show_xml_data();

	//�ͷ�Vector������ռ�õ��ڴ�
	std::vector<std::shared_ptr<sce::Route>>().swap(routes);
	std::vector<std::shared_ptr<sce::EsmStrategy>>().swap(esmstrategys);
	std::vector<std::shared_ptr<sce::EcmStrategy>>().swap(ecmstrategys);
	std::vector<sce::PlatformSiteRelation>().swap(PSRs);
	std::vector<sce::PlatformEmitterRelation>().swap(PERs);
	std::vector<sce::PlatformWeaponRelation>().swap(PWRs);
	std::vector<sce::OwnPlatformEsmRelation>().swap(OPEsmRs);
	std::vector<sce::EsmEsmStrategyRelation>().swap(EsmESRs);
	std::vector<sce::OwnPlatformEcmRelation>().swap(OPEcmRs);
	std::vector<sce::EcmEcmStrategyRelation>().swap(EcmESRs);
	std::vector<sce::OwnPlatformRouteRelation>().swap(OPRRs);
}

void PathPlanGui::listDom(QDomElement &docelem)
{
	bool f_vertex(false), f_platform(false), f_emitter(false), f_esm(false), f_ecm(false), f_op(false), f_route(false), f_site(false), f_weapon(false), f_esms(false), f_ecms(false);
	bool f_psr(false), f_per(false), f_pwr(false), f_oesm(false), f_oecm(false), f_esmesms(false), f_ecmecms(false), f_orr(false);
	QDomNode node = docelem.firstChild();//�ӽڵ�
	if (node.isNull())
	{
		return;
	}
	else
	{
		while (!node.toElement().isNull())
		{
			QDomElement element = node.toElement();//Converts a QDomNode into a QDomElement
			if (!element.isNull())
			{
				if (find_Vertex_data(element, scenario)) f_vertex = true;
				else if (find_Platform_data(element, scenario)) f_platform = true;
				else if (find_Emitter_data(element, scenario)) f_emitter = true;
				else if (find_Esm_data(element, scenario)) f_esm = true;
				else if (find_Ecm_data(element, scenario)) f_ecm = true;
				else if (find_EsmStrategy_data(element, scenario)) f_esms = true;
				else if (find_EcmStrategy_data(element, scenario)) f_ecms = true;
				else if (find_OwnPlatform_data(element, scenario)) f_op = true;
				else if (find_Route_data(element, scenario)) f_route = true;
				else if (find_Site_data(element, scenario)) f_site = true;
				else if (find_Weapon_data(element, scenario)) f_weapon = true;
				else if (find_PlatformSiteRelation_data(element, scenario)) f_psr = true;
				else if (find_PlatformEmitterRelation_data(element, scenario)) f_per = true;
				else if (find_PlatformWeaponRelation_data(element, scenario)) f_pwr = true;
				else if (find_OwnPlatformEsmRelation_data(element, scenario)) f_oesm = true;
				else if (find_EsmEsmStrategyRelation_data(element, scenario)) f_esmesms = true;
				else if (find_OwnPlatformEcmRelation_data(element, scenario)) f_oecm = true;
				else if (find_EcmEcmStrategyRelation_data(element, scenario)) f_ecmecms = true;
				else if (find_OwnPlatformRouteRelation_data(element, scenario)) f_orr = true;

				//listDom(element);
			}
			node = node.nextSibling();//���ֵ�Ԫ�ظ�����
		}

		if (!f_vertex) qDebug() << "No Vertex data was found!";
		if (!f_platform) qDebug() << "No Platform data was found!";
		if (!f_emitter) qDebug() << "No Emitter data was found!";
		if (!f_esm) qDebug() << "No Esm data was found!";
		if (!f_ecm) qDebug() << "No Ecm data was found!";
		if (!f_esms) qDebug() << "No EsmStrategy data was found!";
		if (!f_ecms) qDebug() << "No EcmStrategy data was found!";
		if (!f_op) qDebug() << "No OwnPlatform data was found!";
		if (!f_route) qDebug() << "No Route data was found!";
		if (!f_site) qDebug() << "No Site data was found!";
		if (!f_weapon) qDebug() << "No Weapon data was found!";
		if (!f_psr) 	qDebug() << "No PlatformSiteRelation data was found!";
		if (!f_per) 	qDebug() << "No PlatformEmitterRelation data was found!";
		if (!f_pwr) 	qDebug() << "No PlatformWeaponRelation data was found!";
		if (!f_oesm) qDebug() << "No OwnPlatformEsmRelation data was found!";
		if (!f_esmesms) qDebug() << "No EsmEsmStrategyRelation data was found!";
		if (!f_oecm) qDebug() << "No OwnPlatformEcmRelation data was found!";
		if (!f_ecmecms) qDebug() << "No EcmEcmStrategyRelation data was found!";
		if (!f_orr) qDebug() << "No OwnPlatformRouteRelation data was found!";

		qDebug() << "Data import successfully!";

		//����λ�û�ȡÿ��site�ϵ�Emitter,Weapon
		for (size_t i = 0; i < scenario.getAllSite().size(); ++i)
		{
			auto iterS = scenario.getAllSite().at(i);
			for (size_t j = 0; j < scenario.getAllPlatformSiteRelation().size(); ++j)
			{
				//��ȡ site-weapon��ֵ�ԣ������������ظ��ؼ�ֵ
				auto iterPSR = scenario.getAllPlatformSiteRelation().at(j);
				if (iterPSR.getSiteName() == iterS->getName())
				{
					for (size_t z = 0; z < scenario.getAllPlatformWeaponRelation().size(); ++z)
					{
						auto iterPWR = scenario.getAllPlatformWeaponRelation().at(z);
						if (iterPSR.getPlatformName() == iterPWR.getPlatformName())
						{
							scenario.insertSiteWeaponRelation(iterS, iterPWR.getWeapon());
						}
					}
					//��ȡ site-emitter��ֵ�ԣ������������ظ��ؼ�ֵ
					for (size_t z = 0; z < scenario.getAllPlatformEmitterRelation().size(); ++z)
					{
						auto iterPWR = scenario.getAllPlatformEmitterRelation().at(z);
						if (iterPSR.getPlatformName() == iterPWR.getPlatformName())
						{
							scenario.insertSiteEmitterRelation(iterS, iterPWR.getEmitter());
						}
					}
				}
			}
		}
	}
}

void PathPlanGui::setOpenFileFlag()
{
	open_flag += 1;
}

//void PathPlanGui::initModelFromScenario()
//{
//	size_t rowCnt = scenario.getAllVertex().size();
//	VertexModel->setRowCount(rowCnt);//��������
//	VertexModel->setColumnCount(3);//����Vertex�������ֶ���������
//	//���ñ�ͷ
//	QStringList header_vertex{ QStringLiteral("Longitude"), QStringLiteral("Latitude"), QStringLiteral("Altitude") };//��1���Ǳ�ͷ
//	VertexModel->setHorizontalHeaderLabels(header_vertex);
//
//	//���ñ������
//	QString aText;
//	QStringList tmpList;
//	QStandardItem *aItem_long, *aItem_la,*aItem_al;
//
//	for (size_t i = 0; i < rowCnt; i++)
//	{
//		aItem_long = new QStandardItem(QString::number(scenario.getAllVertex()[i]->getLongitude()));//����item
//		VertexModel->setItem(i, 0, aItem_long); //Ϊģ�͵�ĳ������λ������Item
//
//		aItem_la = new QStandardItem(QString::number(scenario.getAllVertex()[i]->getLatitude()));//����item
//		VertexModel->setItem(i, 1, aItem_la); //Ϊģ�͵�ĳ������λ������Item
//
//		aItem_al = new QStandardItem(QString::number(scenario.getAllVertex()[i]->getAltitude()));//����item
//		VertexModel->setItem(i, 2, aItem_al); //Ϊģ�͵�ĳ������λ������Item
//	}	
//
//}

void PathPlanGui::show_xml_data()
{	
	
	show_Vertex_data();
	show_Platform_data();
	show_Emitter_data();
	show_Weapon_data();
	show_Site_data();
	show_OwnPlatform_data();
	show_Ecm_data();
	show_Esm_data();
	show_EsmStratgy_data();
	show_EcmStratgy_data();
	show_Route_data();//�����ڴ�й¶����
	show_PSRs_data();
	show_PERs_data();
	show_PWRs_data();
	show_OPEsmRs_data();
	show_EsmESRs_data();
	show_OPEcmRs_data();
	show_EcmESRs_data();
	show_OPRRs_data();
}

//void PathPlanGui::on_actAppend_triggered()
//{ //�ڱ����������
//	QList<QStandardItem*>    aItemList; //������ 
//	QStandardItem   *aItem;
//	for (int i = 0; i<2 - 1; i++) //���������1��
//	{
//		aItem = new QStandardItem("0"); //����Item
//		aItemList << aItem;   //��ӵ�����
//	}
//	//��ȡ���һ�еı�ͷ����
//	//QString str = VertexModel->headerData(VertexModel->columnCount() - 1, Qt::Horizontal, Qt::DisplayRole).toString();
//	//aItem = new QStandardItem(str); //���� "��ť"Item
//	//aItem->setCheckable(true);
//	//aItemList << aItem;   //��ӵ�����
//	VertexModel->insertRow(VertexModel->rowCount(), aItemList); //����һ�У���Ҫÿ��Cell��Item
//	QModelIndex curIndex = VertexModel->index(VertexModel->rowCount() - 1, 0);//�������һ�е�ModelIndex
//	theSelection->clearSelection();//���ѡ����
//	theSelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);//���øղ������Ϊ��ǰѡ����
//}

//void PathPlanGui::on_actInsert_triggered()
//{ //�ڱ����������
	//QList<QStandardItem*>    aItemList; //������
	//QStandardItem   *aItem;
	//for (int i = 0; i<2 - 1; i++) //���������1��
	//{
	//	aItem = new QStandardItem("0"); //����Item
	//	aItemList << aItem;   //��ӵ�����
	//}
	//��ȡ���һ�еı�ͷ����
	//QString str = VertexModel->headerData(VertexModel->columnCount() - 1, Qt::Horizontal, Qt::DisplayRole).toString();
	//aItem = new QStandardItem(str); //���� "��ť"Item
	//aItem->setCheckable(true);
	//aItemList << aItem;   //��ӵ�����

	//VertexModel->insertRow(theSelection->currentIndex().row(), aItemList); //����һ�У���Ҫÿ��Cell��Item
	//QModelIndex curIndex = VertexModel->index(theSelection->currentIndex().row() , 0);//�������һ�е�ModelIndex
	//theSelection->clearSelection();//���ѡ����
	//theSelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);//���øղ������Ϊ��ǰѡ����
//}

//void PathPlanGui::on_actDelete_triggered()
//{ //ɾ����
//	//QModelIndex curIndex = theSelection->currentIndex();//��ȡģ������
//	//if (curIndex.row() == VertexModel->rowCount() - 1) //���һ��
//	//	VertexModel->removeRow(curIndex.row()); //ɾ�����һ��
//	//else {
//	//	VertexModel->removeRow(curIndex.row());//ɾ��һ�У����������õ�ǰѡ����
//	//	theSelection->setCurrentIndex(curIndex, QItemSelectionModel::Select);
//	//}
//}

void PathPlanGui::on_actSave_triggered()
{
	
}

void PathPlanGui::show_rada()
{
	choice_emitter = ui.tableWidget_Emitter->currentRow();
	qDebug() << "choice_emitter->" << choice_emitter << endl;
	emit rada_mode();
}

void PathPlanGui::show_mission()
{
	choice_ownplatform = ui.tableWidget_OPlatform->currentRow();
	qDebug() << "choice_ownplatform:" << choice_ownplatform;
	emit sign_mission();
}

void PathPlanGui::ecm_tech()
{
	choice_ecm = ui.tableWidget_Ecm->currentRow();
	emit sign_ecm_tech();
}

void PathPlanGui::show_esmstrategy_section()
{
	choice_esmstrategy = ui.tableWidget_ESMStra->currentRow();
	qDebug() << "choice_esmstrategy" << choice_esmstrategy;
	emit sign_esmstrategy_section();
}

void PathPlanGui::show_ecmstrategy_section()
{
	choice_ecmstrategy = ui.tableWidget_ECMStra->currentRow();
	qDebug() << "choice_ecmstrategy" << choice_ecmstrategy;
	emit sign_ecmstrategy_section();
}

void PathPlanGui::backTab()
{
	if (ui.SceTab->count() > 1 && ui.SceTab->currentIndex() > 0)
	{
		ui.SceTab->setCurrentIndex(ui.SceTab->currentIndex() -1);
	}
	else if(ui.SceTab->count() > 1 && ui.SceTab->currentIndex() == 0 )
	{
		ui.SceTab->setCurrentIndex(ui.SceTab->count()-1);
	}
}

void PathPlanGui::nextTab()
{
	if (ui.SceTab->count() > 1 && ui.SceTab->currentIndex() < (ui.SceTab->count() - 1))
	{
		ui.SceTab->setCurrentIndex(ui.SceTab->currentIndex() + 1);
	}
	else if (ui.SceTab->count() > 1 && ui.SceTab->currentIndex() >= (ui.SceTab->count() -1))
	{
		ui.SceTab->setCurrentIndex(0);
	}
}

void PathPlanGui::show_Vertex_data()
{
	//QStringList tW_Vertex_head{ "Land" , "Air" , "Surface" };
	//ui.tableWidget_Vertex->setHorizontalHeaderLabels(tW_Vertex_head);
	ui.tableWidget_Vertex->setRowCount(0);
	for (int i = 0; i < scenario.getAllVertex().size(); i++) {
		ui.tableWidget_Vertex->insertRow(i);
		ui.tableWidget_Vertex->setItem(i, 0, new QTableWidgetItem(QString::number((scenario.getAllVertex()[i]->getLongitude()), 'f', 2)));
		ui.tableWidget_Vertex->setItem(i, 1, new QTableWidgetItem(QString::number((scenario.getAllVertex()[i]->getLatitude()), 'f', 2)));
		ui.tableWidget_Vertex->setItem(i, 2, new QTableWidgetItem(QString::number((scenario.getAllVertex()[i]->getAltitude()), 'f', 2)));
	}
}

void PathPlanGui::show_Platform_data()
{
	//��ʾplatform������
	ui.tableWidget_Platform->setRowCount(0);
	for (int i = 0; i < scenario.getAllPlatform().size(); i++)
	{
		ui.tableWidget_Platform->insertRow(i);
		ui.tableWidget_Platform->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllPlatform()[i]->getPlatformName())));
		QPointer<QComboBox> cb_Ptype(new QComboBox());
		cb_Ptype->addItems(platformtype_list);
		ui.tableWidget_Platform->setCellWidget(i, 1, cb_Ptype);
		//auto cell = ui.tableWidget_Platform->cellWidget(i, 1);
		//QComboBox *cb_cell = qobject_cast<QComboBox*>(cell);
		//QPointer<QComboBox> cb_Ptype(qobject_cast<QComboBox*>(cell));
		
		if (scenario.getAllPlatform()[i]->getPlatformType() == sce::PlatformType::LAND)
		{
			cb_Ptype->setCurrentIndex(0);
		}
		if (scenario.getAllPlatform()[i]->getPlatformType() == sce::PlatformType::AIR)
		{
			cb_Ptype->setCurrentIndex(1);
		}
		if (scenario.getAllPlatform()[i]->getPlatformType() == sce::PlatformType::SURFACE)
		{
			cb_Ptype->setCurrentIndex(2);
		}
	}	
}

void PathPlanGui::show_Emitter_data()
{
	//��ʾemitter������
	ui.tableWidget_Emitter->setRowCount(0);
	for (int i = 0; i < scenario.getAllEmitter().size(); i++) {
		ui.tableWidget_Emitter->insertRow(i);
		ui.tableWidget_Emitter->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllEmitter()[i]->getName())));
		QPointer<QPushButton> btn(new QPushButton("View"));
		ui.tableWidget_Emitter->setCellWidget(i, 1, btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(show_rada()));
	}
}

void PathPlanGui::show_Weapon_data()
{
	//��ʾweapon������
	ui.tableWidget_Weapon->setRowCount(0);
	for (int i = 0; i < scenario.getAllWeapon().size(); i++)
	{
		ui.tableWidget_Weapon->insertRow(i);
		ui.tableWidget_Weapon->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllWeapon()[i]->getName())));
		ui.tableWidget_Weapon->setItem(i, 1, new QTableWidgetItem(QString::number(scenario.getAllWeapon()[i]->getCEPR())));
		ui.tableWidget_Weapon->setItem(i, 2, new QTableWidgetItem(QString::number(scenario.getAllWeapon()[i]->getWeaponAreaCoverage())));
	}
}

void PathPlanGui::show_Site_data()
{
	//��ʾsite������
	ui.tableWidget_Site->setRowCount(0);
	for (int i = 0; i < scenario.getAllSite().size(); i++)
	{
		ui.tableWidget_Site->insertRow(i);
		ui.tableWidget_Site->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllSite()[i]->getName())));
		ui.tableWidget_Site->setItem(i, 1, new QTableWidgetItem(QString::number((scenario.getAllSite()[i]->getAltitude()), 'f', 2)));
		ui.tableWidget_Site->setItem(i, 3, new QTableWidgetItem(QString::number((scenario.getAllSite()[i]->getLongitude()), 'f', 2)));
		ui.tableWidget_Site->setItem(i, 2, new QTableWidgetItem(QString::number((scenario.getAllSite()[i]->getLatitude()), 'f', 2)));
	}
}

void PathPlanGui::show_OwnPlatform_data()
{
	//��ʾOwnPlatform
	ui.tableWidget_OPlatform->setRowCount(0);
	for (int i = 0; i < scenario.getAllOwnPlatform().size(); i++) {
		ui.tableWidget_OPlatform->insertRow(i);
		ui.tableWidget_OPlatform->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllOwnPlatform()[i]->getName())));
		QPointer<QComboBox> cbOptype(new QComboBox());
		ui.tableWidget_OPlatform->setCellWidget(i, 1, cbOptype);
		ui.tableWidget_OPlatform->setItem(i, 2, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxAcceleration())));
		ui.tableWidget_OPlatform->setItem(i, 3, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxDeceleration())));
		ui.tableWidget_OPlatform->setItem(i, 4, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxClimbRate())));
		ui.tableWidget_OPlatform->setItem(i, 5, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxDiveRate())));
		ui.tableWidget_OPlatform->setItem(i, 6, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxSpeed())));
		ui.tableWidget_OPlatform->setItem(i, 7, new QTableWidgetItem(QString::number(scenario.getAllOwnPlatform()[i]->getMaxTurnRadius())));
		QPointer<QPushButton> cb_mission(new QPushButton("View"));
		ui.tableWidget_OPlatform->setCellWidget(i, 8, cb_mission);
		auto cellWidget = ui.tableWidget_OPlatform->cellWidget(i, 1);
		auto cellWidget_2 = ui.tableWidget_OPlatform->cellWidget(i, 8);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QStringList alist{"Air"};
		combox->addItems(alist);
		QPushButton *btn = qobject_cast<QPushButton*>(cellWidget_2);
		btn->setText("View");
		connect(btn, SIGNAL(clicked()), this, SLOT(show_mission()));
	}
}

void PathPlanGui::show_Ecm_data()
{
	//ECM
	ui.tableWidget_Ecm->setRowCount(0);
	for (int i = 0; i < scenario.getAllEcm().size(); i++) {
		ui.tableWidget_Ecm->insertRow(i);
		ui.tableWidget_Ecm->setItem(i, 0, new QTableWidgetItem(QString::fromStdString((scenario.getAllEcm()[i]->getName()))));
		ui.tableWidget_Ecm->setItem(i, 1, new QTableWidgetItem(QString::number((scenario.getAllEcm()[i]->getPt()), 'f', 2)));
		ui.tableWidget_Ecm->setItem(i, 2, new QTableWidgetItem(QString::number((scenario.getAllEcm()[i]->getGain()), 'f', 2)));
		ui.tableWidget_Ecm->setItem(i, 3, new QTableWidgetItem(QString::number((scenario.getAllEcm()[i]->getRfMin()), 'f', 2)));
		ui.tableWidget_Ecm->setItem(i, 4, new QTableWidgetItem(QString::number((scenario.getAllEcm()[i]->getRfMax()), 'f', 2)));
		QPointer<QPushButton> btn(new QPushButton("View"));
		ui.tableWidget_Ecm->setCellWidget(i, 5, btn);
		auto cell = ui.tableWidget_Ecm->cellWidget(i, 5);
		//QPushButton *tech = qobject_cast<QPushButton*>(cell);
		//tech->setText("View");
		connect(btn, SIGNAL(clicked()), this, SLOT(ecm_tech()));
	}
}

void PathPlanGui::show_Esm_data()
{
	//Esm
	ui.tableWidget_Esm->setRowCount(0);
	for (int i = 0; i < scenario.getAllEsm().size(); i++) {
		ui.tableWidget_Esm->insertRow(i);
		ui.tableWidget_Esm->setItem(i, 0, new QTableWidgetItem(QString::fromStdString((scenario.getAllEsm()[i]->getName()))));
		ui.tableWidget_Esm->setItem(i, 1, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getDwellFreqResolution()), 'f', 2)));
		ui.tableWidget_Esm->setItem(i, 2, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getTuningStep()), 'f', 2)));
		ui.tableWidget_Esm->setItem(i, 3, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getRfCovMin()), 'f', 2)));
		ui.tableWidget_Esm->setItem(i, 4, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getRfCovMax()), 'f', 2)));
		ui.tableWidget_Esm->setItem(i, 5, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getNumPulsesAcquisition()), 'f', 2)));
		ui.tableWidget_Esm->setItem(i, 6, new QTableWidgetItem(QString::number((scenario.getAllEsm()[i]->getNumPulsesAlarm()), 'f', 2)));
	}
}

void PathPlanGui::show_EsmStratgy_data()
{
	//��ʾEsmStrategy������
	ui.tableWidget_ESMStra->setRowCount(0);
	for (int i = 0; i < scenario.getAllEsmStrategy().size(); i++)
	{
		ui.tableWidget_ESMStra->insertRow(i);
		ui.tableWidget_ESMStra->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllEsmStrategy()[i]->getName())));
		
		QPointer<QPushButton> ptn(new QPushButton("View"));
		ui.tableWidget_ESMStra->setCellWidget(i, 1, ptn);
		//QPushButton* ptn = qobject_cast<QPushButton*>(ui.tableWidget_ESMStra->cellWidget(i, 1));
		//ptn->setText("View");
		connect(ptn, SIGNAL(clicked()), this, SLOT(show_esmstrategy_section()));
	}
}

void PathPlanGui::show_EcmStratgy_data()
{
	//��ʾEcmStrategy������
	ui.tableWidget_ECMStra->setRowCount(0);
	for (int i = 0; i < scenario.getAllEcmStrategy().size(); i++) {
		ui.tableWidget_ECMStra->insertRow(i);
		ui.tableWidget_ECMStra->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(scenario.getAllEcmStrategy()[i]->getName())));

		QPointer<QPushButton> ptn(new QPushButton("View"));
		//ptn->setText("View");
		ui.tableWidget_ECMStra->setCellWidget(i, 1, ptn);
		//QPushButton* ptn = qobject_cast<QPushButton*>(ui.tableWidget_ECMStra->cellWidget(i, 1));
		//ptn->setText("View");
		connect(ptn, SIGNAL(clicked()), this, SLOT(show_ecmstrategy_section()));
	}
}

void PathPlanGui::show_Route_data()
{
	//Route
	for (int i = 0; i < ui.tabWidget_Route->count(); ) {
		ui.tabWidget_Route->removeTab(i);
	}
	for (int i = 0; i < scenario.getAllRoute().size(); i++) {
		//��������һ��widget
		QWidget * tab_nn = new QWidget();
		QSpacerItem *horizontalSpacer_0 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);;
		QSpacerItem *horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);;

		QHBoxLayout* hbLayout_n = new QHBoxLayout();
		QVBoxLayout* vbLayout_n = new QVBoxLayout();
		QPointer<QPushButton> add = new QPushButton("Add");
		QPointer<QPushButton> del = new QPushButton("Del");
		QPointer<QPushButton> save = new QPushButton("Save");

		QStringList headers;
		headers << QStringLiteral("Index") << QStringLiteral("Altitude") << QStringLiteral("Index") << QStringLiteral("Latitude") << QStringLiteral("Longitude") << QStringLiteral("Time") << QStringLiteral("Velocity") << QStringLiteral("Acceleration");
		//����route�ı��
		QPointer<QTableWidget> new_table = new QTableWidget();
		new_table->setColumnCount(7);
		new_table->setRowCount(0);
		new_table->setLayoutDirection(Qt::LeftToRight);
		new_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		new_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		new_table->setDragEnabled(true);
		new_table->setSelectionBehavior(QAbstractItemView::SelectItems);
		new_table->setTextElideMode(Qt::ElideMiddle);
		new_table->horizontalHeader()->setCascadingSectionResizes(false);
		new_table->verticalHeader()->setVisible(true);
		new_table->setHorizontalHeaderLabels(headers);
		for (int j = 0; j < scenario.getAllRoute()[i]->getAllWayPoints().size(); j++)
		{
			new_table->insertRow(j);
			new_table->setItem(j, 0, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getIndex()), 10));
			new_table->setItem(j, 1, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getAltitude(), 'f', 2)));
			new_table->setItem(j, 2, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getLatitude(), 'f', 2)));
			new_table->setItem(j, 3, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getLongitude(), 'f', 2)));
			new_table->setItem(j, 4, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getTime(), 'f', 2)));
			new_table->setItem(j, 5, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getVelocity(), 'f', 2)));
			new_table->setItem(j, 6, new QTableWidgetItem(QString::number(scenario.getAllRoute()[i]->getAllWayPoints()[j].getAcceleration(), 'f', 2)));
		}
		hbLayout_n->addItem(horizontalSpacer_0);
		hbLayout_n->addWidget(add);
		hbLayout_n->addWidget(del);
		hbLayout_n->addWidget(save);
		hbLayout_n->addItem(horizontalSpacer_1);
		
		vbLayout_n->addItem(hbLayout_n);
		vbLayout_n->addWidget(new_table);

		tab_nn->setLayout(vbLayout_n);
		ui.tabWidget_Route->setCurrentWidget(tab_nn);
		ui.tabWidget_Route->addTab(tab_nn, QString::fromStdString(scenario.getAllRoute()[i]->getName()));

		int cur_index = ui.tabWidget_Route->currentIndex();
		//����tab��
		MyTab mytab;
		mytab.add = add;
		mytab.del = del;
		mytab.save = save;
		mytab.myindex = cur_index;
		mytab.my_table = new_table;
		connect(mytab.add, &QPushButton::clicked, this, [=]()
		{
			mytab.my_table->insertRow(mytab.my_table->rowCount());

		});
		connect(mytab.del, &QPushButton::clicked, this, [=]
		{
			int ree = QMessageBox::information(this, "", "Confirm deletion?", QStringLiteral("Yes"), QStringLiteral("No"));
			if (ree == 0)
			{
				mytab.my_table->removeRow(mytab.my_table->currentRow());
			}
		});
		connect(mytab.save, &QPushButton::clicked, this, [=]
		{
			int cur = mytab.my_table->currentRow();
			QString a = mytab.my_table->item(cur, 0)->text();
			QString b = mytab.my_table->item(cur, 1)->text();
			QString c = mytab.my_table->item(cur, 2)->text();
			QString d = mytab.my_table->item(cur, 3)->text();
			QString e = mytab.my_table->item(cur, 4)->text();
			QString f = mytab.my_table->item(cur, 5)->text();
			QString g = mytab.my_table->item(cur, 6)->text();
			if (cur + 1 > scenario.getAllRoute()[cur_index]->getAllWayPoints().size())
			{
				sce::WayPoint new_wp(a.toInt(), b.toDouble(), c.toDouble(), d.toDouble(), e.toDouble(), f.toDouble(), g.toDouble());
				scenario.getAllRoute()[cur_index]->addWayPoint(new_wp);
				QMessageBox::about(this, tr("Tip"), tr("Save WayPoint successfully"));

				QDomElement third = dom.createElement("WayPoints");

				QDomElement fourth_1 = dom.createElement("Index");
				QDomElement fourth_2 = dom.createElement("Altitude");
				QDomElement fourth_3 = dom.createElement("Latitude");
				QDomElement fourth_4 = dom.createElement("Longitude");
				QDomElement fourth_5 = dom.createElement("Time");
				QDomElement fourth_6 = dom.createElement("Velocity");
				QDomElement fourth_7 = dom.createElement("Acceleration");

				QDomText text1 = dom.createTextNode(a);
				QDomText text2 = dom.createTextNode(b);
				QDomText text3 = dom.createTextNode(c);
				QDomText text4 = dom.createTextNode(d);
				QDomText text5 = dom.createTextNode(e);
				QDomText text6 = dom.createTextNode(f);
				QDomText text7 = dom.createTextNode(g);

				fourth_1.appendChild(text1);
				fourth_2.appendChild(text2);
				fourth_3.appendChild(text3);
				fourth_4.appendChild(text4);
				fourth_5.appendChild(text5);
				fourth_6.appendChild(text6);
				fourth_7.appendChild(text7);

				third.appendChild(fourth_1);
				third.appendChild(fourth_2);
				third.appendChild(fourth_3);
				third.appendChild(fourth_4);
				third.appendChild(fourth_5);
				third.appendChild(fourth_6);
				third.appendChild(fourth_7);

				QDomNodeList list = dom.elementsByTagName("Route");
				int flag = 0;
				for (int i = 0; i < list.count(); i++)
				{
					QDomElement ele = list.at(i).toElement();
					if (ele.parentNode().nodeName() == "Scenario")
					{
						if (flag == cur_index)
						{
							ele.appendChild(third);
							break;
						}
						flag++;
					}
				}
			}
			else
			{
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setIndex(a.toInt());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setAltitude(b.toDouble());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setLatitude(c.toDouble());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setLongitude(d.toDouble());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setTime(e.toDouble());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setVelocity(f.toDouble());
				scenario.getAllRoute()[cur_index]->getAllWayPoints()[cur].setAcceleration(g.toDouble());
				QMessageBox::about(this, tr("Tip"), tr("Save WayPoint successfully"));

				QDomNodeList list = dom.elementsByTagName("Route");
				int flag = 0;
				for (int i = 0; i < list.count(); i++)
				{
					QDomElement ele = list.at(i).toElement();
					if (ele.parentNode().nodeName() == "Scenario")//�ҵ�Ϊ�ڶ����ڵ��Route
					{
						if (flag == cur_index)
						{
							int flag_2 = 0;
							for (QDomNode qd = ele.firstChild(); !qd.isNull(); qd = qd.nextSibling())
							{
								if (qd.nodeName() == "WayPoints")//�ҵ������WayPoints
								{
									if (flag_2 == cur)
									{
										for (QDomNode qdd = qd.firstChild(); !qdd.isNull(); qdd = qdd.nextSibling())
										{
											if (qdd.nodeName() == "Index")
											{
												qdd.firstChild().setNodeValue(a);
											}
											if (qdd.nodeName() == "Altitude")
											{
												qdd.firstChild().setNodeValue(b);
											}
											if (qdd.nodeName() == "Latitude")
											{
												qdd.firstChild().setNodeValue(c);
											}
											if (qdd.nodeName() == "Longitude")
											{
												qdd.firstChild().setNodeValue(d);
											}
											if (qdd.nodeName() == "Time")
											{
												qdd.firstChild().setNodeValue(e);
											}
											if (qdd.nodeName() == "Velocity")
											{
												qdd.firstChild().setNodeValue(f);
											}
											if (qdd.nodeName() == "Acceleration")
											{
												qdd.firstChild().setNodeValue(g);
											}
										}
										break;
									}
									flag_2++;
								}
							}
							break;
						}
						flag++;
					}
				}
			}
		});

		vTab.push_back(mytab);
	}
}

void PathPlanGui::show_PSRs_data()
{
	//show PlatformSiteRelation
	QStringList platformname_List;
	QStringList sitename_List;
	for (int j = 0; j < scenario.getAllPlatform().size(); j++) {
		platformname_List.append(QString::fromStdString(scenario.getAllPlatform()[j]->getPlatformName()));
	}
	for (int j = 0; j < scenario.getAllSite().size(); j++) {
		sitename_List.append(QString::fromStdString(scenario.getAllSite()[j]->getName()));
	}
	ui.tableWidget_PSR->setRowCount(0);
	for (int i = 0; i < scenario.getAllPlatformSiteRelation().size(); i++)
	{
		ui.tableWidget_PSR->insertRow(i);
		ui.tableWidget_PSR->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_PSR->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_PSR->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_PSR->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(platformname_List);
		combox_1->addItems(sitename_List);
		for (int j = 0; j < platformname_List.size(); j++)
		{
			if (platformname_List[j].toStdString() == scenario.getAllPlatformSiteRelation()[i].getPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < sitename_List.size(); j++)
		{
			if (sitename_List[j].toStdString() == scenario.getAllPlatformSiteRelation()[i].getSiteName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_PERs_data()
{
	//��ʾPlatformEmitterRelation������
	QStringList platformname_List;
	for (int j = 0; j < scenario.getAllPlatform().size(); j++) {
		platformname_List.append(QString::fromStdString(scenario.getAllPlatform()[j]->getPlatformName()));
	}
	QStringList Emittername_List;
	for (int j = 0; j < scenario.getAllEmitter().size(); j++) {
		Emittername_List.append(QString::fromStdString(scenario.getAllEmitter()[j]->getName()));
	}
	ui.tableWidget_PER->setRowCount(0);
	for (int i = 0; i < scenario.getAllPlatformEmitterRelation().size(); i++) {
		ui.tableWidget_PER->insertRow(i);
		ui.tableWidget_PER->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_PER->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_PER->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_PER->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(platformname_List);
		combox_1->addItems(Emittername_List);
		for (int j = 0; j < platformname_List.size(); j++)
		{
			if (platformname_List[j].toStdString() == scenario.getAllPlatformEmitterRelation()[i].getPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < Emittername_List.size(); j++)
		{
			if (Emittername_List[j].toStdString() == scenario.getAllPlatformEmitterRelation()[i].getEmitterName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_PWRs_data()
{
	//��ʾPlatformWeaponRelation������
	ui.tableWidget_PWR->setRowCount(0);
	QStringList platformname_List;
	for (int j = 0; j < scenario.getAllPlatform().size(); j++) {
		platformname_List.append(QString::fromStdString(scenario.getAllPlatform()[j]->getPlatformName()));
	}

	QStringList Weaponname_List;
	for (int j = 0; j < scenario.getAllWeapon().size(); j++) {
		Weaponname_List.append(QString::fromStdString(scenario.getAllWeapon()[j]->getName()));
	}

	for (int i = 0; i < scenario.getAllPlatformWeaponRelation().size(); i++) {
		ui.tableWidget_PWR->insertRow(i);
		ui.tableWidget_PWR->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_PWR->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_PWR->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_PWR->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		
		combox->addItems(platformname_List);
		combox_1->addItems(Weaponname_List);
		for (int j = 0; j < platformname_List.size(); j++)
		{
			if (platformname_List[j].toStdString() == scenario.getAllPlatformWeaponRelation()[i].getPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < Weaponname_List.size(); j++)
		{
			if (Weaponname_List[j].toStdString() == scenario.getAllPlatformWeaponRelation()[i].getWeaponName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_OPEsmRs_data()
{
	//��ʾOwnPlatformEsmRelation������
	QStringList OwnPlatformname_List;
	QStringList Esmname_List;
	for (int j = 0; j < scenario.getAllOwnPlatform().size(); j++) {
		OwnPlatformname_List.append(QString::fromStdString(scenario.getAllOwnPlatform()[j]->getName()));
	}
	for (int j = 0; j < scenario.getAllEsm().size(); j++) {
		Esmname_List.append(QString::fromStdString(scenario.getAllEsm()[j]->getName()));
	}
	ui.tableWidget_OEs->setRowCount(0);
	for (int i = 0; i < scenario.getAllOwnPlatformEsmRelation().size(); i++) {
		ui.tableWidget_OEs->insertRow(i);
		ui.tableWidget_OEs->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_OEs->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_OEs->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_OEs->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(OwnPlatformname_List);
		combox_1->addItems(Esmname_List);
		for (int j = 0; j < OwnPlatformname_List.size(); j++)
		{
			if (OwnPlatformname_List[j].toStdString() == scenario.getAllOwnPlatformEsmRelation()[i].getOwnPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < Esmname_List.size(); j++)
		{
			if (Esmname_List[j].toStdString() == scenario.getAllOwnPlatformEsmRelation()[i].getEsmName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_EsmESRs_data()
{
	//��ʾEsmEsmStrategyRelation������
	QStringList Esmname_List;
	QStringList EsmStrategyname_List;
	for (int j = 0; j < scenario.getAllEsm().size(); j++) {
		Esmname_List.append(QString::fromStdString(scenario.getAllEsm()[j]->getName()));
	}
	for (int j = 0; j < scenario.getAllEsmStrategy().size(); j++) {
		EsmStrategyname_List.append(QString::fromStdString(scenario.getAllEsmStrategy()[j]->getName()));
	}
	ui.tableWidget_EsmES->setRowCount(0);
	for (int i = 0; i < scenario.getAllEsmEsmStrategyRelation().size(); i++) {
		ui.tableWidget_EsmES->insertRow(i);
		ui.tableWidget_EsmES->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_EsmES->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_EsmES->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_EsmES->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(Esmname_List);
		combox_1->addItems(EsmStrategyname_List);
		for (int j = 0; j < Esmname_List.size(); j++)
		{
			if (Esmname_List[j].toStdString() == scenario.getAllEsmEsmStrategyRelation()[i].getEsmName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < EsmStrategyname_List.size(); j++)
		{
			if (EsmStrategyname_List[j].toStdString() == scenario.getAllEsmEsmStrategyRelation()[i].getEsmStrategyName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_OPEcmRs_data()
{
	//��ʾOwnPlatformEcmRelation������
	ui.tableWidget_OPEcmR->setRowCount(0);
	QStringList OwnPlatformname_List;
	QStringList Ecmname_List;
	for (int j = 0; j < scenario.getAllOwnPlatform().size(); j++) {
		OwnPlatformname_List.append(QString::fromStdString(scenario.getAllOwnPlatform()[j]->getName()));
	}
	for (int j = 0; j < scenario.getAllEcm().size(); j++) {
		Ecmname_List.append(QString::fromStdString(scenario.getAllEcm()[j]->getName()));
	}
	for (int i = 0; i < scenario.getAllOwnPlatformEcmRelation().size(); i++) {
		ui.tableWidget_OPEcmR->insertRow(i);
		ui.tableWidget_OPEcmR->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_OPEcmR->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_OPEcmR->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_OPEcmR->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(OwnPlatformname_List);
		combox_1->addItems(Ecmname_List);
		for (int j = 0; j < OwnPlatformname_List.size(); j++)
		{
			if (OwnPlatformname_List[j].toStdString() == scenario.getAllOwnPlatformEcmRelation()[i].getOwnPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < Ecmname_List.size(); j++)
		{
			if (Ecmname_List[j].toStdString() == scenario.getAllOwnPlatformEcmRelation()[i].getEcmName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_EcmESRs_data()
{
	//show EcmEcmStrategyRelation
	QStringList ecmname_List;
	QStringList EcmStrategyname_List;
	for (int j = 0; j < scenario.getAllEcm().size(); j++)
	{
		ecmname_List.append(QString::fromStdString(scenario.getAllEcm()[j]->getName()));
	}
	for (int j = 0; j < scenario.getAllEcmStrategy().size(); j++)
	{
		EcmStrategyname_List.append(QString::fromStdString(scenario.getAllEcmStrategy()[j]->getName()));
	}
	ui.tableWidget_EcmES->setRowCount(0);
	for (int i = 0; i < scenario.getAllEcmEcmStrategyRelation().size(); i++)
	{
		ui.tableWidget_EcmES->insertRow(i);
		ui.tableWidget_EcmES->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_EcmES->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_EcmES->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_EcmES->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(ecmname_List);
		combox_1->addItems(EcmStrategyname_List);
		for (int j = 0; j < ecmname_List.size(); j++)
		{
			if (ecmname_List[j].toStdString() == scenario.getAllEcmEcmStrategyRelation()[i].getEcmName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < EcmStrategyname_List.size(); j++)
		{
			if (EcmStrategyname_List[j].toStdString() == scenario.getAllEcmEcmStrategyRelation()[i].getEcmStrategyName())
			{
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::show_OPRRs_data()
{
	//��ʾOwnPlatformRouteRelation������
	QStringList OwnPlatformname_List;
	QStringList Routename_List;
	for (int j = 0; j < scenario.getAllOwnPlatform().size(); j++) {
		OwnPlatformname_List.append(QString::fromStdString(scenario.getAllOwnPlatform()[j]->getName()));
	}
	for (int j = 0; j < scenario.getAllRoute().size(); j++) {
		Routename_List.append(QString::fromStdString(scenario.getAllRoute()[j]->getName()));
	}
	ui.tableWidget_ORR->setRowCount(0);
	for (int i = 0; i < scenario.getAllOwnPlatformRouteRelation().size(); i++) {
		ui.tableWidget_ORR->insertRow(i);
		ui.tableWidget_ORR->setCellWidget(i, 0, new QComboBox());
		ui.tableWidget_ORR->setCellWidget(i, 1, new QComboBox());
		auto cellWidget = ui.tableWidget_ORR->cellWidget(i, 0);
		auto cellWidget_1 = ui.tableWidget_ORR->cellWidget(i, 1);
		QComboBox *combox = qobject_cast<QComboBox*>(cellWidget);
		QComboBox *combox_1 = qobject_cast<QComboBox*>(cellWidget_1);
		combox->addItems(OwnPlatformname_List);
		combox_1->addItems(Routename_List);
		for (int j = 0; j < OwnPlatformname_List.size(); j++)
		{
			if (OwnPlatformname_List[j].toStdString() == scenario.getAllOwnPlatformRouteRelation()[i].getOwnPlatformName()) {
				combox->setCurrentIndex(j);
				break;
			}
		}
		for (int j = 0; j < Routename_List.size(); j++)
		{
			if (Routename_List[j].toStdString() == scenario.getAllOwnPlatformRouteRelation()[i].getRouteName()) {
				combox_1->setCurrentIndex(j);
				break;
			}
		}
	}
}

void PathPlanGui::onStatusInfo(QtMsgType type, QString msg)
{
	QString text;
	QTextCharFormat chfmt;
	//����״̬��Ϣ��ǩ
	switch (type) {
	case QtDebugMsg: //������Ϣ��ʾ
		text = "INFO";
		chfmt.setForeground(QBrush(Qt::black));
		break;
	case QtWarningMsg: //һ���warning��ʾ
		text = "WARN";
		chfmt.setForeground(QBrush(Qt::yellow));
		break;
	case QtCriticalMsg: //���ش�����ʾ
		text = "ERROR";
		chfmt.setForeground(QBrush(Qt::red));
		break;
	case QtFatalMsg: //����������ʾ
		text = "ERROR";
		chfmt.setForeground(QBrush(Qt::red));
		abort();
		break;
	default:
		break;
	}
	//�ڱ�ǩ֮�󸽼�״̬��Ϣ
	text.append(" " + msg);
	//������λ�����õ�QTextEdit���һ��
	QTextCursor cur = ui.statusTextEditer->textCursor();
	cur.movePosition(QTextCursor::End);
	QTextTableFormat tableformate;
	tableformate.setBorder(0);
	//����table��������ʾ״̬��Ϣ
	QTextTable *table = cur.insertTable(2, 1, tableformate);
	table->cellAt(0, 0).firstCursorPosition().insertText("[" + QTime::currentTime().toString("hh:mm:ss") + "]");
	table->cellAt(1, 0).firstCursorPosition().insertText(text);
	cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor, 1);
	cur.select(QTextCursor::BlockUnderCursor);
	cur.mergeCharFormat(chfmt);
	cur.clearSelection();
	QScrollBar *bar = ui.statusTextEditer->verticalScrollBar();
	bar->setValue(bar->maximum());
	//״̬��Ϣ����+1
	m_status_count++;
	//�����Ϣ��>50����ɾ�������һ����¼
	if (m_status_count >50)
	{
		//��ѡ����������һ���ַ�
		cur.setPosition(1, QTextCursor::MoveAnchor);
		//ѡ�����纬������״̬��Ϣ��table
		cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor, 1);
		cur.select(QTextCursor::BlockUnderCursor);
		//ɾ������Ϣ
		cur.removeSelectedText();
		//�������QTextEditĩ��
		cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
		m_status_count--;
	}
}
