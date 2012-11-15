/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QDebug>
#include <QFile>

// CTK includes
#include "ctkErrorLogFDMessageHandler.h"
#include "ctkErrorLogFDMessageHandler_p.h"
#include "ctkUtils.h"

// STD includes
#include <cstdio>
#ifdef Q_OS_WIN32
# include <fcntl.h>  // For _O_TEXT
# include <io.h>     // For _pipe, _dup and _dup2
#else
# include <unistd.h> // For pipe, dup and dup2
#endif

// --------------------------------------------------------------------------
// ctkFDHandler methods
// See http://stackoverflow.com/questions/5419356/redirect-stdout-stderr-to-a-string
// and http://stackoverflow.com/questions/955962/how-to-buffer-stdout-in-memory-and-write-it-from-a-dedicated-thread

// --------------------------------------------------------------------------
#define APPEND_TO_FILE(stream) \
  { \
  QString s; \
  QTextStream ts(&s); \
  ts << "[instance " << this << "] - " << stream; \
  this->appendToFile("log.txt", s); \
  }
  
// --------------------------------------------------------------------------
ctkFDHandler::ctkFDHandler(ctkErrorLogFDMessageHandler* messageHandler,
                           ctkErrorLogLevel::LogLevel logLevel,
                           ctkErrorLogModel::TerminalOutput terminalOutput)
{
  this->MessageHandler = messageHandler;
  this->LogLevel = logLevel;
  this->TerminalOutput = terminalOutput;
  this->SavedFDNumber = 0;
  this->Enabled = false;
}

#include <iostream>

// --------------------------------------------------------------------------
ctkFDHandler::~ctkFDHandler()
{
}

// --------------------------------------------------------------------------
FILE* ctkFDHandler::terminalOutputFile()
{
  return this->TerminalOutput == ctkErrorLogModel::StandardOutput ? stdout : stderr;
}

// --------------------------------------------------------------------------
void ctkFDHandler::enableFileDescriptorRedirection()
{
  APPEND_TO_FILE(">>>>>>>>>>> Enabling <<<<<<<<<<<<");
  APPEND_TO_FILE("TerminalOutputFile: " << this->terminalOutputFile());
  
  #ifdef Q_OS_WIN32
  int status = _pipe(this->Pipe, 65536, _O_TEXT);
#else
  int status = pipe(this->Pipe);
#endif
  if (status != 0)
    {
    APPEND_TO_FILE("Failed to create pipe !");
    //qCritical().nospace() << "ctkFDHandler - Failed to create pipe !";
    return;
    }
  
  // Flush (stdout|stderr) so that any buffered messages are delivered
  fflush(this->terminalOutputFile());

  // Save position of current standard output
  fgetpos(this->terminalOutputFile(), &this->SavedFDPos);
  
#ifdef Q_OS_WIN32
  APPEND_TO_FILE("_fileno(this->terminalOutputFile()): " << _fileno(this->terminalOutputFile()));
  this->SavedFDNumber = _dup(_fileno(this->terminalOutputFile()));
  APPEND_TO_FILE("SavedFDNumber: " << this->SavedFDNumber);
  _dup2(this->Pipe[1], _fileno(this->terminalOutputFile()));
  APPEND_TO_FILE("Pipe[1]: " << this->Pipe[1]);
  _close(this->Pipe[1]);
  APPEND_TO_FILE("Closed Pipe[1]: " << this->Pipe[1]);
#else
  APPEND_TO_FILE("fileno(this->terminalOutputFile()): " << fileno(this->terminalOutputFile()));
  this->SavedFDNumber = dup(fileno(this->terminalOutputFile()));
  APPEND_TO_FILE("dup: SavedFDNumber: " << this->SavedFDNumber);
  dup2(this->Pipe[1], fileno(this->terminalOutputFile()));
  APPEND_TO_FILE("dup2: Pipe[1]: " << this->Pipe[1]);
  close(this->Pipe[1]);
  APPEND_TO_FILE("Closed Pipe[1]: " << this->Pipe[1]);
#endif

  // Start polling thread
  this->Enabled = true;
  APPEND_TO_FILE("Enabled - Start polling THREAD");
  this->start();
}

// --------------------------------------------------------------------------
void ctkFDHandler::disableFileDescriptorRedirection()
{
  APPEND_TO_FILE(">>>>>>>>>>> Disabling <<<<<<<<<<<<");
    
  // Flush stdout or stderr so that any buffered messages are delivered
  APPEND_TO_FILE("Flushed terminalOutputFile: " << this->terminalOutputFile());
  fflush(this->terminalOutputFile());

  // Stop polling thread
  {
    APPEND_TO_FILE("Stopping polling THREAD");
    QMutexLocker locker(&this->EnableMutex);
    this->Enabled = false;
  }
  
  // Print one character to "unblock" the read function associated with the polling thread
  APPEND_TO_FILE("Print one character to \"unblock\" the read function associated with the polling THREAD");
  ssize_t res = write(fileno(this->terminalOutputFile()), "\n", 1);
  if (res == -1)
    {
    APPEND_TO_FILE("Failed to print unblocking char !");
    return;
    }
  fflush(this->terminalOutputFile());

  APPEND_TO_FILE("Write new line to [terminalOutputFile: " << this->terminalOutputFile() 
    << ", FileNo: " << fileno(this->terminalOutputFile()) << "]");
  QString newline("\n");
#ifdef Q_OS_WIN32
  _write(fileno(this->terminalOutputFile()), qPrintable(newline), newline.size());
#else
  write(fileno(this->terminalOutputFile()), qPrintable(newline), newline.size());
#endif

  // Wait the polling thread graciously terminates
  APPEND_TO_FILE("Wait the polling thread graciously terminates");
  this->wait();

  // Close files and restore standard output to stdout or stderr - which should be the terminal
  APPEND_TO_FILE("dup2: Restoring SavedFDNumber: " << this->SavedFDNumber);
#ifdef Q_OS_WIN32
  _dup2(this->SavedFDNumber, _fileno(this->terminalOutputFile()));
  _close(this->SavedFDNumber);
#else
  dup2(this->SavedFDNumber, fileno(this->terminalOutputFile()));
  close(this->SavedFDNumber);
#endif
  clearerr(this->terminalOutputFile());
  fsetpos(this->terminalOutputFile(), &this->SavedFDPos);

  this->SavedFDNumber = 0;
}

