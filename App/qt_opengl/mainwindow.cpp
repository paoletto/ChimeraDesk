#include "mainwindow.h"
#include "mpvwidget.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QWheelEvent> 
#include <QHostAddress>
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QMetaEnum>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    m_mpv = new MpvWidget(this);
    m_mpv->installEventFilter(this);
    m_mpv->setFocusPolicy(Qt::StrongFocus);
    QHBoxLayout *hb = new QHBoxLayout();
    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(m_mpv);
    vl->addLayout(hb);
    setLayout(vl);

    // m_mpv->command(QStringList() << "loadfile" << "tcp://0.0.0.0:12345?listen");
//    _server.listen(QHostAddress::Any, 12346);
//    connect(&_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));


    connect(&socket, &QAbstractSocket::connected,
            this, &MainWindow::onNewConnection);
    connect(&socket, &QIODevice::readyRead, this, &MainWindow::onReadyRead);
    connect(&socket, &QAbstractSocket::stateChanged,
            this, &MainWindow::onSocketStateChanged);
    connect(&socket, &QAbstractSocket::errorOccurred, this, &MainWindow::onErrorOccurred);
//    socket.connectToHost(QHostAddress("127.0.0.1"), 12346);
    onNewConnection();
}

void MainWindow::onNewConnection()
{
   qInfo()<<"onNewConnection()";
//   m_mpv->command(QStringList() << "loadfile" << "tcp://0.0.0.0:12345?listen");
   m_mpv->command(QStringList() << "loadfile" << "tcp://127.0.0.1:12345");
}

void MainWindow::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    qInfo()<<"onSocketStateChanged()"<<socketState;
}

void MainWindow::onReadyRead()
{
    qInfo()<<"onReadyRead()";
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    qDebug() <<  data;
}

void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qDebug() << "onErrorOccurred " << QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(error);
    QCoreApplication::quit();
}

bool MainWindow::sendMessage(QString msg)
{
    msg += " ";
    if (socket.isOpen()) {
        socket.write(msg.toUtf8().leftJustified(64, '.'));
        qInfo()<<msg<<"SENT";
        return true;
    }
    return false;
}

QString MainWindow::buttonName(int num)
{
    switch (num)
    {
      case 1:
        return "Button.left";
      case 2:
        return "Button.right";
      case 3:
        return "Button.middle";
      case 4:
        return "Button.middle";
      default:
        return "Button.left";
    }
}

QString MainWindow::keyStr(int code, QString text)
{
  qInfo()<<QString("keyStr(%1)").arg(code);
  switch (code)
  {
    case 32:
      return QString("Key.space");
    case 16777216:
      return QString("Key.esc");
    case 16777217:
      return QString("Key.tab");
    case 16777219:
      return QString("Key.backspace");
    case 16777220:
      return QString("Key.enter");
    case 16777234:
      return QString("Key.left");
    case 16777235:
      return QString("Key.up");
    case 16777236:
      return QString("Key.right");
    case 16777237:
      return QString("Key.down");
    case 16777248:
      return QString("Key.shift_l");
    case 16777249:
      return QString("Key.ctrl_l");
    case 16777299:
      return QString("Key.cmd_l");
    case 16777251:
      return QString("Key.alt_l");
    case 16781571:
      return QString("Key.alt_r");
    default:
      return QString("%1").arg(code);
  }
}

// translate to remote screen coordinates taking into account window size and borders
QPoint MainWindow::translateMouseCoords(QPoint mp)
{
  int osd_border_top = int(m_mpv->getProperty("osd-dimensions/mt").toDouble()); 
  int osd_border_left = int(m_mpv->getProperty("osd-dimensions/ml").toDouble());
  int w = int(m_mpv->getProperty("osd-width").toDouble()) - osd_border_left * 2;
  int h = int(m_mpv->getProperty("osd-height").toDouble()) - osd_border_top * 2;
  int video_w = int(m_mpv->getProperty("video-params/w").toDouble());
  int video_h = int(m_mpv->getProperty("video-params/h").toDouble());

  qDebug()<<"OSD borders:"<<osd_border_top<<osd_border_left<<"\n";
  qDebug()<<"Content w,h:"<<w<<h<<"\n";
  qDebug()<<"Remote Screen w,h:"<<video_w<<video_h<<"\n";

  int x = mp.x() - osd_border_left;
  int y = mp.y() - osd_border_top;
  
  int nx = (x - 0) * video_w / w + 0;
  int ny = (y - 0) * video_h / h + 0;
  
  mp.setX(nx);
  mp.setY(ny);

  return mp;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
//  qDebug()<<event->type();

  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    QString msg = QString("keyboard press %1 %2").arg(keyEvent->key()).arg(keyEvent->text());
    sendMessage(msg);
  }
  else if (event->type() == QEvent::KeyRelease)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    QString msg = QString("keyboard release %1 %2").arg(keyEvent->key()).arg(keyEvent->text());
    sendMessage(msg);
  }

  else if (event->type() == QEvent::MouseMove)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint mp = translateMouseCoords(mouseEvent->pos());
    QString msg = QString("mouse %1 %2 move").arg(mp.x()).arg(mp.y());
    sendMessage(msg);
  }
  else if (event->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint mp = translateMouseCoords(mouseEvent->pos());
    QString msg = QString("mouse %1 %2 click %3").arg(mp.x()).arg(mp.y()).arg(buttonName(mouseEvent->button()));
    sendMessage(msg);
  }
  else if (event->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint mp = translateMouseCoords(mouseEvent->pos());
    QString msg = QString("mouse %1 %2 release %3").arg(mp.x()).arg(mp.y()).arg(buttonName(mouseEvent->button()));
    qInfo()<<mp;
    sendMessage(msg);
  }

  else if (event->type() == QEvent::Wheel)
  {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
    int dx = (wheelEvent->angleDelta().x() < 0) ? -1 : (wheelEvent->angleDelta().x() > 0) ? 1 : 0;
    int dy = (wheelEvent->angleDelta().y() < 0) ? -1 : (wheelEvent->angleDelta().y() > 0) ? 1 : 0;
    QPoint mp = translateMouseCoords(wheelEvent->pos());
    QString msg = QString("mouse %1 %2 scroll %3 %4")
      .arg(mp.x())
      .arg(mp.y())
      .arg(dx)
      .arg(dy);
    sendMessage(msg);
  }
   
  return false;
}
