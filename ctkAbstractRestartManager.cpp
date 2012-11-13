
// Qt includes
#include <QDebug>

#include "ctkAbstractRestartManager.h"
#include "ctkErrorLogFDMessageHandler.h"
#include "ctkErrorLogModel.h"

//------------------------------------------------------------------------------
ctkAbstractRestartManager::ctkAbstractRestartManager(QObject * parent) : Superclass(parent)
{
  Self::displayStandardOutputNo();
  this->ErrorLogModel = new ctkErrorLogModel(this);
  this->ErrorLogModel->setTerminalOutputs(ctkErrorLogModel::All);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogFDMessageHandler);
}

//------------------------------------------------------------------------------
void ctkAbstractRestartManager::displayStandardOutputNo()
{
#ifdef Q_OS_WIN32
  qDebug() << "stdout:" << _fileno(stdout);
  qDebug() << "stderr:" << _fileno(stderr);
#else
  qDebug() << "stdout:" << fileno(stdout);
  qDebug() << "stderr:" << fileno(stderr);
#endif 
}

//------------------------------------------------------------------------------
ctkErrorLogModel* ctkAbstractRestartManager::errorLogModel()const
{
  return this->ErrorLogModel;
}

//------------------------------------------------------------------------------
void ctkAbstractRestartManager::restart()
{
  qDebug() << "Restarting";
}

#include "moc_ctkAbstractRestartManager.h"

