
#ifndef __ctkAbstractRestartManager_h
#define __ctkAbstractRestartManager_h

// Qt includes
#include <QMutex>
#include <QObject>
#include <QThread>

// STD includes
#include <cstdio> // For _fileno or fileno

class ctkErrorLogModel;
class ctkAbstractRestartManager;

//-----------------------------------------------------------------------------
class ctkFDConsumerThread : public QThread
{
public:
  typedef QThread Superclass;
  ctkFDConsumerThread(QObject * parent = 0);
  virtual ~ctkFDConsumerThread();
  
  void appendToFile(const QString& fileName, const QString& text);

  bool enabled()const;
  void setEnabled(bool value);
  
protected:
  virtual void run();

public:
  ctkAbstractRestartManager * RestartManager;
    
  int    Pipe[2]; // 0: Read, 1: Write
  FILE*  TerminalOutputFile;
  int    SavedFDNumber;
  fpos_t SavedFDPos;
  
private:
  mutable QMutex EnableMutex;
  bool Enabled;
};

//-----------------------------------------------------------------------------
class ctkAbstractRestartManager : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  typedef ctkAbstractRestartManager Self;
  ctkAbstractRestartManager(QObject * parent = 0);
  virtual ~ctkAbstractRestartManager();
  
  static void displayStandardOutputNo();
  
  ctkErrorLogModel* errorLogModel()const;
  
  void appendToFile(const QString& fileName, const QString& text);
  
public slots:
  virtual void restart();

  void enableFileDescriptorReading();
  void disableFileDescriptorReading();
  
private:
  Q_DISABLE_COPY(ctkAbstractRestartManager);
  
  ctkFDConsumerThread Thread;
  QMutex AppendToFileMutex;
  ctkErrorLogModel * ErrorLogModel;
};

#endif

