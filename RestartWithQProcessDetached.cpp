
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QVBoxLayout>
#include <QPushButton>

//-----------------------------------------------------------------------------
class RestartManager : public QObject
{
  Q_OBJECT
public:
  RestartManager(QObject * parent = 0) : QObject(parent){}
  virtual ~RestartManager(){}
public slots:
  void restart()
  {
    qDebug() << "Restarting ...";
    QCoreApplication * coreApp = QCoreApplication::instance();
    QProcess::startDetached(coreApp->applicationFilePath(), coreApp->arguments());
    QCoreApplication::quit();
  }
private:
  Q_DISABLE_COPY(RestartManager);
};

//-----------------------------------------------------------------------------
int main(int argc, char*argv[])
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

#include "moc_RestartWithQProcessDetached.cpp"

