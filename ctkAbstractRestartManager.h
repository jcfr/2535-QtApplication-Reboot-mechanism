
#ifndef __ctkAbstractRestartManager_h
#define __ctkAbstractRestartManager_h

// Qt includes
#include <QObject>

// STD includes
#include <cstdio> // For _fileno or fileno

class ctkErrorLogModel;

//-----------------------------------------------------------------------------
class ctkAbstractRestartManager : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  typedef ctkAbstractRestartManager Self;
  ctkAbstractRestartManager(QObject * parent = 0);
  virtual ~ctkAbstractRestartManager(){}
  
  static void displayStandardOutputNo();
  
  ctkErrorLogModel* errorLogModel()const;
  
public slots:
  virtual void restart();
  
private:
  Q_DISABLE_COPY(ctkAbstractRestartManager);
  
  ctkErrorLogModel * ErrorLogModel;
};

#endif

