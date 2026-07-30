// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_QRCODESCANWIDGET_H
#define FEATHER_QRCODESCANWIDGET_H

#include <QWidget>
#include <QCamera>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaCaptureSession>
#include <QVideoSink>
#else
#include <QVideoProbe>
#endif

#include <bcur/ur-decoder.hpp>

class QrScanThread;

namespace Ui {
    class QrCodeScanWidget;
}

class QrCodeScanWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit QrCodeScanWidget(QWidget *parent);
    ~QrCodeScanWidget() override;

    QString decodedString = "";
    std::string getURData();
    std::string getURType();
    QString getURError();
    
    void startCapture(bool scan_ur = false);
    void reset();
    void stop();
    void pause();

signals:
    void finished(bool success);
    
private slots:
    void onCameraSwitched(int index);
    void onDecoded(const QString &data);

private:
    void refreshCameraList();
    QImage videoFrameToImage(const QVideoFrame &videoFrame);
    void handleFrameCaptured(const QVideoFrame &videoFrame);

    QScopedPointer<Ui::QrCodeScanWidget> ui;

    bool m_scan_ur = false;
    QrScanThread *m_thread;
    QScopedPointer<QCamera> m_camera;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMediaCaptureSession m_captureSession;
    QVideoSink m_sink;
#else
    QVideoProbe *m_probe = nullptr;
#endif

    ur::URDecoder m_decoder;
    bool m_done = false;
    bool m_handleFrames = true;
};

#endif //FEATHER_QRCODESCANWIDGET_H
