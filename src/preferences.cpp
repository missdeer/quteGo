/*
 * preferences.cpp
 */

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QWhatsThis>

#include "preferences.h"
#include "clientwin.h"
#include "imagehandler.h"
#include "mainwindow.h"
#include "qgo.h"

#if defined(Q_OS_MACX)
#    include <CoreFoundation/CFBundle.h>
#    include <CoreFoundation/CFString.h>
#endif // Q_OS_MACX

template<class T> QVariant pref_vec_model<T>::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (row < 0 || col != 0)
        return QVariant();

    size_t r = row;
    if (r >= m_entries.size())
    {
        r -= m_entries.size();
        if (r >= m_extra.size())
            return QVariant();
        if (role == Qt::BackgroundRole)
            return QBrush(Qt::lightGray);
        if (role != Qt::DisplayRole)
            return QVariant();
        return m_extra[r].title;
    }
    if (role != Qt::DisplayRole)
        return QVariant();
    const T &e = m_entries[r];
    return e.title;
}

template<class T> const T *pref_vec_model<T>::find(const QModelIndex &index) const
{
    int row = index.row();
    if (row < 0)
        return nullptr;

    size_t r = row;
    if (r >= m_entries.size())
    {
        r -= m_entries.size();
        if (r >= m_extra.size())
            return nullptr;
        return &m_extra[r];
    }
    return &m_entries[r];
}

template<class T> bool pref_vec_model<T>::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count < 1 || (size_t)(row + count) > m_entries.size())
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    m_entries.erase(m_entries.begin() + row, m_entries.begin() + (row + count));
    endRemoveRows();
    return true;
}

template<class T> void pref_vec_model<T>::add_or_replace(T x)
{
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        T &e = m_entries[i];
        if (e.title == x.title)
        {
            e = x;
            emit dataChanged(index(i, 0), index(i, 0));
            return;
        }
    }
    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.push_back(x);
    endInsertRows();
}

template<class T> QModelIndex pref_vec_model<T>::index(int row, int col, const QModelIndex &) const
{
    return createIndex(row, col);
}

template<class T> QModelIndex pref_vec_model<T>::parent(const QModelIndex &) const
{
    return QModelIndex();
}

template<class T> int pref_vec_model<T>::rowCount(const QModelIndex &) const
{
    return m_entries.size() + m_extra.size();
}

template<class T> int pref_vec_model<T>::columnCount(const QModelIndex &) const
{
    return 1;
}

#if 0
template<class T>
bool pref_vec_model<T>::remove (const QString &title)
{
	for (size_t i = 0; i < m_entries.size (); i++)
		if (m_entries[i].title == title) {
			beginRemoveRows (QModelIndex (), i, i);
			m_entries.erase (m_entries.begin () + i);
			endRemoveRows ();
			break;
		}
}
#endif

template<class T> QVariant pref_vec_model<T>::headerData(int section, Qt::Orientation ot, int role) const
{
    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignLeft;
    }

    if (role != Qt::DisplayRole || ot != Qt::Horizontal)
        return QVariant();
    switch (section)
    {
    case 0:
        return tr("Name");
    }
    return QVariant();
}

std::vector<Host> standard_servers {{"-- IGS --", "igs.joyjoy.net", 7777, "guest", "", "SJIS"},
                                    {"-- LGS --", "lgs.taiwango.net", 9696, "guest", "", "Big5"},
                                    {"-- WING --", "wing.gr.jp", 1515, "guest", "", ""}};

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent), m_hosts_model(g_setting->m_hosts, standard_servers), m_engines_model(g_setting->m_engines)
{
    setupUi(this);
    setModal(true);

    connect(buttonApply, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_apply));
    connect(buttonCancel, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_reject));
    connect(buttonHelp, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::startHelpMode));
    connect(buttonOk, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_accept));
    connect(enginePathButton, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_getComputerPath));
    connect(pb_engine_delete, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_delete_engine));
    connect(pb_engine_add, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_add_engine));
    connect(engineName, &QLineEdit::textChanged, this, &PreferencesDialog::slot_engineChanged);
    connect(pb_engine_new, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_new_engine));
    connect(pb_server_delete, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_delete_server));
    connect(pb_server_add, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_add_server));
    connect(LineEdit_title, &QLineEdit::textChanged, this, &PreferencesDialog::slot_serverChanged);
    connect(pb_server_new, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_new_server));
    connect(blackStonePicturePathButton, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_getBlackStonePicturePath));
    connect(whiteStonePicturePathButton, &QPushButton::clicked, this, QOverload<>::of(&PreferencesDialog::slot_getWhiteStonePicturePath));
    connect(stoneSizePercentSlider, &QSlider::valueChanged, this, &PreferencesDialog::slot_stoneSizePercentChanged);

    int engine_w = enginelabel_1->width();
    engine_w     = std::max(engine_w, enginelabel_2->width());
    engine_w     = std::max(engine_w, enginelabel_3->width());
    engine_w     = std::max(engine_w, enginelabel_4->width());
    engine_w     = std::max(engine_w, enginelabel_5->width());
    enginelabel_1->setMinimumWidth(engine_w);
    enginelabel_2->setMinimumWidth(engine_w);
    enginelabel_3->setMinimumWidth(engine_w);
    enginelabel_4->setMinimumWidth(engine_w);
    enginelabel_5->setMinimumWidth(engine_w);

    // init random-number generator
    srand((unsigned)time(nullptr));

    LineEdit_port->setValidator(new QIntValidator(0, 9999, this));
    anMaxMovesEdit->setValidator(new QIntValidator(0, 999, this));
    anDepthEdit->setValidator(new QIntValidator(0, 999, this));
    slideXEdit->setValidator(new QIntValidator(100, 9999, this));
    slideYEdit->setValidator(new QIntValidator(100, 9999, this));

    // clear edit field
    LineEdit_title->clear();

    init_from_settings();

    // Default codec
    ComboBox_codec->addItem("");
    auto codecs = QTextCodec::availableCodecs();
    for (auto it : codecs)
        ComboBox_codec->addItem(it);

    m_ih = new ImageHandler;

    int w        = stoneView->width();
    int h        = stoneView->height();
    m_stone_size = std::min(w / 2, h);
    m_ih->init(5);
    m_stone_canvas = new QGraphicsScene(0, 0, w, h, stoneView);
    stoneView->setScene(m_stone_canvas);
    m_b_stone = new QGraphicsPixmapItem;
    m_w_stone = new QGraphicsPixmapItem;
    m_stone_canvas->addItem(m_w_stone);
    m_stone_canvas->addItem(m_b_stone);
    m_w_stone->setZValue(1);

    connect(radioButtonStones_2D, &QRadioButton::toggled, this, &PreferencesDialog::select_stone_look);
    connect(radioButtonStones_3D, &QRadioButton::toggled, this, &PreferencesDialog::select_stone_look);
    connect(radioButtonStone_picture, &QRadioButton::toggled, this, &PreferencesDialog::select_stone_look);
    connect(blackStonePicturePathEdit, &QLineEdit::textChanged, [=]() { update_b_stones(); });
    connect(whiteStonePicturePathEdit, &QLineEdit::textChanged, [=]() { update_w_stones(); });

    connect(whiteColorButton, &QToolButton::clicked, this, &PreferencesDialog::select_white_color);
    connect(blackColorButton, &QToolButton::clicked, this, &PreferencesDialog::select_black_color);
    connect(blackRoundSlider, &QSlider::valueChanged, [=](int) { update_b_stones(); });
    connect(blackHardSlider, &QSlider::valueChanged, [=](int) { update_b_stones(); });
    connect(blackSpecSlider, &QSlider::valueChanged, [=](int) { update_b_stones(); });
    connect(blackFlatSlider, &QSlider::valueChanged, [=](int) { update_b_stones(); });
    connect(whiteRoundSlider, &QSlider::valueChanged, [=](int) { update_w_stones(); });
    connect(whiteHardSlider, &QSlider::valueChanged, [=](int) { update_w_stones(); });
    connect(whiteSpecSlider, &QSlider::valueChanged, [=](int) { update_w_stones(); });
    connect(whiteFlatSlider, &QSlider::valueChanged, [=](int) { update_w_stones(); });
    connect(ambientSlider, &QSlider::valueChanged, [=](int) {
        update_w_stones();
        update_b_stones();
    });
    connect(stripesCheckBox, &QCheckBox::toggled, [=](int) { update_w_stones(); });

    void (QComboBox::*cic)(int) = &QComboBox::currentIndexChanged;
    connect(woodComboBox, cic, [=](int i) {
        GobanPicturePathButton->setEnabled(i == 0);
        LineEdit_goban->setEnabled(i == 0);
        update_board_image();
    });
    connect(LineEdit_goban, &QLineEdit::editingFinished, [=]() { update_board_image(); });

    update_board_image();
    update_w_stones();
    update_b_stones();
    connect(stoneView, &SizeGraphicsView::resized, [=]() {
        update_board_image();
        update_w_stones();
        update_b_stones();
    });

    connect(GobanPicturePathButton, &QToolButton::clicked, this, &PreferencesDialog::slot_getGobanPicturePath);
    connect(TablePicturePathButton, &QToolButton::clicked, this, &PreferencesDialog::slot_getTablePicturePath);

    update_dbpaths(g_setting->m_dbpaths);
    dbPathsListView->setModel(&m_dbpath_model);
    connect(dbPathsListView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection &, const QItemSelection &) {
        update_db_selection();
    });
    connect(dbDirsButton, &QPushButton::clicked, this, &PreferencesDialog::slot_dbdir);
    connect(dbCfgButton, &QPushButton::clicked, this, &PreferencesDialog::slot_dbcfg);
    connect(dbRemButton, &QPushButton::clicked, this, &PreferencesDialog::slot_dbrem);

    ListView_engines->setModel(&m_engines_model);
    connect(ListView_engines, &ClickableListView::current_changed, [this]() { update_current_engine(); });
    ListView_hosts->setModel(&m_hosts_model);
    connect(ListView_hosts, &ClickableListView::current_changed, [this]() { update_current_host(); });

    connect(fontStandardButton, &QPushButton::clicked, [this]() { selectFont(fontStandardButton, g_setting->fontStandard); });
    connect(fontMarksButton, &QPushButton::clicked, [this]() { selectFont(fontMarksButton, g_setting->fontMarks); });
    connect(fontCommentsButton, &QPushButton::clicked, [this]() { selectFont(fontCommentsButton, g_setting->fontComments); });
    connect(fontListsButton, &QPushButton::clicked, [this]() { selectFont(fontListsButton, g_setting->fontLists); });
    connect(fontClocksButton, &QPushButton::clicked, [this]() { selectFont(fontClocksButton, g_setting->fontClocks); });
    connect(fontConsoleButton, &QPushButton::clicked, [this]() { selectFont(fontConsoleButton, g_setting->fontConsole); });
}

