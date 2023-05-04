#include "backend.hpp"

const QString &Backend::debugText() const
{
    return _debugText;
}

void Backend::setDebugText(const QString &newDebugText)
{
    if (_debugText == newDebugText)
        return;
    _debugText = newDebugText;
    emit debugTextChanged();
}

Backend::Backend(QObject *parent) : QObject(parent)
{

}
