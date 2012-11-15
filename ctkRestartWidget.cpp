
// Qt includes
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>

#include "ctkAbstractRestartManager.h"
#include "ctkErrorLogModel.h"
#include "ctkRestartWidget.h"

//------------------------------------------------------------------------------
ctkRestartWidget::ctkRestartWidget(QWidget * parent) : QWidget(parent)
{
  this->RestartManager = 0;
  this->setupUi(this);
}

//------------------------------------------------------------------------------
ctkRestartWidget::~ctkRestartWidget()
{
}

//------------------------------------------------------------------------------
void ctkRestartWidget::setRestartManager(ctkAbstractRestartManager * restartManager)
{
  if (this->RestartManager == restartManager)
    {
    return;
    }
  if (this->RestartManager)
    {
    disconnect(this->EnableMsgHandlersButton, SIGNAL(clicked()));
    disconnect(this->RestartButton, SIGNAL(clicked()));
    disconnect(this->EnableFDReadingButton, SIGNAL(clicked()));
    disconnect(this->DisableFDReadingButton, SIGNAL(clicked()));
    }
  this->RestartManager = restartManager;
  if (this->RestartManager)
    {
    connect(this->EnableMsgHandlersButton, SIGNAL(clicked(bool)), 
      this->RestartManager->errorLogModel(), SLOT(setAllMsgHandlerEnabled(bool)));
    connect(this->RestartButton, SIGNAL(clicked()), 
      this->RestartManager, SLOT(restart()));
    connect(this->EnableFDReadingButton, SIGNAL(clicked()), 
      this->RestartManager, SLOT(enableFileDescriptorReading()));
    connect(this->DisableFDReadingButton, SIGNAL(clicked()), 
      this->RestartManager, SLOT(disableFileDescriptorReading()));
    }
}

//------------------------------------------------------------------------------
void ctkRestartWidget::setupUi(QWidget * widget)
{
  QVBoxLayout * layout = new QVBoxLayout();
  widget->setLayout(layout);
  
  this->RestartButton = new QPushButton("Restart");
  layout->addWidget(this->RestartButton);
  
  this->EnableMsgHandlersButton = new QPushButton("Enable Message Handlers");
  this->EnableMsgHandlersButton->setCheckable(true);
  layout->addWidget(this->EnableMsgHandlersButton);
  
  this->EnableFDReadingButton = new QPushButton("Test: Enable file descriptor reading");
  layout->addWidget(this->EnableFDReadingButton);
  
  this->DisableFDReadingButton = new QPushButton("Test: Disable file descriptor reading");
  layout->addWidget(this->DisableFDReadingButton);
  
  QPushButton * quitButton = new QPushButton("Quit");
  QObject::connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
  layout->addWidget(quitButton);
}

#include "moc_ctkRestartWidget.h"