void PreferencesDialog::update_dbpaths(const QStringList &l)
{
    m_dbpaths = l;
    m_dbpath_model.clear();
    for (auto &it : m_dbpaths)
    {
        QStandardItem *item = new QStandardItem(it);
        item->setEditable(false);
        item->setDropEnabled(false);
        m_dbpath_model.appendRow(item);
    }
}

void PreferencesDialog::update_db_selection()
{
    QItemSelectionModel   *sel       = dbPathsListView->selectionModel();
    const QModelIndexList &selected  = sel->selectedRows();
    bool                   selection = selected.length() != 0;

    dbRemButton->setEnabled(selection);
}

void PreferencesDialog::slot_dbdir(bool)
{
    QString dir;
    if (!m_last_added_dbdir.isEmpty())
    {
        QDir d(m_last_added_dbdir);
        d.cdUp();
        dir = d.absolutePath();
    }
    QString dirname = QFileDialog::getExistingDirectory(this, QObject::tr("Add a database directory"), dir);
    if (dirname.isEmpty())
        return;
    QDir  d(dirname);
    QFile path(d.filePath("kombilo.db"));
    if (!path.exists())
    {
        QMessageBox::warning(this,
                             tr("Directory contains no database"),
                             tr("The directory could not be added because no "
                                "kombilo.db file could be found."));
        return;
    }
    for (auto &it : m_dbpaths)
    {
        QDir d2(it);
        if (d == d2)
        {
            QMessageBox::warning(this,
                                 tr("Directory already in the list"),
                                 tr("The directory could not be added because it "
                                    "already exists in the list."));
            return;
        }
    }
    m_last_added_dbdir = dirname;
    m_dbpaths.append(dirname);
    m_dbpath_model.appendRow(new QStandardItem(dirname));
    m_dbpaths_changed = true;
}

void PreferencesDialog::slot_dbcfg(bool)
{
    if (m_dbpath_model.rowCount() != 0)
    {
        QMessageBox mb(QMessageBox::Question,
                       tr("Overwrite database paths"),
                       tr("This operation replaces existing database paths.\nDo "
                          "you still want to import from kombilo.cfg?"),
                       QMessageBox::Yes | QMessageBox::No);
        if (mb.exec() != QMessageBox::Yes)
            return;
    }
    QString filename = QFileDialog::getOpenFileName(
        this, QObject::tr("Open kombilo.cfg"), g_setting->readEntry("LAST_DIR"), QObject::tr("CFG Files (*.cfg);;All Files (*)"));
    if (filename.isEmpty())
        return;

    QSettings cfg(filename, QSettings::IniFormat);

    QStringList l;
    int         i = 0;
    for (;;)
    {
        QVariant v = cfg.value("databases/d" + QString::number(i));
        if (v.isNull())
            break;
        QStringList vl = v.toStringList();
        qDebug() << vl.length() << vl;
        if (vl.length() == 3 && vl[2] == "kombilo")
            l << vl[1];
        i++;
    }
    update_dbpaths(l);
    m_dbpaths_changed = true;
}

void PreferencesDialog::slot_dbrem(bool)
{
    QItemSelectionModel   *sel       = dbPathsListView->selectionModel();
    const QModelIndexList &selected  = sel->selectedRows();
    bool                   selection = selected.length() != 0;

    if (!selection)
        return;

    QModelIndex i = selected.first();
    int         r = i.row();
    m_dbpath_model.removeRows(r, 1);
    m_dbpaths.removeAt(r);
    m_dbpaths_changed = true;
}

