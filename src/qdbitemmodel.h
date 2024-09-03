#ifndef QDBITEMMODEL_H
#define QDBITEMMODEL_H

#include <QAbstractTableModel>

class QDBItemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit QDBItemModel(QObject *parent = nullptr);
    ~QDBItemModel() override;
    QDBItemModel(const QDBItemModel &)            = delete;
    QDBItemModel(QDBItemModel &&)                 = delete;
    QDBItemModel &operator=(const QDBItemModel &) = delete;
    QDBItemModel &operator=(QDBItemModel &&)      = delete;

    void                        loadQDB(const QString &filename);
    [[nodiscard]] int           rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int           columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex   index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QByteArray                  getSGFContent(int index);
    QString                     getSGFName(int index);
private:
    int     m_rowCount {0};
    QString m_connectionName;
};

#endif // QDBITEMMODEL_H
