#ifndef MainWindow_H
#define MainWindow_H

#include <QtWidgets/QWidget>
#include <QMainWindow>
#include <QEvent>
#include <QTcpServer>
#include <QTcpSocket>

class MpvWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow();
public Q_SLOTS:
    bool eventFilter(QObject *obj, QEvent *event);
public slots:
    void onNewConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError);
    void onFileLoaded();
protected:
    void resizeEvent(QResizeEvent *event) override;
    QSize getVideoSize();
public Q_SLOTS:
    void arResize(QSize);
private:
    bool sendMessage(QString msg);
    QString buttonName(int num);
    QString keyStr(int code, QString text);
    QPoint translateMouseCoords(QPoint mp);
    MpvWidget *m_mpv;
//    QTcpServer  _server;
//    QList<QTcpSocket*>  _sockets;
    QTcpSocket socket;
    QSize m_adjustedSize{0,0};
    qreal m_ar = 2560.0 / 1440.0; //1024. / 768.;
    QSize m_remoteSize{1362, 973};
    QSize m_videoSize{0,0};
};

#endif // MainWindow_H