void PreferencesDialog::update_board_image()
{
    int     w = stoneView->width();
    int     h = stoneView->height();
    QImage  image(w, h, QImage::Format_RGB32);
    QPixmap pm;
    QString filename = LineEdit_goban->text();
    int     idx      = woodComboBox->currentIndex();
    if (idx > 0)
        filename = QString(":/BoardWindow/images/board/wood%1.png").arg(idx);
    QPixmap p(filename);
    if (p.isNull())
        p = QPixmap(":/BoardWindow/images/board/wood1.png");

    // Paint table and board on the pixmap
    QPainter painter;

    painter.begin(&image);
    painter.setPen(Qt::NoPen);

    /* @@@ Scaling to the narrow strip of wood in the preferences dialog won't
       look the same as on the real board.  It's unclear what best to do here.  */
    if (0 && scaleWoodCheckBox->isChecked())
        painter.drawPixmap(0, 0, w, h, p);
    else
        painter.drawTiledPixmap(0, 0, w, h, p);
    painter.end();
    m_stone_canvas->setSceneRect(0, 0, w, h);
    m_stone_canvas->setBackgroundBrush(QBrush(image));

    m_stone_size   = std::min(w / 2, h);
    int real_width = m_stone_size * 9. / 8;
    int w_diff     = real_width - m_stone_size;
    m_w_stone->setPos(w / 2 - real_width, 0);
    m_b_stone->setPos(w / 2 - w_diff, 0);
}

