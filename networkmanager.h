#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void sendGetRequest(const QUrl &url);

signals:
    void requestFinished(const QByteArray &responseData);  // ���� �����͸� ����
    void errorOccurred(const QString &errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkAccessManager;
};

#endif // NETWORKMANAGER_H
