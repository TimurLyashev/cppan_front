#include "mainwindow.h"
#include "QGridLayout"
#include "QLabel"
#include "QLineEdit"
#include "QPushButton"
#include "QTabBar"
#include <QButtonGroup>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>

#include <build.h>
#include <database.h>
#include <package_store.h>
#include <resolver.h>

void MainWindow::fillTables() {
  auto &pdbr = getPackagesDatabase(); // remote database
  auto setPackagesRemote = pdbr.getMatchingPackages<std::set>();

  auto &pdbl = getServiceDatabase(); // local database
  auto setPackagesLocal = pdbl.getInstalledPackages();

  int pkgCounterRemote = -1;
  int pkgCounterLocal = -1;

  QStandardItem *itemName;
  QStandardItem *itemCheck;
  QStandardItem *itemVersion;

  for (auto &pkg : setPackagesLocal) {
    QString pkgVersion = QString::fromStdString(pkg.version.toString());
    QString pkgName = QString::fromStdString(pkg.ppath);
    itemVersion = new QStandardItem(pkgVersion);
    itemName = new QStandardItem(pkgName);
    pkgCounterLocal++;
    modelLocal->setItem(pkgCounterLocal, 1, itemName);
    modelLocal->setItem(pkgCounterLocal, 2, itemVersion);
    itemCheck = new QStandardItem(true);
    itemCheck->setCheckable(true);
    itemCheck->setCheckState(Qt::Checked);
    modelLocal->setItem(pkgCounterLocal, 0, itemCheck);
  }

  for (auto &pkg : setPackagesRemote) {
    QString pkgVersion = 0;
    QString pkgName = QString::fromStdString(pkg.toString());
    itemName = new QStandardItem(pkgName);

    modelRemote->setItem(pkgCounterRemote, 1, itemName);
    QList<QStandardItem *> lstName =
        modelLocal->findItems(pkgName, Qt::MatchExactly, 1);
    auto versions = pdbr.getVersionsForPackage(pkg);
    std::reverse(std::begin(versions), std::end(versions));
    for (auto &v : versions) {

      pkgVersion = QString::fromStdString(v.toString());
      itemVersion = new QStandardItem(pkgVersion);
      modelRemote->setItem(pkgCounterRemote, 2, itemVersion);
      itemCheck = new QStandardItem(true);
      itemCheck->setCheckable(true);

      QList<QStandardItem *> lstVersion =
          modelLocal->findItems(pkgVersion, Qt::MatchExactly, 2);

      if ((lstName.size() != 0) && (lstVersion.size() != 0))
        itemCheck->setCheckState(Qt::Checked);
      else
        itemCheck->setCheckState(Qt::Unchecked);

      modelRemote->setItem(pkgCounterRemote, 0, itemCheck);

      pkgCounterRemote++;
    }
  }
}

PackagesSet PackageModel::generateDifferenceLocalePackages() {
  PackagesSet Modified;
  auto &pdbl = getServiceDatabase(); // local database
  auto setPackagesLocal = pdbl.getInstalledPackages();
  std::vector<QString> ModifiedPackageName;
  std::vector<QString>::iterator it;
  for (int i = 0; i <= rowCount(); i++) {
    QModelIndex indexCheker = index(i, 0);
    QModelIndex indexName = index(i, 1);

    if (indexCheker.data(Qt::CheckStateRole) == Qt::Unchecked) {
      ModifiedPackageName.push_back(indexName.data().toString());
    }
  }

  for (auto &package : setPackagesLocal) {
    it = find(ModifiedPackageName.begin(), ModifiedPackageName.end(),
              QString::fromStdString(package.ppath));
    if (it != ModifiedPackageName.end())
      Modified.insert(package);
  }

  return Modified;
}

void PackageModel::RemoveUnchekedLocale() {
  auto PackagesToDelete = generateDifferenceLocalePackages();

  cleanPackages(PackagesToDelete, 255); // 255 - All
}

Packages PackageModel::generateDifferenceRemotePackagesInstall() {
  Packages DifferenceInstall;
  Packages::iterator search;
  auto &pdbr = getPackagesDatabase(); // remote database
  auto setPackagesRemote = pdbr.getMatchingPackages<std::set>();
  auto &pdbl = getServiceDatabase(); // local database
  auto setPackagesLocal = pdbl.getInstalledPackages();

  for (int i = 0; i <= rowCount(); i++) {
    QModelIndex indexCheker = index(i, 0);

    int j = i;
    QModelIndex indexName = index(j, 1);
    while (j >= 0) {
      indexName = index(j, 1);
      if (indexName.data().toString().size() != 0)
        break;
      else
        j--;
    }

    QModelIndex indexVersion = index(i, 2);

    if (indexCheker.data(Qt::CheckStateRole) == Qt::Checked) {
      Package p;
      p.ppath = indexName.data().toString().toStdString();
      p.version = indexVersion.data().toString().toStdString();
      p.createNames();
      DifferenceInstall[p.ppath.toString()] = p;
    }
  }

  for (auto &package : setPackagesLocal) {

    search = DifferenceInstall.find(package.ppath.toString());
    if (search != DifferenceInstall.end())
      DifferenceInstall.erase(search);
  }

  return DifferenceInstall;
}

void PackageModel::InstallChekedRemote() {
  auto PackagesToInstall = generateDifferenceRemotePackagesInstall();

  resolve_dependencies(PackagesToInstall);
  auto &sdb = getServiceDatabase();

  for (auto &package : PackagesToInstall) {
    sdb.addInstalledPackage(package.second);
  }
}

