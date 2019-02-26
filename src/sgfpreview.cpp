#include <QPushButton>
#include <QFileDialog>

#include <fstream>

#include <private/qzipreader_p.h>

#include "thirdparty/Qt7z/Qt7z/qt7zfileinfo.h"
#include "thirdparty/Qt7z/Qt7z/qt7zpackage.h"
#include "thirdparty/QtRAR/src/qtrar.h"
#include "thirdparty/QtRAR/src/qtrarfile.h"
#include "thirdparty/QtRAR/src/qtrarfileinfo.h"

#include "gogame.h"
#include "sgfpreview.h"


SGFPreview::SGFPreview (QWidget *parent, const QString &dir)
	: QDialog (parent), m_empty_board (go_board (19), black),
	  m_empty_game (std::make_shared<game_record> (go_board (19), black, game_info ("White", "Black"))),
	  m_game (m_empty_game)
{
	setupUi (this);

	QVBoxLayout *l = new QVBoxLayout (dialogWidget);
	fileDialog = new QFileDialog (dialogWidget, Qt::Widget);
	fileDialog->setOption (QFileDialog::DontUseNativeDialog, true);
	fileDialog->setWindowFlags (Qt::Widget);
	fileDialog->setNameFilters ({ 
	tr ("All supported files (*.sgf *.zip *.rar *.7z *.qdb)"),
	tr ("SGF files (*.sgf *.SGF)"), 
	tr ("Archieve files (*.zip *.rar *.7z)"),
	tr ("q5Go Kifu Libraries (*.qdb)"),
	tr ("All files (*)"),
	 });
	fileDialog->setDirectory (dir);

	setWindowTitle (tr("Open SGF file"));
	l->addWidget (fileDialog);
	l->setContentsMargins (0, 0, 0, 0);
	fileDialog->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
	fileDialog->show ();
	connect (encodingList, &QComboBox::currentTextChanged, this, &SGFPreview::reloadPreview);
	connect (overwriteSGFEncoding, &QGroupBox::toggled, this, &SGFPreview::reloadPreview);
	connect (archiveItemList, &QListWidget::currentTextChanged, this, &SGFPreview::archiveItemSelected);
	connect (fileDialog, &QFileDialog::currentChanged, this, &SGFPreview::setPath);
	connect (fileDialog, &QFileDialog::accepted, this, &QDialog::accept);
	connect (fileDialog, &QFileDialog::rejected, this, &QDialog::reject);
	boardView->reset_game (m_game);
	boardView->set_show_coords (false);
	archiveItemList->setVisible(false);
}

SGFPreview::~SGFPreview ()
{
}

void SGFPreview::clear ()
{
	boardView->reset_game (m_empty_game);
	m_game = nullptr;

	// ui->displayBoard->clearData ();

	File_WhitePlayer->setText("");
	File_BlackPlayer->setText("");
	File_Date->setText("");
	File_Handicap->setText("");
	File_Result->setText("");
	File_Komi->setText("");
	File_Size->setText("");
	archiveItemList->setVisible(false);
	archiveItemList->clear();
}

void SGFPreview::extractZip(const QString &path)
{
	QZipReader zr(path);

	auto fil = zr.fileInfoList();
	archiveItemList->setVisible(!fil.isEmpty());
	for (auto & fi : fil)
	{
		if (fi.isFile && fi.filePath.endsWith(".sgf", Qt::CaseInsensitive))
			archiveItemList->addItem(fi.filePath);
	}
}

void SGFPreview::previewZipFile(const QString &package, const QString &item)
{
	QZipReader zr(package);
	auto data = zr.fileData(item);
	QBuffer b(&data);
	b.open(QIODevice::ReadOnly);
	previewSGF(b, item);
}

void SGFPreview::extractRar(const QString &path)
{
	QtRAR rar(path);
	bool success = rar.open(QtRAR::OpenModeList);

	if (!success) {
		return ;
	}

	// TODO: Support password
	if (rar.isHeadersEncrypted() || rar.isFilesEncrypted()) {
		return ;
	}

	QStringList fileNameList = rar.fileNameList();
	archiveItemList->setVisible(!fileNameList.isEmpty());
	for (auto & fn : fileNameList)
	{
		if (fn.endsWith(".sgf", Qt::CaseInsensitive))
			archiveItemList->addItem(fn);
	}
}

void SGFPreview::previewRarFile(const QString &package, const QString &item)
{
	QtRAR rar(package);
	bool success = rar.open(QtRAR::OpenModeList);

	if (!success) {
		return ;
	}

	// TODO: Support password
	if (rar.isHeadersEncrypted() || rar.isFilesEncrypted()) {
		return ;
	}

	QtRARFile *file = new QtRARFile(package, item);
	if (file->open(QIODevice::ReadOnly))
	{
		previewSGF(*file, item);
	}
}