void PreferencesDialog::init_from_settings()
{
    int idx = g_setting->readIntEntry("SKIN_INDEX");
    GobanPicturePathButton->setEnabled(idx == 0);
    LineEdit_goban->setEnabled(idx == 0);
    woodComboBox->setCurrentIndex(idx);

    LineEdit_goban->setText(g_setting->readEntry("SKIN"));
    LineEdit_Table->setText(g_setting->readEntry("SKIN_TABLE"));
    scaleWoodCheckBox->setChecked(g_setting->readBoolEntry("SKIN_SCALE_WOOD"));
    languageComboBox->insertItems(1, g_setting->getAvailableLanguages());
    languageComboBox->setCurrentIndex(g_setting->convertLanguageCodeToNumber());

    fileSelComboBox->setCurrentIndex(g_setting->readIntEntry("FILESEL"));

    radioButtonStones_2D->setChecked((g_setting->readIntEntry("STONES_LOOK") == 1));
    radioButtonStones_3D->setChecked((g_setting->readIntEntry("STONES_LOOK") == 2));
    radioButtonStone_real->setChecked((g_setting->readIntEntry("STONES_LOOK") == 3));
    radioButtonStone_picture->setChecked((g_setting->readIntEntry("STONES_LOOK") == 4));
    stoneSizePercentSlider->setValue(g_setting->readIntEntry("STONES_SIZE_PERCENT"));
    whiteStonePicturePathEdit->setText(g_setting->readEntry("STONES_WPICTURE"));
    blackStonePicturePathEdit->setText(g_setting->readEntry("STONES_BPICTURE"));
    stripesCheckBox->setChecked(g_setting->readBoolEntry("STONES_STRIPES"));
    whiteSpecSlider->setValue(g_setting->readIntEntry("STONES_WSPEC"));
    blackSpecSlider->setValue(g_setting->readIntEntry("STONES_BSPEC"));
    whiteHardSlider->setValue(g_setting->readIntEntry("STONES_WHARD"));
    blackHardSlider->setValue(g_setting->readIntEntry("STONES_BHARD"));
    whiteRoundSlider->setValue(g_setting->readIntEntry("STONES_WROUND"));
    blackRoundSlider->setValue(g_setting->readIntEntry("STONES_BROUND"));
    whiteFlatSlider->setValue(g_setting->readIntEntry("STONES_WFLAT"));
    blackFlatSlider->setValue(g_setting->readIntEntry("STONES_BFLAT"));
    ambientSlider->setValue(g_setting->readIntEntry("STONES_AMBIENT"));

    lineScaleCheckBox->setChecked(g_setting->readBoolEntry("BOARD_LINESCALE"));
    lineWidenCheckBox->setChecked(g_setting->readBoolEntry("BOARD_LINEWIDEN"));

    stoneSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_STONE"));
    autoplaySoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_AUTOPLAY"));
    talkSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_TALK"));
    matchSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_MATCH"));
    passSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_PASS"));
    gameEndSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_GAMEEND"));
    timeSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_TIME"));
    saySoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_SAY"));
    enterSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_ENTER"));
    leaveSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_LEAVE"));
    disConnectSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_DISCONNECT"));
    connectSoundCheckBox->setChecked(g_setting->readBoolEntry("SOUND_CONNECT"));

    soundMatchCheckBox->setChecked(g_setting->readBoolEntry("SOUND_MATCH_BOARD"));
    soundObserveCheckBox->setChecked(g_setting->readBoolEntry("SOUND_OBSERVE"));
    soundNormalCheckBox->setChecked(g_setting->readBoolEntry("SOUND_NORMAL"));
    soundComputerCheckBox->setChecked(g_setting->readBoolEntry("SOUND_COMPUTER"));

    variationComboBox->setCurrentIndex(g_setting->readIntEntry("VAR_GHOSTS"));
    varChildrenComboBox->setCurrentIndex(g_setting->readBoolEntry("VAR_CHILDREN") != 0);
    varSGFStyleComboBox->setCurrentIndex(g_setting->readIntEntry("VAR_SGF_STYLE"));
    varDiagsCheckBox->setChecked(g_setting->readBoolEntry("VAR_IGNORE_DIAGS"));

    int coords = (g_setting->readBoolEntry("BOARD_COORDS") ? (g_setting->readBoolEntry("SGF_BOARD_COORDS") ? 2 : 1) : 0);
    coordsComboBox->setCurrentIndex(coords);
    coordSizeSlider->setValue(g_setting->readIntEntry("COORDS_SIZE"));
    cursorCheckBox->setChecked(g_setting->readBoolEntry("CURSOR"));
    tooltipsCheckBox->setChecked(!(g_setting->readBoolEntry("TOOLTIPS")));
    timerComboBox->setCurrentIndex(g_setting->readIntEntry("TIMER_INTERVAL"));
    BYTimeSpin->setValue(g_setting->readIntEntry("BY_TIMER"));
    sgfTimeTagsCheckBox->setChecked(g_setting->readBoolEntry("SGF_TIME_TAGS"));
    int sidebar = g_setting->readBoolEntry("SIDEBAR_LEFT") ? 0 : 1;
    sidebarComboBox->setCurrentIndex(sidebar);
    antiClickoCheckBox->setChecked(g_setting->readBoolEntry("ANTICLICKO"));
    hitboxCheckBox->setChecked(g_setting->readBoolEntry("ANTICLICKO_HITBOX"));

    gameTreeSizeSlider->setValue(g_setting->readIntEntry("GAMETREE_SIZE"));
    diagShowComboBox->setCurrentIndex(g_setting->readIntEntry("BOARD_DIAGMODE"));
    diagClearCheckBox->setChecked(g_setting->readBoolEntry("BOARD_DIAGCLEAR"));
    diagHideCheckBox->setChecked(g_setting->readBoolEntry("GAMETREE_DIAGHIDE"));

    // Client Window tab
    LineEdit_watch->setText(g_setting->readEntry("WATCH"));
    LineEdit_exclude->setText(g_setting->readEntry("EXCLUDE"));
    CheckBox_extUserInfo->setChecked(g_setting->readBoolEntry("EXTUSERINFO"));
    //	CheckBox_useNmatch->setChecked(setting->readBoolEntry("USE_NMATCH"));
    checkBox_Nmatch_Black->setChecked(g_setting->readBoolEntry("NMATCH_BLACK"));
    checkBox_Nmatch_White->setChecked(g_setting->readBoolEntry("NMATCH_WHITE"));
    checkBox_Nmatch_Nigiri->setChecked(g_setting->readBoolEntry("NMATCH_NIGIRI"));
    HandicapSpin_Nmatch->setValue(g_setting->readIntEntry("NMATCH_HANDICAP"));
    timeSpin_Nmatch->setValue(g_setting->readIntEntry("NMATCH_MAIN_TIME"));
    BYSpin_Nmatch->setValue(g_setting->readIntEntry("NMATCH_BYO_TIME"));

    computerWhiteButton->setChecked(g_setting->readBoolEntry("COMPUTER_WHITE"));
    computerSizeSpin->setValue(g_setting->readIntEntry("COMPUTER_SIZE"));
    computerHandicapSpin->setValue(g_setting->readIntEntry("COMPUTER_HANDICAP"));
    humanName->setText(g_setting->readEntry("HUMAN_NAME"));

    anChildMovesCheckBox->setChecked(g_setting->readBoolEntry("ANALYSIS_CHILDREN"));
    anPruneCheckBox->setChecked(g_setting->readBoolEntry("ANALYSIS_PRUNE"));
    anHideCheckBox->setChecked(g_setting->readBoolEntry("ANALYSIS_HIDEOTHER"));
    anVarComboBox->setCurrentIndex(g_setting->readIntEntry("ANALYSIS_VARTYPE"));
    winrateComboBox->setCurrentIndex(g_setting->readIntEntry("ANALYSIS_WINRATE"));
    anDepthEdit->setText(QString::number(g_setting->readIntEntry("ANALYSIS_DEPTH")));
    anMaxMovesEdit->setText(QString::number(g_setting->readIntEntry("ANALYSIS_MAXMOVES")));

    // Go Server tab
    boardSizeSpin->setValue(g_setting->readIntEntry("DEFAULT_SIZE"));
    timeSpin->setValue(g_setting->readIntEntry("DEFAULT_TIME"));
    BYSpin->setValue(g_setting->readIntEntry("DEFAULT_BY"));
    komiSpinDefault->setValue(g_setting->readIntEntry("DEFAULT_KOMI"));
    automaticNegotiationCheckBox->setChecked(g_setting->readBoolEntry("DEFAULT_AUTONEGO"));
    CheckBox_autoSave->setChecked(g_setting->readBoolEntry("AUTOSAVE"));
    CheckBox_autoSave_Played->setChecked(g_setting->readBoolEntry("AUTOSAVE_PLAYED"));

    toroidDupsSpin->setValue(g_setting->readIntEntry("TOROID_DUPS"));

    slideLinesSpinBox->setValue(g_setting->readIntEntry("SLIDE_LINES"));
    slideMarginSpinBox->setValue(g_setting->readIntEntry("SLIDE_MARGIN"));
    slideXEdit->setText(QString::number(g_setting->readIntEntry("SLIDE_X")));
    slideYEdit->setText(QString::number(g_setting->readIntEntry("SLIDE_Y")));
    slideItalicCheckBox->setChecked(g_setting->readBoolEntry("SLIDE_ITALIC"));
    slideBoldCheckBox->setChecked(g_setting->readBoolEntry("SLIDE_BOLD"));
    slideWBCheckBox->setChecked(g_setting->readBoolEntry("SLIDE_WB"));
    slideCoordsCheckBox->setChecked(g_setting->readBoolEntry("SLIDE_COORDS"));

    fontStandardButton->setText(g_setting->fontToString(g_setting->fontStandard));
    fontMarksButton->setText(g_setting->fontToString(g_setting->fontMarks));
    fontCommentsButton->setText(g_setting->fontToString(g_setting->fontComments));
    fontListsButton->setText(g_setting->fontToString(g_setting->fontLists));
    fontClocksButton->setText(g_setting->fontToString(g_setting->fontClocks));
    fontConsoleButton->setText(g_setting->fontToString(g_setting->fontConsole));

    fontStandardButton->setFont(g_setting->fontStandard);
    fontMarksButton->setFont(g_setting->fontMarks);
    fontCommentsButton->setFont(g_setting->fontComments);
    fontListsButton->setFont(g_setting->fontLists);
    fontClocksButton->setFont(g_setting->fontClocks);
    fontConsoleButton->setFont(g_setting->fontConsole);

    ignoreSGFParserErrorCheckBox->setChecked(g_setting->readBoolEntry("IGNORE_SGF_PARSER_ERRORS"));
    suppressSGFParserErrorWarningCheckBox->setChecked(g_setting->readBoolEntry("SUPPRESS_SGF_PARSER_ERROR_WARNING"));
    showNumberMoveCountSpinBox->setValue(g_setting->readIntEntry("MOVE_COUNT_MOVE_NUMBER"));
    showMoveNumberGroupBox->setChecked(g_setting->readBoolEntry("SHOW_MOVE_NUMBER"));
}

void PreferencesDialog::select_stone_look(bool)
{
    update_w_stones();
    update_b_stones();
}

void PreferencesDialog::select_white_color(bool)
{
    QColor c = QColorDialog::getColor(m_ih->white_color(), this, tr("Select white stone base color"));
    m_ih->set_white_color(c);
    update_w_stones();
}

void PreferencesDialog::select_black_color(bool)
{
    QColor c = QColorDialog::getColor(m_ih->black_color(), this, tr("Select black stone base color"));
    m_ih->set_black_color(c);
    update_b_stones();
}

void PreferencesDialog::update_stone_params()
{
    QString whiteStonePicturePath = whiteStonePicturePathEdit->text();
    QString blackStonePicturePath = blackStonePicturePathEdit->text();
    int     sizePercent           = stoneSizePercentSlider->value();
    double  br                    = 2.05 + (100 - blackRoundSlider->value()) / 30.0;
    double  wr                    = 2.05 + (100 - whiteRoundSlider->value()) / 30.0;
    double  bs                    = blackSpecSlider->value() / 100.0;
    double  ws                    = whiteSpecSlider->value() / 100.0;
    double  bf                    = blackFlatSlider->value();
    double  wf                    = whiteFlatSlider->value();
    double  bh                    = 1 + blackHardSlider->value() / 10.0;
    double  wh                    = 1 + whiteHardSlider->value() / 10.0;
    double  ambient               = ambientSlider->value() / 100.0;
    bool    clamshell             = stripesCheckBox->isChecked();
    int     look                  = 3;
    if (radioButtonStones_2D->isChecked())
        look = 1;
    else if (radioButtonStones_3D->isChecked())
        look = 2;
    else if (radioButtonStone_picture->isChecked())
        look = 4;
    blackGroupBox->setEnabled(look == 3);
    whiteGroupBox->setEnabled(look == 3);
    m_ih->set_stone_params(wh, bh, ws, bs, wr, br, wf, bf, ambient, look, clamshell, whiteStonePicturePath, blackStonePicturePath, sizePercent);
}

