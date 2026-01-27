#pragma once

#include <QObject.h>
#include <QImage.h>

#include "ImageProcessor.h"

// -------------------------- ImageWoker --------------------------
class ImageWorker : public QObject {
    Q_OBJECT

private:
    ImageProcessor m_processor; // Der Worker nutzt den Prozessor

public slots:
    void process( const QString &path ) {
        QImage img(path);
        m_processor.applyFilter(img); // Delegiert die Arbeit an den Prozessor
        emit imageReady(img);         // Meldet Ergebnis zurück
    }
    
signals:
    void imageReady( const QImage &img );

};