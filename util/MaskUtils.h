inline QVector<QColor> defaultMaskColors()
{
    return {
        Qt::transparent,        // 0
        QColor(255, 0, 0),
        QColor(0, 255, 0),
        QColor(0, 0, 255),
        QColor(255, 255, 0),
        QColor(255, 0, 255),
        QColor(0, 255, 255),
        QColor(255, 128, 0),
        QColor(128, 0, 255),
        QColor(0, 128, 255)
    };
}