void PreferencesDialog::update_w_stones()
{
    update_stone_params();

    int real_size = m_stone_size;
    if (!radioButtonStones_2D->isChecked())
        real_size = m_stone_size * 9. / 8;

    QPixmap pm(real_size, real_size);
    pm.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pm);
    QImage img(m_stone_size, m_stone_size, QImage::Format_ARGB32);
    if (!radioButtonStones_2D->isChecked())
    {
        m_ih->paint_shadow_stone(img, m_stone_size);
        painter.drawImage(0, real_size - m_stone_size, img);
    }
    m_ih->paint_one_stone(img, true, m_stone_size);
    painter.drawImage(real_size - m_stone_size, 0, img);
    painter.end();
    m_w_stone->setPixmap(pm);
}

void PreferencesDialog::update_b_stones()
{
    update_stone_params();

    int real_size = m_stone_size;
    if (!radioButtonStones_2D->isChecked())
        real_size = m_stone_size * 9. / 8;

    QPixmap pm(real_size, real_size);
    pm.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pm);
    QImage img(m_stone_size, m_stone_size, QImage::Format_ARGB32);
    if (!radioButtonStones_2D->isChecked())
    {
        m_ih->paint_shadow_stone(img, m_stone_size);
        painter.drawImage(0, real_size - m_stone_size, img);
    }
    m_ih->paint_one_stone(img, false, m_stone_size);
    painter.drawImage(real_size - m_stone_size, 0, img);
    painter.end();
    m_b_stone->setPixmap(pm);
}

/*
 *  Destroys the object and frees any allocated resources
 */
PreferencesDialog::~PreferencesDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*****************************************************************
 *
 *               Overwritten own implementations
 *
 *****************************************************************/

