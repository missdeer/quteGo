
/*
 *   qgo_interface.h
 */

#ifndef QGO_INTERFACE_H
#define QGO_INTERFACE_H

#undef SHOW_INTERNAL_TIME
#undef SHOW_MOVES_DLG

#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QTimerEvent>

#include "defines.h"
#include "gogame.h"
#include "gs_globals.h"
#include "qgo.h"
#include "setting.h"
#include "tables.h"

//-----------

class Game;

//-----------

class GameInfo
{
public:
    GameInfo() {};
    ~GameInfo() {};
    // 15 Game 43 I: xxxx (4 223 16) vs yyyy (5 59 13)
    // 15 144(B): B12
    QString nr;
    QString type;
    QString wname;
    QString wprisoners;
    QString wtime;
    QString wstones;
    QString bname;
    QString bprisoners;
    QString btime;
    QString bstones;
    QString mv_col; // T..Time, B..Black, W..White, I..Init
    QString mv_nr;
    QString mv_pt;
};

//-----------

class MainWindow_IGS;
class qGoIF;

class qGoBoard
    : public QObject
    , public game_state::observer
{
    Q_OBJECT

    std::shared_ptr<game_record> m_game      = nullptr;
    stone_color                  m_own_color = none;
    qGoIF                       *m_qgoif;
    QString                     *m_title = nullptr;
    QString                      m_comments;
    bool                         m_scoring   = false;
    bool                         m_connected = true;

    /* State used while receiving a game result.  */
    go_board *m_scoring_board = nullptr;

    QStandardItemModel m_observers;

    virtual void observed_changed() override {}
    void         send_coords(int x, int y);
    void         update_time_info(game_state *);

public:
    qGoBoard(qGoIF *, int game_id);
    ~qGoBoard();

    void game_startup();
    void disconnected(bool remove_from_list);
    int  get_id() const
    {
        return id;
    }
    void set_id(int i)
    {
        id = i;
    }
    void set_title(const QString &);
    void set_komi(const QString &);
    void set_freegame(bool);
    bool get_havegd()
    {
        return m_game != nullptr;
    }
    bool get_sentmovescmd()
    {
        return sent_movescmd;
    }
    void set_sentmovescmd(bool m)
    {
        sent_movescmd = m;
    }
    bool get_adj()
    {
        return adjourned;
    }
    QString get_bplayer()
    {
        return QString::fromStdString(m_game->name_black());
    }
    QString get_wplayer()
    {
        return QString::fromStdString(m_game->name_white());
    }
    void set_adj(bool a)
    {
        adjourned = a;
    }
    void set_game(Game *g, GameMode mode, stone_color own_color);

    void     set_Mode_real(GameMode);
    GameMode get_Mode()
    {
        return gameMode;
    }
    void set_move(stone_color, QString, QString);
    void game_result(const QString &, const QString &);
    void remote_undo(const QString &);
    void mark_dead_stone(int x, int y);
    void enter_scoring_mode(bool may_be_reentry);
    void leave_scoring_mode(void);
    void player_toggle_dead(int x, int y);
    void move_played(int, int);

    void            send_kibitz(const QString &);
    MainWindow_IGS *get_win()
    {
        return win;
    }
    void    setTimerInfo(const QString &, const QString &, const QString &, const QString &);
    QString secToTime(int);
    void    set_stopTimer();
    void    set_runTimer();
    void    set_gamePaused(bool p)
    {
        game_paused = p;
    }
    int get_boardsize()
    {
        return m_game->boardsize();
    }
    int get_mvcount()
    {
        return mv_counter;
    }
    void set_myColorIsBlack(bool b);
    bool get_myColorIsBlack()
    {
        return m_own_color == black;
    }
    void    set_requests(const QString &handicap, const QString &komi, assessType);
    void    check_requests();
    QString get_reqKomi()
    {
        return req_komi;
    }
    QString get_currentKomi()
    {
        return QString::fromStdString(m_game->komi());
    }
    void dec_mv_counter()
    {
        mv_counter--;
    }
    int get_mv_counter()
    {
        return mv_counter;
    }
    bool get_requests_set()
    {
        return requests_set;
    }
    void set_gsName(GSName g)
    {
        gsName = g;
    }
    void addtime_b(int m);
    void addtime_w(int m);
    void set_myName(const QString &n)
    {
        myName = n;
    }

    void observer_list_start();
    void observer_list_entry(const QString &, const QString &);
    void observer_list_end();

    void receive_score_begin();
    void receive_score_line(int, const QString &);
    void receive_score_end();

    // teaching features
    bool       ExtendedTeachingGame;
    bool       IamTeacher;
    bool       IamPupil;
    bool       havePupil;
    bool       haveControls;
    QString    ttOpponent;
    bool       mark_set;
    int        mark_counter;
    assessType m_freegame;

signals:
    // to qGoIF
    //	void signal_closeevent(int);
    void signal_sendcommand(const QString &, bool);

public slots:
    // MainWindow
    void slot_closeevent();
    void slot_sendcomment(const QString &);

    // Board
    void slot_doPass();
    void slot_doResign();
    void slot_doUndo();
    void slot_doAdjourn();
    void slot_doDone();

    // normaltools
    void slot_addtimePauseW();
    void slot_addtimePauseB();

    // teachtools
    void slot_ttOpponentSelected(const QString &);
    void slot_ttControls(bool);
    void slot_ttMark(bool);

protected:
    virtual void timerEvent(QTimerEvent *) override;

private:
    int             timer_id;
    bool            game_paused;
    bool            sent_movescmd;
    bool            adjourned;
    bool            myColorIsBlack;
    GameMode        gameMode;
    int             id;
    MainWindow_IGS *win;

    int mv_counter;
    int stated_mv_count;

    int        bt_i, wt_i;
    QString    bt, wt;
    QString    b_stones, w_stones;
    QString    req_handicap;
    QString    req_komi;
    assessType req_free;
    bool       requests_set;
    GSName     gsName;
    QString    myName;
    int        BY_timer;

#ifdef SHOW_INTERNAL_TIME
    int chk_b, chk_w;
#endif
};

