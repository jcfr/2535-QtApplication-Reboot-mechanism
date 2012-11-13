
#ifndef __ctkRestartWidget_h
#define __ctkRestartWidget_h

// Qt includes
#include <QWidget>

class QPushButton;
class ctkAbstractRestartManager;

class ctkRestartWidget : public QWidget
{
public:
  ctkRestartWidget(QWidget * parent = 0);
  virtual ~ctkRestartWidget();
  
  void setRestartManager(ctkAbstractRestartManager * restartManager);

protected:
  void setupUi(QWidget * widget = 0);
  
private:
  Q_DISABLE_COPY(ctkRestartWidget);
  
  QPushButton * EnableMsgHandlersButton;
  QPushButton * RestartButton;
  ctkAbstractRestartManager * RestartManager;
};

#endif

