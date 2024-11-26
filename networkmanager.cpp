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
    QNetworkReply *reply = networkAccessManager->get(request);

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        QByteArray responseData = reply->readAll();
        emit dataReady(responseData); // 데이터를 바로 전송
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError code) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
    });
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