void PreferencesDialog::slot_apply()
{
    qDebug() << "onApply";

    g_setting->writeBoolEntry("IGNORE_SGF_PARSER_ERRORS", ignoreSGFParserErrorCheckBox->isChecked());
    g_setting->writeBoolEntry("SUPPRESS_SGF_PARSER_ERROR_WARNING", suppressSGFParserErrorWarningCheckBox->isChecked());
    g_setting->writeIntEntry("MOVE_COUNT_MOVE_NUMBER", showNumberMoveCountSpinBox->value());
    g_setting->writeBoolEntry("SHOW_MOVE_NUMBER", showMoveNumberGroupBox->isChecked());

    bool ok;
    int  slide_x = slideXEdit->text().toInt(&ok);
    if (!ok || slide_x < 100 || slide_x > 10000)
    {
        QMessageBox::warning(this,
                             tr("Invalid slide width"),
                             tr("Please enter valid dimensions for slide export "
                                "(100x100 or larger)."));
        return;
    }
    int slide_y = slideYEdit->text().toInt(&ok);
    if (!ok || slide_y < 100 || slide_y > 10000)
    {
        QMessageBox::warning(this,
                             tr("Invalid slide height"),
                             tr("Please enter valid dimensions for slide export "
                                "(100x100 or larger)."));
        return;
    }
    if (slide_y > slide_x)
    {
        QMessageBox::warning(this, tr("Invalid slide dimensions"), tr("Slide export dimensions must be wider than they are tall."));
        return;
    }
    g_setting->writeIntEntry("SLIDE_X", slide_x);
    g_setting->writeIntEntry("SLIDE_Y", slide_y);
    g_setting->writeIntEntry("SLIDE_LINES", slideLinesSpinBox->value());
    g_setting->writeIntEntry("SLIDE_MARGIN", slideMarginSpinBox->value());
    g_setting->writeBoolEntry("SLIDE_ITALIC", slideItalicCheckBox->isChecked());
    g_setting->writeBoolEntry("SLIDE_BOLD", slideBoldCheckBox->isChecked());
    g_setting->writeBoolEntry("SLIDE_WB", slideWBCheckBox->isChecked());
    g_setting->writeBoolEntry("SLIDE_COORDS", slideCoordsCheckBox->isChecked());

    g_setting->writeIntEntry("SKIN_INDEX", woodComboBox->currentIndex());
    g_setting->writeEntry("SKIN", LineEdit_goban->text());
    g_setting->writeEntry("SKIN_TABLE", LineEdit_Table->text());
    g_setting->writeBoolEntry("SKIN_SCALE_WOOD", scaleWoodCheckBox->isChecked());
    g_setting->obtain_skin_images();

    g_setting->writeEntry("LANG", g_setting->convertNumberToLanguage(languageComboBox->currentIndex()));
    g_setting->writeIntEntry("FILESEL", fileSelComboBox->currentIndex());
    //	setting->writeBoolEntry("STONES_SHADOW",
    // stonesShadowCheckBox->isChecked()); 	setting->writeBoolEntry("STONES_SHELLS",
    // stonesShellsCheckBox->isChecked());
    int i = 3;
    if (radioButtonStones_2D->isChecked())
        i = 1;
    else if (radioButtonStones_3D->isChecked())
        i = 2;
    else if (radioButtonStone_picture->isChecked())
        i = 4;
    g_setting->writeBoolEntry("STONES_STRIPES", stripesCheckBox->isChecked());
    g_setting->writeIntEntry("STONES_LOOK", i);
    g_setting->writeIntEntry("STONES_WSPEC", whiteSpecSlider->value());
    g_setting->writeIntEntry("STONES_BSPEC", blackSpecSlider->value());
    g_setting->writeIntEntry("STONES_WHARD", whiteHardSlider->value());
    g_setting->writeIntEntry("STONES_BHARD", blackHardSlider->value());
    g_setting->writeIntEntry("STONES_WROUND", whiteRoundSlider->value());
    g_setting->writeIntEntry("STONES_BROUND", blackRoundSlider->value());
    g_setting->writeIntEntry("STONES_WFLAT", whiteFlatSlider->value());
    g_setting->writeIntEntry("STONES_BFLAT", blackFlatSlider->value());
    g_setting->writeIntEntry("STONES_AMBIENT", ambientSlider->value());
    g_setting->writeEntry("STONES_WPICTURE", whiteStonePicturePathEdit->text());
    g_setting->writeEntry("STONES_BPICTURE", blackStonePicturePathEdit->text());
    g_setting->writeIntEntry("STONES_SIZE_PERCENT", stoneSizePercentSlider->value());
    g_setting->writeEntry("STONES_BCOL", black_color().name());
    g_setting->writeEntry("STONES_WCOL", white_color().name());

    g_setting->writeBoolEntry("BOARD_LINESCALE", lineScaleCheckBox->isChecked());
    g_setting->writeBoolEntry("BOARD_LINEWIDEN", lineWidenCheckBox->isChecked());

    g_setting->writeBoolEntry("SOUND_STONE", stoneSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_AUTOPLAY", autoplaySoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_TALK", talkSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_MATCH", matchSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_GAMEEND", passSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_PASS", gameEndSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_TIME", timeSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_SAY", saySoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_ENTER", enterSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_LEAVE", leaveSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_DISCONNECT", disConnectSoundCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_CONNECT", connectSoundCheckBox->isChecked());

    g_setting->writeBoolEntry("SOUND_MATCH_BOARD", soundMatchCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_OBSERVE", soundObserveCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_NORMAL", soundNormalCheckBox->isChecked());
    g_setting->writeBoolEntry("SOUND_COMPUTER", soundComputerCheckBox->isChecked());

    g_setting->writeIntEntry("VAR_GHOSTS", variationComboBox->currentIndex());
    g_setting->writeBoolEntry("VAR_CHILDREN", varChildrenComboBox->currentIndex() == 1);
    g_setting->writeIntEntry("VAR_SGF_STYLE", varSGFStyleComboBox->currentIndex());
    g_setting->writeBoolEntry("VAR_IGNORE_DIAGS", varDiagsCheckBox->isChecked());

    int coords  = coordsComboBox->currentIndex();
    int sidebar = sidebarComboBox->currentIndex();
    g_setting->writeBoolEntry("BOARD_COORDS", coords > 0);
    g_setting->writeBoolEntry("SGF_BOARD_COORDS", coords == 2);
    g_setting->writeIntEntry("COORDS_SIZE", coordSizeSlider->value());
    g_setting->writeBoolEntry("SIDEBAR_LEFT", sidebar == 0);
    g_setting->writeBoolEntry("CURSOR", cursorCheckBox->isChecked());
    // setting->writeBoolEntry("SMALL_STONES",
    // smallerStonesCheckBox->isChecked());
    g_setting->writeBoolEntry("TOOLTIPS", !(tooltipsCheckBox->isChecked()));
    g_setting->writeIntEntry("BY_TIMER", BYTimeSpin->text().toInt());
    g_setting->writeIntEntry("TIMER_INTERVAL", timerComboBox->currentIndex());
    g_setting->writeBoolEntry("SGF_TIME_TAGS", sgfTimeTagsCheckBox->isChecked());
    g_setting->writeBoolEntry("ANTICLICKO", antiClickoCheckBox->isChecked());
    g_setting->writeBoolEntry("ANTICLICKO_HITBOX", hitboxCheckBox->isChecked());

    // Client Window Tab
    g_setting->writeEntry("WATCH", LineEdit_watch->text());
    g_setting->writeEntry("EXCLUDE", LineEdit_exclude->text());
    g_setting->writeBoolEntry("EXTUSERINFO", CheckBox_extUserInfo->isChecked());
    //	setting->writeBoolEntry("USE_NMATCH", CheckBox_useNmatch->isChecked());

    // Checks wether the nmatch parameters have been modified, in order to send a
    // new nmatchrange command
    if ((g_setting->readBoolEntry("NMATCH_BLACK") != checkBox_Nmatch_Black->isChecked()) ||
        (g_setting->readBoolEntry("NMATCH_WHITE") != checkBox_Nmatch_White->isChecked()) ||
        (g_setting->readBoolEntry("NMATCH_NIGIRI") != checkBox_Nmatch_Nigiri->isChecked()) ||
        (g_setting->readIntEntry("NMATCH_MAIN_TIME") != timeSpin_Nmatch->text().toInt()) ||
        (g_setting->readIntEntry("NMATCH_BYO_TIME") != BYSpin_Nmatch->text().toInt()) ||
        (g_setting->readIntEntry("NMATCH_HANDICAP") != HandicapSpin_Nmatch->text().toInt()) ||
        (g_setting->readIntEntry("DEFAULT_SIZE") != boardSizeSpin->text().toInt()) ||
        (g_setting->readIntEntry("DEFAULT_TIME") != timeSpin->text().toInt()) || (g_setting->readIntEntry("DEFAULT_BY") != BYSpin->text().toInt()))
        g_setting->nmatch_settings_modified = true;

    g_setting->writeBoolEntry("NMATCH_BLACK", checkBox_Nmatch_Black->isChecked());
    g_setting->writeBoolEntry("NMATCH_WHITE", checkBox_Nmatch_White->isChecked());
    g_setting->writeBoolEntry("NMATCH_NIGIRI", checkBox_Nmatch_Nigiri->isChecked());
    g_setting->writeIntEntry("NMATCH_MAIN_TIME", timeSpin_Nmatch->text().toInt());
    g_setting->writeIntEntry("NMATCH_BYO_TIME", BYSpin_Nmatch->text().toInt());
    g_setting->writeIntEntry("NMATCH_HANDICAP", HandicapSpin_Nmatch->text().toInt());

    g_setting->writeIntEntry("DEFAULT_SIZE", boardSizeSpin->text().toInt());
    g_setting->writeIntEntry("DEFAULT_TIME", timeSpin->text().toInt());
    g_setting->writeIntEntry("DEFAULT_BY", BYSpin->text().toInt());
    g_setting->writeIntEntry("DEFAULT_KOMI", komiSpinDefault->text().toFloat());
    g_setting->writeBoolEntry("DEFAULT_AUTONEGO", automaticNegotiationCheckBox->isChecked());
    g_setting->writeBoolEntry("AUTOSAVE", CheckBox_autoSave->isChecked());
    g_setting->writeBoolEntry("AUTOSAVE_PLAYED", CheckBox_autoSave_Played->isChecked());

    // Computer Tab
    g_setting->writeBoolEntry("COMPUTER_WHITE", computerWhiteButton->isChecked());
    g_setting->writeIntEntry("COMPUTER_SIZE", computerSizeSpin->text().toInt());
    g_setting->writeIntEntry("COMPUTER_HANDICAP", computerHandicapSpin->text().toInt());
    g_setting->writeEntry("HUMAN_NAME", humanName->text());

    g_setting->writeBoolEntry("ANALYSIS_CHILDREN", anChildMovesCheckBox->isChecked());
    g_setting->writeBoolEntry("ANALYSIS_PRUNE", anPruneCheckBox->isChecked());
    g_setting->writeBoolEntry("ANALYSIS_HIDEOTHER", anHideCheckBox->isChecked());
    g_setting->writeIntEntry("ANALYSIS_VARTYPE", anVarComboBox->currentIndex());
    g_setting->writeIntEntry("ANALYSIS_WINRATE", winrateComboBox->currentIndex());

    g_setting->writeIntEntry("ANALYSIS_DEPTH", anDepthEdit->text().toInt());
    g_setting->writeIntEntry("ANALYSIS_MAXMOVES", anMaxMovesEdit->text().toInt());

    g_setting->writeIntEntry("GAMETREE_SIZE", gameTreeSizeSlider->value());
    g_setting->writeIntEntry("BOARD_DIAGMODE", diagShowComboBox->currentIndex());
    g_setting->writeBoolEntry("BOARD_DIAGCLEAR", diagClearCheckBox->isChecked());
    g_setting->writeBoolEntry("GAMETREE_DIAGHIDE", diagHideCheckBox->isChecked());

    g_setting->writeIntEntry("TOROID_DUPS", toroidDupsSpin->text().toInt());

    if (m_dbpaths_changed)
    {
        g_setting->m_dbpaths       = m_dbpaths;
        g_setting->dbpaths_changed = true;
        m_dbpaths_changed        = false;
    }
    if (m_engines_changed)
    {
        g_setting->m_engines       = m_engines_model.entries();
        g_setting->engines_changed = true;
        m_engines_changed        = false;
    }
    if (m_hosts_changed)
    {
        g_setting->m_hosts       = m_hosts_model.entries();
        g_setting->hosts_changed = true;
        m_hosts_changed        = false;
    }
    g_setting->extract_frequent_settings();

    client_window->preferencesAccept();
}

void PreferencesDialog::startHelpMode()
{
    QWhatsThis::enterWhatsThisMode();
}

void PreferencesDialog::selectFont(QPushButton *button, QFont &font)
{
    // Open a font dialog to select a new font
    bool  ok;
    QFont f = QFontDialog::getFont(&ok, font, this);
    if (ok)
    {
        font = f;
        button->setText(g_setting->fontToString(f));
        button->setFont(f);
    }
}

bool PreferencesDialog::avoid_losing_data()
{
    if (m_changing_engine)
    {
        if (!enginePath->text().isEmpty() || !engineArgs->text().isEmpty() || !engineKomi->text().isEmpty())
        {
            QMessageBox mb(QMessageBox::Question,
                           tr("Unsaved data"),
                           QString(tr("The engine input fields contain\n"
                                      "potentially unsaved data.\n"
                                      "Really close the preferences?")),
                           QMessageBox::Yes | QMessageBox::No);
            return mb.exec() == QMessageBox::No;
        }
    }
    if (m_changing_host)
    {
        if (!LineEdit_host->text().isEmpty() || !LineEdit_port->text().isEmpty() || !LineEdit_login->text().isEmpty())
        {
            QMessageBox mb(QMessageBox::Question,
                           tr("Unsaved data"),
                           QString(tr("The host input fields contain\n"
                                      "potentially unsaved data.\n"
                                      "Really close the preferences?")),
                           QMessageBox::Yes | QMessageBox::No);
            return mb.exec() == QMessageBox::No;
        }
    }
    return false;
}

void PreferencesDialog::slot_accept()
{
    if (avoid_losing_data())
        return;
    saveSizes();

    slot_apply();
    accept();
}

void PreferencesDialog::slot_reject()
{
    if (avoid_losing_data())
        return;

    saveSizes();
    reject();
}

void PreferencesDialog::saveSizes()
{
    // save size and position of window
    client_window->savePrefFrame(pos(), size());
}

void PreferencesDialog::slot_add_engine()
{
    QString komi = engineKomi->text();
    if (!komi.isEmpty())
    {
        QDoubleValidator kv;
        kv.setDecimals(2);
        int pos = 0;
        if (kv.validate(komi, pos) != QValidator::Acceptable)
        {
            QMessageBox::warning(this, tr("Invalid komi entered"), tr("Please enter a valid komi before adding the engine."));
            return;
        }
    }
    if (engineSize->text().isEmpty())
    {
        if (engineAnalysis->isChecked())
        {
            QMessageBox::warning(this,
                                 tr("Missing board size"),
                                 tr("Analysis engines require a board size to be specified.\n"
                                    "If your engine allows multiple board sizes, you need to "
                                    "configure them in separate entires."));
            return;
        }
    }
    else
    {
        int sz = engineSize->text().toInt();
        if (sz < 5 || sz > 25)
        {
            QMessageBox::warning(this, tr("Invalid board size"), tr("Only a range of 5 to 25 is allowed."));
            return;
        }
    }
    const QString name = engineName->text();
    // check if at least title and path are set
    if (!name.isEmpty() && !enginePath->text().isEmpty())
    {
        Engine new_e(engineName->text(), enginePath->text(), engineArgs->text(), engineKomi->text(), engineAnalysis->isChecked(), engineSize->text());
        m_engines_model.add_or_replace(new_e);
        m_engines_changed = true;
    }

    clear_engine();
}

void PreferencesDialog::clear_engine()
{
    engineName->clear();
    enginePath->clear();
    engineArgs->clear();
    engineKomi->clear();
    engineAnalysis->setChecked(false);
    engineSize->clear();
    m_changing_engine = false;
}

void PreferencesDialog::slot_delete_engine()
{
    QModelIndex idx = ListView_engines->currentIndex();
    if (!idx.isValid())
        return;

    if ((size_t)idx.row() >= m_engines_model.entries().size())
        return;

    m_engines_model.removeRows(idx.row(), 1);
    m_engines_changed = true;
    clear_engine();
}

void PreferencesDialog::slot_new_engine()
{
    clear_engine();
}

void PreferencesDialog::update_current_engine()
{
    QModelIndex idx   = ListView_engines->currentIndex();
    bool        valid = idx.isValid();

    pb_engine_delete->setEnabled(valid);

    if (!valid)
        return;

    const Engine *e = m_engines_model.find(idx);
    engineName->setText(e->title);
    enginePath->setText(e->path);
    engineArgs->setText(e->args);
    engineKomi->setText(e->komi);
    engineAnalysis->setChecked(e->analysis);
    engineSize->setText(e->boardsize);
}

void PreferencesDialog::slot_stoneSizePercentChanged(int v)
{
    stoneSizePercentLabel->setText(QString::number(v));
}

void PreferencesDialog::slot_engineChanged(const QString &title)
{
    for (auto &e : m_engines_model.entries())
        if (e.title == title)
        {
            m_changing_engine = true;
            pb_engine_add->setText(tr("Change"));
            return;
        }

    m_changing_engine = false;
    pb_engine_add->setText(tr("Add"));
}

void PreferencesDialog::clear_host()
{
    LineEdit_title->clear();
    LineEdit_host->clear();
    LineEdit_port->clear();
    LineEdit_login->clear();
    ComboBox_codec->setCurrentText("");

    m_changing_host = false;
}

void PreferencesDialog::slot_add_server()
{
    // check if at least title and host inserted
    if (!LineEdit_title->text().isEmpty() && !LineEdit_host->text().isEmpty())
    {
        // check if title already exists
        bool         check;
        unsigned int tmp = LineEdit_port->text().toUInt(&check);
        if (!check)
        {
            tmp = 0;
            qWarning("Failed to convert port to integer!");
        }

        Host new_h(LineEdit_title->text(), LineEdit_host->text(), tmp, LineEdit_login->text(), LineEdit_pass->text(), ComboBox_codec->currentText());
        m_hosts_model.add_or_replace(new_h);
        m_hosts_changed = true;
    }

    clear_host();
}

void PreferencesDialog::slot_delete_server()
{
    QModelIndex idx = ListView_hosts->currentIndex();
    if (!idx.isValid())
        return;
    if ((size_t)idx.row() >= m_hosts_model.entries().size())
        return;

    m_hosts_model.removeRows(idx.row(), 1);
    m_hosts_changed = true;
    clear_host();
}

void PreferencesDialog::slot_new_server()
{
    clear_host();
}

void PreferencesDialog::update_current_host()
{
    QModelIndex idx           = ListView_hosts->currentIndex();
    bool        valid         = idx.isValid();
    bool        standard_host = valid && (size_t)idx.row() >= m_hosts_model.entries().size();

    pb_server_delete->setEnabled(valid && !standard_host);

    if (!valid)
        return;

    const Host *h = m_hosts_model.find(idx);
    LineEdit_title->setText(standard_host ? "" : h->title);
    LineEdit_host->setText(h->host);
    LineEdit_port->setText(QString::number(h->port));
    LineEdit_login->setText(h->login_name);
    LineEdit_pass->setText(h->password);
    ComboBox_codec->setCurrentText(h->codec);
}

void PreferencesDialog::slot_serverChanged(const QString &title)
{
    for (auto &h : m_hosts_model.entries())
        if (h.title == title)
        {
            m_changing_host = true;
            pb_server_add->setText(tr("Change"));
            return;
        }

    m_changing_host = false;
    pb_server_add->setText(tr("Add"));
}

// play the sound if check box has been clicked
void PreferencesDialog::on_soundButtonGroup_buttonClicked(QAbstractButton *cb)
{
    qDebug() << "button text = " << cb->text();

    if (cb->text() == tr("Stones"))
        g_quteGo->playClick();
    else if (cb->text() == tr("Pass"))
        g_quteGo->playPassSound();
    else if (cb->text() == tr("Autoplay"))
        g_quteGo->playAutoPlayClick();
    else if (cb->text().startsWith(tr("Time"), Qt::CaseInsensitive))
        g_quteGo->playTimeSound();
    else if (cb->text() == tr("Talk"))
        g_quteGo->playTalkSound();
    else if (cb->text() == tr("Say"))
        g_quteGo->playSaySound();
    else if (cb->text() == tr("Match"))
        g_quteGo->playMatchSound();
    else if (cb->text() == tr("Enter"))
        g_quteGo->playEnterSound();
    else if (cb->text() == tr("Game end"))
        g_quteGo->playGameEndSound();
    else if (cb->text() == tr("Leave"))
        g_quteGo->playLeaveSound();
    else if (cb->text() == tr("Disconnect"))
        g_quteGo->playDisConnectSound();
    else if (cb->text() == tr("Connect"))
        g_quteGo->playConnectSound();
}

void PreferencesDialog::slot_getComputerPath()
{
    QString fileName(QFileDialog::getOpenFileName(this, tr("Choose GTP engine path"), g_setting->readEntry("LAST_DIR"), tr("All Files (*)")));
    if (fileName.isEmpty())
        return;

    enginePath->setText(fileName);
}

void PreferencesDialog::slot_getGobanPicturePath()
{
#if defined(Q_OS_MACX)
    // On the Mac, we want to default this dialog to the directory in the bundle
    // that contains the wood images we ship with the app
    // get the bundle path and find our resources
    CFURLRef    bundleRef  = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
    QString     path       = (QString)CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding()) + "/Contents/Resources/Backgrounds";
#else
    QString path = g_setting->readEntry("LAST_DIR");
#endif
    QString old_name = LineEdit_goban->text();
    if (!old_name.isEmpty())
    {
        QFileInfo info(old_name);
        if (info.exists())
        {
            QDir dir = info.dir();
            path     = dir.absolutePath();
        }
    }
    QString fileName(QFileDialog::getOpenFileName(this, tr("Select a goban wood image"), path, tr("Images (*.png *.xpm *.jpg)")));
    if (fileName.isEmpty())
        return;

    LineEdit_goban->setText(fileName);
    update_board_image();
}

