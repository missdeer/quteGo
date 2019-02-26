#ifndef SGFPREVIEW_H
#define SGFPREVIEW_H

#include <QStringList>

#include "ui_sgfpreview.h"

class QFileDialog;

class SGFPreview : public QDialog, public Ui::SGFPreview
{
	Q_OBJECT

	QFileDialog *fileDialog;
	game_state m_empty_board;
	std::shared_ptr<game_record> m_empty_game;
	std::shared_ptr<game_record> m_game;

	void setPath (const QString &path);
	void previewSGF (QIODevice &device, const QString &path);
	void reloadPreview ();
	void clear ();
	void extractZip (const QString &path);
	void previewZipFile(const QString &package, int index, const QString &item);
	void extractRar (const QString &path);
	void previewRarFile(const QString &package, int index, const QString &item);
	void extract7Zip (const QString &path);
	void preview7ZipFile(const QString &package, int index, const QString &item);
	void extractQDB (const QString &path);
	void previewQDBFile(const QString &package, int index, const QString &item);
	void archiveItemSelected (int index);
public:
	SGFPreview (QWidget * parent, const QString &dir);
	~SGFPreview ();
	virtual void accept () override;
	QStringList selected ();

	std::shared_ptr<game_record> selected_record () { return m_game; }
};

#endif
