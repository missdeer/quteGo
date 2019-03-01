#ifndef SGFPREVIEW_H
#define SGFPREVIEW_H

#include <QStringList>
#include "archivehandler.h"
#include "ui_sgfpreview.h"

class QFileDialog;

class SGFPreview : public QDialog, public Ui::SGFPreview
{
	Q_OBJECT

	QFileDialog *fileDialog;
	game_state m_empty_board;
	std::shared_ptr<game_record> m_empty_game;
	std::shared_ptr<game_record> m_game;
	ArchiveHandlerPtr m_archive;

	void setPath (const QString &path);
	void previewSGF (QIODevice &device, const QString &path);
	void reloadPreview ();
	void clear ();
	void extractQDB (const QString &path);
	void previewQDBFile(const QString &package, const QString &item);
	void archiveItemSelected (const QString &item);
	void previewArchiveItem(const QString &item);
public:
	SGFPreview (QWidget * parent, const QString &dir);
	~SGFPreview ();
	virtual void accept () override;
	QStringList selected ();

	std::shared_ptr<game_record> selected_record () { return m_game; }
	ArchiveHandlerPtr selected_archive() { return m_archive; }
};

#endif
