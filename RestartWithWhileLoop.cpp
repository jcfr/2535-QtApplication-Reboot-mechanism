
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QObject>

#include "ctkAbstractRestartManager.h"
#include "ctkRestartWidget.h"

// See https://qt-project.org/wiki/ApplicationRestart


//-----------------------------------------------------------------------------
class ctkRestartManager : public ctkAbstractRestartManager
{
  Q_OBJECT
public:
  typedef ctkRestartManager Self;
  typedef ctkAbstractRestartManager Superclass;
  ctkRestartManager(QObject * parent = 0) : Superclass(parent){}
  virtual ~ctkRestartManager(){}
  
  static int const EXIT_CODE_REBOOT;
  
public slots:
  void restart()
  {
    this->Superclass::restart();
    QCoreApplication::exit(Self::EXIT_CODE_REBOOT);
  }
private:
  Q_DISABLE_COPY(ctkRestartManager);
};

//-----------------------------------------------------------------------------
int const ctkRestartManager::EXIT_CODE_REBOOT = -123456789;

//-----------------------------------------------------------------------------
int mainLoop(int argc, char*argv[])
{
  QApplication app(argc, argv);
  
  qDebug() << "PID" << QApplication::applicationPid();
  
  ctkRestartManager restartManager;
  ctkRestartWidget restartWidget;
  restartWidget.setRestartManager(&restartManager);
  
  restartWidget.show();
    
  return app.exec();
}

//-----------------------------------------------------------------------------
int main(int argc, char*argv[])
{
  int currentExitCode = 0;
  do
    {
    currentExitCode = mainLoop(argc, argv);
    }
  while(currentExitCode == ctkRestartManager::EXIT_CODE_REBOOT);

  return currentExitCode;
}

#include "moc_RestartWithQProcessDetached.cpp"

