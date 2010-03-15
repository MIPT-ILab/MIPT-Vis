/**
 * File: GUI/GuiMW.h - Implementational header for MainWindow of GUI in MiptVis tool.
 * Copyright (C) 2009  MiptVis, Le Manh Ha
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "gui_impl.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
class GuiView;
class QHBoxLayout;
class QVBoxLayout;
class QWidget;
class QSplitter;
class QString;
class GuiGraph;
class GuiTextEdit;
class QGroupBox;
class QPushButton;
QT_END_NAMESPACE

/**
 * Class MainWindow
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void load();
    void save();
    void help();
    void about();
    void doLayoutSlot();
    void textHandle();
    void centreOnNode();
    void convertDumpToXMLSlot();
    void doCentreOnNode( int nodeNumber);
    void saveTextToNode();

	void enableGravity();
	void disableGravity();
	void makeGravity();
	void switchVnodesShow();

private:
    void createMenus();
    void createActions();
    void createStatusBar();
    void setCurrentFile( const QString & fileName);
	void createDockWindows();

    QString strippedName( const QString &fullFileName);
    
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    QAction *loadAct;
    QAction *saveAct;
    QAction *helpAct;
    QAction *aboutAct;
    QAction *doLayoutAct;
    QAction *convertDumpToXMLAct;
    QAction *enGravityAct;
    QAction *disGravityAct;
    QAction *showVirtualNodesAct;

	QTimer *gravity_timer;

    GuiView *view;
    QHBoxLayout *layout;
    QWidget *widget;
    QWidget *widget1;
    QWidget *widget2;
    QSplitter *splitter;
    GuiGraph *graph;
    QString currentFile;
    GuiTextEdit *nodeTextEdit;
    QGroupBox *groupBox;
    QVBoxLayout *textLayout;
    QVBoxLayout *centreOnNodeLayout;
    QPushButton *saveTextButton;
    QPushButton *centreOnNodeButton;
	QSpinBox *spinBox;

};

#endif