void PreferencesDialog::slot_getTablePicturePath()
{
#if defined(Q_OS_MACX)
    // On the Mac, we want to default this dialog to the directory in the bundle
    // that contains the wood images we ship with the app
    // get the bundle path and find our resources
    CFURLRef    bundleRef  = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
    QString     path       = (QString)CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding()) + "/Contents/Resources/Backgrounds";
#else
    QString path = g_setting->readEntry("LAST_DIR");
#endif
    QString old_name = LineEdit_Table->text();
    if (!old_name.isEmpty())
    {
        QFileInfo info(old_name);
        if (info.exists())
        {
            QDir dir = info.dir();
            path     = dir.absolutePath();
        }
    }
    QString fileName(QFileDialog::getOpenFileName(this, tr("Select a table background image"), path, tr("Images (*.png *.xpm *.jpg)")));
    if (fileName.isEmpty())
        return;

    LineEdit_Table->setText(fileName);
}

void PreferencesDialog::slot_getWhiteStonePicturePath()
{
#if defined(Q_OS_MACX)
    // On the Mac, we want to default this dialog to the directory in the bundle
    // that contains the wood images we ship with the app
    // get the bundle path and find our resources
    CFURLRef    bundleRef  = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
    QString     path       = (QString)CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding()) + "/Contents/Resources/Stones";
