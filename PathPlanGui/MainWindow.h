#pragma once

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <Qtxml>
#include <string>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QPointer>
#include <QTextTableFormat>
#include "ui_MainWindow.h"

#include "Scenario.hpp"
#include "xml_prase.h"

extern int choice_emitter;
extern int choice_radar;
extern int choice_esmstrategy;
extern int choice_ecmstrategy;
extern int choice_section;
extern int choice_ecm;
extern int choice_ownplatform;
extern int choice_point;

extern QDomDocument dom;
extern QFile file;
extern sce::Scenario scenario;

class MyTab
{
public:
	MyTab() {}
	~MyTab() {}
	QPointer<QPushButton> add;
	QPointer<QPushButton> del;
	QPointer<QPushButton> save;
	int myindex;
	QPointer<QTableWidget>  my_table;
};


//��־������
class logger : public QObject
{
	Q_OBJECT
public:
	~logger() {}
	static logger *_instance;
	static logger *instance();

public:
	void loggerMaster(QtMsgType type, const QString &);

signals:
	void G_sndMsg(QtMsgType type, const QString &);

private:
	//��Ĭ�Ϲ��캯������Ϊ˽��
	logger(QObject *parent = 0) :
		QObject(parent) {}
};

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

class PathPlanGui : public QMainWindow
{
	Q_OBJECT
private:
	QLabel *LabCurFile = new QLabel("file");    //��ǰ�ļ�
	QLabel *LabCellPos = new QLabel("Position");   //��ǰ��Ԫ�����к�
	QLabel *LabCellText = new QLabel("Text"); //��ǰ��Ԫ������

	size_t open_flag{ 0 };//�ļ��򿪴�����־λ
	size_t m_status_count{0}; //״̬��Ϣ����

	QVector<MyTab> vTab;
	QVector<QTableWidget*> route_v;
	QPushButton * tab_btn = NULL;
	int route_index = -1;

	QStringList platformtype_list{ "Land" , "Air" , "Surface" };
	QStringList ownplatformtype_list{ "Air" };

	//QStandardItemModel * VertexModel; //����ģ��
	//QStandardItemModel * PlatformModel; //����ģ��
	//QStandardItemModel * EmitterModel; //����ģ��
	//QStandardItemModel * SiteModel; //����ģ��
	//QStandardItemModel * WeaponModel; 
	//QStandardItemModel * OwnPlatformModel;
	//QStandardItemModel * MissionModel;
	//QStandardItemModel * PointModel;
	//
	//QStandardItemModel * LocationModel;

	//QStandardItemModel * EsmModel;
	//QStandardItemModel * EcmModel;
	//QStandardItemModel * EsmStrategyModel;
	//QStandardItemModel * EcmStrategyModel;
	//QStandardItemModel * EsmStrategySectionModel;
	//QStandardItemModel * EcmStrategySectionModel;
	//QStandardItemModel * DwellSquenceModel;

	//QStandardItemModel * RouteModel;
	//QStandardItemModel * WayPointModel;

	//QStandardItemModel * RfModel;
	//QStandardItemModel * Rf_valuesModel;
	//QStandardItemModel * PwModel;
	//QStandardItemModel * Pw_valuesModel;
	//QStandardItemModel * PriModel;
	//QStandardItemModel * Pri_valuesModel;
	//QStandardItemModel * ScanModel;
	//QStandardItemModel * ErpModel;
	//QStandardItemModel * RadarModeModel;

	//QStandardItemModel * PSRelationModel;
	//QStandardItemModel * PERelationModel;
	//QStandardItemModel * PWRelationModel;
	//QStandardItemModel * OPEsmRelationModel;
	//QStandardItemModel * EsmESRelationModel;
	//QStandardItemModel * OPEcmRelationModel;
	//QStandardItemModel * EcmESRelationModel;
	//QStandardItemModel * OPRRelationModel;

	//QItemSelectionModel *theSelection; //ѡ��ģ��


	
public:
	PathPlanGui(QWidget *parent = Q_NULLPTR);

	void listDom(QDomElement & docelem);
	std::vector<std::shared_ptr<sce::Route>> routes;
	std::vector<std::shared_ptr<sce::EsmStrategy>> esmstrategys;
	std::vector<std::shared_ptr<sce::EcmStrategy>> ecmstrategys;
	std::vector<sce::PlatformSiteRelation> PSRs;
	std::vector<sce::PlatformEmitterRelation> PERs;
	std::vector<sce::PlatformWeaponRelation> PWRs;
	std::vector<sce::OwnPlatformEsmRelation> OPEsmRs;
	std::vector<sce::EsmEsmStrategyRelation> EsmESRs;
	std::vector<sce::OwnPlatformEcmRelation> OPEcmRs;
	std::vector<sce::EcmEcmStrategyRelation> EcmESRs;
	std::vector<sce::OwnPlatformRouteRelation> OPRRs;

	void show_Vertex_data();
	void show_Platform_data();
	void show_Emitter_data();
	void show_Weapon_data();
	void show_Site_data();
	void show_OwnPlatform_data();
	void show_Ecm_data();
	void show_Esm_data();
	void show_EsmStratgy_data();
	void show_EcmStratgy_data();
	void show_Route_data();
	void show_PSRs_data();
	void show_PERs_data();
	void show_PWRs_data();
	void show_OPEsmRs_data();
	void show_EsmESRs_data();
	void show_OPEcmRs_data();
	void show_EcmESRs_data();
	void show_OPRRs_data();
	
signals:
	void go_next();//��ʾ��һҳ
	void rada_mode();
	void sign_add_rada();
	void sign_mission();
	void sign_added_mission();
	void sign_show_xml_data();
	void sign_esmstrategy_section();
	void sign_ecmstrategy_section();
	void sign_ecm_tech();
	//void outputMsg(QtMsgType type, const QString &msg);

private slots:
	//��ǰѡ��Ԫ�����仯
	//void on_currentChanged(const QModelIndex &current, const QModelIndex &previous);
	void on_actOpen_triggered();
	void show_xml_data();
	void setOpenFileFlag();

	void on_actSave_triggered();
	void show_rada();
	void show_mission();
	void ecm_tech();
	void show_esmstrategy_section();
	void show_ecmstrategy_section();

	void backTab();
	void nextTab();

	void onStatusInfo(QtMsgType type, QString msg);

private:
	Ui::PathPlanGuiClass ui;
};