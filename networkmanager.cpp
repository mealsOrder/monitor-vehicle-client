#include "networkmanager.h"
#include <QNetworkRequest>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent),
      networkAccessManager(new QNetworkAccessManager(this))
{
    // ���� ó�� �ñ׳� ����
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &NetworkManager::onReplyFinished);
}

NetworkManager::~NetworkManager()
{
}

void NetworkManager::sendGetRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    networkAccessManager->get(request);  // HTTP GET ��û ����
}

void NetworkManager::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        emit requestFinished(responseData);  // ���� �� ��ȣ ����
    } else {
        emit errorOccurred(reply->errorString());  // ���� �� ��ȣ ����
    }
    reply->deleteLater();  // ���� ��ü ����
}
