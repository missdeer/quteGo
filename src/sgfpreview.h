#ifndef SGFPREVIEW_H
#define SGFPREVIEW_H

#include <QStringList>

#include "archivehandler.h"
#include "ui_sgfpreview.h"

class QFileDialog;
class QLayout;

class SGFPreview
    : public QDialog
    , public Ui::SGFPreview
{
    Q_OBJECT

    QFileDialog      *fileDialog;
    game_state        m_empty_board;
    go_game_ptr       m_empty_game;
    go_game_ptr       m_game;
    ArchiveHandlerPtr m_archive;

    void setPath(const QString &path);
    void previewSGF(QIODevice &device, const QString &path);
    void reloadPreview();
    void clear();
    QLayout *takeArhiveItemListWidget();

public:
    SGFPreview(QWidget *parent, const QString &dir);
    ~SGFPreview();
    virtual void accept() override;
    virtual void reject() override;
    QStringList  selected();

    go_game_ptr       selected_record();
    ArchiveHandlerPtr selected_archive();

private slots:
    void onArchiveItemActivated();
    void onArchiveCurrentItemChanged();
};

#endif
