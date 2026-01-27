#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <QString>

// ---------------------- Compress utils ----------------------
namespace CompressUtils 
{

  QString toGZipBase64( const QImage &image ) {
    // 1. QImage in ein QByteArray schreiben (z.B. als PNG)
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // Nutze PNG für verlustfreie Speicherung

    // 2. Daten mit GZip (qCompress) komprimieren
    // Hinweis: qCompress nutzt das zlib-Format. 
    // Der zweite Parameter (9) ist das maximale Kompressionslevel.
    QByteArray compressed = qCompress(ba, 9);

    // 3. In Base64 umwandeln und als String zurückgeben
    return QString(compressed.toBase64());
  }

  QImage fromGZipBase64( const QString &base64String ) {
    // 1. Base64 zurück in Bytes
    QByteArray compressed = QByteArray::fromBase64(base64String.toUtf8());

    // 2. Dekomprimieren
    QByteArray ba = qUncompress(compressed);

    // 3. Bild aus Bytes laden
    QImage image;
    image.loadFromData(ba, "PNG");
    return image;
  }
  
  void saveToFile( const QString &filename, const QString &text ) {
    QFile file(filename);
    // Datei zum Schreiben öffnen (WriteOnly und Text-Modus)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // Explizit UTF-8 setzen (Best Practice)
        out.setEncoding(QStringConverter::Utf8); 
        out << text;
        file.close();
    }
  }

}