#include "backend.hpp"

const QString &backend::debugText() const
{
    return _debugText;
}

void backend::setDebugText(const QString &newDebugText)
{
    if (_debugText == newDebugText)
        return;
    _debugText = newDebugText;
    emit debugTextChanged();
}

backend::backend(QObject *parent) : QObject(parent)
{

}
