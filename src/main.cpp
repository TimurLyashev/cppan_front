#include "mainwindow.h"
#include <QApplication>

#include <config.h>
#include <database.h>
#include <settings.h>

#include <primitives/log.h>

#ifdef QT_STATIC
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif

void load_current_config() {
  try {
    // load local settings for storage dir
    Config c;
    // c.allow_relative_project_names = true;
    // c.allow_local_dependencies = true;
    c.load_current_config_settings();
  } catch (...) {
    // ignore everything
  }

  // load proxy settings early
  httpSettings.proxy = Settings::get_local_settings().proxy;
}

void init() {
  // initial sequence
  LoggerSettings log_settings;
  log_settings.log_level = "INFO";
  log_settings.simple_logger = true;
  log_settings.print_trace = true;
  initLogger(log_settings);

  // initialize CPPAN structures (settings), do not remove
  auto &us = Settings::get_local_settings();
  // us.disable_update_checks = true;

  load_current_config();
  getServiceDatabase();
}

int main(int argc, char *argv[]) {
  init();
  QApplication a(argc, argv);
  MainWindow w;
  w.setWindowTitle("CPPAN Package Manager");
  w.setBaseSize(800, 600);
  w.show();

  return a.exec();
}
