
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QVBoxLayout>
#include <QPushButton>

// See https://qt-project.org/wiki/ApplicationRestart


//-----------------------------------------------------------------------------
class RestartManager : public QObject
{
  Q_OBJECT
public:
  typedef RestartManager Self;
  RestartManager(QObject * parent = 0) : QObject(parent){}
  virtual ~RestartManager(){}
  
  static int const EXIT_CODE_REBOOT;
  
public slots:
  void restart()
  {
    qDebug() << "Restarting ...";
    QCoreApplication::exit(Self::EXIT_CODE_REBOOT);
  }
private:
  Q_DISABLE_COPY(RestartManager);
};

//-----------------------------------------------------------------------------
int const RestartManager::EXIT_CODE_REBOOT = -123456789;

//-----------------------------------------------------------------------------
int mainLoop(int argc, char*argv[])
{
  QApplication app(argc, argv);
  
  QWidget topLevel;
  QVBoxLayout * layout = new QVBoxLayout();
  topLevel.setLayout(layout);
  
  QPushButton * restartButton = new QPushButton("Restart");
  RestartManager restartManager;
  QObject::connect(restartButton, SIGNAL(clicked()), &restartManager, SLOT(restart()));
  layout->addWidget(restartButton);
  
  QPushButton * quitButton = new QPushButton("Quit");
  QObject::connect(quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
  layout->addWidget(quitButton);
  
  topLevel.show();
    
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
  while(currentExitCode == RestartManager::EXIT_CODE_REBOOT);

  return currentExitCode;
}

#include "moc_RestartWithQProcessDetached.cpp"