// --------------------------------------------------------------------------
void ctkFDHandler::setEnabled(bool value)
{
  if (this->Enabled == value)
    {
    return;
    }

  if (value)
    {
    this->enableFileDescriptorRedirection();
    }
  else
    {
    this->disableFileDescriptorRedirection();
    }

  ctkErrorLogTerminalOutput * terminalOutput =
      this->MessageHandler->terminalOutput(this->TerminalOutput);
  if(terminalOutput)
    {
    APPEND_TO_FILE("Set terminal output file descriptor: " << this->SavedFDNumber);
    terminalOutput->setFileDescriptor(this->SavedFDNumber);
    }
}

// --------------------------------------------------------------------------
bool ctkFDHandler::enabled()const
{
  QMutexLocker locker(&this->EnableMutex);
  return this->Enabled;
}

// --------------------------------------------------------------------------
void ctkFDHandler::appendToFile(const QString& fileName, const QString& text)
{
  QMutexLocker locker(&this->AppendToFileMutex);
  QFile f(fileName);
  f.open(QFile::Append);
  QTextStream s(&f);
  s << QDateTime::currentDateTime().toString() << " - " << text << "\n";
  f.close();
}

// --------------------------------------------------------------------------
void ctkFDHandler::run()
{
  APPEND_TO_FILE("Thread - STARTED");
  while(true)
    {
    char c = '\0';
    QString line;
    while(c != '\n')
      {
      APPEND_TO_FILE("Thread - Attempt to read one char from Pipe[0]: " << this->Pipe[0]);
#ifdef Q_OS_WIN32
      int res = _read(this->Pipe[0], &c, 1); // When used with pipe, read() is blocking
#else
      ssize_t res = read(this->Pipe[0], &c, 1); // When used with pipe, read() is blocking
#endif
      APPEND_TO_FILE("Thread - Number of char read from Pipe[0]: " << this->Pipe[0] << ", count: " << res);
      if (!this->enabled())
        {
        break;
        }
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

    Q_ASSERT(this->MessageHandler);
    this->MessageHandler->handleMessage(
      ctk::qtHandleToString(QThread::currentThreadId()),
      this->LogLevel,
      this->MessageHandler->handlerPrettyName(),
      line);
    }
  APPEND_TO_FILE("Thread - STOPPED (Exit while loop)");
}

// --------------------------------------------------------------------------
// ctkErrorLogFDMessageHandlerPrivate

// --------------------------------------------------------------------------
class ctkErrorLogFDMessageHandlerPrivate
{
public:
  ctkErrorLogFDMessageHandlerPrivate();
  ~ctkErrorLogFDMessageHandlerPrivate();

  ctkFDHandler * StdOutFDHandler;
  ctkFDHandler * StdErrFDHandler;
};

// --------------------------------------------------------------------------
// ctkErrorLogFDMessageHandlerPrivate methods

//------------------------------------------------------------------------------
ctkErrorLogFDMessageHandlerPrivate::ctkErrorLogFDMessageHandlerPrivate()
{
}

//------------------------------------------------------------------------------
ctkErrorLogFDMessageHandlerPrivate::~ctkErrorLogFDMessageHandlerPrivate()
{
  delete this->StdOutFDHandler;
  delete this->StdErrFDHandler;
}

//------------------------------------------------------------------------------
// ctkErrorLogFDMessageHandler methods

//------------------------------------------------------------------------------
QString ctkErrorLogFDMessageHandler::HandlerName = QLatin1String("FD");

// --------------------------------------------------------------------------
ctkErrorLogFDMessageHandler::ctkErrorLogFDMessageHandler() :
  Superclass(), d_ptr(new ctkErrorLogFDMessageHandlerPrivate())
{
  Q_D(ctkErrorLogFDMessageHandler);
  d->StdOutFDHandler = new ctkFDHandler(this, ctkErrorLogLevel::Info, ctkErrorLogModel::StandardOutput);
  d->StdErrFDHandler = new ctkFDHandler(this, ctkErrorLogLevel::Critical, ctkErrorLogModel::StandardError);
}

// --------------------------------------------------------------------------
ctkErrorLogFDMessageHandler::~ctkErrorLogFDMessageHandler()
{
}

// --------------------------------------------------------------------------
QString ctkErrorLogFDMessageHandler::handlerName()const
{
  return ctkErrorLogFDMessageHandler::HandlerName;
}

// --------------------------------------------------------------------------
void ctkErrorLogFDMessageHandler::setEnabledInternal(bool value)
{
  Q_D(ctkErrorLogFDMessageHandler);
  d->StdOutFDHandler->setEnabled(value);
  //d->StdErrFDHandler->setEnabled(value);
}
