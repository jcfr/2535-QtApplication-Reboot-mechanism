
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QProcess>

#include "ctkAbstractRestartManager.h"
#include "ctkRestartWidget.h"

//-----------------------------------------------------------------------------
class ctkRestartManager : public ctkAbstractRestartManager
{
  Q_OBJECT
public:
  typedef ctkAbstractRestartManager Superclass;
  ctkRestartManager(QObject * parent = 0) : Superclass(parent){}
  virtual ~ctkRestartManager(){}
public slots:
  void restart()
  {
    this->Superclass::restart();
    QCoreApplication * coreApp = QCoreApplication::instance();
    QProcess::startDetached(coreApp->applicationFilePath(), coreApp->arguments());
    QCoreApplication::quit();
  }
private:
  Q_DISABLE_COPY(ctkRestartManager);
};

//-----------------------------------------------------------------------------
int main(int argc, char*argv[])
{
  QApplication app(argc, argv);
  
  qDebug() << "PID" << QApplication::applicationPid();
  
  ctkRestartManager restartManager;
  ctkRestartWidget restartWidget;
  restartWidget.setRestartManager(&restartManager);

  restartWidget.show();
  
  return app.exec();
}

#include "moc_RestartWithQProcessDetached.cpp"

