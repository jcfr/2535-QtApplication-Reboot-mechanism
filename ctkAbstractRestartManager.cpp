
// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QTextStream>

#include "ctkAbstractRestartManager.h"
#include "ctkErrorLogFDMessageHandler.h"
#include "ctkErrorLogModel.h"

// STD includes
#include <cstdio>
#ifdef Q_OS_WIN32
# include <fcntl.h>  // For _O_TEXT
# include <io.h>     // For _pipe, _dup and _dup2
#else
# include <unistd.h> // For pipe, dup and dup2
#endif

// --------------------------------------------------------------------------
#define APPEND_TO_FILE(stream) \
  { \
  QString s; \
  QTextStream ts(&s); \
  ts << "[instance " << this << "] - " << stream; \
  this->appendToFile("log.txt", s); \
  }

// --------------------------------------------------------------------------
ctkFDConsumerThread::ctkFDConsumerThread(QObject * parent):Superclass(parent)
{
  this->Enabled = false;
  this->TerminalOutputFile = stdout;
  this->SavedFDNumber = 0;
  this->RestartManager = 0;
}

// --------------------------------------------------------------------------
ctkFDConsumerThread::~ctkFDConsumerThread()
{
}

// --------------------------------------------------------------------------
void ctkFDConsumerThread::appendToFile(const QString& fileName, const QString& text)
{
  if (this->RestartManager)
    {
    this->RestartManager->appendToFile(fileName, text);
    }
}

// --------------------------------------------------------------------------
bool ctkFDConsumerThread::enabled()const
{
  QMutexLocker locker(&this->EnableMutex);
  return this->Enabled;
}

// --------------------------------------------------------------------------
void ctkFDConsumerThread::setEnabled(bool value)
{
  QMutexLocker locker(&this->EnableMutex);
  this->Enabled = value;
}

// --------------------------------------------------------------------------
void ctkFDConsumerThread::run()
{
  APPEND_TO_FILE("Thread - STARTED");
  while(true)
    {
    char c = '\0';
    QString line;
    while(c != '\n')
      {
      APPEND_TO_FILE("Thread - SHOULD BLOCK - Attempt to read one char from Pipe[0]: " << this->Pipe[0]);
#ifdef Q_OS_WIN32
      int res = _read(this->Pipe[0], &c, 1); // When used with pipe, read() is blocking
#else
      ssize_t res = read(this->Pipe[0], &c, 1); // When used with pipe, read() is blocking
#endif
      APPEND_TO_FILE("Thread - Number of char read from Pipe[0]: " << this->Pipe[0] << ", count: " << res);
      if (res == -1)
        {
        APPEND_TO_FILE("Thread - Failed to read one char from Pipe[0]: " << this->Pipe[0]);
        break;
        }
      if (c != '\n')
        {
        APPEND_TO_FILE("Thread - Read one char from Pipe[0]: " << this->Pipe[0] << ", c [" << (char)c << "]" );
        line += c;
        }
      }

    APPEND_TO_FILE("Thread - Enabled: " << this->enabled());
    if (!this->enabled())
      {
      break;
      }
    }
  APPEND_TO_FILE("Thread - STOPPED (Exit while loop)");
}
  
// --------------------------------------------------------------------------
void ctkAbstractRestartManager::appendToFile(const QString& fileName, const QString& text)
{
  QMutexLocker locker(&this->AppendToFileMutex);
  QFile f(fileName);
  f.open(QFile::Append);
  QTextStream s(&f);
  s << QDateTime::currentDateTime().toString() << " - " << text << "\n";
  f.close();
}
  
//------------------------------------------------------------------------------
ctkAbstractRestartManager::ctkAbstractRestartManager(QObject * parent) : Superclass(parent)
{
  this->Thread.RestartManager = this;
  Self::displayStandardOutputNo();
  this->ErrorLogModel = new ctkErrorLogModel(this);
  this->ErrorLogModel->setTerminalOutputs(ctkErrorLogModel::All);
  this->ErrorLogModel->registerMsgHandler(new ctkErrorLogFDMessageHandler);
}

//------------------------------------------------------------------------------
ctkAbstractRestartManager::~ctkAbstractRestartManager()
{
  this->ErrorLogModel->setAllMsgHandlerEnabled(false);
}