void SGFPreview::extract7Zip(const QString &path)
{
	Qt7zPackage pkg(path);
	if (!pkg.open()) {
		return ;
	}

	QStringList fileNameList = pkg.fileNameList();
	archiveItemList->setVisible(!fileNameList.isEmpty());
	for (auto & fn : fileNameList)
	{
		if (fn.endsWith(".sgf", Qt::CaseInsensitive))
			archiveItemList->addItem(fn);
	}
}

void SGFPreview::preview7ZipFile(const QString &package, const QString &item)
{
	Qt7zPackage pkg(package);
	if (!pkg.open()) {
		return ;
	}

	QBuffer buffer;
	buffer.open(QBuffer::ReadWrite);
	if (pkg.extractFile(item, &buffer))
	{
		previewSGF(buffer, item);
	}
}

void SGFPreview::extractQDB(const QString &path)
{

}

void SGFPreview::previewQDBFile(const QString &package, const QString &item)
{

}

void SGFPreview::archiveItemSelected(const QString &item)
{
	if (!archiveItemList->isVisible() || item.isEmpty() || fileDialog->selectedFiles().isEmpty())
		return;
	QFileInfo fi(fileDialog->selectedFiles ()[0]);
	if (fi.suffix().compare("zip", Qt::CaseInsensitive) == 0)
		previewZipFile(fi.absoluteFilePath(), item);
	else if (fi.suffix().compare("rar", Qt::CaseInsensitive) == 0)
		previewRarFile(fi.absoluteFilePath(), item);
	else if (fi.suffix().compare("7z", Qt::CaseInsensitive) == 0)
		preview7ZipFile(fi.absoluteFilePath(), item);
	else if (fi.suffix().compare("qdb", Qt::CaseInsensitive) == 0)
		previewQDBFile(fi.absoluteFilePath(), item);
}

QStringList SGFPreview::selected ()
{
	return fileDialog->selectedFiles ();
}

void SGFPreview::setPath(const QString &path)
{
	if (!QFileInfo(path).isFile())
		return;

	clear ();
	QFileInfo fi(path);
	if (fi.suffix().compare("sgf", Qt::CaseInsensitive) == 0)
	{
		QFile f (path);
		if (f.open (QIODevice::ReadOnly))
			previewSGF(f, path);
	}
	else if (fi.suffix().compare("zip", Qt::CaseInsensitive) == 0)
		extractZip(path);
	else if (fi.suffix().compare("rar", Qt::CaseInsensitive) == 0)
		extractRar(path);
	else if (fi.suffix().compare("7z", Qt::CaseInsensitive) == 0)
		extract7Zip(path);
	else if (fi.suffix().compare("qdb", Qt::CaseInsensitive) == 0)
		extractQDB(path);
}

void SGFPreview::previewSGF(QIODevice &device, const QString& path)
{
	try {
		// IOStreamAdapter adapter (&f);
		QTextCodec *codec = nullptr;
		if (overwriteSGFEncoding->isChecked ()) {
			if (encodingList->currentIndex() == 0) {
				device.seek(0);
				QByteArray data = device.readAll();
				device.seek(0);
				codec = charset_detect(data);
			}
			else
				codec = QTextCodec::codecForName (encodingList->currentText ().toLatin1 ());
		}
		sgf *sgf = load_sgf (device);
		m_game = sgf2record (*sgf, codec);
		m_game->set_filename (path.toStdString ());

		boardView->reset_game (m_game);
		game_state *st = m_game->get_root ();
		for (int i = 0; i < 20 && st->n_children () > 0; i++)
			st = st->next_primary_move ();
		boardView->set_displayed (st);

		File_WhitePlayer->setText (QString::fromStdString (m_game->name_white ()));
		File_BlackPlayer->setText (QString::fromStdString (m_game->name_black ()));
		File_Date->setText (QString::fromStdString (m_game->date ()));
		File_Handicap->setText (QString::fromStdString(m_game->handicap ()));
		File_Result->setText (QString::fromStdString (m_game->result ()));
		File_Komi->setText (QString::fromStdString(m_game->komi ()));
		File_Size->setText (QString::number (st->get_board ().size_x ()));
	} catch (...) {
	}
}


void SGFPreview::reloadPreview ()
{
	auto files = fileDialog->selectedFiles ();
	if (!files.isEmpty ())
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
			setPath (files.at (0));
	}
}

void SGFPreview::accept ()
{
	QDialog::accept ();
}
