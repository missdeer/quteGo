#pragma once
#include "qdbhandler.h"

class SDBHandler : public QDBHandler
{
    Q_OBJECT
public:
    explicit SDBHandler(const QString &archive);
    ~SDBHandler() override = default;
};