//------------------------------------------------------------------------------
void ctkAbstractRestartManager::displayStandardOutputNo()
{
#ifdef Q_OS_WIN32
  qDebug() << "stdout:" << _fileno(stdout) << " - " << stdout;
  qDebug() << "stderr:" << _fileno(stderr) << " - " << stderr;
#else
  qDebug() << "stdout:" << fileno(stdout) << " - " << stdout;
  qDebug() << "stderr:" << fileno(stderr) << " - " << stderr;
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

//------------------------------------------------------------------------------
void ctkAbstractRestartManager::enableFileDescriptorReading()
{
  ctkFDConsumerThread * thread = &this->Thread;
  
  if (thread->enabled())
    {
    return;
    }
  
  APPEND_TO_FILE(">>>>>>>>>>>>>>> Enabling <<<<<<<<<<<<<<");

  // Flush (stdout|stderr) so that any buffered messages are delivered
  fflush(thread->TerminalOutputFile);

  // Save position of current standard output
  fgetpos(thread->TerminalOutputFile, &thread->SavedFDPos);
  
  int status = pipe(thread->Pipe);
  if (status != 0)
    {
    APPEND_TO_FILE("Failed to create pipe !");
    return;
    }
  
  APPEND_TO_FILE("fileno(thread->TerminalOutputFile): " << fileno(thread->TerminalOutputFile));
  thread->SavedFDNumber = dup(fileno(thread->TerminalOutputFile));
  APPEND_TO_FILE("dup: thread->savedFDNumber: " << thread->SavedFDNumber);
  dup2(thread->Pipe[1], fileno(thread->TerminalOutputFile));
  APPEND_TO_FILE("dup2: thread->Pipe[1]: " << thread->Pipe[1]);
  close(thread->Pipe[1]);
  APPEND_TO_FILE("Closed thread->Pipe[1]: " << thread->Pipe[1]);
  
  thread->setEnabled(true);
  thread->start();
  
  QString text("Hello !\n");
  write(fileno(thread->TerminalOutputFile), qPrintable(text), text.size());

  /*for(int i = 0; i < text.size(); ++i)
    { 
    char c = '\0';
    int res = read(thread->Pipe[0], &c, 1); // When used with pipe, read() is blocking
    APPEND_TO_FILE("Number of char read from Pipe[0]: " << thread->Pipe[0] << ", count: " << res << ", c: " << c);
    if (res <= 0)
      {
      APPEND_TO_FILE("Failed to read one char from Pipe[0]: " << thread->Pipe[0]);
      break;
      }
    }*/
    
  /*char c2 = '\0';
  APPEND_TO_FILE("SHOULD BLOCK");
  int res2 = read(thread->Pipe[0], &c2, 1); // When used with pipe, read() is blocking
  APPEND_TO_FILE("NOT HAPPEN - Number of char read from Pipe[0]: " << thread->Pipe[0] << ", count: " << res2 << ", c: " << c2);
  if (res2 <= 0)
    {
    APPEND_TO_FILE("Failed to read one char from Pipe[0]: " << thread->Pipe[0]);
    }*/
}

//------------------------------------------------------------------------------
void ctkAbstractRestartManager::disableFileDescriptorReading()
{
  ctkFDConsumerThread * thread = &this->Thread;
  
  if (!thread->enabled())
    {
    return;
    }
    
  APPEND_TO_FILE(">>>>>>>>>>>>>>> Disabling <<<<<<<<<<<<<<");
  
  fflush(thread->TerminalOutputFile);

  thread->setEnabled(false);

  APPEND_TO_FILE("Write new line to [terminalOutputFile: " << thread->TerminalOutputFile 
    << ", FileNo: " << fileno(thread->TerminalOutputFile) << "]");
  QString newline("\n");
  write(fileno(thread->TerminalOutputFile), qPrintable(newline), newline.size());
  fflush(thread->TerminalOutputFile);

  // Close files and restore standard output to stdout or stderr - which should be the terminal
  APPEND_TO_FILE("dup2: Restoring SavedFDNumber: " << thread->SavedFDNumber);
  dup2(thread->SavedFDNumber, fileno(thread->TerminalOutputFile));
  APPEND_TO_FILE("dup2: thread->SavedFDNumber: " << thread->SavedFDNumber);
  close(thread->SavedFDNumber);
  APPEND_TO_FILE("Closed thread->SavedFDNumber: " << thread->SavedFDNumber);

  clearerr(thread->TerminalOutputFile);
  fsetpos(thread->TerminalOutputFile, &thread->SavedFDPos);
  
  close(thread->Pipe[0]);
  APPEND_TO_FILE("Closed thread->Pipe[0]: " << thread->Pipe[0]);
  
  {
    QString text("Hello Again!\n");
    write(fileno(thread->TerminalOutputFile), qPrintable(text), text.size());
  }
  
  thread->SavedFDNumber = 0;
}

#include "moc_ctkAbstractRestartManager.h"

