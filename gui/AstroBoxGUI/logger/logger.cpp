#include "logger.h"

logger::logger(QObject *parent) : QObject(parent)
{
    _stream.setLocale(QLocale());   // make floats with commas so Excel doesn't convert them to dates

    if (_file)
    {
        _file->open(QFile::WriteOnly | QFile::Append);
        _stream.setDevice(_file);
    }
}

logger::~logger()
{
    _stream.flush();
    if (_file)
    {
        if (_file->isOpen())
            _file->close();
    }
}

void logger::append(const uint32_t controllerTime, const float voltage, const float current)
{
    const QDateTime& now = QDateTime::currentDateTime();

    if (!_file->size())
        _stream << "Date" << _separator << "Time" << _separator << "Controller" << _separator << "Battery Voltage" << _separator << "Battery Current";

    _stream << endl << now.toString(QStringLiteral("yyyy.MM.dd")) << _separator << now.toString(QStringLiteral("hh:mm:ss.z")) << _separator
            << QString::number(controllerTime) << _separator << voltage << _separator << current;
}