#else
    QString path = g_setting->readEntry("LAST_DIR");
#endif
    QString old_name = whiteStonePicturePathEdit->text();
    if (!old_name.isEmpty())
    {
        QFileInfo info(old_name);
        if (info.exists())
        {
            QDir dir = info.dir();
            path     = dir.absolutePath();
        }
    }
    QString fileName(QFileDialog::getOpenFileName(this, tr("Select a white stone image"), path, tr("Images (*.png *.xpm *.jpg *.svg)")));
    if (fileName.isEmpty())
        return;

    whiteStonePicturePathEdit->setText(fileName);
}

void PreferencesDialog::slot_getBlackStonePicturePath()
{
#if defined(Q_OS_MACX)
    // On the Mac, we want to default this dialog to the directory in the bundle
    // that contains the wood images we ship with the app
    // get the bundle path and find our resources
    CFURLRef    bundleRef  = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleRef, kCFURLPOSIXPathStyle);
    QString     path       = (QString)CFStringGetCStringPtr(bundlePath, CFStringGetSystemEncoding()) + "/Contents/Resources/Stones";
#else
    QString path = g_setting->readEntry("LAST_DIR");
#endif
    QString old_name = blackStonePicturePathEdit->text();
    if (!old_name.isEmpty())
    {
        QFileInfo info(old_name);
        if (info.exists())
        {
            QDir dir = info.dir();
            path     = dir.absolutePath();
        }
    }
    QString fileName(QFileDialog::getOpenFileName(this, tr("Select a black stone image"), path, tr("Images (*.png *.xpm *.jpg *.svg)")));
    if (fileName.isEmpty())
        return;

    blackStonePicturePathEdit->setText(fileName);
}

void PreferencesDialog::slot_main_time_changed(int n)
{
    timeSpin_Nmatch->setMinimum(n);
}

void PreferencesDialog::slot_BY_time_changed(int n)
{
    BYSpin_Nmatch->setMinimum(n);
}

QColor PreferencesDialog::white_color()
{
    return m_ih->white_color();
}

QColor PreferencesDialog::black_color()
{
    return m_ih->black_color();
}
