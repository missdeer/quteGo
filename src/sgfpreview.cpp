#include <QFileDialog>
#include <QPushButton>

#include "sgfpreview.h"
#include "archivehandlerfactory.h"
#include "gogame.h"

SGFPreview::SGFPreview(QWidget *parent, const QString &dir)
    : QDialog(parent),
      m_empty_board(go_board(19), black),
      m_empty_game(std::make_shared<game_record>(go_board(19), black, game_info("White", "Black"))),
      m_game(m_empty_game)
{
    setupUi(this);
    QVBoxLayout* pArchiveItemListContainerLayout = new QVBoxLayout;
    archiveItemListContainer->setLayout(pArchiveItemListContainerLayout);
    pArchiveItemListContainerLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *pFileDialogContainerLayout = new QVBoxLayout(dialogWidget);
    fileDialog     = new QFileDialog(dialogWidget, Qt::Widget);
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog->setWindowFlags(Qt::Widget);
    QStringList extensions {"*.sgf"};
    for (const auto &ext : ArchiveHandlerFactory::extensions())
    {
        extensions.append("*." + ext);
    }
    QString     allFilter = tr("All supported files (%1)").arg(extensions.join(' '));
    QStringList nameFilters {
        allFilter,
        tr("SGF files (*.sgf *.SGF)"),
    };
    nameFilters.append(ArchiveHandlerFactory::nameFilters());
    nameFilters.append(tr("All files (*)"));

    fileDialog->setNameFilters(nameFilters);
    fileDialog->setDirectory(dir);

    setWindowTitle(tr("Open file"));
    pFileDialogContainerLayout->addWidget(fileDialog);
    pFileDialogContainerLayout->setContentsMargins(0, 0, 0, 0);
    fileDialog->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    fileDialog->show();
    connect(encodingList, &QComboBox::currentTextChanged, this, &SGFPreview::reloadPreview);
    connect(overwriteSGFEncoding, &QGroupBox::toggled, this, &SGFPreview::reloadPreview);

    connect(fileDialog, &QFileDialog::currentChanged, this, &SGFPreview::setPath);
    connect(fileDialog, &QFileDialog::accepted, this, &QDialog::accept);
    connect(fileDialog, &QFileDialog::rejected, this, &QDialog::reject);
    boardView->reset_game(m_game);
    boardView->set_show_coords(false);
    archiveItemListContainer->setVisible(false);
}

SGFPreview::~SGFPreview() {}

void SGFPreview::clear()
{
    boardView->reset_game(m_empty_game);
    m_game = nullptr;
    m_archive.reset();
    // ui->displayBoard->clearData ();

    File_WhitePlayer->setText("");
    File_BlackPlayer->setText("");
    File_Date->setText("");
    File_Handicap->setText("");
    File_Result->setText("");
    File_Komi->setText("");
    File_Size->setText("");
    File_Event->setText("");
    File_Round->setText("");
    archiveItemListContainer->setVisible(false);
}

QStringList SGFPreview::selected()
{
    return fileDialog->selectedFiles();
}

go_game_ptr SGFPreview::selected_record()
{
    return m_game;
}

ArchiveHandlerPtr SGFPreview::selected_archive()
{
    return m_archive;
}

QLayout *SGFPreview::takeArhiveItemListWidget()
{
    auto *pArchiveItemListContainerLayout = archiveItemListContainer->layout();
    Q_ASSERT(pArchiveItemListContainerLayout != nullptr);
    while (pArchiveItemListContainerLayout->takeAt(0) != nullptr)
        ;

    return pArchiveItemListContainerLayout;
}

void SGFPreview::setPath(const QString &path)
{
    if (!QFileInfo(path).isFile())
        return;

    clear();
    if (path.endsWith(QStringLiteral(".sgf"), Qt::CaseInsensitive))
    {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly))
        {
            previewSGF(file, path);
            return;
        }
    }

    auto archiveHandler = ArchiveHandlerFactory::createArchiveHandler(path);
    m_archive.reset(archiveHandler);
    auto *pArchiveItemListContainerLayout = takeArhiveItemListWidget();
    Q_ASSERT(m_archive);
    auto *pArchiveItemListWidget = m_archive->getArchiveItemListWidget();
    pArchiveItemListContainerLayout->addWidget(pArchiveItemListWidget);
    if (m_archive->hasSGF())
    {
        pArchiveItemListWidget->setParent(archiveItemListContainer);
        connect(m_archive.get(), &ArchiveHandler::itemActivated, this, &SGFPreview::onArchiveItemActivated);
        connect(m_archive.get(), &ArchiveHandler::currentItemChanged, this, &SGFPreview::onArchiveCurrentItemChanged);
        archiveItemListContainer->setVisible(true);
        previewSGF(*m_archive->getCurrentSGFContent(), m_archive->getCurrentSGFName());
    }
}

void SGFPreview::onArchiveItemActivated()
{
    Q_ASSERT(m_archive && m_archive->hasSGF());
    previewSGF(*m_archive->getCurrentSGFContent(), m_archive->getCurrentSGFName());
}

void SGFPreview::onArchiveCurrentItemChanged()
{
    Q_ASSERT(m_archive && m_archive->hasSGF());
    previewSGF(*m_archive->getCurrentSGFContent(), m_archive->getCurrentSGFName());
}

void SGFPreview::previewSGF(QIODevice &device, const QString &path)
{
    try
    {
        QTextCodec *codec = nullptr;
        if (overwriteSGFEncoding->isChecked())
        {
            if (encodingList->currentIndex() == 0)
            {
                QByteArray data = device.readAll();
                device.seek(0);
                codec = charset_detect(data);
            }
            else
                codec = QTextCodec::codecForName(encodingList->currentText().toLatin1());
        }
        sgf *sgf = load_sgf(device);
        m_game   = sgf2record(*sgf, codec);
        m_game->set_filename(path.toStdString());

        boardView->reset_game(m_game);
        game_state *st = m_game->get_root();
        for (int i = 0; i < 20 && st->n_children() > 0; i++)
            st = st->next_primary_move();
        boardView->set_displayed(st);

        File_WhitePlayer->setText(QString::fromStdString(m_game->name_white()));
        File_BlackPlayer->setText(QString::fromStdString(m_game->name_black()));
        File_Date->setText(QString::fromStdString(m_game->date()));
        File_Handicap->setText(QString::fromStdString(m_game->handicap()));
        File_Result->setText(QString::fromStdString(m_game->result()));
        File_Komi->setText(QString::fromStdString(m_game->komi()));
        File_Size->setText(QString::number(st->get_board().size_x()));
        File_Event->setText(QString::fromStdString(m_game->event()));
        File_Round->setText(QString::fromStdString(m_game->round()));
    }
    catch (...)
    {
    }
}

void SGFPreview::reloadPreview()
{
    auto files = fileDialog->selectedFiles();
    if (!files.isEmpty())
    {
        if (archiveItemListContainer->isVisible())
        {
            if (m_archive && m_archive->hasSGF())
                previewSGF(*m_archive->getCurrentSGFContent(), m_archive->getCurrentSGFName());
        }
        else
            setPath(files.at(0));
    }
}

void SGFPreview::accept()
{
    takeArhiveItemListWidget();
    QDialog::accept();
}

void SGFPreview::reject()
{
    takeArhiveItemListWidget();
    QDialog::reject();
}
