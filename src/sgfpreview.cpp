#include <fstream>

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

    QVBoxLayout *l = new QVBoxLayout(dialogWidget);
    fileDialog     = new QFileDialog(dialogWidget, Qt::Widget);
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog->setWindowFlags(Qt::Widget);
    fileDialog->setNameFilters({
        tr("All supported files (*.sgf *.zip *.rar *.7z *.qdb)"),
        tr("SGF files (*.sgf *.SGF)"),
        tr("Archieve files (*.zip *.rar *.7z)"),
        tr("quteGo Kifu Libraries (*.qdb)"),
        tr("All files (*)"),
    });
    fileDialog->setDirectory(dir);

    setWindowTitle(tr("Open SGF file"));
    l->addWidget(fileDialog);
    l->setContentsMargins(0, 0, 0, 0);
    fileDialog->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    fileDialog->show();
    connect(encodingList, &QComboBox::currentTextChanged, this, &SGFPreview::reloadPreview);
    connect(overwriteSGFEncoding, &QGroupBox::toggled, this, &SGFPreview::reloadPreview);
    connect(archiveItemList, &QListWidget::currentTextChanged, this, &SGFPreview::archiveItemSelected);
    connect(archiveItemList, &QListWidget::itemActivated, [=]() { accept(); });
    connect(fileDialog, &QFileDialog::currentChanged, this, &SGFPreview::setPath);
    connect(fileDialog, &QFileDialog::accepted, this, &QDialog::accept);
    connect(fileDialog, &QFileDialog::rejected, this, &QDialog::reject);
    boardView->reset_game(m_game);
    boardView->set_show_coords(false);
    archiveItemList->setVisible(false);
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
    archiveItemList->setVisible(false);
    archiveItemList->clear();
}

void SGFPreview::extractQDB(const QString &path) {}

void SGFPreview::previewQDBFile(const QString &package, const QString &item) {}

void SGFPreview::archiveItemSelected(const QString &item)
{
    if (!archiveItemList->isVisible() || item.isEmpty() || fileDialog->selectedFiles().isEmpty())
        return;

    if (m_archive)
    {
        previewArchiveItem(item);
        return;
    }

    QFileInfo fi(fileDialog->selectedFiles()[0]);
    if (fi.suffix().compare("qdb", Qt::CaseInsensitive) == 0)
        previewQDBFile(fi.absoluteFilePath(), item);
}

void SGFPreview::previewArchiveItem(const QString &item)
{
    QIODevice *device = m_archive->getSGFContent(item);
    if (device)
        previewSGF(*device, item);
}

QStringList SGFPreview::selected()
{
    return fileDialog->selectedFiles();
}

void SGFPreview::setPath(const QString &path)
{
    if (!QFileInfo(path).isFile())
        return;

    clear();
    QFileInfo fi(path);
    if (fi.suffix().compare("sgf", Qt::CaseInsensitive) == 0)
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly))
            previewSGF(f, path);
        return;
    }

    auto archiveHandler = ArchiveHandlerFactory::createArchiveHandler(path);
    if (archiveHandler)
    {
        m_archive.reset(archiveHandler);
        auto fileList = m_archive->getSGFFileList();
        archiveItemList->setVisible(!fileList.isEmpty());
        archiveItemList->addItems(fileList);
        return;
    }

    if (fi.suffix().compare("qdb", Qt::CaseInsensitive) == 0)
        extractQDB(path);
}

void SGFPreview::previewSGF(QIODevice &device, const QString &path)
{
    try
    {
        // IOStreamAdapter adapter (&f);
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
        if (archiveItemList->isVisible())
        {
            if (archiveItemList->currentRow() >= 0)
            {
                QString item = archiveItemList->currentItem()->text();
                archiveItemSelected(item);
            }
        }
        else
            setPath(files.at(0));
    }
}

void SGFPreview::accept()
{
    QDialog::accept();
}