void PackageModel::buildInstalledPackages() {
  auto &pdbl = getServiceDatabase(); // local database
  auto setPackagesLocal = pdbl.getInstalledPackages();

  for (auto &package : setPackagesLocal) {
    build_package(package.target_name);
  }
}

void PackageModel::buildSelectedPackages() {
  auto &pdbl = getServiceDatabase(); // local database
  auto setPackagesLocal = pdbl.getInstalledPackages();

  for (auto &package : setPackagesLocal) {
    build_package(package.target_name);
  }
}

void PackageTable::searchPackage() {

  // tableView->selectRow(idRow);
  //// Имитируем нажатие кнопки Tab, чтобы выделить строку
  // QKeyEvent* pe = new QKeyEvent(QEvent::KeyPress,
  //    Qt::Key_Tab, Qt::NoModifier, "Tab");
  // QApplication::sendEvent(this, pe);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  auto buttonInstall = new QPushButton("Install");
  auto buttonRemove = new QPushButton("Remove");
  auto buttonBuildAll = new QPushButton("Build All");
  auto buttonBuild = new QPushButton("Build");

  buttonBuild->setFixedWidth(100);
  buttonInstall->setFixedWidth(100);
  buttonRemove->setFixedWidth(100);

  auto buttonGroup = new QButtonGroup;
  buttonGroup->addButton(buttonInstall);
  buttonGroup->addButton(buttonBuild);

  auto tabPackages = new QTabWidget;
  tabPackages->addTab(tablePackagesLocal, "Local");
  tabPackages->addTab(tablePackagesRemote, "Remote");

  auto labelPackagesList = new QLabel;
  auto labelPackagesInfo = new QLabel;
  auto labelInfo = new QLabel;

  tablePackagesRemote->setMaximumWidth(700);
  tablePackagesRemote->setMinimumWidth(700);
  tablePackagesLocal->setMaximumWidth(700);
  tablePackagesLocal->setMinimumWidth(700);

  labelPackagesList->setText("Packages List");
  labelPackagesInfo->setText("Package Information");
  labelInfo->setText(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua.");
  labelInfo->setWordWrap(true);
  labelInfo->adjustSize();

  auto lineSearch = new QLineEdit("Search");

  QGridLayout *mainLayout = new QGridLayout;
  QGridLayout *leftLayout = new QGridLayout;
  QGridLayout *rightLayout = new QGridLayout;
  mainLayout->setMargin(1);

  mainLayout->addLayout(leftLayout, 0, 0);
  // mainLayout->addLayout(rightLayout, 0, 1); //отображение информации о пакете

  leftLayout->addWidget(labelPackagesList, 0, 0);
  leftLayout->setColumnMinimumWidth(0, 360);
  leftLayout->setColumnStretch(0, 0);
  leftLayout->setColumnStretch(1, 10);
  leftLayout->addWidget(lineSearch, 1, 0);
  leftLayout->addWidget(tabPackages, 2, 0, 1, 2);
  leftLayout->addWidget(buttonInstall, 3, 0);
  leftLayout->addWidget(buttonRemove, 3, 1);
  leftLayout->addWidget(buttonBuild, 4, 0);
  leftLayout->addWidget(buttonBuildAll, 4, 1);

  // rightLayout->addWidget(labelPackagesInfo, 0, 0);
  // rightLayout->addWidget(labelInfo, 3, 0, 1, 2);
  // rightLayout->addWidget(buttonInstall, 4, 0);
  // rightLayout->addWidget(buttonRemove, 4, 1);
  // rightLayout->addWidget(buttonBuild, 5, 0);

  mainLayout->setMargin(1);

  tablePackagesRemote->setModel(modelRemote);
  tablePackagesRemote->setEditTriggers(QAbstractItemView::NoEditTriggers);

  tablePackagesLocal->setModel(modelLocal);
  tablePackagesLocal->setEditTriggers(QAbstractItemView::NoEditTriggers);

  QStringList horizontalHeader;
  horizontalHeader.append("");
  horizontalHeader.append("Name");
  horizontalHeader.append("Version");

  QStringList verticalHeader;
  verticalHeader.append("1");
  verticalHeader.append("2");

  modelRemote->setHorizontalHeaderLabels(horizontalHeader);
  modelRemote->setVerticalHeaderLabels(verticalHeader);

  modelLocal->setHorizontalHeaderLabels(horizontalHeader);
  modelLocal->setVerticalHeaderLabels(verticalHeader);

  auto centralWidget = new QWidget;
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  fillTables();

  connect(buttonRemove, SIGNAL(clicked()), modelLocal,
          SLOT(RemoveUnchekedLocale()));
  connect(buttonRemove, SIGNAL(clicked()), SLOT(fillTables()));

  connect(buttonInstall, SIGNAL(clicked()), modelRemote,
          SLOT(InstallChekedRemote()));
  connect(buttonInstall, SIGNAL(clicked()), SLOT(fillTables()));
  connect(buttonInstall, SIGNAL(clicked()), SLOT(fillTables()));

  connect(buttonBuildAll, SIGNAL(clicked()), modelLocal,
          SLOT(buildInstalledPackages()));

  connect(buttonBuild, SIGNAL(clicked()), modelLocal,
          SLOT(buildSelectedPackages()));

  connect(lineSearch, SIGNAL(editingFinished()), tabPackages->currentWidget(),
          SLOT(searchPackage()));

  tablePackagesRemote->resizeColumnsToContents();
  tablePackagesLocal->resizeColumnsToContents();

  resize(minimumSizeHint());
  tablePackagesRemote->minimumSizeHint();
  tablePackagesRemote->adjustSize();
}
