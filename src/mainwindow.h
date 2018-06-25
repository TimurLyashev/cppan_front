#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QPushButton"
#include "QTableView"
#include <QMainWindow>
#include <QStandardItem>
#include <QStandardItemModel>
#include <database.h>
#include <package.h>

class PackageTable : public QTableView {
  Q_OBJECT
public slots:
  void searchPackage();
};

class PackageModel : public QStandardItemModel {
  Q_OBJECT
public:
  PackagesSet generateDifferenceLocalePackages();
  Packages generateDifferenceRemotePackagesInstall();

public slots:
  void RemoveUnchekedLocale();
  void InstallChekedRemote();
  void buildInstalledPackages();
  void buildSelectedPackages();
};

class LocaleDB : public ServiceDatabase {};

class MainWindow : public QMainWindow {
  Q_OBJECT

  PackageTable *tablePackagesRemote = new PackageTable;
  PackageTable *tablePackagesLocal = new PackageTable;

  PackageModel *modelRemote = new PackageModel;
  PackageModel *modelLocal = new PackageModel;

public:
  explicit MainWindow(QWidget *parent = 0);

private slots:

private:
public slots:
  void fillTables();
};

class PackageDB : public PackagesDatabase {};

class ServiseButtons : public QPushButton {};

#endif // MAINWINDOW_H