//-----------

class qGoIF : public QObject
{
    Q_OBJECT

    void game_end(qGoBoard *, const QString &txt);

public:
    qGoIF(QWidget *);
    ~qGoIF();
    //	bool set_observe(QString);
    void set_initIF();
    void set_myName(const QString &n)
    {
        myName = n;
    }
    void set_gsName(GSName n)
    {
        gsName = n;
    }
    void     set_localboard(QString file = QString::null);
    void     set_localgame();
    QWidget *get_parent()
    {
        return parent;
    }
    void wrapupMatchGame(qGoBoard *, bool);

    void window_closing(qGoBoard *);
    void remove_board(qGoBoard *);

    /* Called by parser.cpp.  */
    void observer_list_start(int);
    void observer_list_entry(int, const QString &, const QString &);
    void observer_list_end(int);

    void game_end(const QString &id, const QString &txt);
    void game_end(const QString &player1, const QString &player2, const QString &txt);

signals:
    void signal_sendcommand(const QString &, bool);
    void signal_addToObservationList(int);

public slots:
    // parser/mainwindow
    void slot_move(GameInfo *);
    void slot_gamemove(Game *);
    void slot_kibitz(int, const QString &, const QString &);
    void slot_title(const GameInfo *, const QString &);
    void slot_komi(const QString &, const QString &, bool);
    void slot_freegame(bool);
    void slot_matchcreate(const QString &, const QString &);
    void slot_removestones(const QString &, const QString &);
    void slot_undo(const QString &, const QString &);
    void slot_result(const QString &, const QString &, bool, const QString &);
    void slot_matchsettings(const QString &, const QString &, const QString &, assessType);
    void slot_requestDialog(const QString &, const QString &, const QString &, const QString &);
    void slot_timeAdded(int, bool);
    // void slot_undoRequest(const QString &);

    // qGoBoard
    //	void slot_closeevent(int) {};
    void slot_sendcommand(const QString &, bool);
    void set_observe(const QString &);

protected:
    bool parse_move(int src, GameInfo *gi = 0, Game *g = 0, QString txt = QString::null);

private:
    qGoBoard *find_game_id(int);
    qGoBoard *find_game_players(const QString &, const QString &);

    QWidget *parent;
    // actual pointer, for speedup reason
    qGoBoard         *qgobrd;
    QString           myName;
    QList<qGoBoard *> boardlist;
    GSName            gsName;
    int               localBoardCounter;
    //	int      lockObserveCmd;
};

#endif
